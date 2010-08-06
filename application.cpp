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
#include "options.h"
#include "uri.h"
#include "splash_screen.h"
#include "graphics.h"
#include "window.h"

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
    : QApplication(argc, argv), hasGLMultisample(false), splash(NULL),
      pendingOpen(0), xlr(NULL)
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
    bool showSplash = true;
    if (arguments().contains("-nosplash") || arguments().contains("-h"))
        showSplash = false;

    // Show splash screen
    if (showSplash)
    {
        splash = new SplashScreen();
        splash->show();
        splash->raise();
        QApplication::processEvents();
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
        // Nothing (dialog box already shown by Repository class)
    }

    // Create default folder for Tao documents
    // ("Save as..." box will land there)
    createDefaultProjectFolder();

    loadSettings();

    // Setup the XL runtime environment
    updateSearchPaths();
    QFileInfo syntax    ("system:xl.syntax");
    QFileInfo stylesheet("system:xl.stylesheet");
    QFileInfo builtins  ("system:builtins.xl");
    XL::Compiler *compiler = new XL::Compiler("xl_tao");
    xlr = new XL::Main(argc, argv, *compiler,
                       +syntax.canonicalFilePath(),
                       +stylesheet.canonicalFilePath(),
                       +builtins.canonicalFilePath());
    XL::MAIN = xlr;

    if (!savedUri.isEmpty())
        loadUri(savedUri);
}


Application::~Application()
// ----------------------------------------------------------------------------
//   Save user settings when we quit the application
// ----------------------------------------------------------------------------
{
    saveSettings();
}


bool Application::processCommandLine()
// ----------------------------------------------------------------------------
//   Handle command-line files or URIs
// ----------------------------------------------------------------------------
{
    // Fetch info for XL files
    QFileInfo user      ("xl:user.xl");
    QFileInfo theme     ("xl:theme.xl");
    QFileInfo tutorial  ("system:tutorial.ddd");

    EnterGraphics(xlr->context);
    if (user.exists())
        contextFiles.push_back(+user.canonicalFilePath());
    if (theme.exists())
        contextFiles.push_back(+theme.canonicalFilePath());

    // Create the windows for each file or URI on the command line
    hadWin = false;
    XL::source_names &names = xlr->file_names;
    XL::source_names::iterator it;
    for (it = names.begin(); it != names.end(); it++)
    {
        if (splash)
            splash->raise();
        QString sourceFile = +(*it);
        Tao::Window *window = new Tao::Window (xlr, contextFiles);
        if (splash)
        {
            window->splashScreen = splash;
            QObject::connect(splash, SIGNAL(destroyed(QObject*)),
                             window, SLOT(removeSplashScreen()));
        }
        window->deleteOnOpenFailed = true;
        connect(window, SIGNAL(openFinished(bool)),
                this, SLOT(onOpenFinished(bool)));
        int st = window->open(sourceFile);
        switch (st)
        {
        case 0:
            break;
        case 1:
            window->show();
            hadWin = true;
            break;
        case 2:
            window->show();
            if (splash)
                splash->raise();
            pendingOpen++;
            break;
        default:
            Q_ASSERT(!"Unexpected return value");
            break;
        }
    }

    if (!findFirstTaoWindow())
    {
        // Open tutorial file read-only
        QString tuto = tutorial.canonicalFilePath();
        Tao::Window *untitled = new Tao::Window(xlr, contextFiles);
        untitled->open(tuto, true);
        untitled->isUntitled = true;
        untitled->isReadOnly = true;
        untitled->show();
        hadWin = true;
    }

    if (splash && pendingOpen == 0)
    {
        splash->close();
        splash->deleteLater();
        splash = NULL;
    }

    return (hadWin || pendingOpen);
}


Window * Application::findFirstTaoWindow()
// ----------------------------------------------------------------------------
//   Enumerate Tao top-level windows and return first instance, or NULL
// ----------------------------------------------------------------------------
{
    Window *window = NULL;
    foreach (QWidget *widget, QApplication::topLevelWidgets())
    {
        window = dynamic_cast<Window *>(widget);
        if (window)
            break;
    }
    return window;
}


