#ifndef FILE_MONITOR_H
#define FILE_MONITOR_H
// *****************************************************************************
// file_monitor.h                                                  Tao3D project
// *****************************************************************************
//
// File description:
//
//     Be notified when files or directories are created, modified or deleted.
//     Monitoring occurs in its own thread, therefore it never blocks the main
//     thread.
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2012, Jérôme Forissier <jerome@taodyne.com>
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

#include "tao.h"
#include "module_api.h"
#include <QDateTime>
#include <QFileInfo>
#include <QMap>
#include <QMutex>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QThread>
#include <iostream>



namespace Tao {

class FileMonitorThread;

class FileMonitor : public QObject
// ----------------------------------------------------------------------------
//    Monitor a set of file and directory paths for changes.
// ----------------------------------------------------------------------------
{
    friend class FileMonitorThread;

    Q_OBJECT

public:
    FileMonitor(QString name = QString());
    FileMonitor(const FileMonitor &o);
    virtual ~FileMonitor();

    void           removeAllPaths();

public slots:
    void           addPath(const QString &path);
    void           removePath(const QString &path);

signals:
    void           created(const QString &path, const QString absolutePath);
    void           changed(const QString &path, const QString absolutePath);
    void           deleted(const QString &path, const QString absolutePath);


protected:
    enum NotificationKind
    {
        None, Created, Changed, Deleted
    };

    struct MonitoredFile
    {
        MonitoredFile(const QString &path)
            : path(path), lastNotification(None) {}
        MonitoredFile()
            : lastNotification(None) {}

        QString              path;
        QDateTime            cachedModified;
        QString              cachedAbsolutePath;
        NotificationKind     lastNotification;
    };

protected:
    std::ostream&  debug();
    QString        id();
    void           onCreated(const QString &path, const QString absolutePath);
    void           onChanged(const QString &path, const QString absolutePath);
    void           onDeleted(const QString &path, const QString absolutePath);

protected:
    QMap<QString, MonitoredFile>      files;
    QSharedPointer<FileMonitorThread> thread;
    QString                           name;
};


class FileMonitorThread : public QThread
// ----------------------------------------------------------------------------
//    A singleton that implements path monitoring in its own thread.
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    static QSharedPointer<FileMonitorThread> instance();

public:
    static int                               pollInterval;

public:
    void           addPath(const QString &path);
    void           removePath(const QString &path);

    void           addMonitor(FileMonitor *monitor);
    void           removeMonitor(FileMonitor *monitor);

signals:
    void           created(const QString &path, const QString absolutePath);
    void           changed(const QString &path, const QString absolutePath);
    void           deleted(const QString &path, const QString absolutePath);

protected:
    FileMonitorThread();
public:
    virtual ~FileMonitorThread();

public:

    class FileInfo : public QFileInfo
    {
    public:
        FileInfo(const QString &path)
            : QFileInfo(path), refs(1), ignore(false) {}
        FileInfo()
            : refs(1), ignore(false) {}

    public:
        QDateTime  lastModified() const;

    public:
        int   refs;
        bool  ignore;
    };

protected:
    std::ostream&  debug();
    void           mergePending();
    void           addPathNoLock(const QString &path);

protected slots:
    void           checkFiles();

protected:
    QMutex                   mutex;
    QMap<QString, FileInfo>  files;
    QList<FileMonitor *>     monitors;
    bool                     dontPollReadOnlyFiles;

    QMutex                   pendingMutex;
    QStringList              pending;

private:
    static QWeakPointer<FileMonitorThread> inst;
};


class FileMonitorApi : public QObject
// ----------------------------------------------------------------------------
//    File monitor wrapper for module API
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    FileMonitorApi(ModuleApi::file_info_callback created,
                   ModuleApi::file_info_callback changed,
                   ModuleApi::file_info_callback deleted,
                   void * userData,
                   std::string name = "");
    virtual ~FileMonitorApi();

public:
    // Functions exported by ModuleApi
    static void *newFileMonitor(ModuleApi::file_info_callback created,
                                ModuleApi::file_info_callback changed,
                                ModuleApi::file_info_callback deleted,
                                void * userData,
                                std::string name);
    static void fileMonitorAddPath(void *fileMonitor, std::string path);
    static void fileMonitorRemovePath(void *fileMonitor, std::string path);
    static void fileMonitorRemoveAllPaths(void *fileMonitor);
    static void deleteFileMonitor(void *fileMonitor);

private slots:
    void  onCreated(const QString &path, const QString absolutePath);
    void  onChanged(const QString &path, const QString absoluteFilePath);
    void  onDeleted(const QString &path, const QString absoluteFilePath);

private:
    ModuleApi::file_info_callback    created, changed, deleted;
    void                           * userData;
    FileMonitor                    * mon;
};

}

#endif // file_monitor.h
