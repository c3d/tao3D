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
#include "tao_main.h"
#include "error_message_dialog.h"
#include "options.h"
#include "uri.h"
#include "splash_screen.h"
#include "graphics.h"
#include "window.h"
#include "font_file_manager.h"
#include "module_manager.h"
#include "traces.h"

#include <QString>
#include <QSettings>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QLineEdit>
#include <QDir>
#include <QDebug>
#include <QtWebKit>
#include <QProcessEnvironment>

#if defined(CONFIG_MINGW)
#include <windows.h>
#elif defined(CONFIG_MACOSX)
extern "C" void UpdateSystemActivity(uint8_t);
#define UsrActivity 1
#elif defined(CONFIG_LINUX)
#include <X11/extensions/scrnsaver.h>
#endif

XL_DEFINE_TRACES

TAO_BEGIN


Application::Application(int & argc, char ** argv)
// ----------------------------------------------------------------------------
//    Build the Tao application
// ----------------------------------------------------------------------------
    : QApplication(argc, argv), hasGLMultisample(false), splash(NULL),
      pendingOpen(0), xlr(NULL), screenSaverBlocked(false), moduleManager(NULL)
{
    // Set some useful parameters for the application
    setApplicationName ("Tao");
    setOrganizationName ("Taodyne");
    setOrganizationDomain ("taodyne.com");
    setWindowIcon(QIcon(":/images/tao.png"));

    // Internal clean option
    if (arguments().contains("--internal-use-only-clean-environment"))
    {
        internalCleanEverythingAsIfTaoWereNeverRun();
        exit(0);
    }
    bool showSplash = true;
    if (arguments().contains("-nosplash") || arguments().contains("-h"))
        showSplash = false;

    if (arguments().contains("-norepo"))
        RepositoryFactory::no_repo = true;

    // Show splash screen
    if (showSplash)
    {
        splash = new SplashScreen();
        splash->show();
        splash->raise();
        QApplication::processEvents();
    }

    // Setup the XL runtime environment
    // Do it soon because debug traces are activated by this
    XL_INIT_TRACES();
    updateSearchPaths();
    QFileInfo syntax    ("system:xl.syntax");
    QFileInfo stylesheet("system:xl.stylesheet");
    QFileInfo builtins  ("system:builtins.xl");
    XL::Main * xlr = new Main(argc, argv, "xl_tao",
                              +syntax.canonicalFilePath(),
                              +stylesheet.canonicalFilePath(),
                              +builtins.canonicalFilePath());
    loadDebugTraceSettings();

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

    loadFonts();

    // The aboutToQuit signal is the recommended way for cleaning things up
    connect(this, SIGNAL(aboutToQuit()), this, SLOT(cleanup()));

#if defined (CONFIG_LINUX)
    xDisplay = XOpenDisplay(NULL);
    Q_ASSERT(xDisplay);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    ssHeartBeatCommand = env.value("TAO_SS_HEARTBEAT_CMD");
#endif

    XL::MAIN = this->xlr = xlr;
    loadModules();               // Needs a valid XL::MAIN

    // We're ready to go
    if (!savedUri.isEmpty())
        loadUri(savedUri);
}


void Application::loadModules()
// ----------------------------------------------------------------------------
//   Initialize module manager and load all modules that user has enabled
// ----------------------------------------------------------------------------
{

    if (splash)
        splash->showMessage(tr("Checking module configuration"));
    moduleManager = ModuleManager::moduleManager();
    if (splash)
        splash->showMessage(tr("Loading modules"));
    connect(moduleManager, SIGNAL(loading(QString)),
            this, SLOT(moduleIsLoading(QString)));
    moduleManager->loadAll();
}


void Application::moduleIsLoading(QString name)
// ----------------------------------------------------------------------------
//   Show module names on splash screen as they are loading
// ----------------------------------------------------------------------------
{
    if (splash)
    {
        QString msg = QString(tr("Loading modules [%1]")).arg(name);
        splash->showMessage(msg);
    }
}