void Application::loadUri(QString uri)
// ----------------------------------------------------------------------------
//   Create a new window to load a document from a Tao URI
// ----------------------------------------------------------------------------
{
    if (!xlr)
    {
        // Event delivered before Application constructor could complete.
        // Save URI, constructor will open it
        savedUri = uri;
        return;
    }

    Window *window = findFirstTaoWindow();
    if (!window)
    {
        window = new Tao::Window (xlr, contextFiles);
        window->deleteOnOpenFailed = true;
    }
    int st = window->open(uri);
    if (st == 0)
        QMessageBox::warning(window, tr("Error"),
                             tr("Could not open %1.\n"
                                "Please check the address.\n").arg(uri));
    pendingOpen++;
}


void Application::onOpenFinished(bool ok)
// ----------------------------------------------------------------------------
//   Decrement count of pending opens, delete splash screen accordingly
// ----------------------------------------------------------------------------
{
    if (ok)
        hadWin = true;

    if (--pendingOpen == 0 && splash)
    {
        splash->close();
        splash->deleteLater();
        splash = NULL;
    }
}


static void printSearchPath(QString prefix)
// ----------------------------------------------------------------------------
//   Display a Qt search path to stdout (debugging)
// ----------------------------------------------------------------------------
{
    std::cerr << "Qt search path '" << +prefix << ":' set to '";
    QStringList list = QDir::searchPaths(prefix);
    int c = list.count();
    for (int i = 0; i < c; i++)
    {
        std::cerr << +list[i];
        if (i != c-1)
            std::cerr << ":";
    }
    std::cerr << "'\n";
}


static void setSearchPaths(QString prefix, QStringList value)
// ----------------------------------------------------------------------------
//   Set search paths for a prefix
// ----------------------------------------------------------------------------
{
    QDir::setSearchPaths(prefix, value);
    IFTRACE(paths)
        printSearchPath(prefix);
}

void Application::updateSearchPaths(QString currentProjectFolder)
// ----------------------------------------------------------------------------
//   Set the current project folder and all the dependant paths.
// ----------------------------------------------------------------------------
{
    if (currentProjectFolder.isEmpty())
        currentProjectFolder = ".";

    // Initialize dir search path for XL files
    QStringList xl_dir_list;
    xl_dir_list << currentProjectFolder
                << defaultTaoPreferencesFolderPath()
                << defaultTaoApplicationFolderPath();
    setSearchPaths("xl", xl_dir_list);

    // Initialize dir search path for XL system files
    QStringList xl_sys_list;
    xl_sys_list << defaultTaoApplicationFolderPath();
    setSearchPaths("system", xl_sys_list);

    // Setup search path for images
    QStringList images_dir_list;
    images_dir_list
        << currentProjectFolder + "/images"
        << currentProjectFolder;

    setSearchPaths("doc", images_dir_list);
    setSearchPaths("image", images_dir_list);
    setSearchPaths("svg", images_dir_list);
    setSearchPaths("texture", images_dir_list);
    setSearchPaths("icon", images_dir_list);

    // Setup search path for 3D objects
    QStringList object_dir_list;
    object_dir_list
        << currentProjectFolder + "/objects"
        << currentProjectFolder;

    setSearchPaths("object", images_dir_list);
}


QString Application::defaultUserDocumentsFolderPath()
// ----------------------------------------------------------------------------
//    Try to guess the best documents folder to use by default
// ----------------------------------------------------------------------------
{
#ifdef CONFIG_MINGW
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

#endif // CONFIG_MINGW

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
#ifdef CONFIG_MINGW
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
#endif // CONFIG_MINGW

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
#ifdef CONFIG_MINGW
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
#endif // CONFIG_MINGW

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
    IFTRACE(settings)
    {
        std::cerr << "URL completions saved:\n";
        foreach (QString s, urlList)
            std::cerr << "  " << +s << "\n";
    }
    QSettings().setValue("PathCompletions", QVariant(pathList));
    IFTRACE(settings)
    {
        std::cerr << "Path completions saved:\n";
        foreach (QString s, pathList)
            std::cerr << "  " << +s << "\n";
    }
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

    // Cleanup obsolete URI/project mappings (QSettings) before we open any URI
    Uri::gc();
}


bool Application::event(QEvent *e)
// ----------------------------------------------------------------------------
//    Process file open / URI open events
// ----------------------------------------------------------------------------
{
    QFileOpenEvent * foe;
    switch(e->type())
    {
    case QEvent::FileOpen:
        {
            foe = (QFileOpenEvent *)e;
            QString uri = foe->url().toString();
            IFTRACE(uri)
                    std::cerr << "URL event: " << +uri << "\n";
            loadUri(uri);
            return true;
        }
    default:
        break;
    }

    return QCoreApplication::event(e);
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
