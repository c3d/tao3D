// ****************************************************************************
//  file_monitor.cpp                                               Tao project
// ****************************************************************************
//
//   File Description:
//
//     Be notified when files or directories are created, modified or deleted.
//     Monitoring occurs in its own thread.
//
//
//
//
//
//
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2012 Jerome Forissier <jerome@taodyne.com>
//  (C) 2012 Taodyne SAS
// ****************************************************************************

#include "file_monitor.h"
#include "base.h"
#include "tao_utf8.h"
#include <QDir>
#include <QMutexLocker>
#include <QTimer>



namespace Tao {


FileMonitor::FileMonitor(QString name)
// ----------------------------------------------------------------------------
//    Construct a file monitor object
// ----------------------------------------------------------------------------
    : name(name)
{
    IFTRACE(filemon)
        debug() << "Creating\n";

    thread = FileMonitorThread::instance();
    thread->addMonitor(this);
}


FileMonitor::FileMonitor(const FileMonitor &o)
// ----------------------------------------------------------------------------
//    Copy constructor
// ----------------------------------------------------------------------------
    : QObject(), thread(o.thread), name(o.name)
{
    foreach (MonitoredFile file, o.files)
        addPath(file.path);
    thread->addMonitor(this);
}


FileMonitor::~FileMonitor()
// ----------------------------------------------------------------------------
//    Destructor
// ----------------------------------------------------------------------------
{
    IFTRACE(filemon)
        debug() << "Deleting\n";

    removeAllPaths();
    thread->removeMonitor(this);
}


void FileMonitor::addPath(const QString &path)
// ----------------------------------------------------------------------------
//   Add a new path to the list of files and directories being monitored
// ----------------------------------------------------------------------------
{
    bool prefixed = (QRegExp("^[a-zA-Z]+:").indexIn(path) != -1);
    bool absolute = QDir::isAbsolutePath(path);
    if (!absolute && !prefixed)
    {
        IFTRACE(filemon)
            debug() << "Error: relative path not supported: '" << +path << "'\n";
        return;
    }
    if (!files.contains(path))
    {
        IFTRACE(filemon)
            debug() << "Adding: '" << +path << "'\n";

        MonitoredFile mf(path);
        QFileInfo file(path);
        if (file.exists())
        {
            mf.lastNotification = Created;
            mf.cachedCanonicalPath =  file.canonicalFilePath();
            mf.cachedModified = file.lastModified();
            emit created(path, mf.cachedCanonicalPath);
        }
        files[path] = mf;
        thread->addPath(path);
    }
    else
    {
        IFTRACE(filemon)
            debug() << "Path '" << +path << "' already monitored\n";
    }
}


void FileMonitor::removePath(const QString &path)
// ----------------------------------------------------------------------------
//   Remove a path from the list of files and directories being monitored
// ----------------------------------------------------------------------------
{
    if (files.contains(path))
    {
        IFTRACE(filemon)
            debug() << "Removing: '" << +path << "'\n";

        files.remove(path);
        Q_ASSERT(!files.contains(path));
        thread->removePath(path);
    }
}


void FileMonitor::removeAllPaths()
// ----------------------------------------------------------------------------
//   Clear all monitored items
// ----------------------------------------------------------------------------
{
    foreach (MonitoredFile file, files)
        removePath(file.path);
}


void FileMonitor::onCreated(const QString &path, const QString canonicalPath)
// ----------------------------------------------------------------------------
//   Called from the monitoring thread when a file is created.
// ----------------------------------------------------------------------------
{
    emit created(path, canonicalPath);
}


void FileMonitor::onChanged(const QString &path, const QString canonicalPath)
// ----------------------------------------------------------------------------
//   Called from the monitoring thread when file/canonical path changes.
// ----------------------------------------------------------------------------
{
    emit changed(path, canonicalPath);
}


void FileMonitor::onDeleted(const QString &path, const QString canonicalPath)
// ----------------------------------------------------------------------------
//   Called from the monitoring thread when a file is deleted or renamed.
// ----------------------------------------------------------------------------
{
    emit deleted(path, canonicalPath);
}


QString FileMonitor::id()
// ----------------------------------------------------------------------------
//   Programmer-readable identifier
// ----------------------------------------------------------------------------
{
    QString ret(name);
    if (name.isEmpty())
        ret.sprintf("%p",this);
    return ret;
}


std::ostream & FileMonitor::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[FileMonitor " << +id() << "] ";
    return std::cerr;
}




