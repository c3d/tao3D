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

#include "application.h"
#include "base.h"
#include "module_manager.h"
#include "version.h"
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


Assistant::Assistant(QWidget *parent)
    : registered(false), proc(NULL), parent(parent)
{
    warnTitle = tr("Tao Presentations Help");
}


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
    if (!registered)
    {
        QStringList helpFiles;
        helpFiles << ModuleManager::moduleManager()->qchFiles();
        // Show modules in alphabetical order
        // REVISIT: this is a quick and dirty (and buggy) way
        helpFiles.sort();
        // Show main Tao help file always first
        QString taoMainHelp = Application::applicationDirPath()
                + "/doc/" + TaoApp->lang + "/qch/TaoPresentations.qch";
        // Load english doc if localized one is not available
        if (!QFileInfo(taoMainHelp).exists())
            taoMainHelp = Application::applicationDirPath()
                    + "/doc/en/qch/TaoPresentations.qch";
        helpFiles.prepend(taoMainHelp);
        registerQchFiles(helpFiles);
        registered = true;
    }

    if (!startAssistant())
        return;
return;
    QByteArray ba("SetSource ");
    ba.append("qthelp://com.taodyne.TaoPresentations.1.0.0/");

    proc->write(ba + page.toLocal8Bit() + '\n');
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
#if !defined(Q_OS_MAC)
    app += QLatin1String("assistant");
#else
    app += QLatin1String("Tao Presentations Help.app/Contents/MacOS/"
                         "Tao Presentations Help");
#endif
    return app;
}


QString Assistant::taoCollectionFilePath()
// ----------------------------------------------------------------------------
//   Return absolute path to user's collection file
// ----------------------------------------------------------------------------
{
    QString sep = QDir::separator();
    return Application::applicationDirPath()
            + sep + "doc" + sep + "TaoPresentations.qhc";
}


QString Assistant::userCollectionFilePath()
// ----------------------------------------------------------------------------
//   Return absolute path to user's collection file
// ----------------------------------------------------------------------------
{
    return Application::defaultTaoPreferencesFolderPath()
            + QDir::separator() + "TaoPresentations.qhc";
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
            QMessageBox::critical(0, QObject::tr("Tao Presentations"),
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
        QByteArray ba = proc->readAllStandardOutput();
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
