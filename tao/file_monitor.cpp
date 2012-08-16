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
#include <QMutexLocker>




namespace Tao {



// ============================================================================
//
//    Each instance of FileMonitor holds its own list of paths.
//
// ============================================================================

FileMonitor::FileMonitor()
// ----------------------------------------------------------------------------
//    Construct a file monitor object
// ----------------------------------------------------------------------------
    : thread(FileMonitorThread::instance())
{
    IFTRACE(filemon)
        debug() << "Creation\n";

    FileMonitorThread *t = thread.data();

    connect(this, SIGNAL(added(QString)),   t, SLOT(addPath(QString)));
    connect(this, SIGNAL(removed(QString)), t, SLOT(removePath(QString)));

    connect(t, SIGNAL(created(QString)), this, SLOT(onCreated(QString)));
    connect(t, SIGNAL(changed(QString)), this, SLOT(onChanged(QString)));
    connect(t, SIGNAL(deleted(QString)), this, SLOT(onDeleted(QString)));
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
    if (!paths.contains(path))
    {
        paths.append(path);
        emit added(path);
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
        emit removed(path);
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


void FileMonitor::onCreated(const QString &path)
// ----------------------------------------------------------------------------
//   Called by FileMonitorThread. Emit signal if we know this path.
// ----------------------------------------------------------------------------
{
    if (paths.contains(path))
        emit created(path);
}


void FileMonitor::onChanged(const QString &path)
// ----------------------------------------------------------------------------
//   Called by FileMonitorThread. Emit signal if we know this path.
// ----------------------------------------------------------------------------
{
    if (paths.contains(path))
        emit changed(path);
}


void FileMonitor::onDeleted(const QString &path)
// ----------------------------------------------------------------------------
//   Called by FileMonitorThread. Emit signal if we know this path.
// ----------------------------------------------------------------------------
{
    if (paths.contains(path))
        emit deleted(path);
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
//    The FileMonitorThread is a singleton used by all FileMonitor instances.
//
// ============================================================================

// We use a weak pointer so that when the last FileMonitor object is deleted,
// the singleton is also deleted and inst becomes NULL.
QWeakPointer<FileMonitorThread> FileMonitorThread::inst;


FileMonitorThread::FileMonitorThread()
// ----------------------------------------------------------------------------
//    Start the monitoring thread
// ----------------------------------------------------------------------------
    : done(false)
{
    IFTRACE(filemon)
        debug() << "Creation\n";
}


FileMonitorThread::~FileMonitorThread()
// ----------------------------------------------------------------------------
//    Stop and destroy the monitoring thread
// ----------------------------------------------------------------------------
{
    IFTRACE(filemon)
        debug() << "Stopping\n";
    done = true;
    wait(2000);
}


void FileMonitorThread::addPath(const QString &path)
// ----------------------------------------------------------------------------
//    Increment use count for path
// ----------------------------------------------------------------------------
{
    QMutexLocker locker(&mutex);
    refs[path]++;
    IFTRACE(filemon)
    {
        if (refs[path] == 1)
            debug() << "Adding to monitored list: '" << +path << "'\n";
    }
}


void FileMonitorThread::removePath(const QString &path)
// ----------------------------------------------------------------------------
//    Decrement use count for path
// ----------------------------------------------------------------------------
{
    QMutexLocker locker(&mutex);
    Q_ASSERT(refs.contains(path));
    int &count = refs[path];
    Q_ASSERT(count > 0);
    if (--count == 0)
    {
        refs.remove(path);
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

}
