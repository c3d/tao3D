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
#include "tao_utf8.h"
#include "error_message_dialog.h"

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
    : QApplication(argc, argv), hasGLMultisample(false)
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

    // Parse command line options
    XL::Errors errors(new XL::Positions());
    XL::Options options(errors, argc, argv);

    // Get the first file name and guess its location to initialize
    // the project folder. If there is no filename, or it is not found the
    // currentProjectFolder will be initialized to "".
    text project = options.ParseNext();
    currentProjectFolder = QFileInfo(+project).canonicalPath();

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
        ErrorMessageDialog dialog;
        dialog.setWindowTitle(tr("Framebuffer support"));
        dialog.showMessage(tr("This system does not support framebuffers."
                              " Performance may not be optimal."
                              " Consider updating the OpenGL drivers."));
    }
    {
        QGLWidget gl(QGLFormat(QGL::SampleBuffers), NULL);
        hasGLMultisample = gl.format().sampleBuffers();
    }
    if (!hasGLMultisample)
    {
        ErrorMessageDialog dialog;
        dialog.setWindowTitle(tr("Multisample support"));
        dialog.showMessage(tr("This system does not support GL sample buffers."
                              " Shapes and large text may look jaggy."));
    }
    if (!RepositoryFactory::available())
    {
        ErrorMessageDialog dialog;
        dialog.setWindowTitle(tr("Version control software"));
        dialog.showMessage(tr("No supported version control software was "
                              "found. Some functions will not be available. "
                              "Consider re-installing the application, "
                              "or installing Git v1.7.0 or later."));
    }

    // Create default folder for Tao documents
    // ("Save as..." box will land there)
    createDefaultProjectFolder();

    loadSettings();
}


Application::~Application()
// ----------------------------------------------------------------------------
//   Save user settings when we quit the application
// ----------------------------------------------------------------------------
{
    saveSettings();
}

void Application::updateSearchPathes()
// ----------------------------------------------------------------------------
//   Set the current project folder and all the dependant pathes.
// ----------------------------------------------------------------------------
{
    // Initialize dir search path for XL files
    QStringList xl_dir_list;
    xl_dir_list << currentProjectFolder
                << defaultTaoPreferencesFolderPath()
                << defaultTaoApplicationFolderPath();
    QDir::setSearchPaths("xl", xl_dir_list);

    // Initialize dir search path for XL system files
    QStringList xl_sys_list;
    xl_sys_list << defaultTaoApplicationFolderPath();
    QDir::setSearchPaths("system", xl_sys_list);

    // Setup search path for images
    QStringList images_dir_list;
    images_dir_list
        << currentProjectFolder + "/images"
        << currentProjectFolder;

    QDir::setSearchPaths("doc", images_dir_list);
    QDir::setSearchPaths("image", images_dir_list);
    QDir::setSearchPaths("texture", images_dir_list);
    QDir::setSearchPaths("icon", images_dir_list);

}

