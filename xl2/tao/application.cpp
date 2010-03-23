// ****************************************************************************
//  application.h                                                  Tao project
// ****************************************************************************
//
//   File Description:
//
//    The Tao application
//
//
//
//
//
//
//
//
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "application.h"
#include "widget.h"
#include "repository.h"
#include "git_backend.h"
#include "tao.h"

#include <QString>
#include <QSettings>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QLineEdit>
#include <QDir>
#include <QDebug>
#include <QtWebKit>

TAO_BEGIN


Application::Application(int & argc, char ** argv)
// ----------------------------------------------------------------------------
//    Build the Tao application
// ----------------------------------------------------------------------------
    : QApplication(argc, argv), repository(NULL)
{
    // Set some useful parameters for the application
    setApplicationName ("Tao");
    setOrganizationName ("Taodyne SAS");
    setOrganizationDomain ("taodyne.com");
    setWindowIcon(QIcon(":/images/tao.png"));

    // Internal clean option
    if (arguments().contains("--internal-use-only-clean-environment"))
    {
        internalCleanEverythingAsIfTaoWereNeverRun();
        std::exit(0);
    }

    // Web settings
    QWebSettings *gs = QWebSettings::globalSettings();
    gs->setAttribute(QWebSettings::JavascriptEnabled, true);
    gs->setAttribute(QWebSettings::JavaEnabled, true);
    gs->setAttribute(QWebSettings::PluginsEnabled, true);
    gs->setAttribute(QWebSettings::JavascriptCanOpenWindows, true);
    gs->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);
    gs->setAttribute(QWebSettings::LinksIncludedInFocusChain, true);
    gs->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, true);
    gs->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled,true);
    gs->setAttribute(QWebSettings::LocalStorageEnabled, true);

    // Configure the proxies for URLs
    QNetworkProxyFactory::setUseSystemConfiguration(true);

    // Basic sanity tests to check if we can actually run
    if (!QGLFormat::hasOpenGL())
    {
        QMessageBox::warning(NULL, tr("OpenGL support"),
                             tr("This system doesn't support OpenGL."));
        exit(1);
    }
    if (!QGLFramebufferObject::hasOpenGLFramebufferObjects())
    {
        // Check frame buffer support (non-fatal)
	QMessageBox::information(NULL, tr("Framebuffer support"),
				 tr("This system does not support framebuffers."
                                    " Performance may not be optimal."
                                    " Consider updating the OpenGL drivers."));
    }


}


Application::~Application()
// ----------------------------------------------------------------------------
//    Delete application's repository object
// ----------------------------------------------------------------------------
{
    delete repository;
}


bool Application::openLibrary()
// ----------------------------------------------------------------------------
//   Open the default library, or ask the user which one to choose
// ----------------------------------------------------------------------------
{
    QString path = userDocumentLibraryPath();
    return openLibrary(path, false);
}


