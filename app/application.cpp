// *****************************************************************************
// application.cpp                                                 Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2011-2012, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2010-2011, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2015,2017,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2013, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
// (C) 2011-2012, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
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

#include "application.h"
#include "init_cleanup.h"
#include "widget.h"
#include "repository.h"
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
#include "display_driver.h"
#include "gc_thread.h"
#include "text_drawing.h"
#include "license.h"
#include "preferences_pages.h"
#include "update_application.h"
#include "examples_menu.h"
#ifdef CFG_WITH_EULA
#include "eula_dialog.h"
#endif
#ifdef CFG_LICENSE_DOWNLOAD
#include "license_download.h"
#endif
#include "nag_screen.h"
#include "tao_gl.h"
#include <recorder/recorder.h>
#include <stdlib.h>
#include <string>

#include <QString>
#include <QSettings>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QLineEdit>
#include <QDir>
#include <QDebug>
#ifndef CFG_NO_QTWEBKIT
#include <QtWebKit>
#endif
#include <QFileOpenEvent>
#include <QProcessEnvironment>
#include <QStringList>
#include <QDesktopServices>
#include <QGLFramebufferObject>



#if defined(CONFIG_MINGW)
#include <windows.h>
#elif defined(CONFIG_MACOSX)
extern "C" void UpdateSystemActivity(uint8_t);
#define UsrActivity 1
#elif defined(CONFIG_LINUX)
#include <X11/extensions/scrnsaver.h>
#endif
#if defined(Q_OS_WIN32)
#include "dde_widget.h"
#endif


RECORDER(tao_app,       64, "Tao application");
RECORDER(tao_warning,   16, "Tao application warnings");
RECORDER(tao_error,     16, "Tao application errors");
RECORDER(tao_gc,        16, "Garbage collection thread in Tao");
RECORDER(tao_glinfo,    16, "OpenGL information");
RECORDER(fileload,      16, "Loading files in Tao");
RECORDER(paths,         16, "File paths");
RECORDER(settings,      16, "Tao application settings");
RECORDER(uri,           16, "Tao URI handling");
RECORDER(ipc,           16, "Tao inter-process communications");

