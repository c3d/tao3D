// ****************************************************************************
//  assistant.cpp                                                  Tao project
// ****************************************************************************
//
//   File Description:
//
//     Show online documentation using the Qt Assistant application.
// 
//     Inspired from the Qt documentation: Simple Text Viewer Example
//
//
//
//
//
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2011 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2011 Taodyne SAS
// ****************************************************************************

#include "assistant.h"
#include "version.h"
#include "base.h"

#include <QByteArray>
#include <QDir>
#include <QCoreApplication>
#include <QProcess>
#include <QMessageBox>
#include <QSettings>

#include <iostream>


TAO_BEGIN


Assistant::Assistant()
    : proc(NULL)
{}


Assistant::~Assistant()
{
    if (proc && proc->state() == QProcess::Running) {
        proc->terminate();
        proc->waitForFinished(3000);
    }
    delete proc;
}


void Assistant::showDocumentation(const QString &page)
{
    if (!startAssistant())
        return;
return;
    QByteArray ba("SetSource ");
    ba.append("qthelp://com.taodyne.TaoPresentations.1.0.0/");

    proc->write(ba + page.toLocal8Bit() + '\n');
}


void Assistant::registerQchFiles(QStringList files)
{
    QSettings settings;
    QStringList reg = settings.value("registeredDocFiles").toStringList();
    files.sort();
    reg.sort();
    if (files != reg)
    {
        QProcess *proc = new QProcess();
        QString app = appPath();
        QStringList args;
        args << collectionFileArgs();
        foreach (QString f, files)
            args << "-register" << f;
        if (files.size())
        {
            IFTRACE(assistant)
            {
                debug() << "Registering doc files: running:\n";
                debug() << "  " << app.toStdString() << " "
                        << args.join(" ").toStdString() << "\n";
            }
        }

        proc->start(app, args);
        proc->waitForStarted();
        proc->waitForFinished(3000);
        IFTRACE(assistant)
        {
            QByteArray err = proc->readAllStandardError();
            if (err.size())
            {
                QString serr;
                serr = QString::fromLocal8Bit(err.constData(), err.size());
                debug() << serr.toStdString() << "\n";
            }
        }

        delete proc;
    }
}


QString Assistant::appPath()
{
    QString app = QCoreApplication::applicationDirPath()
                + QDir::separator();
#if !defined(Q_OS_MAC)
    app += QLatin1String("assistant");
#else
    app += QLatin1String("Assistant.app/Contents/MacOS/Assistant");
#endif
    return app;
}


QStringList Assistant::collectionFileArgs()
{
    QStringList args;
    args << QLatin1String("-collectionFile")
         << QCoreApplication::applicationDirPath()
         + QLatin1String("/doc/TaoPresentations.qhc");
    return args;
}


bool Assistant::startAssistant()
{
    if (!proc)
        proc = new QProcess();

    if (proc->state() != QProcess::Running)
    {
        QString app = appPath();
        QStringList args;
        args << collectionFileArgs()
             << QLatin1String("-enableRemoteControl");
        IFTRACE(assistant)
        {
            debug() << "Starting assistant:\n";
            debug() << "  " << app.toStdString() << " "
                    << args.join(" ").toStdString() << "\n";
        }

        proc->start(app, args);

        if (!proc->waitForStarted())
        {
            QMessageBox::critical(0, QObject::tr("Tao Presentations"),
                QObject::tr("Unable to launch Help Viewer (%1)").arg(app));
            return false;
        }
    }
    return true;
}


std::ostream & Assistant::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[Assistant] ";
    return std::cerr;
}

TAO_END
