#ifndef WEBUI_H
#define WEBUI_H
// *****************************************************************************
// webui.h                                                         Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2013-2014, Jérôme Forissier <jerome@taodyne.com>
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

#include "tao_process.h"
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