namespace Tao {

QPixmap * Application::padlockIcon = NULL;

extern const char *GITREV_;

Application::Application(int & argc, char ** argv)
// ----------------------------------------------------------------------------
//    Build the Tao application
// ----------------------------------------------------------------------------
    : QApplication(argc, argv), hasGLMultisample(false),
      hasFBOMultisample(false), hasGLStereoBuffers(false), hasMipmap(false),
      updateApp(NULL), readyToLoad(false), edition(Unknown),
      startDir(QDir::currentPath()),
      splash(NULL), win(NULL), xlr(NULL), screenSaverBlocked(false),
      moduleManager(NULL), peer(NULL)
{
    kstring versionString = qVersion();
    int major = 0, minor = 0, patch = 0;
    if (sscanf(versionString, "%u.%u.%u", &major, &minor, &patch) < 2)
        std::cerr << "qVersion() format error: '" << versionString << "'\n";
    qtVersion = (major << 16) | (minor << 8) | patch;

#if defined(Q_OS_WIN32)
    installDDEWidget();
#endif

    // Initialize application when event loop starts. Some initialization code
    // requires the vent lopp to run properly (splash screen update...)
    QTimer::singleShot(0, this, SLOT(deferredInit()));
    // Do not set the flag before the real main window is show, otherwise a
    // URI download dialog (or any other top-level dialog) may cause the app
    // to exit when closed.
    setQuitOnLastWindowClosed(false);
}


void Application::deferredInit()
// ----------------------------------------------------------------------------
//   Application initialization, run as soon as event loop is started
// ----------------------------------------------------------------------------
{
    // Set some useful parameters for the application
    setApplicationName (APP_NAME);
    setOrganizationName ("Taodyne");
    setOrganizationDomain ("taodyne.com");

    // UI translation
    installTranslators();

    QDir::setCurrent(applicationDirPath());

    // Check command-line options that cause an immediate exit
    // Note: --version is tested earlier, in main()

    QStringList cmdLineArguments = arguments();
    if (cmdLineArguments.contains("--internal-use-only-clean-environment"))
    {
        internalCleanEverythingAsIfTaoWereNeverRun();
        ::exit(0);
    }
#if defined (CFG_WITH_EULA)
    if (cmdLineArguments.contains("--reset-eula"))
    {
        EulaDialog::resetAccepted();
        ::exit(0);
    }
#endif
    if (cmdLineArguments.contains("--glinfo"))
    {
        {
            QGLWidget gl;
            gl.makeCurrent();

            std::cout << "OpenGL vendor:     " << glGetString(GL_VENDOR)
                      << "\nOpenGL renderer:   " << glGetString(GL_RENDERER)
                      << "\nOpenGL version:    " << glGetString(GL_VERSION)
                      << "\nOpenGL extensions: " << glGetString(GL_EXTENSIONS)
                      << "\n";
        }
        ::exit(0);
    }

#ifdef Q_OS_MACX
    // Bug #2300 Tex Gyre Adventor font doesn't show up with LANG=fr_FR.UTF-8
    // Core Text bug?
    setlocale(LC_NUMERIC, "C");
#endif

    if (cmdLineArguments.contains("-norepo") ||
        cmdLineArguments.contains("-nogit"))
    {
        std::cerr << "No Git repository\n";
        RepositoryFactory::no_repo = true;
    }

    // Setup the XL runtime environment
    // Do it soon because debug traces are activated by this
    updateSearchPaths();
    QFileInfo syntax    ("system:xl.syntax");
    QFileInfo stylesheet("system:xl.stylesheet");
    QFileInfo builtins  ("system:builtins.xl");

    // Create the globals (MAIN)
    QVector<char *> args;
    foreach (QString arg, arguments())
        args.append(strdup(arg.toUtf8().constData()));
    XL::path_list bin { +syntax.absoluteDir().absolutePath() };
    XL::path_list lib {  +syntax.absoluteDir().absolutePath() };
    xlr = new Main(args.size(), args.data(), bin, lib,
                   "xl_tao",
                   +syntax.absoluteFilePath(),
                   +stylesheet.absoluteFilePath(),
                   +builtins.absoluteFilePath());

#ifdef CFG_LICENSE_DOWNLOAD
    licDownload = new LicenseDownloadUI;
    fetchLicenses();
#endif

    loadLicenses();

    // Adjust file polling frequency
    FileMonitorThread::pollInterval = Opt::syncInterval;

    // OpenGL checks
    if (!checkGL())
    {
        exit(1);
        return;
    }

    // Create and start garbage collection thread
    record(tao_gc, "Creating GC thread (%+s)",
           Opt::gcThread ? "threaded" : "non-threaded");
    gcThread = new GCThread;
    if (Opt::gcThread)
    {
        RECORD(tao_gc, "Starting separate GC thread");
        gcThread->moveToThread(gcThread);
        gcThread->start();
    }

    // #1891 load padlock icon for license dialog
    QPixmap pm(":/images/tao_padlock.svg");
    padlockIcon = new QPixmap(pm.scaled(64, 64, Qt::IgnoreAspectRatio,
                                        Qt::SmoothTransformation));

    QString studioPro = QString("Tao3D Studio %1").arg(GITREV_);
#ifdef TAO_PLAYER
    QString playerPro = QString("Tao3D Player %1").arg(GITREV_);
    if (Licenses::Has(+playerPro) || Licenses::Has(+studioPro))
        edition = Application::PlayerPro;
    else
        edition = Application::Player;
#else
#ifdef CFG_LIBRE_EDITION
    edition = Application::Libre;
#else
    if (Licenses::Has(+studioPro))
        edition = Application::StudioPro;
    else
        edition = Application::Studio;
#endif // CFG_LIBRE_EDITION
#endif

#ifndef TAO_PLAYER
    // Design edition shows nag screen on startup after 30 days
    if (edition == Studio)
    {
        QString keyName("FirstRun");
        QDateTime now = QDateTime::currentDateTime();
        QDateTime firstRun = QSettings().value(keyName).toDateTime();
        if (!firstRun.isValid())
        {
            QSettings().setValue(keyName, QVariant(now));
        }
        else
        {
            int days = firstRun.daysTo(now);
            if (days > 30)
            {
                QString info;
                info = tr("<p>You have been using Tao3D, "
                          "%1 Edition for %2 days.</p>"
                          "<p>By purchasing a %1 Pro license, you will:"
                          "<ul><li> benefit from additional features "
                          "and support"
                          "<li> help us improve our products</ul>"
                          "Thank you.</p>")
                        .arg(editionStr()).arg(days);
                NagScreen nag;
                nag.setInformativeText(info);
                nag.exec();
            }
        }
    }
#endif

    // Create main window
    record(tao_app, "Creating main window");
    win = new Window (xlr, contextFiles);

    // Update the help menu
    checkExamples();

#if defined (CFG_WITH_EULA)
    // Show End-User License Agreement if not previously accepted for this
    // version
    if (!EulaDialog::previouslyAccepted())
    {
        EulaDialog eula;
#if defined (Q_OS_MACX)
        eula.show();
        eula.raise();
#endif
        if (eula.exec() != QMessageBox::Ok)
        {
            exit(1);
            return;
        }
    }
#endif

    // Possibly run in client/server mode (a single instance of Tao)
    if (arguments().contains("--one-instance"))
        if (singleInstanceClientTalkedToServer())
            return;

    // Show splash screen
    bool showSplash = !(cmdLineArguments.contains("-nosplash") ||
                        cmdLineArguments.contains("-h"));
    if (showSplash)
    {
        record(tao_app, "Creating splash screen");
        splash = new SplashScreen();
        splash->show();
        splash->raise();
    }

    // Now time to install the "persistent" error handler
    install_first_exception_handler();

    // Initialize the graphics just below contents of basics.tbl
    Initialize();
    record(tao_app, "Create main symbol table for XL");
    xlr->context.CreateScope();

    // Load settings
    loadDebugTraceSettings();

#ifndef CFG_NO_QTWEBKIT
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
#endif

    // Configure the proxies for URLs
    QNetworkProxyFactory::setUseSystemConfiguration(true);

#ifndef CFG_NOGIT
    if (!RepositoryFactory::available())
    {
        // Nothing (dialog box already shown by Repository class)
    }
#endif //CFG_NOGIT

    // Create default folder for Tao documents
    // ("Save as..." box will land there)
    createDefaultProjectFolder();

    record(tao_app, "Loading settings");
    loadSettings();

    record(tao_app, "Loading fonts");
    loadFonts();

    // The aboutToQuit signal is the recommended way for cleaning things up
    connect(this, SIGNAL(aboutToQuit()), this, SLOT(cleanup()));

#if defined (CONFIG_LINUX)
    xDisplay = XOpenDisplay(NULL);
    XL_ASSERT(xDisplay);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    ssHeartBeatCommand = env.value("TAO_SS_HEARTBEAT_CMD");
#endif

    XL::MAIN = xlr;
    if (Opt::enableModules)
        checkModules();

    // Check for update now if wanted
    if(GeneralPage::checkForUpdateOnStartup())
    {
        // Application updater
        record(tao_app, "Creating application updater");
        updateApp = new UpdateApplication;

        QDateTime now = QDateTime::currentDateTime();
        QDateTime last = updateApp->lastChecked();
        if (!last.isValid() || last.secsTo(now) > 24*60*60)
            updateApp->check();
    }

    // Record application start time (licensing)
    startTime = Widget::trueCurrentTime();

    // We're ready to go
    processCommandLineFile();

    record(tao_app, "End of deferred init");
}


Application::~Application()
// ----------------------------------------------------------------------------
//   Delete the Tao application
// ----------------------------------------------------------------------------
{
    if (gcThread)
    {
        record(tao_gc, "Stopping GC thread %p", gcThread);
        gcThread->quit();
        gcThread->wait();
        record(tao_gc, "GC thread %p stopped, deleting", gcThread);
        delete gcThread;
    }
    if (updateApp)
    {
        record(tao_app, "Deleting appliation updater %p", updateApp);
        delete updateApp;
    }
}


#if defined(Q_OS_WIN32)
void Application::installDDEWidget()
// ----------------------------------------------------------------------------
//   Create DDE widget (receives file/URI open requests from the OS)
// ----------------------------------------------------------------------------
{
    // DDEWidget handles file/URI open request from the system (double click on
    // .ddd file, click on a Tao URI).
    // It is created ASAP to minimize risk of timeout (Windows7 waits no more
    // than ~2 seconds after launching the process and before showing an error
    // dialog "There was a problem sending a command to the program").
    // Prevent window from being drawn by show()
    dde.setAttribute(Qt::WA_DontShowOnScreen);
    // show() is required or widget won't receive winEvent()
    dde.show();
    // Mark window 'not visible' to Qt, or closing the last top-level window
    // would not cause the application to exit
    dde.hide();
}
#endif


#ifdef CFG_LICENSE_DOWNLOAD
bool Application::fetchLicenses()
// ----------------------------------------------------------------------------
//   Use LicenseDownload object to fetch licenses for any .taokey.src file
// ----------------------------------------------------------------------------
{
    QList<QDir> dirs;

#if QT_VERSION >= 0x050000
    // Thank you Qt5 for this insanity, just in case we have several ~/Desktop
    QStringList desktops = QStandardPaths::standardLocations(
        QStandardPaths::DesktopLocation);
    foreach(QString desktop, desktops)
        dirs << QDir(desktop);
#else
    // Qt4 is a dummy, it only knows about one desktop folder. Stooopid Qt4.
    QString desktop = QDesktopServices::storageLocation(
        QDesktopServices::DesktopLocation);
    dirs << QDir(desktop);
#endif

    dirs << QDir(Application::userLicenseFolderPath())
         << QDir(Application::appLicenseFolderPath());

    // Add paths given on the command line
    QString paths = +XL::MAIN->options.license_dirs;
    foreach (QString path, paths.split(":", QString::SkipEmptyParts))
    {
        QFileInfo info(QDir(TaoApp->startDir), path);
        dirs << QDir(info.absoluteFilePath());
    }

    QFileInfoList files;
    foreach (QDir dir, dirs)
    {
        files << dir.entryInfoList(QStringList("*.taokey.src"), QDir::Files);
    }
    licDownload->processFiles(files);

    return true;
}

#endif


bool Application::loadLicenses()
// ----------------------------------------------------------------------------
//   Load all Tao license files that can be found in standard locations
// ----------------------------------------------------------------------------
{
    QList<QDir> dirs;
    dirs << QDir(Application::userLicenseFolderPath())
         << QDir(Application::appLicenseFolderPath());

#ifndef CFG_NO_LICENSE
    foreach (QDir dir, dirs)
    {
        QFileInfoList licenses = dir.entryInfoList(QStringList("*.taokey"),
                                                   QDir::Files);
        Licenses::AddLicenseFiles(licenses);
    }
#endif // CFG_NO_LICENSE
    return true;
}


bool Application::installTranslators()
// ----------------------------------------------------------------------------
//   Setup translations
// ----------------------------------------------------------------------------
{
    // Load translations, based on (the first that is defined wins):
    //  - Application preferences
    //  - LANG
    //  - Preferred language from system
#if QT_VERSION >= 0x040800
    if (char *env = getenv("LANG"))
        lang = QString::fromLocal8Bit(env).left(2);
    else
        lang = QLocale::system().uiLanguages().value(0).left(2);
#else
    lang = QLocale().name().left(2);
#endif
    if (lang == "C")
        lang = "en";
    lang = QSettings().value("uiLanguage", lang).toString();
    if (translator.load(QString("tao_") + lang, applicationDirPath()))
        installTranslator(&translator);
    if (qtTranslator.load(QString("qt_") + lang, applicationDirPath()))
        installTranslator(&qtTranslator);
    if (qtHelpTranslator.load(QString("qt_help_")+ lang, applicationDirPath()))
        installTranslator(&qtHelpTranslator);

    return true;
}


static text getGLText(GLenum name)
// ----------------------------------------------------------------------------
//   Helper function. Return a GL string value as a QString.
// ----------------------------------------------------------------------------
{
    return +QString::fromLocal8Bit((const char*) glGetString(name));
}


bool Application::checkGL()
// ----------------------------------------------------------------------------
//   Check if GL implementation can be used
// ----------------------------------------------------------------------------
{
    // These variables are static so that we can display info in recorder dump
    static text GLVendor = "?";
    static text GLRenderer = "?";
    static text GLVersion = "?";
    static text GLExtensions = "?";

    {
        // Record before asking for info since LLVM-based rendering pipes
        // often crash during creation of QGLWidget
        record(tao_glinfo, "Reading GL info - QTVersion is %lX", qtVersion);

        // We need a valid GL context to read the information strings
        QGLWidget gl;
        gl.makeCurrent();

        bool validContext = QGLContext::currentContext()->isValid();
        record(tao_glinfo, "Current GL context is %+s",
               validContext ? "valid" : "invalid");
        if (validContext)
        {
            GLVendor     = getGLText(GL_VENDOR);
            GLRenderer   = getGLText(GL_RENDERER);
            GLVersion    = getGLText(GL_VERSION);
            GLExtensions = getGLText(GL_EXTENSIONS);
            record(tao_glinfo, "GL Vendor     %+s", GLVendor.c_str());
            record(tao_glinfo, "GL Renderer   %+s", GLRenderer.c_str());
            record(tao_glinfo, "GL Version    %+s", GLVersion.c_str());
            record(tao_glinfo, "GL Extensions %+s", GLExtensions.c_str());

#if defined(Q_OS_WIN32) && QT_VERSION <= 0x050000
            hasMipmap = (glGenerateMipmap != NULL);
#else
            hasMipmap = true;
#endif
            record(tao_glinfo, "GL %+s mimpap support",
                   hasMipmap ? "has" : "does not have");
        }
    }

    // Basic sanity tests to check if we can actually run
    QGLFormat::OpenGLVersionFlags version = QGLFormat::openGLVersionFlags ();
    QGLFormat::OpenGLVersionFlags required = QGLFormat::OpenGL_Version_2_0;
    record(tao_glinfo,
           "OpenGL version flags %lX, required %lX (2.0)", version, required);
    if (version < required)
    {
        QString msg = tr("This system does not support OpenGL 2.0.\n"
                         "Vendor:   %1\n"
                         "Renderer: %2\n"
                         "Version:  %3")
            .arg(+GLVendor)
            .arg(+GLRenderer)
            .arg(+GLVersion);
        QMessageBox::warning(NULL, tr("Insufficient OpenGL support"), msg);
        return false;
    }

    bool hasFbo = QGLFramebufferObject::hasOpenGLFramebufferObjects();
    record(tao_glinfo, "Frame buffer objects are %+s",
           hasFbo ? "supported" : "not supported");
    if (!QGLFramebufferObject::hasOpenGLFramebufferObjects())
    {
        QMessageBox::warning(NULL,
                             tr("Insufficient graphics capabilities"),
                             tr("This system does not support "
                                "Frame Buffer Objects."));
        return false;
    }

    useShaderLighting = PerformancesPage::perPixelLighting();

    // Workaround QTBUG-55291
    if (qtVersion < 0x050700 || qtVersion >= 0x050702)
    {
        QGLWidget gl((QGLFormat(QGL::StereoBuffers)));
        hasGLStereoBuffers = gl.format().stereo();
        record(tao_glinfo, "GL stereo buffers are %+s",
               hasGLStereoBuffers ? "supported" : "not supported");
    }
    else
    {
        record(tao_warning, "Checking quad buffer disabled - QTBUG-55291");
    }

    if (hasFbo)                 // Better be true here ;-)
    {
        QGLWidget gl((QGLFormat(QGL::SampleBuffers)));
        int samples = gl.format().samples();
        hasGLMultisample = samples > 1;
        record(tao_glinfo, "Multisample support is %+s, %d samples per pixel",
               hasGLMultisample ? "present" : "not present", samples);

        gl.makeCurrent();

        QGLFramebufferObjectFormat format;
        format.setSamples(4);
        QGLFramebufferObject fbo(100, 100, format);
        samples = 0;
        if (fbo.isValid())
        {
            samples = fbo.format().samples();
            hasFBOMultisample = samples > 1;
        }

        record(tao_glinfo,
               "FBO multisample support is %+s, %d samples per pixel",
               hasFBOMultisample ? "present" : "not present", samples);

        // Enable font bitmap cache only if we don't have multisampling
        TextUnit::cacheEnabled = !(hasGLMultisample || hasFBOMultisample);
        record(tao_glinfo, "Font bitmap cache is %+s",
               TextUnit::cacheEnabled ? "enabled" : "disabled");
    }
    if (!hasGLMultisample)
    {
        record(tao_warning, "No multisample - Bitmap text may look jagged");
        ErrorMessageDialog dialog;
        dialog.setWindowTitle(tr("Information"));
        dialog.showMessage(tr("On this system, graphics and text edges may "
                              "look jagged."));
    }

    record(tao_glinfo, "GL driver did not crash so far...");
    return true;
}


void Application::checkModules()
// ----------------------------------------------------------------------------
//   Initialize module manager, check module configuration
// ----------------------------------------------------------------------------
{
    record(tao_app, "Initializing modules");
    moduleManager = ModuleManager::moduleManager();
    connect(moduleManager, SIGNAL(checking(QString)),
            this, SLOT(checkingModule(QString)));
    connect(moduleManager, SIGNAL(updating(QString)),
            this, SLOT(updatingModule(QString)));
    moduleManager->init();
    // Load and initialize only auto-load modules (the ones that do not have an
    // import_name, or have the auto_load property set)
    record(tao_app, "Loading auto-load modules");
    moduleManager->loadAutoLoadModules(XL::MAIN->context);
    record(tao_app, "Modules successfully loaded");
}


void Application::checkingModule(QString name)
// ----------------------------------------------------------------------------
//   Show module names on splash screen as they are being checked
// ----------------------------------------------------------------------------
{
    if (splash)
    {
        QString msg = QString(tr("Checking modules [%1]")).arg(name);
        splash->showMessage(msg);
    }
}


void Application::updatingModule(QString name)
// ----------------------------------------------------------------------------
//   Show module being updated
// ----------------------------------------------------------------------------
{
    if (splash)
    {
        QString msg = QString(tr("Updating modules [%1]")).arg(name);
        splash->showMessage(msg);
    }
}


void Application::checkExamples()
// ----------------------------------------------------------------------------
//   Download examples from
// ----------------------------------------------------------------------------
{
    QUrl url("http://www.taodyne.com/tao-examples-list");
    static ExamplesMenuUpdater examplesMenuUpdater(url);
}


void Application::updateHelpMenu()
// ----------------------------------------------------------------------------
//   Update the examples menu after downloaded from the web
// ----------------------------------------------------------------------------
{
    window()->createHelpMenus();
}


void Application::cleanup()
// ----------------------------------------------------------------------------
//   Perform last-minute cleanup before application exit
// ----------------------------------------------------------------------------
{
    if (updateApp)
        updateApp->cancel();
    // Closing windows will save windows settings (geometry)
    closeAllWindows();
    saveSettings();
    if (screenSaverBlocked)
        blockScreenSaver(false);
    if (moduleManager)
        moduleManager->saveConfig();

    if (isTrialVersion())
    {
        // Gentle reminder that Tao is not free
        NagScreen nag;
        nag.exec();
    }
}


void Application::processCommandLineFile()
// ----------------------------------------------------------------------------
//   Handle command-line files or URIs
// ----------------------------------------------------------------------------
{
    record(tao_app, "Process command line file");
    XL_ASSERT(win);

    // Find file or URI
    QString toOpen = getFileOrUriFromCommandLine();

    // Fetch info for XL files
    QFileInfo user      ("xl:user.xl");
    QFileInfo theme     ("xl:theme.xl");
    record(tao_app, "user.xl absolute path is %s, file %+s",
           +user.absoluteFilePath(),
           user.exists() ? "exists" : "does not exist");
    record(tao_app, "theme.xl absolute path is %s, file %+s",
           +theme.absoluteFilePath(),
           theme.exists() ? "exists" : "does not exist");
    if (user.exists())
        contextFiles.push_back(+user.absoluteFilePath());
    if (theme.exists())
        contextFiles.push_back(+theme.absoluteFilePath());

    if (splash)
        win->splashScreen = splash;

    win->setWindowModified(false);
    if (toOpen.isEmpty())
    {
        toOpen = pendingOpen;
        pendingOpen = "";
    }
    if (toOpen.isEmpty())
    {
        toOpen = win->welcomePath();
        record(tao_app, "No file to open, selected welcome at %s", +toOpen);
    }
    XL_ASSERT(!toOpen.isEmpty());

    // This code makes size() and geometry() valid for the main window
    // its Tao widget, respectively, so that these dimensions may be used
    // during win->open() (primitives window_width/window_height/window_size)
    win->setAttribute(Qt::WA_DontShowOnScreen);
    win->show();
    win->hide();
    win->setAttribute(Qt::WA_DontShowOnScreen, false);

    // #3148
    if (toOpen.contains("://") && !toOpen.startsWith("file://"))
        win->open(applicationDirPath() + "/blank.ddd");

    int st = win->open(toOpen);
    win->markChanged(false);
    if (st == 0)
        win->open(win->welcomePath());
    win->show();
    record(tao_app, "Main window shown, win=%p", win);

    // Now that main window has been shown (if it had to), we can set the
    // "quit on last window closed" flag.
    setQuitOnLastWindowClosed(true);

    if (splash)
    {
        record(tao_app, "Deleting splash screen %p",
               (Tao::SplashScreen *) splash);
        splash->close();
        delete splash;
    }

    checkOfflineRendering();

    readyToLoad = true;
    if (!pendingOpen.isEmpty())
    {
        loadUri(pendingOpen);
        pendingOpen = "";
    }
}


void Application::loadUri(QString uri)
// ----------------------------------------------------------------------------
//   Load a Tao URI into main window
// ----------------------------------------------------------------------------
{
    record(fileload,
           "Load URI %s, %+s", +uri, readyToLoad ? "ready" : "pending");
    if (readyToLoad)
        win->open(uri);
    else
        pendingOpen = uri;
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
    //     2. Call (once) the xdg-screensaver command
    //     3. Periodically call the Xlib API: XResetScreenSaver
    //     4. Periodically execute the command given in the
    //        TAO_SS_HEARTBEAT_CMD environment variable, if defined.
    //   Therefore, all screen savers that support the Xlib or XSS APIs or the
    //   xdg-screensaver script, are automatically disabled. For other screen
    //   savers you will have to define TAO_SS_HEARTBEAT_CMD.
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
        if (win)
        {
            QString xdgss = QString("xdg-screensaver suspend %1").arg(win->winId());
            QProcess::execute(xdgss);
        }
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
        if (win)
        {
            QString xdgss = QString("xdg-screensaver resume %1").arg(win->winId());
            QProcess::execute(xdgss);
        }
#endif
    }
}


