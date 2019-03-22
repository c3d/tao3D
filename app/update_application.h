#ifndef UPDATE_APPLICATION_H
#define UPDATE_APPLICATION_H
// *****************************************************************************
// update_application.h                                            Tao3D project
// *****************************************************************************
//
// File description:
//
//     Define how update Tao application.
//
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2012, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2013-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2012-2013, Jérôme Forissier <jerome@taodyne.com>
// (C) 2012, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
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

#include "http_ua.h"
#include "repository.h"
#include "tao_process.h"
#include "tao.h"
#include <QDateTime>
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


struct UpdateApplication : QObject, HttpUserAgent
// ------------------------------------------------------------------------
//   Asynchronously update the main application
// ------------------------------------------------------------------------
{
private:
    Q_OBJECT

    enum State { Idle, WaitingForUpdate, Downloading, Downloaded,
                 NetworkErrorCheck, NetworkErrorDownload };

public:
    UpdateApplication();
    ~UpdateApplication();

    void      check(bool show = false);
    QDateTime lastChecked();

private:
    void     startDownload();
    void     readIniFile();
    bool     createFile();
    void     saveDownloadedData();
    void     showNoUpdateAvailable();
    void     showDownloadSuccessful();
    void     resetRequest();
    QString  appName();
    QString  remoteVer();
    void     connectSignals(QNetworkReply *reply);
    void     setLastChecked(QDateTime when);

public slots:
    void     cancel();

private slots:
    void     processReply();
    void     downloadFinished();
    void     downloadProgress(qint64 bytesRcvd, qint64 bytesTotal);
    void     networkError(QNetworkReply::NetworkError err);

    std::ostream & debug();

private:
    State                    state;

    // Update info
    double                   remoteVersion;
    QUrl                     url;
    QString                  description;

    // I/O
    QFile *                  file;
    QProgressDialog *        progress;
    bool                     show;
    QString                  dialogTitle;
    QPixmap                  downloadIcon,
                             checkmarkIcon,
                             connectionErrorIcon;

    // Network
    QNetworkReply *          reply;
    QNetworkRequest          request;
    QNetworkAccessManager *  manager;
    QTime                    downloadTime;
    int                      code;
    QString                  errorString;
};

}

#endif // UPDATE_APPLICATION_H
