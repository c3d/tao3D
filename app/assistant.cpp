// *****************************************************************************
// assistant.cpp                                                   Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2013-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011-2013, Jérôme Forissier <jerome@taodyne.com>
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

#include "assistant.h"

#include "application.h"
#include "base.h"
#include "module_manager.h"
#include "version2.h"
#include "tao_utf8.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QHelpEngineCore>
#include <QMessageBox>
#include <QProcess>

#include <iostream>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE


namespace Tao {


Assistant * Assistant::assistant = NULL;

Assistant::Assistant(QWidget *parent)
    : registered(false), proc(NULL), parent(parent)
{
    warnTitle = tr("Tao3D Help");
}


Assistant::~Assistant()
{
    if (proc && proc->state() == QProcess::Running) {
        proc->terminate();
        proc->waitForFinished(3000);
    }
    delete proc;
}


Assistant * Assistant::instance()
{
    if (!assistant)
        assistant = new Assistant();
    return assistant;
}


void Assistant::showDocumentation(const QString &page)
{
    if (!installed())
        return;

    Q_UNUSED(page);
    if (!registered)
    {
        QStringList helpFiles;
        helpFiles << ModuleManager::moduleManager()->qchFiles();
        // Show modules in alphabetical order
        // REVISIT: this is a quick and dirty (and buggy) way
        helpFiles.sort();
        // Show main Tao help file always first
        QString taoMainHelp = Application::applicationDirPath()
                + "/doc/" + TaoApp->lang + "/qch/Tao3D-"
                VERSION ".qch";
        // Load english doc if localized one is not available
        if (!QFileInfo(taoMainHelp).exists())
            taoMainHelp = Application::applicationDirPath()
                    + "/doc/en/qch/Tao3D-"
                    VERSION ".qch";
        helpFiles.prepend(taoMainHelp);
        registerQchFiles(helpFiles);
        registered = true;
    }

    startAssistant();
    return;
}


void Assistant::showKeywordHelp(const QString keyword)
// ----------------------------------------------------------------------------
//   Show help about keyword
// ----------------------------------------------------------------------------
{
    if (!installed())
        return;

    IFTRACE(assistant)
            debug() << "Show keyword help: '" << +keyword << "'\n";

    instance()->startAssistant();

    QByteArray ba("activatekeyword ");
    ba.append(keyword);

    instance()->proc->write(ba + '\n');
}


QStringList Assistant::registeredFiles(QString collectionFile)
// ----------------------------------------------------------------------------
//   Return the list of help files registered in collection
// ----------------------------------------------------------------------------
{
    QHelpEngineCore collection(collectionFile);
    if (!collection.setupData())
    {
        QMessageBox::warning(parent, warnTitle,
                             tr("Could not open file '%1': %2").
                             arg(collectionFile).
                             arg(collection.error()));
        return QStringList();
    }

    QStringList docs = collection.registeredDocumentations();
    QStringList files;
    foreach (QString doc, docs)
        files << collection.documentationFileName(doc);
    IFTRACE(assistant)
    {
        debug() << "Registered help files in collection '"
                << +collectionFile << "'\n";
        foreach (QString file, files)
        {
            QString ns = QHelpEngineCore::namespaceName(file);
            debug() << "  = '" << +file << "' (ns '" << +ns << "')\n";
        }

    }
    return files;
}


QStringList Assistant::registeredNamespaces(QString collectionFile)
// ----------------------------------------------------------------------------
//   Return the list of namespaces registered in collection
// ----------------------------------------------------------------------------
{
    QHelpEngineCore collection(collectionFile);
    if (!collection.setupData())
    {
        QMessageBox::warning(parent, warnTitle,
                             tr("Could not open file '%1': %2").
                             arg(collectionFile).
                             arg(collection.error()));
        return QStringList();
    }

    QStringList namespaces = collection.registeredDocumentations();
    IFTRACE(assistant)
    {
        debug() << "Registered namespaces in collection '"
                << +collectionFile << "'\n";
        foreach (QString ns, namespaces)
            debug() << "  = '" << +ns << "'\n";
    }
    return namespaces;
}


QStringList Assistant::stringListDifference(const QStringList &a,
                                            const QStringList &b)
// ----------------------------------------------------------------------------
//   Return all elements in 'a' not in 'b'
// ----------------------------------------------------------------------------
{
    QStringList ret;
    foreach (QString s, a)
        if (!b.contains(s))
            ret << s;
    return ret;
}


bool Assistant::registerDocumentation(const QStringList &files,
                                      const QString &collectionFile)
// ----------------------------------------------------------------------------
//   Add help files to a collection
// ----------------------------------------------------------------------------
{
    QHelpEngineCore collection(collectionFile);
    if (!collection.setupData())
        return false;
    IFTRACE(assistant)
            debug() << "Registering files into collection '" << +collectionFile
                    << "'\n";
    bool ok = true;
    foreach (QString file, files)
    {
        bool reg = collection.registerDocumentation(file);
        IFTRACE(assistant)
        {
            QString ns = QHelpEngineCore::namespaceName(file);
            debug() << "  + '" << +file << "' (ns '" << +ns << "')\n";
            if (!reg)
                debug() << "     FAILED\n";
        }
        ok &= reg;
    }
    return ok;
}


bool Assistant::unregisterDocumentation(const QStringList &files,
                                        const QString &collectionFile)
// ----------------------------------------------------------------------------
//   Remove help files from a collection
// ----------------------------------------------------------------------------
{
    QHelpEngineCore collection(collectionFile);
    if (!collection.setupData())
        return false;
    IFTRACE(assistant)
        debug() << "Unregistering files from collection '"
                << +collectionFile << "'\n";
    bool ok = true;
    foreach (QString file, files)
    {
        QString ns = QHelpEngineCore::namespaceName(file);
        IFTRACE(assistant)
            debug() << "  - '" << +file
                    << "' (ns '" << +ns << "')\n";
        if (ns != "")
        ok &= collection.unregisterDocumentation(ns);
    }
    return ok;
}


bool Assistant::unregisterDocumentationNS(const QStringList &namespaces,
                                          const QString &collectionFile)
// ----------------------------------------------------------------------------
//   Remove help files from a collection
// ----------------------------------------------------------------------------
{
    QHelpEngineCore collection(collectionFile);
    if (!collection.setupData())
        return false;
    IFTRACE(assistant)
        debug() << "Unregistering namespaces from collection '"
                << +collectionFile << "'\n";
    bool ok = true;
    foreach (QString ns, namespaces)
    {
        IFTRACE(assistant)
            debug() << "  - '" << +ns << "'\n";
        ok &= collection.unregisterDocumentation(ns);
    }
    return ok;
}


void Assistant::registerQchFiles(QStringList files)
// ----------------------------------------------------------------------------
//   Make sure the list of help files in user's collection contains 'files'
// ----------------------------------------------------------------------------
{
    QString userCollection = userCollectionFile();
    if (userCollection == "")
        return;

    IFTRACE(assistant)
    {
        debug() << "Checking if user collection file is up-to-date\n";
        debug() << "Module documentation files:\n";
        foreach(QString file, files)
            debug() << "  = '" << +file << "'\n";
    }

    QStringList userRegistered = registeredFiles(userCollection);

    if (files != userRegistered)
    {
        // Need to synchronize user collection file
        // Note - this will not unregister non-existing (deleted) files
        // because we can't get their namespace, and we need the namespace
        // to unregister. Not a problem because Assistant ignores missing
        // files.
        unregisterDocumentation(userRegistered, userCollection);
        registerDocumentation(files, userCollection);

        IFTRACE(assistant)
            (void)registeredFiles(userCollection);
    }

    IFTRACE(assistant)
        debug() << "User collection is up-to-date\n";
}


QString Assistant::assistantPath()
// ----------------------------------------------------------------------------
//   Return path to Qt Assistant application bundled with Tao
// ----------------------------------------------------------------------------
{
    QString app = QCoreApplication::applicationDirPath()
                + QDir::separator();
#if defined(Q_OS_LINUX)
    app += QLatin1String("assistant");
#elif defined(Q_OS_WIN)
    app += QLatin1String("assistant.exe");
#elif defined(Q_OS_MAC)
    app += QLatin1String("Tao3D Help.app/Contents/MacOS/"
                         "Tao3D Help");
#else
#error Please define Assistant path for this platform
#endif
    return app;
}


bool Assistant::installed()
// ----------------------------------------------------------------------------
//   Return true if the help viewer is installed
// ----------------------------------------------------------------------------
{
    return QFile(assistantPath()).exists();
}


QString Assistant::taoCollectionFilePath()
// ----------------------------------------------------------------------------
//   Return absolute path to user's collection file
// ----------------------------------------------------------------------------
{
    QString sep = QDir::separator();
    return Application::applicationDirPath()
            + sep + "doc" + sep + "Tao3D.qhc";
}


QString Assistant::userCollectionFilePath()
// ----------------------------------------------------------------------------
//   Return absolute path to user's collection file
// ----------------------------------------------------------------------------
{
    return Application::defaultTaoPreferencesFolderPath()
            + QDir::separator() + "Tao3D.qhc";
}


QString Assistant::userCollectionFile()
// ----------------------------------------------------------------------------
//   Return absolute path to user's collection file (create it if needed)
// ----------------------------------------------------------------------------
{
    QString to = userCollectionFilePath();
    if (!QFileInfo(to).exists())
    {
        QString from = taoCollectionFilePath();
        IFTRACE(assistant)
            debug() << "Copying " << +from << " to " << +to << "\n";

        if (!QFileInfo(from).exists())
        {
            QMessageBox::warning(parent, warnTitle,
                                 tr("File '%1' does not exist. Re-installing "
                                    "may fix the issue.").arg(from));
            return "";
        }
        QDir dir(Application::defaultTaoPreferencesFolderPath());
        if (!dir.exists())
        {
            QString path = QDir::toNativeSeparators(dir.absolutePath());
            if (!dir.mkpath(dir.absolutePath()))
            {
                QMessageBox::warning(parent, warnTitle,
                                     tr("Could not create folder: '%1'").
                                         arg(path));
                return "";
            }
        }
        if (!QFile::copy(from, to))
        {
            QMessageBox::warning(parent, warnTitle,
                                 tr("Could not create file: '%1'").arg(to));

        }

        // CHECK THIS
        // Remove registered files because paths are incorrect (relative?)
        QStringList taoNamespaces = registeredNamespaces(to);
        unregisterDocumentationNS(taoNamespaces, to);
        QStringList taoFiles = registeredFiles(taoCollectionFilePath());
        registerDocumentation(taoFiles, to);
    }
    return to;
}


QStringList Assistant::collectionFileArgs()
// ----------------------------------------------------------------------------
//   Command line arguments for Assistant to select user's collection file
// ----------------------------------------------------------------------------
{
    QStringList args;
    args << QLatin1String("-collectionFile") << userCollectionFile();
    return args;
}


bool Assistant::startAssistant()
// ----------------------------------------------------------------------------
//   Start assistant if not already running
// ----------------------------------------------------------------------------
{
    if (!proc)
    {
        proc = new QProcess();
        connect(proc, SIGNAL(readyReadStandardOutput()),
                this, SLOT(readStandardOutput()));
        connect(proc, SIGNAL(readyReadStandardError()),
                this, SLOT(readStandardError()));
    }

    if (proc->state() != QProcess::Running)
    {
        QString app = assistantPath();
        QStringList args;
        args << collectionFileArgs()
             << QLatin1String("-enableRemoteControl");
        IFTRACE(assistant)
        {
            QStringList all, all2;
            all << app << args;
            foreach (QString arg, all)
            {
                if (arg.contains(" "))
                    all2 << "'" + arg + "'";
                else
                    all2 << arg;
            }
            debug() << "Starting assistant:\n";
            debug() << "  " << all2.join(" ").toStdString() << "\n";
        }

        proc->start(app, args);

        if (!proc->waitForStarted())
        {
            QMessageBox::critical(0, QObject::tr("Tao3D"),
                QObject::tr("Unable to launch Help Viewer (%1)").arg(app));
            return false;
        }
    }
    return true;
}


void Assistant::readStandardOutput()
// ----------------------------------------------------------------------------
//   Dump child process standard output if debug traces are active
// ----------------------------------------------------------------------------
{
    IFTRACE(assistant)
    {
        QByteArray ba = proc->readAllStandardOutput();
        QString output = QString::fromLocal8Bit(ba.data());
        QStringList lines = output.split('\n');
        foreach (QString line, lines)
            debug() << "(stdout) " << +line << "\n";
    }
}


void Assistant::readStandardError()
// ----------------------------------------------------------------------------
//   Dump child process standard error if debug traces are active
// ----------------------------------------------------------------------------
{
    IFTRACE(assistant)
    {
        QByteArray ba = proc->readAllStandardError();
        QString output = QString::fromLocal8Bit(ba.data());
        QStringList lines = output.split('\n');
        foreach (QString line, lines)
            debug() << "(stderr) " << +line << "\n";
    }
}


std::ostream & Assistant::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[Assistant] ";
    return std::cerr;
}

}
