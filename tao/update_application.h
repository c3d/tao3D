#ifndef UPDATE_APPLICATION_H
#define UPDATE_APPLICATION_H
// ****************************************************************************
//  update_application.h                                            Tao project
// ****************************************************************************
//
//   File Description:
//
//     Update Tao application.
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
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
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

public:
    UpdateApplication();

    void     check(bool msg = false);
    void     start();
    void     update();
    void     extract();

private slots:
    void     processCheckForUpdate();
    void     downloadFinished();
    void     downloadReadyRead();
    void     downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void     cancelDownload();

    std::ostream & debug();

private:
    QString                system;           // Operating system
    QString                edition;          // Tao edition
    double                 version;          // Tao version

    QString                url;
    QFile*                 file;
    QFileInfo              info;

    QProgressDialog*       dialog;
    QNetworkReply*         reply;
    QNetworkAccessManager* manager;
    QTime                  downloadTime;
    bool                   downloadRequestAborted;
};

}

#endif // UPDATE_APPLICATION_H