void Application::cleanup()
// ----------------------------------------------------------------------------
//   Perform last-minute cleanup before application exit
// ----------------------------------------------------------------------------
{
    saveSettings();
    if (screenSaverBlocked)
        blockScreenSaver(false);
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

    EnterGraphics(xlr->globals);
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
            delete window;
            window = NULL;
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


void Application::blockScreenSaver(bool block)
// ----------------------------------------------------------------------------
//   Disable screen saver or restore it to previous state
// ----------------------------------------------------------------------------
{
    // Preventing the screen saver from running is system-specific.
    // Unfortunately, Qt provides no interface for this.
    //
    // MacOSX
    //   Simulate activity by calling UpdateSystemActivity(UsrActivity)
    //   periodically (every 30s).
    //
    // Windows
    //   There is one function call to disable the screen saver, another one
    //   to enable it.
    //
    // Linux
    //   The technique depends on the screen saver being used. Here is what we
    //   do to prevent the screen saver from running:
    //     1. Call (once) the XSS API: XScreenSaverSuspend
    //     2. Periodically call the Xlib API: XResetScreenSaver
    //     3. Periodically execute the command given in the
    //        TAO_SS_HEARTBEAT_CMD environment variable, if defined.
    //   Therefore, all screen savers that support the Xlib or XSS APIs are
    //   automatically disabled. For other screen savers you will have to
    //   define TAO_SS_HEARTBEAT_CMD.
    //     * Example for Gnome:
    //         TAO_SS_HEARTBEAT_CMD="gnome-screensaver-command -p"
    //     * Example for xscreensaver:
    //         TAO_SS_HEARTBEAT_CMD="xscreensaver-command -deactivate"

    if (block && screenSaverBlocked)
        return;

    screenSaverBlocked = block;
    if (block)
    {
#if   defined(CONFIG_MACOSX)
        QTimer::singleShot(30000, this, SLOT(simulateUserActivity()));
#elif defined(CONFIG_MINGW)
        SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, FALSE, 0, 0);
#elif defined(CONFIG_LINUX)
        XScreenSaverSuspend(xDisplay, True);
        QTimer::singleShot(30000, this, SLOT(simulateUserActivity()));
#endif
    }
    else
    {
#if   defined(CONFIG_MACOSX)
        // Nothing
#elif defined(CONFIG_MINGW)
        // CHECKTHIS: I suspect we should do this only if screen saver
        // was enabled when we disabled it. But this looks fundamentally
        // flawed: what if several apps do this at the same time?
        SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, TRUE, 0, 0);
#elif defined(CONFIG_LINUX)
        XScreenSaverSuspend(xDisplay, False);
#endif
    }
}


#if defined (CONFIG_MACOSX) || defined (CONFIG_LINUX)

void Application::simulateUserActivity()
// ----------------------------------------------------------------------------
//   Simulate user activity so that screensaver won't kick in
// ----------------------------------------------------------------------------
{
    if (!screenSaverBlocked)
        return;

#if defined (CONFIG_MACOSX)
    UpdateSystemActivity(UsrActivity);
#elif defined (CONFIG_LINUX)
    XResetScreenSaver(xDisplay);
    if (!ssHeartBeatCommand.isEmpty())
        QProcess::execute(ssHeartBeatCommand);
#endif
    QTimer::singleShot(30000, this, SLOT(simulateUserActivity()));
}

#endif

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
        return QDir::toNativeSeparators(path);
    }

#endif // CONFIG_MINGW

    // Trying to find a home sub-directory ending with "Documents"
    QFileInfoList list = QDir::home().entryInfoList(
            QDir::NoDotAndDotDot | QDir::Dirs );
    for (int i = 0; i < list.size(); i++)
    {
        QFileInfo info = list[i];
        if (info.fileName().endsWith("documents", Qt::CaseInsensitive))
        {
            return QDir::toNativeSeparators(info.canonicalFilePath());
        }
    }
    // Last default would be home itself
    return QDir::toNativeSeparators(QDir::homePath());
}


QString Application::defaultProjectFolderPath()
// ----------------------------------------------------------------------------
//    The folder proposed by default  "Save as..." for a new (Untitled) file
// ----------------------------------------------------------------------------
{
    return QDir::toNativeSeparators(defaultUserDocumentsFolderPath()
                                    + tr("/Tao"));
}