QString Application::defaultUserDocumentsFolderPath()
// ----------------------------------------------------------------------------
//    Try to guess the best documents folder to use by default
// ----------------------------------------------------------------------------
{
#ifdef QT_WS_WIN
    // Looking at the Windows registry
    QSettings settings(
            "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows"
            "\\CurrentVersion\\Explorer",
            QSettings::NativeFormat);

    // For Windows Vista/7
    // Typically C:\Users\username\Documents
    // For Windows XP
    // Typically C:\Documents and Settings\username\My Documents
    QString path = settings.value("User Shell Folders\\Personal").toString();

    if (!path.isNull())
    {
        return path;
    }

#endif // QT_WS_WIN

    // Trying to ding a home sub-directory ending with "Documents"
    QFileInfoList list = QDir::home().entryInfoList(
            QDir::NoDotAndDotDot | QDir::Dirs );
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


QString Application::defaultProjectFolderPath()
// ----------------------------------------------------------------------------
//    The folder proposed by default  "Save as..." for a new (Untitled) file
// ----------------------------------------------------------------------------
{
    return defaultUserDocumentsFolderPath() + tr("/Tao");
}


QString Application::defaultPreferencesFolderPath()
// ----------------------------------------------------------------------------
//    Try to guess the best user preference folder to use by default
// ----------------------------------------------------------------------------
{
#ifdef QT_WS_WIN
    // Looking at the Windows registry
    QSettings settings(
            "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows"
            "\\CurrentVersion\\Explorer",
            QSettings::NativeFormat);
    // For Windows Vista/7, typically
    // C:\Users\username\???
    // For Windows XP, typically
    // C:\Documents and Settings\username\Local Settings\Application Data
    QString path = settings.value("User Shell Folders\\Local AppData")
                   .toString();
    if (!path.isNull())
    {
        return path;
    }
#endif // QT_WS_WIN

    // Trying to ding a home sub-directory ending with "Documents"
    QFileInfoList list = QDir::home().entryInfoList(
            QDir::NoDotAndDotDot | QDir::Dirs );
    for (int i = 0; i < list.size(); i++)
    {
        QFileInfo info = list[i];
        if (info.fileName().endsWith("pref", Qt::CaseInsensitive))
        {
            return info.canonicalFilePath();
        }
    }
    // Last default would be home itself
    return QDir::homePath();
}


QString Application::defaultTaoPreferencesFolderPath()
// ----------------------------------------------------------------------------
//    The folder proposed to find user.xl, style.xl, etc...
//    (user preferences for tao application)
// ----------------------------------------------------------------------------
{
    return defaultPreferencesFolderPath() + tr("/xl.d");
}


QString Application::defaultTaoApplicationFolderPath()
// ----------------------------------------------------------------------------
//    Try to guess the best application folder to use by default
// ----------------------------------------------------------------------------
{
    return applicationDirPath();
}


QString Application::defaultUserImagesFolderPath()
// ----------------------------------------------------------------------------
//    Try to guess the best Images folder to use by default
// ----------------------------------------------------------------------------
{
#ifdef QT_WS_WIN
    // Looking at the Windows registry
    QSettings settings(
            "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows"
            "\\CurrentVersion\\Explorer",
            QSettings::NativeFormat);
    // For Windows Vista/7
    // Typically C:\Users\username\Documents\Pictures
    // For Windows XP
    // Typically C:\Documents and Settings\username\My Documents\My Pictures
    QString path = settings.value("User Shell Folders\\My Pictures")
                   .toString();
    if (!path.isNull())
    {
        return path;
    }
#endif // QT_WS_WIN

    // Trying to ding a home sub-directory ending with "images" or "pictures"
    QFileInfoList list = QDir::home().entryInfoList(
            QDir::NoDotAndDotDot | QDir::Dirs );
    for (int i = 0; i < list.size(); i++)
    {
        QFileInfo info = list[i];
        if (info.fileName().endsWith("images", Qt::CaseInsensitive) ||
            info.fileName().endsWith("pictures", Qt::CaseInsensitive) )
        {
            return info.canonicalFilePath();
        }
    }
    // Last default would be home itself
    return QDir::homePath();
}


bool Application::createDefaultProjectFolder()
// ----------------------------------------------------------------------------
//    Create Tao folder in user's documents folder (default path for saving)
// ----------------------------------------------------------------------------
{
    return QDir().mkdir(defaultProjectFolderPath());
}


bool Application::createDefaultTaoPrefFolder()
// ----------------------------------------------------------------------------
//    Create Tao folder in user's preference folder.
// ----------------------------------------------------------------------------
{
    return QDir().mkdir(defaultTaoPreferencesFolderPath());
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
                                  "environment\n"
                                  "A confirmation will be asked for each "
                                  "item to be deleted. You may also choose to "
                                  "delete all items at once."),
                               QMessageBox::Ok | QMessageBox::Cancel,
                               QMessageBox::Cancel);
    if (ret  != QMessageBox::Ok)
        return;

    // Tao folder under user's document folder
    QString path = defaultProjectFolderPath();
    if (ret != QMessageBox::YesAll)
        ret = QMessageBox::question(NULL, tr("Tao"),
                                    tr("Do you want to delete:\n\n"
                                       "User's default Tao folder?") +
                                    " (" + path + ")",
                                    QMessageBox::Yes    | QMessageBox::No |
                                    QMessageBox::YesAll | QMessageBox::Cancel);
    if (ret == QMessageBox::Cancel)
        return;
    if (ret == QMessageBox::Yes || ret == QMessageBox::YesAll)
        recursiveDelete(path);

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


void Application::saveSettings()
// ----------------------------------------------------------------------------
//    Save application settings so they are avaible on next start
// ----------------------------------------------------------------------------
{
    QSettings().setValue("UrlCompletions", QVariant(urlList));
    QSettings().setValue("PathCompletions", QVariant(pathList));
}


void Application::loadSettings()
// ----------------------------------------------------------------------------
//    Load application settings
// ----------------------------------------------------------------------------
{
    urlList = QSettings().value("UrlCompletions").toStringList();
    pathList = QSettings().value("PathCompletions").toStringList();
    // Normally not required, but initial implementation of completion used to
    // create duplicates :(
    urlList.removeDuplicates();
    pathList.removeDuplicates();
}


QStringList Application::pathCompletions()
// ----------------------------------------------------------------------------
//    Return paths the user previously entered in miscellaneous dialog boxes
// ----------------------------------------------------------------------------
{
    return pathList;
}


QStringList Application::urlCompletions()
// ----------------------------------------------------------------------------
//    Return urls the user previously entered in miscellaneous dialog boxes
// ----------------------------------------------------------------------------
{
    return urlList;
}


void Application::addPathCompletion(QString path)
// ----------------------------------------------------------------------------
//    Append a path to paths completions if not already present
// ----------------------------------------------------------------------------
{
    if (!pathList.contains(path))
        pathList.append(path);
}


void Application::addUrlCompletion(QString url)
// ----------------------------------------------------------------------------
//    Append an URL to URL completions if not already present
// ----------------------------------------------------------------------------
{
    if (!urlList.contains(url))
        urlList.append(url);
}


TAO_END
