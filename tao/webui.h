#ifndef WEBUI_H
#define WEBUI_H
// ****************************************************************************
//  webui.h                                                        Tao project
// ****************************************************************************
//
//   File Description:
//
//    Interface with the web-based document editor.
//
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
//  (C) 2013 Jerome Forissier <jerome@taodyne.com>
//  (C) 2013 Taodyne SAS
// ****************************************************************************

#include "process.h"
#include <QByteArray>
#include <QObject>
#include <QString>
#include <iostream>

namespace Tao {

class WebUI : public QObject
// ----------------------------------------------------------------------------
//   1 server instance (1 document) and possibly several web browser instances
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    WebUI(QObject *parent = 0);
    virtual ~WebUI();

public:
    void           launch(QString path);
    void           stopServer();

public slots:
    void           securitySettingChanged();

protected:
    std::ostream & debug();
    QString        nodePath();
    QString        serverDir();
    void           startServer(bool andBrowser = true);
    void           launchBrowser();

protected slots:
    void           serverStartError();
    void           readServerOut(QByteArray newOut);
    void           readServerError(QByteArray newOut);

protected:
    Process        server;
    QString        path;    // Path to .ddd document
    unsigned       port;    // TCP port the server is listening on
    QString        token;   // Security token shared between client and server
    bool           startBrowser; // Start browser when server is ready?
};

} // namespace Tao

#endif // WEBUI_H
