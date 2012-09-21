#ifndef FILE_MONITOR_H
#define FILE_MONITOR_H
// ****************************************************************************
//  file_monitor.h                                                 Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2012 Jerome Forissier <jerome@taodyne.com>
//  (C) 2012 Taodyne SAS
// ****************************************************************************

#include "tao.h"
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
    void           created(const QString &path, const QString canonicalPath);
    void           changed(const QString &path, const QString canonicalPath);
    void           deleted(const QString &path, const QString canonicalPath);


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
        QString              cachedCanonicalPath;
        NotificationKind     lastNotification;
    };

protected:
    std::ostream&  debug();
    QString        id();
    void           onCreated(const QString &path, const QString canonicalPath);
    void           onChanged(const QString &path, const QString canonicalPath);
    void           onDeleted(const QString &path, const QString canonicalPath);

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
    void           created(const QString &path, const QString canonicalPath);
    void           changed(const QString &path, const QString canonicalPath);
    void           deleted(const QString &path, const QString canonicalPath);

protected:
    FileMonitorThread();
public:
    virtual ~FileMonitorThread();

protected:

    class FileInfo : public QFileInfo
    {
    public:
        FileInfo(const QString &path)
            : QFileInfo(path), refs(1), ignore(false) {}
        FileInfo()
            : refs(1), ignore(false) {}

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

}

#endif // file_monitor.h
