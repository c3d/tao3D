// ****************************************************************************
//  webui.cpp                                                      Tao project
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

#include "base.h"
#include "process.h"
#include "tao_utf8.h"
#include "webui.h"
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QLatin1String>
#include <QMessageBox>
#include <QRegExp>
#include <QStringList>
#include <QUrl>

namespace Tao {

WebUI::WebUI(QObject *parent)
// ----------------------------------------------------------------------------
//   Constructor
// ----------------------------------------------------------------------------
    : QObject(parent), server(this)
{
    connect(&server, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(serverStartError()));
    connect(&server, SIGNAL(standardOutputUpdated(QByteArray)),
            this,    SLOT(readServerOut(QByteArray)));
    server.setWd(serverDir());
}


WebUI::~WebUI()
// ----------------------------------------------------------------------------
//   Destructor
// ----------------------------------------------------------------------------
{
    stop();
}

void WebUI::launch(QString path)
// ----------------------------------------------------------------------------
//   Start server (if needed), open editor in default browser
// ----------------------------------------------------------------------------
{
    if (path != docPath)
    {
        stop();
        IFTRACE(webui)
            debug() << "Starting server (document: " << +path << ")\n";
        QStringList args;
        args << "server.js" << path;
        server.start(nodePath(), args);
    }
}


void WebUI::stop()
// ----------------------------------------------------------------------------
//   Stop server if running
// ----------------------------------------------------------------------------
{
    if (server.state() != QProcess::NotRunning)
    {
        IFTRACE(webui)
            debug() << "Stopping server\n";
        server.terminate();
        if (!server.waitForFinished(5000))
        {
            IFTRACE(webui)
                debug() << "Killing server\n";
            server.kill();
        }
        IFTRACE(webui)
            debug() << "Server stopped\n";
    }
}


void WebUI::serverStartError()
// ----------------------------------------------------------------------------
//   Notify user that server failed to start
// ----------------------------------------------------------------------------
{
    QMessageBox::warning(NULL, tr("Error"),
                         tr("Could not start document editor (server)"));
}


void WebUI::readServerOut(QByteArray newOut)
// ----------------------------------------------------------------------------
//   Launch browser when server says it is listening
// ----------------------------------------------------------------------------
{
    QRegExp re("Server listening on port ([0-9]+)");
    if (QString::fromUtf8(newOut.data()).contains(re))
        launchBrowser(re.cap(1).toInt());
}


void WebUI::launchBrowser(unsigned port)
// ----------------------------------------------------------------------------
//   Open a web browser window, connected to the Node.js server
// ----------------------------------------------------------------------------
{
    QString url = QString("http://localhost:%1/").arg(port);
    IFTRACE(webui)
        debug() << "Launching browser at: " << +url << "\n";
    if (!QDesktopServices::openUrl(QUrl(url)))
        QMessageBox::warning(NULL, tr("Error"),
                             tr("Could not launch web browser"));
}


std::ostream & WebUI::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[WebUI " << (void*)this << "] ";
    return std::cerr;
}


QString WebUI::nodePath()
// ----------------------------------------------------------------------------
//   The full path to the Node.js executable
// ----------------------------------------------------------------------------
{
    QString path = QCoreApplication::applicationDirPath()
            + QDir::separator() + "nodejs"
            + QDir::separator() + "bin"
            + QDir::separator();
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    path += QLatin1String("node");
#elif defined(Q_OS_WIN)
    path += QLatin1String("node.exe");
#else
#error Please define Node.js path for this platform
#endif
    return path;
}


QString WebUI::serverDir()
// ----------------------------------------------------------------------------
//   The full path to the directory containing server.js
// ----------------------------------------------------------------------------
{
    return QCoreApplication::applicationDirPath()
            + QDir::separator() + "webui"
            + QDir::separator() + "server";
}

} // namespace Tao