QString Application::appDataPath()
// ----------------------------------------------------------------------------
//    Try to guess the best user preference folder to use by default
// ----------------------------------------------------------------------------
{
#if   defined (CONFIG_MACOSX)
    return QDir::homePath() + "/Library/Application Support";
#elif defined (CONFIG_MINGW)
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString path = env.value("APPDATA");
    if (path != "")
        return path;
#endif

    // Default would be home itself
    return QDir::toNativeSeparators(QDir::homePath());
}


QString Application::defaultTaoPreferencesFolderPath()
// ----------------------------------------------------------------------------
//    The folder proposed to find user.xl, style.xl, etc...
//    (user preferences for tao application)
// ----------------------------------------------------------------------------
{
#if   defined (CONFIG_LINUX)
    QString tao = "/.tao";
#else // Win, MacOS
    QString tao = "/Tao";
#endif
    return QDir::toNativeSeparators(appDataPath() + tao);
}


QString Application::defaultTaoApplicationFolderPath()
// ----------------------------------------------------------------------------
//    Try to guess the best application folder to use by default
// ----------------------------------------------------------------------------
{
    return QDir::toNativeSeparators(applicationDirPath());
}


QString Application::defaultTaoFontsFolderPath()
// ----------------------------------------------------------------------------
//    Try to guess the best application fonts folder to use by default
// ----------------------------------------------------------------------------
{
    return QDir::toNativeSeparators(applicationDirPath()+"/fonts");
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
        return QDir::toNativeSeparators(path);
    }
#endif // CONFIG_MINGW

    // Trying to find a home sub-directory ending with "pictures"
    QFileInfoList list = QDir::home().entryInfoList(
            QDir::NoDotAndDotDot | QDir::Dirs );
    for (int i = 0; i < list.size(); i++)
    {
        QFileInfo info = list[i];
        if (info.fileName().endsWith("pictures", Qt::CaseInsensitive) )
        {
            return QDir::toNativeSeparators(info.canonicalFilePath());
        }
    }
    // Last default would be home itself
    return QDir::toNativeSeparators(QDir::homePath());
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
                                       "User's Tao documents folder?") +
                                    " (" + path + ")",
                                    QMessageBox::Yes    | QMessageBox::No |
                                    QMessageBox::YesAll | QMessageBox::Cancel);
    if (ret == QMessageBox::Cancel)
        return;
    if (ret == QMessageBox::Yes || ret == QMessageBox::YesAll)
        recursiveDelete(path);

    path = defaultTaoPreferencesFolderPath();
    if (ret != QMessageBox::YesAll)
        ret = QMessageBox::question(NULL, tr("Tao"),
                                    tr("Do you want to delete:\n\n"
                                       "User's Tao prefs/modules folder?") +
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
}


void Application::loadDebugTraceSettings()
// ----------------------------------------------------------------------------
//    Enable any debug traces previously saved by user
// ----------------------------------------------------------------------------
//    Traces can only be enabled by this method, not disabled.
//    This means that any trace activated through the command line can't be
//    cleared by previously saved settings, and will thus be active (as
//    expected).
{
    QStringList enabled;
    enabled = QSettings().value(DEBUG_TRACES_SETTING_NAME).toStringList();
    foreach (QString trace, enabled)
        XL::Traces::enable(+trace);
}


void Application::saveDebugTraceSettings()
// ----------------------------------------------------------------------------
//    Save the names of the debug traces that are currently enabled
// ----------------------------------------------------------------------------
{
    QStringList enabled;

    std::set<std::string>::iterator it;
    std::set<std::string> names = XL::Traces::names();
    for (it = names.begin(); it != names.end(); it++)
        if (XL::Traces::enabled(*it))
            enabled.append(+*it);

    if (!enabled.isEmpty())
        QSettings().setValue(DEBUG_TRACES_SETTING_NAME, enabled);
    else
        QSettings().remove(DEBUG_TRACES_SETTING_NAME);
}


void Application::loadFonts()
// ----------------------------------------------------------------------------
//    Load default fonts
// ----------------------------------------------------------------------------
{
    FontFileManager::loadApplicationFonts();
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