bool Application::openLibrary(QString path, bool confirm)
// ----------------------------------------------------------------------------
//   Open the default library, or ask the user which one to choose
// ----------------------------------------------------------------------------
{
    bool ok = true;
    bool saveSettings = !confirm;

    do
    {
        if (path.isNull())
        {
            path = defaultDocumentLibraryPath();
            ok = false;
        }
        path = QDir::cleanPath(path);

        if (!ok)
            path = QInputDialog::getText(NULL, tr("Welcome to Tao!"),
                                         tr("Please choose a directory "
                                             "for your project:"),
                                         QLineEdit::Normal, path, &ok);
        if (path.isNull() || !ok)
            break;
 
        delete repository;
        repository = Repository::repository(path);
        if (!repository)
        {
            QMessageBox box;
            box.setWindowTitle("Tao Library");
            box.setText
                (tr("The project '%1' does not exist.")
                 .arg(path));
            box.setInformativeText
                (tr("Do you want to create a new project, "
                    "skip project selection and continue without a project, "
                    "or choose another location for the project?"));
            box.setIcon(QMessageBox::Question);
            QPushButton *choose = box.addButton(tr("Choose..."),
                                                QMessageBox::ActionRole);
            QPushButton *skip = box.addButton(tr("Skip"),
                                              QMessageBox::RejectRole);
            QPushButton *create = box.addButton(tr("Create"),
                                                QMessageBox::AcceptRole);
            box.setDefaultButton(create);
            int index = box.exec(); (void) index;
            QAbstractButton *which = box.clickedButton();

            if (which == choose)
            {
                ok = false;
            }
            else if (which == create)
            {
                repository = new GitRepository(path);
                repository->initialize();
                ok = repository->valid();
                if (!ok)
                {
                    delete repository;
                    repository = NULL;
                }
            }
            else if (which == skip)
            {
                // Continue with repository == NULL
                saveSettings = false;
            }
            else
            {
                QMessageBox::question(NULL, tr("Puzzled"),
                                      tr("How did you do that?"),
                                      QMessageBox::No);
            }
        }

        // Select the task branch, either current branch or without _tao_undo
        if (ok && repository && repository->valid())
        {
            text task = repository->branch();
            size_t len = task.length() - (sizeof (TAO_UNDO_SUFFIX) - 1);
            text currentBranch = task;
            bool onUndoBranch = task.find(TAO_UNDO_SUFFIX) == len;
            bool setTask = false;
            if (onUndoBranch)
                task = task.substr(0, len);

            if (confirm)
            {
                QMessageBox box;
                QString repo = repository->userVisibleName();
                box.setIcon(QMessageBox::Question);
                box.setWindowTitle
                    (tr("Existing %1 repository").arg(repo));
                if (onUndoBranch)
                {
                    box.setText
                        (tr("The project '%1' looks like "
                            "a valid %2 repository previously used by Tao.")
                         .arg(path).arg(repo));
                    box.setInformativeText
                        (tr("This repository appears to have been used by Tao, "
                            "because the current branch is named '%1'. "
                            "Do you want to use this repository for Tao "
                            "and perform new commits there, "
                            "to skip project selection, or "
                            "to choose another repository location?")
                         .arg(+currentBranch));
                }
                else
                {
                    box.setText
                        (tr("The project '%1' looks like a valid "
                            "%2 repository, but not currently used by Tao.")
                         .arg(path).arg(repo));
                    box.setInformativeText
                        (tr("This repository appears to not be currently in "
                            "use by Tao, because the current branch is "
                            "named '%1'. "
                            "Do you want to use this repository for Tao "
                            "and perform new commits in '%2' branch, "
                            "to skip project selection, or "
                            "to choose another repository location?")
                         .arg(+currentBranch)
                         .arg(+currentBranch + TAO_UNDO_SUFFIX));
                }

                QPushButton *choose = box.addButton(tr("Choose..."),
                                                    QMessageBox::ActionRole);
                QPushButton *skip = box.addButton(tr("Skip"),
                                                  QMessageBox::NoRole);
                QPushButton *use = box.addButton(tr("Use now"),
                                                 QMessageBox::YesRole);
                QPushButton *remember = box.addButton(tr("Use always"),
                                                      QMessageBox::YesRole);
                box.setDefaultButton(use);
                int index = box.exec(); (void) index;
                QAbstractButton *which = box.clickedButton();

                if (which == use)
                {
                    setTask = true;
                }
                else if (which == skip)
                {
                    setTask = false;
                }
                else if (which == choose)
                {
                    ok = false;
                    setTask = false;
                }
                else if (which == remember)
                {
                    setTask = true;
                    saveSettings = true;
                }
                else
                {
                    QMessageBox::question(NULL, tr("Coin?"),
                                          tr("How did you do that?"),
                                          QMessageBox::Discard);
                }
            }

            if (setTask)
                if (!repository->setTask(task))
                    QMessageBox::information
                        (NULL, tr("Task selection"),
                         tr("An error occured setting the task:\n%1.")
                         .arg(+repository->errors),
                         QMessageBox::Ok);
        }

    } while (!ok);

    if (ok && saveSettings)
        QSettings().setValue("defaultlibrary", path);

    return ok;
}


QDir Application::libraryDirectory()
// ----------------------------------------------------------------------------
//   Return the best path candidate for the library
// ----------------------------------------------------------------------------
{
    QDir result(QDir::current());
    if (repository)
    {
        result.cd(repository->path);
    }
    else
    {
        QString path = QSettings().value("defaultlibrary").toString();
        if (!path.isNull())
            result.cd(path);
    }
    return result;    
}


void pqs(const QString &qs)
// ----------------------------------------------------------------------------
//   Print a QString for debug purpose
// ----------------------------------------------------------------------------
{
    qDebug() << qs << "\n";
}


