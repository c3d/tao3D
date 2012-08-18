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
    : thread(FileMonitorThread::instance())
{
    FileMonitorThread *t = thread.data();

    connect(t, SIGNAL(created(QString, QString)),
            this, SLOT(onCreated(QString, QString)));
    connect(t, SIGNAL(changed(QString, QString)),
            this, SLOT(onChanged(QString, QString)));
    connect(t, SIGNAL(deleted(QString, QString)),
            this, SLOT(onDeleted(QString, QString)));

    IFTRACE(filemon)
        debug() << "Created\n";
}


FileMonitor::~FileMonitor()
// ----------------------------------------------------------------------------
//    Destructor
// ----------------------------------------------------------------------------
{
    IFTRACE(filemon)
        debug() << "Deleting\n";

    removeAllPaths();
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
    if (!paths.contains(path))
    {
        QFileInfo file(path);
        if (file.exists())
            emit created(path, file.canonicalFilePath());
        paths.append(path);
        thread->addPath(path);
    }
}


void FileMonitor::removePath(const QString &path)
// ----------------------------------------------------------------------------
//   Remove a path from the list of files and directories being monitored
// ----------------------------------------------------------------------------
{
    if (paths.contains(path))
    {
        paths.removeOne(path);
        thread->removePath(path);
        Q_ASSERT(!paths.contains(path));
    }
}


void FileMonitor::removeAllPaths()
// ----------------------------------------------------------------------------
//   Clear all monitored items
// ----------------------------------------------------------------------------
{
    foreach (QString path, paths)
        removePath(path);
}


void FileMonitor::onCreated(const QString &path, const QString canonicalPath)
// ----------------------------------------------------------------------------
//   Called by FileMonitorThread. Emit signal if we know this path.
// ----------------------------------------------------------------------------
{
    if (paths.contains(path))
        emit created(path, canonicalPath);
}


void FileMonitor::onChanged(const QString &path, const QString canonicalPath)
// ----------------------------------------------------------------------------
//   Called by FileMonitorThread. Emit signal if we know this path.
// ----------------------------------------------------------------------------
{
    if (paths.contains(path))
        emit changed(path, canonicalPath);
}


void FileMonitor::onDeleted(const QString &path, const QString canonicalPath)
// ----------------------------------------------------------------------------
//   Called by FileMonitorThread. Emit signal if we know this path.
// ----------------------------------------------------------------------------
{
    if (paths.contains(path))
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
        file.ignore = false;
        return;
    }

    IFTRACE(filemon)
        debug() << "Adding to monitored list: '" << +path << "'\n";

    FileInfo file(path);
    if (file.exists())
    {
        // 'created' signal already sent synchronously by FileMonitor
        file.cachedModified = file.lastModified();
        file.lastNotification = Created;
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
    {
        files.remove(path);
        IFTRACE(filemon)
            debug() << "Removing from monitored list: '" << +path << "\n";
    }
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

            QDateTime lastModified;
            QString canonicalPath;
            switch (file.lastNotification)
            {
            case None:
            case Deleted:
                IFTRACE(filemon)
                    debug() << "Created: '" << +path << "' ('"
                            << +file.canonicalFilePath() << "')\n";
                file.cachedModified = file.lastModified();
                file.cachedCanonicalPath = file.canonicalFilePath();
                file.lastNotification = Created;
                emit created(path, file.canonicalFilePath());
                break;
            case Created:
            case Changed:
                lastModified = file.lastModified();
                canonicalPath = file.canonicalFilePath();
                if (lastModified > file.cachedModified ||
                    canonicalPath != file.cachedCanonicalPath)
                {
                    IFTRACE(filemon)
                    {
                        const QString fmt("dd MMM yyyy hh:mm:ss.zzz");
                        debug() << "Changed: '" << +path << "' ('"
                                << +file.cachedCanonicalPath << "' "
                                << +file.cachedModified.toString(Qt::ISODate)
                                << " -> '"
                                << +canonicalPath << "' "
                                << +lastModified.toString(Qt::ISODate) << ")\n";
                    }
                    file.cachedModified = lastModified;
                    file.cachedCanonicalPath = canonicalPath;
                    file.lastNotification = Changed;
                    emit changed(path, file.canonicalFilePath());
                }
                break;
            default:
                break;
            }
        }
        else
        {
            if (file.lastNotification != Deleted)
            {
                IFTRACE(filemon)
                    debug() << "Deleted: '" << +path << "' ('"
                            << +file.cachedCanonicalPath << "')\n";
                file.cachedModified = QDateTime();
                file.cachedCanonicalPath = QString();
                file.lastNotification = Deleted;
                emit deleted(path, file.canonicalFilePath());
            }
        }
    }

    QTimer::singleShot(pollInterval, this, SLOT(checkFiles()));
}

}