// ============================================================================
//
//    FileMonitorThread is a singleton used by all FileMonitor instances
//
// ============================================================================

// We use a weak pointer so that when the last FileMonitor object is deleted,
// the singleton is also deleted and inst becomes NULL.
QWeakPointer<FileMonitorThread> FileMonitorThread::inst;
int FileMonitorThread::pollInterval = 1000; /* ms */


FileMonitorThread::FileMonitorThread()
// ----------------------------------------------------------------------------
//    Start the monitoring thread
// ----------------------------------------------------------------------------
    : dontPollReadOnlyFiles(true)
{
    IFTRACE(filemon)
        debug() << "Starting polling thread (" << pollInterval << " ms)\n";

    moveToThread(this);
    // Using a single-shot timer avoids accumulating timeout events in case
    // checkFiles() takes longer than one period of time
    QTimer::singleShot(pollInterval, this, SLOT(checkFiles()));
    start();
}


FileMonitorThread::~FileMonitorThread()
// ----------------------------------------------------------------------------
//    Stop the monitoring thread
// ----------------------------------------------------------------------------
{
    IFTRACE(filemon)
        debug() << "Stopping\n";

    quit();
    wait(2000);
}


void FileMonitorThread::addPath(const QString &path)
// ----------------------------------------------------------------------------
//    Add path or increment use count, don't block on main mutex
// ----------------------------------------------------------------------------
{
    if (mutex.tryLock())
    {
        addPathNoLock(path);
        mutex.unlock();
    }
    else
    {
        // Path will be added to the main list when the next fileCheck()
        // completes
        QMutexLocker locker(&pendingMutex);
        pending.append(path);
    }
}


void FileMonitorThread::addPathNoLock(const QString &path)
// ----------------------------------------------------------------------------
//    Add path or increment use count. Assumes mutex is already locked.
// ----------------------------------------------------------------------------
{
    if (files.contains(path))
    {
        FileInfo &file = files[path];
        file.refs++;
        return;
    }

    files[path] = FileInfo(path);
}


void FileMonitorThread::removePath(const QString &path)
// ----------------------------------------------------------------------------
//    Decrement use count for path or delete it
// ----------------------------------------------------------------------------
{
    QMutexLocker locker(&mutex);
    Q_ASSERT(files.contains(path));
    FileInfo &info = files[path];
    Q_ASSERT(info.refs > 0);
    if (--info.refs == 0)
        files.remove(path);
}


void FileMonitorThread::addMonitor(FileMonitor *monitor)
// ----------------------------------------------------------------------------
//    Add a FileMonitor instance
// ----------------------------------------------------------------------------
{
    QMutexLocker locker(&mutex);
    Q_ASSERT(!monitors.contains(monitor));
    monitors.append(monitor);

    IFTRACE(filemon)
        debug() << "FileMonitor '" << +monitor->id() << "' added (count="
                << monitors.count() << ")\n";
}


void FileMonitorThread::removeMonitor(FileMonitor *monitor)
// ----------------------------------------------------------------------------
//    Remove a FileMonitor instance
// ----------------------------------------------------------------------------
{
    QMutexLocker locker(&mutex);
    Q_ASSERT(monitors.contains(monitor));
    monitors.removeOne(monitor);
    Q_ASSERT(!monitors.contains(monitor));

    IFTRACE(filemon)
        debug() << "FileMonitor '" << +monitor->id() << "' removed (count="
                << monitors.count() << ")\n";
}


QSharedPointer<FileMonitorThread> FileMonitorThread::instance()
// ----------------------------------------------------------------------------
//    Return a pointer to the instance of the file monitor thread.
// ----------------------------------------------------------------------------
{
    QSharedPointer<FileMonitorThread> ptr;
    if (!inst)
    {
        ptr = QSharedPointer<FileMonitorThread>(new FileMonitorThread);
        inst = ptr.toWeakRef();
    }
    else
    {
        ptr = inst.toStrongRef();
        Q_ASSERT(ptr);
    }
    return ptr;
}