void Application::internalCleanEverythingAsIfTaoWereNeverRun()
// ----------------------------------------------------------------------------
//    Clean persistent stuff that previous Tao runs may have created
// ----------------------------------------------------------------------------
{
    int ret;
    ret = QMessageBox::warning(NULL, tr("Tao"),
                               tr("Cleaning the Tao environment"
                                  "\n\n"
                                  "This command allows you to clean the Tao "
                                  "environment: user preferences, default "
                                  "document library, etc.\n"
                                  "A confirmation will be asked for each "
                                  "item to be deleted. You may also choose to "
                                  "delete all items at once."),
                               QMessageBox::Ok | QMessageBox::Cancel,
                               QMessageBox::Cancel);
    if (ret  != QMessageBox::Ok)
        return;

    // Default document library
    QString path = defaultDocumentLibraryPath();
    ret = QMessageBox::question(NULL, tr("Tao"),
                                tr("Do you want to delete:\n\n"
                                   "Default document library?") +
                                " (" + path + ")",
                                QMessageBox::Yes    | QMessageBox::No |
                                QMessageBox::YesAll | QMessageBox::Cancel);
    if (ret == QMessageBox::Cancel)
        return;
    if (ret == QMessageBox::Yes || ret == QMessageBox::YesAll)
        recursiveDelete(path);

    // User's document library, if not the default
    path = userDocumentLibraryPath();
    if (!path.isNull() && path != defaultDocumentLibraryPath())
    {
        if (ret != QMessageBox::YesAll)
            ret = QMessageBox::question(NULL, tr("Tao"),
                                        tr("Do you want to delete:\n\n"
                                           "User's document library?") +
                                        " (" + path + ")",
                                        QMessageBox::Yes    | QMessageBox::No |
                                        QMessageBox::YesAll | QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel)
            return;
        if (ret == QMessageBox::Yes || ret == QMessageBox::YesAll)
            recursiveDelete(path);
    }

    // User preferences
    if (ret != QMessageBox::YesAll)
        ret = QMessageBox::question(NULL, tr("Tao"),
                                    tr("Do you want to delete:\n\n"
                                       "Tao user preferences?"),
                                    QMessageBox::Yes    | QMessageBox::No |
                                    QMessageBox::YesAll | QMessageBox::Cancel);
    if (ret == QMessageBox::Cancel)
        return;
    if (ret == QMessageBox::Yes || ret == QMessageBox::YesAll)
        QSettings().clear();
}


QString Application::defaultDocumentsFolderPath()
// ----------------------------------------------------------------------------
//    Try to guess the best documents folder to use by default
// ----------------------------------------------------------------------------
{
#ifdef QT_WS_WIN
    // Looking at the Windows registry
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\CurrentVersion\\Explorer",
                       QSettings::NativeFormat);
    // For Windows Vista/7
    QString path = settings.value("User Shell Folders\\Personal").toString();
    if (!path.isNull())
    {
        // Typically C:\Users\username\Documents
        return path;
    }
    // For Windows XP
    path = settings.value("User Shell Folders\\Personal").toString();
    if (!path.isNull())
    {
        // Typically C:\Documents and Settings\username\My Documents
        return path;
    }
#endif // QT_WS_WIN

    // Trying to ding a home sub-directory ending with "Documents"
    QFileInfoList list = QDir::home().entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs );
    for (int i = 0; i < list.size(); i++)
    {
        QFileInfo info = list[i];
        if (info.fileName().endsWith("documents", Qt::CaseInsensitive))
        {
            return info.canonicalFilePath();
        }
    }
    // Last default would be home itself
    return QDir::homePath();
}

QString Application::defaultDocumentLibraryPath()
// ----------------------------------------------------------------------------
//    The path proposed by default (first time run) for the user's doc library
// ----------------------------------------------------------------------------
{
    return defaultDocumentsFolderPath() + tr("/Tao Document Library");
}


QString Application::userDocumentLibraryPath()
// ----------------------------------------------------------------------------
//    The path to the current user's document library
// ----------------------------------------------------------------------------
{
    return QSettings().value("defaultlibrary").toString();
}


bool Application::recursiveDelete(QString path)
// ----------------------------------------------------------------------------
//    Delete a directory including all its files and sub-directories
// ----------------------------------------------------------------------------
{
    QDir dir(path);

    bool err = false;
    if (dir.exists())
    {
        QFileInfoList list = dir.entryInfoList(QDir::NoDotAndDotDot |
                                               QDir::Dirs | QDir::Files |
                                               QDir::Hidden);
        for (int i = 0; (i < list.size()) && !err; i++)
        {
            QFileInfo entryInfo = list[i];
            QString path = entryInfo.absoluteFilePath();
            if (entryInfo.isDir())
                err = recursiveDelete(path);
            else
                if (!QFile(path).remove())
                    err = true;
        }
        if (!QDir().rmdir(path))
            err = true;
    }
    return err;
}

TAO_END