void Application::enableVSync(bool on)
// ----------------------------------------------------------------------------
//   Propagate VSync setting to all Tao widgets
// ----------------------------------------------------------------------------
{
    Window *window = NULL;
    foreach (QWidget *widget, QApplication::topLevelWidgets())
    {
        window = dynamic_cast<Window *>(widget);
        if (window)
            window->taoWidget->enableVSync(NULL, on);
    }
}


bool Application::addError(const char *msg)
// ----------------------------------------------------------------------------
//   Send error message to main window
// ----------------------------------------------------------------------------
{
    if (win)
    {
        win->addError(QString::fromLocal8Bit(msg));
        return true;
    }
    return false;
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

static void displayAvailableModes()
// ----------------------------------------------------------------------------
//   List the available modes on the console
// ----------------------------------------------------------------------------
{
    std::cout << "Available rendering modes are:\n";
    QStringList names = DisplayDriver::allDisplayFunctions();
    foreach (QString name, names)
        std::cout << "  " << +name << "\n";
}


bool Application::checkOfflineRendering()
// ----------------------------------------------------------------------------
//   Start offline rendering if command line switch present and we have 1 doc
// ----------------------------------------------------------------------------
{
    Widget      *widget  = win->taoWidget;
    text        display = Opt::displayMode.value;

    if (display != "")
    {
        if (display == "help")
            displayAvailableModes();
        else
            widget->setDisplayMode(XL::xl_false, display);
    }

    QString ropts = +Opt::renderMode.value;
    if (ropts == "")
        return false;

    QStringList parms = ropts.split(",");
    int nparms = parms.size();
    if (nparms < 8 || nparms > 9)
    {
        std::cerr << +tr("-render: Activate off-line rendering:\n"
                         "pagenum,width,height,start-time,duration,"
                         "page-time-offset,fps,folder[,display-mode].");
        return false;
    }

    int idx = 0;
    int page, x, y;
    double start, duration, fps, offset;
    QString folder = "";

    page = parms[idx++].toInt();
    x = parms[idx++].toInt();
    y = parms[idx++].toInt();
    start = parms[idx++].toDouble();
    duration = parms[idx++].toDouble();
    offset = parms[idx++].toDouble();
    fps = parms[idx++].toDouble();
    folder = parms[idx++];
    if (nparms >= 9)
        display = +parms[idx++];

    std::cout << "Starting offline rendering:"
              << " pagenum=" << page << " width=" << x << " height=" << y
              << " start-time=" << start << " duration=" << duration
              << " page-time-offset=" << offset
              << " fps=" << fps << " folder=\"" << +folder << "\""
              << " display-mode=\"" << display << "\"\n";

    connect(widget, SIGNAL(renderFramesProgress(int)),
            this,   SLOT(printRenderingProgress(int)));
    widget->renderFrames(x, y, start, duration,
                         folder, fps, page, offset,
                         +display);
    return true;
}


void Application::printRenderingProgress(int percent)
// ----------------------------------------------------------------------------
//   Print progress when "rendering to files" command line option is active
// ----------------------------------------------------------------------------
{
    std::cout << percent << "%";
    if (percent < 100)
        std::cout << "..." << std::flush;
    else
        std::cout << "\n";
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
            return QDir::toNativeSeparators(info.absoluteFilePath());
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


QString Application::defaultTaoPreferencesFolderPath()
// ----------------------------------------------------------------------------
//    The folder proposed to find user.xl, theme.xl, etc...
// ----------------------------------------------------------------------------
{
#if QT_VERSION >= 0x050000
    // Thank you Qt5 for this insanity, just in case we have several ~/Desktop
    QStringList data = QStandardPaths::standardLocations(
        QStandardPaths::DataLocation);
    if (data.size() >= 1)
        return data[0];
    return "";                  // WARNING?
#else
    return QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#endif
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


QString Application::appLicenseFolderPath()
// ----------------------------------------------------------------------------
//    Licenses packaged with the application
// ----------------------------------------------------------------------------
{
    return QDir::toNativeSeparators(applicationDirPath()+"/licenses");
}


QString Application::userLicenseFolderPath()
// ----------------------------------------------------------------------------
//    User licenses (persist even when Tao is uninstalled/upgraded)
// ----------------------------------------------------------------------------
{
    // Create folder if it does not exist
    QDir dir(defaultTaoPreferencesFolderPath()+"/licenses");
    if (!dir.exists())
        dir.mkpath(dir.absolutePath());
    return QDir::toNativeSeparators(dir.absolutePath());
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
            return QDir::toNativeSeparators(info.absoluteFilePath());
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


double Application::runTime()
// ----------------------------------------------------------------------------
//   The number of seconds since Tao was started
// ----------------------------------------------------------------------------
{
    return (Widget::trueCurrentTime() - TaoApp->startTime);
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
    record(settings, "URL completions saved");
    IFTRACE(settings)
    {
        foreach (QString s, urlList)
            record(settings, "  url %s", +s);
    }

    QSettings().setValue("PathCompletions", QVariant(pathList));
    record(settings, "Path completions saved");
    IFTRACE(settings)
    {
        foreach (QString s, pathList)
            record(settings, "  path %s", +s);
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
        recorder_trace_set(+trace);
}


void Application::saveDebugTraceSettings()
// ----------------------------------------------------------------------------
//    Save the names of the debug traces that are currently enabled
// ----------------------------------------------------------------------------
{
    QStringList enabled;

    for (recorder_info *rec = recorder_list(); rec; rec = rec->next)
        if (rec->trace)
            enabled.append(rec->name);

    if (!enabled.isEmpty())
        QSettings().setValue(DEBUG_TRACES_SETTING_NAME, enabled);
    else
        QSettings().remove(DEBUG_TRACES_SETTING_NAME);
}


void Application::loadFonts()
// ----------------------------------------------------------------------------
//    Load default fonts and fonts from the command line
// ----------------------------------------------------------------------------
{
    FontFileManager::loadApplicationFonts();

    // Process font directories given on command line

    QList<QDir> dirs;
    QStringList args(arguments());
    int size = args.size();
    for (int i = 0; i < size; i++)
    {
        if ((args[i] == "-fontpath") && (i < size-1))
        {
            i++;
            QString path = args[i];
            QFileInfo info(QDir(startDir), path);
            if (info.isDir() && info.isReadable())
                dirs << QDir(info.absoluteFilePath());
            else
                std::cerr << "Invalid directory: '" << +path << "'\n";
        }
    }
    foreach(QDir d, dirs)
        FontFileManager().LoadFonts(d);
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

    return QApplication::event(e);
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


QString Application::getFileOrUriFromCommandLine()
// ----------------------------------------------------------------------------
//    Return first path or URI given on command line
// ----------------------------------------------------------------------------
{
    XL_ASSERT(xlr);

    XL::source_names &names = xlr->file_names;
    XL::source_names::iterator it;
    for (it = names.begin(); it != names.end(); it++)
    {
        QString arg = +(*it);
        // Make relative paths absolute.
        if (!arg.contains("://") && !QFileInfo(arg).isAbsolute())
            arg = startDir + "/" + arg;
        return arg;
    }

    return QString();
}


bool Application::singleInstanceClientTalkedToServer()
// ----------------------------------------------------------------------------
//    Start as a server, or send file/URI to existing server and exit
// ----------------------------------------------------------------------------
{
    // Returns true if file on command line has been processed
    QString id;
    if (char *env = getenv("TAO_INSTANCE"))
        id = QString::fromLocal8Bit(env);

    peer = new QtLocalPeer(this, id);
    if (peer->isClient())
    {
        record(ipc, "Starting as client, instance '%s'", +id);
        QString uri = getFileOrUriFromCommandLine();
        if (!uri.isEmpty())
        {
            record(ipc, "Sending to server: %s", +uri);
            if (!peer->sendMessage(uri, 10000))
            {
                record(ipc, "Failed, proceeding with file open");
                return false;
            }
            record(ipc, "Command sent, exiting");
            exit(0);
            return true;
        }
        record(tao_error, "No file or URI found on command line");
        ::exit(1);
    }
    else
    {
        record(ipc, "Starting as server, instance '%s'", +id);
        connect(peer, SIGNAL(messageReceived(QString)),
                this, SLOT(loadUri(QString)));
    }
    return false;
}
}


void pqs(const QString &qs)
// ----------------------------------------------------------------------------
//   Print a QString for debug purpose
// ----------------------------------------------------------------------------
{
    std::cerr << qs.toUtf8().constData() << "\n";
}