std::ostream & FileMonitorThread::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[FileMonitorThread] ";
    return std::cerr;
}


void FileMonitorThread::mergePending()
// ----------------------------------------------------------------------------
//   Add files from the pending list to the main list (if any)
// ----------------------------------------------------------------------------
{
    // 'mutex' MUST be locked when calling this method.

    QMutexLocker locker(&pendingMutex);
    if (pending.isEmpty())
        return;

    IFTRACE(filemon)
         debug() << "Moving " << pending.size() << " paths from 'pending'\n";
    foreach (QString path, pending)
        addPathNoLock(path);
    pending.clear();
}


void FileMonitorThread::checkFiles()
// ----------------------------------------------------------------------------
//   Check all files for changes
// ----------------------------------------------------------------------------
{
    QMutexLocker locker(&mutex);

    QTime time;
    IFTRACE(filemon)
        time.start();

    foreach (QString path, files.keys())
    {
        FileInfo &file = files[path];

        if (file.ignore)
            continue;

        file.refresh();
        bool prefixed = (QRegExp("^[a-zA-Z]+:").indexIn(path) != -1);
        if (prefixed)
        {
            // Qt bug? QFileInfo tries to resolve the prefixed path only once
            // This works around the problem
            file.setFile("");
            file.setFile(path);
        }

        if (file.exists())
        {
            if (dontPollReadOnlyFiles && !file.isWritable())
            {
                IFTRACE(filemon)
                    debug() << "Read-only file: '" << +path << "' ('"
                            << +file.canonicalFilePath() << "')\n";
                file.ignore = true;
                continue;
            }

            foreach (FileMonitor *monitor, monitors)
            {
                if (!monitor->files.contains(path))
                    continue;

                FileMonitor::MonitoredFile &mf = monitor->files[path];
                QDateTime lastModified;
                QString canonicalPath;
                switch (mf.lastNotification)
                {
                case FileMonitor::None:
                case FileMonitor::Deleted:
                    IFTRACE(filemon)
                            debug() << "Created: '" << +path << "' ('"
                                    << +file.canonicalFilePath() << "')\n";
                    mf.cachedModified = file.lastModified();
                    mf.cachedCanonicalPath = file.canonicalFilePath();
                    mf.lastNotification = FileMonitor::Created;
                    monitor->onCreated(path, file.canonicalFilePath());
                    break;
                case FileMonitor::Created:
                case FileMonitor::Changed:
                    lastModified = file.lastModified();
                    canonicalPath = file.canonicalFilePath();
                    if ((mf.cachedModified.isValid() &&
                         lastModified > mf.cachedModified)
                        || canonicalPath != mf.cachedCanonicalPath)
                    {
                        IFTRACE(filemon)
                        {
                            const QString fmt("dd MMM yyyy hh:mm:ss.zzz");
                            debug() << "Changed: '" << +path << "' ('"
                                    << +mf.cachedCanonicalPath << "' "
                                    << +mf.cachedModified.toString(Qt::ISODate)
                                    << " -> '"
                                    << +canonicalPath << "' "
                                    << +lastModified.toString(Qt::ISODate) << ")\n";
                        }
                        mf.cachedModified = lastModified;
                        mf.cachedCanonicalPath = canonicalPath;
                        mf.lastNotification = FileMonitor::Changed;
                        monitor->onChanged(path, file.canonicalFilePath());
                    }
                    break;
                default:
                    break;
                }
            } // for each monitor
        }
        else
        {
            foreach (FileMonitor *monitor, monitors)
            {
                if (!monitor->files.contains(path))
                    continue;

                FileMonitor::MonitoredFile &mf = monitor->files[path];
                if (mf.lastNotification != FileMonitor::Deleted &&
                    mf.lastNotification != FileMonitor::None)
                {
                    IFTRACE(filemon)
                        debug() << "Deleted: '" << +path << "' ('"
                                << +mf.cachedCanonicalPath << "')\n";
                    mf.cachedModified = QDateTime();
                    mf.cachedCanonicalPath = QString();
                    mf.lastNotification = FileMonitor::Deleted;
                    monitor->onDeleted(path, file.canonicalFilePath());
                }
            } // for each monitor
        }
    }

    IFTRACE(filemon)
    {
        int ms = time.elapsed();
        debug() << files.size() << " paths checked in " << ms << "ms\n";
        time.start();
    }

    mergePending();
    QTimer::singleShot(pollInterval, this, SLOT(checkFiles()));
}



