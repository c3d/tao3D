#ifndef UPDATE_APPLICATION_H
#define UPDATE_APPLICATION_H
// ****************************************************************************
//  update_application.h                                            Tao project
// ****************************************************************************
//
//   File Description:
//
//     Define how update Tao application.
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
//  (C) 2012 Baptiste Soulisse <baptiste.soulisse@taodyne.com>
//  (C) 2012 Taodyne SAS
// ****************************************************************************
#include "repository.h"
#include "process.h"
#include "tao.h"
#include <QObject>
#include <QString>
#include <QProgressDialog>
#include <QFile>
#include <QtNetwork>
#include <QMessageBox>
#include <QGridLayout>
#include <QPushButton>
#include <QProgressBar>

namespace Tao {


class UpdateApplication : public QObject
// ------------------------------------------------------------------------
//   Asynchronously update the main application
// ------------------------------------------------------------------------
{
    Q_OBJECT

    enum State { Idle, WaitingForUpdate, Downloading };

public:
    UpdateApplication();
    ~UpdateApplication();

    void     check(bool show = false);

private:
    void     startDownload();
    void     readIniFile();
    bool     createFile();
    void     showNoUpdateAvailable();
    void     resetRequest();
    QString  appName();

public slots:
    void     cancel();

private slots:
    void     processReply();
    void     downloadFinished();
    void     downloadProgress(qint64 bytesRcvd, qint64 bytesTotal);

    std::ostream & debug();

private:
    State                    state;

    // Tao info
    double                   version;
    QString                  edition;
    QString                  target;

    // Update info
    double                   remoteVersion;
    QUrl                     url;

    // I/O
    QFile *                  file;
    QProgressDialog *        progress;
    bool                     show;
    QString                  dialogTitle;
    QPixmap                  downloadIcon, checkmarkIcon;

    // Network
    QNetworkReply *          reply;
    QNetworkRequest          request;
    QNetworkAccessManager *  manager;
    QTime                    downloadTime;
    int                      code;
};

}

#endif // UPDATE_APPLICATION_H
