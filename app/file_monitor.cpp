// *****************************************************************************
// file_monitor.cpp                                                Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2012,2014-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2012-2013, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

#include "file_monitor.h"
#include "base.h"
#include "tao_utf8.h"
#include <QDir>
#include <QMutexLocker>
#include <QTimer>
#include <sys/stat.h>



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
        FileMonitorThread::FileInfo file(path);
        if (file.exists())
        {
            mf.lastNotification = Created;
            mf.cachedAbsolutePath =  file.absoluteFilePath();
            mf.cachedModified = file.lastModified();
            emit created(path, mf.cachedAbsolutePath);
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
        XL_ASSERT(!files.contains(path));
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


void FileMonitor::onCreated(const QString &path, const QString absolutePath)
// ----------------------------------------------------------------------------
//   Called from the monitoring thread when a file is created.
// ----------------------------------------------------------------------------
{
    emit created(path, absolutePath);
}


void FileMonitor::onChanged(const QString &path, const QString absolutePath)
// ----------------------------------------------------------------------------
//   Called from the monitoring thread when file/absolute path changes.
// ----------------------------------------------------------------------------
{
    emit changed(path, absolutePath);
}


void FileMonitor::onDeleted(const QString &path, const QString absolutePath)
// ----------------------------------------------------------------------------
//   Called from the monitoring thread when a file is deleted or renamed.
// ----------------------------------------------------------------------------
{
    emit deleted(path, absolutePath);
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
    XL_ASSERT(files.contains(path));
    FileInfo &info = files[path];
    XL_ASSERT(info.refs > 0);
    if (--info.refs == 0)
        files.remove(path);
}


void FileMonitorThread::addMonitor(FileMonitor *monitor)
// ----------------------------------------------------------------------------
//    Add a FileMonitor instance
// ----------------------------------------------------------------------------
{
    QMutexLocker locker(&mutex);
    XL_ASSERT(!monitors.contains(monitor));
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
    XL_ASSERT(monitors.contains(monitor));
    monitors.removeOne(monitor);
    XL_ASSERT(!monitors.contains(monitor));

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
        XL_ASSERT(ptr);
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
                            << +file.absoluteFilePath() << "')\n";
                file.ignore = true;
                continue;
            }

            foreach (FileMonitor *monitor, monitors)
            {
                if (!monitor->files.contains(path))
                    continue;

                FileMonitor::MonitoredFile &mf = monitor->files[path];
                QDateTime lastModified;
                QString absolutePath;
                switch (mf.lastNotification)
                {
                case FileMonitor::None:
                case FileMonitor::Deleted:
                    IFTRACE(filemon)
                            debug() << "Created: '" << +path << "' ('"
                                    << +file.absoluteFilePath() << "')\n";
                    mf.cachedModified = file.lastModified();
                    mf.cachedAbsolutePath = file.absoluteFilePath();
                    mf.lastNotification = FileMonitor::Created;
                    monitor->onCreated(path, file.absoluteFilePath());
                    break;
                case FileMonitor::Created:
                case FileMonitor::Changed:
                    lastModified = file.lastModified();
                    absolutePath = file.absoluteFilePath();
                    if ((mf.cachedModified.isValid() &&
                         lastModified > mf.cachedModified)
                        || absolutePath != mf.cachedAbsolutePath)
                    {
                        IFTRACE(filemon)
                        {
                            const QString fmt("dd MMM yyyy hh:mm:ss.zzz");
                            debug() << "Changed: '" << +path << "' ('"
                                    << +mf.cachedAbsolutePath << "' "
                                    << +mf.cachedModified.toString(Qt::ISODate)
                                    << " -> '"
                                    << +absolutePath << "' "
                                    << +lastModified.toString(Qt::ISODate) << ")\n";
                        }
                        mf.cachedModified = lastModified;
                        mf.cachedAbsolutePath = absolutePath;
                        mf.lastNotification = FileMonitor::Changed;
                        monitor->onChanged(path, file.absoluteFilePath());
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
                                << +mf.cachedAbsolutePath << "')\n";
                    mf.cachedModified = QDateTime();
                    mf.cachedAbsolutePath = QString();
                    mf.lastNotification = FileMonitor::Deleted;
                    monitor->onDeleted(path, file.absoluteFilePath());
                }
            } // for each monitor
        }
    }

#if 0
    IFTRACE(filemon)
    {
        int ms = time.elapsed();
        debug() << files.size() << " paths checked in " << ms << "ms\n";
        time.start();
    }
#endif

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
                               const QString absolutePath)
// ----------------------------------------------------------------------------
//   Forward 'created' signal to callback
// ----------------------------------------------------------------------------
{
    if (created)
        created(+path, +absolutePath, userData);
}


void FileMonitorApi::onChanged(const QString &path,
                               const QString absolutePath)
// ----------------------------------------------------------------------------
//   Forward 'changed' signal to callback
// ----------------------------------------------------------------------------
{
    if (changed)
        changed(+path, +absolutePath, userData);
}


void FileMonitorApi::onDeleted(const QString &path,
                               const QString absolutePath)
// ----------------------------------------------------------------------------
//   Forward 'deleted' signal to callback
// ----------------------------------------------------------------------------
{
    if (deleted)
        deleted(+path, +absolutePath, userData);
}


QDateTime FileMonitorThread::FileInfo::lastModified() const
// ----------------------------------------------------------------------------
//   Return last modified date of file, or of symlink if path is a symlink
// ----------------------------------------------------------------------------
{
#ifndef Q_OS_WIN
    // On Windows, there are no symbolic links. There are .lnk files but the
    // Qt doc seems to imply that the lastModified() date is the date of the
    // .lnk file (which, if true, would achieve what we're trying to do here
    // for MacOSX and Linux). And there's no lstat anyways...
    if (isSymLink())
    {
        QByteArray ba = absoluteFilePath().toUtf8();
        struct stat st;
        if (lstat(ba.constData(), &st) == 0)
        {
            return QDateTime::fromTime_t(st.st_mtime);
        }
    }
#endif

    return QFileInfo::lastModified();
}

}
