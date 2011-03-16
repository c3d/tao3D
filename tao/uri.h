#ifndef URI_H
#define URI_H
// ****************************************************************************
//  uri.h                                                          Tao project
// ****************************************************************************
//
//   File Description:
//
//    This class holds the URIs that can be used to access a Tao document,
//    and the operations needed to resolve them.
//    A Tao URI relates either to a document (a file) or a project. In either
//    cases, the whole project is cloned.
//    Valid Tao URIs:
//      - Any valid Git URI (project URIs)
//        ssh://example.net/path/to/my/project
//        git://example.net/path/to/my/project
//        etc.
//      - Tao scheme (currently defaults to git over ssh)
//        tao://example.net/path/to/my/
//
//    URIs may contain additional parameters:
//        tao://example.net/project?d=main.ddd   (open main.ddd)
//        tao://example.net/project?r=mybranch   (checkout branch 'mybranch')
//        tao://example.net/project?r=bb8cb4a    (checkout commit bb8cb4a)
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "repository.h"
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QProgressDialog>


namespace Tao {

class Process;

class Uri : public QObject, public QUrl
{
    Q_OBJECT

public:
    Uri();
    Uri(QString uri);
    virtual ~Uri();

public:
    bool                  get();
    bool                  isLocal();

signals:
    void                  progressMessage(QString message);
    void                  docReady(QString path);
    void                  getFailed();

protected:
    QString               fileName();
    QString               rev();

    QString               documentOrProjectPath();
    QString               keyName();
    QString               newProject();
    QString               docPath(QString project);
    QString               repoUri();
    bool                  showRepoErrorDialog();

    void                  refreshSettings();
    void                  clearLocalProject();
    bool                  addLocalProject(const QString &path);
    QStringList           localProjects();

    bool                  fetchAndCheckout();
    bool                  cloneAndCheckout();
    bool                  checkout();

    QString               localRev(QString rev);

protected slots:
    void                  onDownloadFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void                  onDownloadError(QProcess::ProcessError error);
    void                  abortDownload();

private:
    repository_ptr        repo;
    bool                  done;
    QProgressDialog *     progress;
    process_p             proc;
    QString               procOut;
    int                   pos;
    QString               project;
    bool                  aborted;
    bool                  clone;

private:
   static bool            doRefresh;
};

}

#endif // URI_H