// ============================================================================
//
//    Functions exported by the module API
//
// ============================================================================

FileMonitorApi::FileMonitorApi(ModuleApi::file_info_callback created,
                               ModuleApi::file_info_callback changed,
                               ModuleApi::file_info_callback deleted,
                               void * userData,
                               std::string name)
// ----------------------------------------------------------------------------
//   Constructor
// ----------------------------------------------------------------------------
    : created(created), changed(changed), deleted(deleted), userData(userData),
      mon(new FileMonitor(+name))
{
    connect(mon,  SIGNAL(created(QString,QString)),
            this, SLOT(onCreated(QString,QString)));
    connect(mon,  SIGNAL(changed(QString,QString)),
            this, SLOT(onChanged(QString,QString)));
    connect(mon,  SIGNAL(deleted(QString,QString)),
            this, SLOT(onDeleted(QString,QString)));
}


FileMonitorApi::~FileMonitorApi()
// ----------------------------------------------------------------------------
//   Destructor
// ----------------------------------------------------------------------------
{
    delete mon;
}


void *FileMonitorApi::newFileMonitor(ModuleApi::file_info_callback created,
                                     ModuleApi::file_info_callback changed,
                                     ModuleApi::file_info_callback deleted,
                                     void *userData,
                                     std::string name)
// ----------------------------------------------------------------------------
//   Create file monitor object, register callbacks
// ----------------------------------------------------------------------------
{
    return new FileMonitorApi(created, changed, deleted, userData, name);
}


void FileMonitorApi::fileMonitorAddPath(void *fileMonitor, std::string path)
// ----------------------------------------------------------------------------
//   Monitor path
// ----------------------------------------------------------------------------
{
    FileMonitorApi *api = (FileMonitorApi *)fileMonitor;
    api->mon->addPath(+path);
}


void FileMonitorApi::fileMonitorRemovePath(void *fileMonitor, std::string path)
// ----------------------------------------------------------------------------
//   Stop monitoring path
// ----------------------------------------------------------------------------
{
    FileMonitorApi *api = (FileMonitorApi *)fileMonitor;
    api->mon->removePath(+path);
}


void FileMonitorApi::fileMonitorRemoveAllPaths(void *fileMonitor)
// ----------------------------------------------------------------------------
//   Stop monitoring all previously registered paths
// ----------------------------------------------------------------------------
{
    FileMonitorApi *api = (FileMonitorApi *)fileMonitor;
    api->mon->removeAllPaths();
}


void FileMonitorApi::deleteFileMonitor(void *fileMonitor)
// ----------------------------------------------------------------------------
//   Delete monitor
// ----------------------------------------------------------------------------
{
    FileMonitorApi *api = (FileMonitorApi *)fileMonitor;
    delete api;
}


void FileMonitorApi::onCreated(const QString &path,
                               const QString canonicalPath)
// ----------------------------------------------------------------------------
//   Forward 'created' signal to callback
// ----------------------------------------------------------------------------
{
    if (created)
        created(+path, +canonicalPath, userData);
}


void FileMonitorApi::onChanged(const QString &path,
                               const QString canonicalPath)
// ----------------------------------------------------------------------------
//   Forward 'changed' signal to callback
// ----------------------------------------------------------------------------
{
    if (changed)
        changed(+path, +canonicalPath, userData);
}


void FileMonitorApi::onDeleted(const QString &path,
                               const QString canonicalPath)
// ----------------------------------------------------------------------------
//   Forward 'deleted' signal to callback
// ----------------------------------------------------------------------------
{
    if (deleted)
        deleted(+path, +canonicalPath, userData);
}

}
