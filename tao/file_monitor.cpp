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


FileMonitor::FileMonitor()
// ----------------------------------------------------------------------------
//    Construct a file monitor object
// ----------------------------------------------------------------------------
{
    IFTRACE(filemon)
        debug() << "Creating\n";

    thread = FileMonitorThread::instance();
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
}


void FileMonitor::removePath(const QString &path)
// ----------------------------------------------------------------------------
//   Remove a path from the list of files and directories being monitored
// ----------------------------------------------------------------------------
{
    if (files.contains(path))
    {
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


std::ostream & FileMonitor::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[FileMonitor " << (void*)this << "] ";
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
//    Add path or increment use count
// ----------------------------------------------------------------------------
{
    QMutexLocker locker(&mutex);
    if (files.contains(path))
    {
        FileInfo &file = files[path];
        file.refs++;
        return;
    }

    IFTRACE(filemon)
        debug() << "Adding: '" << +path << "'\n";

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
    {
        files.remove(path);
        IFTRACE(filemon)
            debug() << "Removing: '" << +path << "'\n";
    }
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
        debug() << "FileMonitor " << (void*)monitor << " added (count="
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
        debug() << "FileMonitor " << (void*)monitor << " removed (count="
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


void FileMonitorThread::checkFiles()
// ----------------------------------------------------------------------------
//   Check all files for changes
// ----------------------------------------------------------------------------
{
    QMutexLocker locker(&mutex);

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

    QTimer::singleShot(pollInterval, this, SLOT(checkFiles()));
}

}
