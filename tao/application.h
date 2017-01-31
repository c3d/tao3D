#ifndef APPLICATION_H
#define APPLICATION_H
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
// This software is licensed under the GNU General Public License v3.
// See file COPYING for details.
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "main.h"
#include <QtGlobal> // for Q_OS_WIN32
#if defined (Q_OS_WIN32)
#include "dde_widget.h"
#endif
#include "qtlocalpeer.h"
#include "texture_cache.h"
#include <QApplication>
#include <QDir>
#include <QStringList>
#include <QTranslator>
#include <QPixmap>
#include <QPointer>
#include <QSharedPointer>

#ifdef CONFIG_LINUX
typedef struct _XDisplay Display;
#endif

namespace Tao {

struct Widget;
struct Window;
struct SplashScreen;
struct ModuleManager;
struct GCThread;
struct UpdateApplication;
#ifndef CFG_NO_LICENSE_DOWNLOAD
struct LicenseDownloadUI;
#endif


#define TaoApp  ((Application *) qApp)

class Application : public QApplication
// ----------------------------------------------------------------------------
//    The main Tao application
// ----------------------------------------------------------------------------
{
public:
    enum TaoEdition { Unknown,Design,DesignPro,Player,PlayerPro,GPL,Other };

public:
    static QPixmap *padlockIcon;

    Q_OBJECT

public:
    Application(int & argc, char ** argv);
    virtual ~Application();

    void           internalCleanEverythingAsIfTaoWereNeverRun();
    static QString defaultProjectFolderPath();
    static QString defaultTaoPreferencesFolderPath();
    static QString defaultTaoApplicationFolderPath();
    static QString defaultTaoFontsFolderPath();
    static QString defaultUserImagesFolderPath();
    static QString appLicenseFolderPath();
    static QString userLicenseFolderPath();
    static double  runTime();
    static bool    isTrialVersion()
    {
        return (TaoApp->edition == Player || TaoApp->edition == Design);
    }
    static QString editionStr()
    {
        switch (TaoApp->edition)
        {
        case Application::Design:
            return "Design";
        case Application::DesignPro:
            return "Design Pro";
        case Application::Player:
            return "Player";
        case Application::PlayerPro:
            return "Player Pro";
        case Application::GPL:
            return "Libre";
        case Application::Other:
            return "Other";
        case Application::Unknown:
            XL_ASSERT(!"Edition not set");
        default:
            XL_ASSERT(!"Unexpected edition value");
            return "Unknown";
        }
    }

public:
    QStringList    pathCompletions();
    QStringList    urlCompletions();
    void           addPathCompletion(QString path);
    void           addUrlCompletion(QString url);
    void           processCommandLineFile();
    void           blockScreenSaver(bool block);
    void           enableVSync(bool on);
    bool           addError(const char *msg);

public slots:
    void           loadUri(QString uri);
    void           saveDebugTraceSettings();
    void           checkingModule(QString name);
    void           updatingModule(QString name);

protected:
    void           saveSettings();
    void           loadSettings();
    void           loadDebugTraceSettings();
    void           loadFonts();
#if defined (Q_OS_WIN32)
    void           installDDEWidget();
#endif
#ifndef CFG_NO_LICENSE_DOWNLOAD
    bool           fetchLicenses();
#endif
    bool           loadLicenses();
    bool           installTranslators();
    bool           checkGL();
    void           checkModules();
    void           checkExamples();
    virtual bool   event(QEvent *e);
    QString        getFileOrUriFromCommandLine();
    bool           singleInstanceClientTalkedToServer();

protected slots:
    void           deferredInit();
    void           cleanup();
#if defined (CONFIG_MACOSX) || defined (CONFIG_LINUX)
    void           simulateUserActivity();
#endif
    bool           checkOfflineRendering();
    void           printRenderingProgress(int percent);

protected:
    static QString defaultUserDocumentsFolderPath();
    static bool    createDefaultProjectFolder();

public:
    Window *       window()             { XL_ASSERT(win); return win; }
    QWidget *      windowWidget()       { return (QWidget*)window(); }
    void           updateSearchPaths(QString path = "");
    void           updateHelpMenu();
    static bool    createDefaultTaoPrefFolder();
    static bool    recursiveDelete(QString path);

public:
    bool               hasGLMultisample, hasFBOMultisample;
    bool               hasGLStereoBuffers, hasMipmap;
    bool               useShaderLighting;
    QString            lang;
    GCThread *         gcThread;
    UpdateApplication* updateApp;
#ifndef CFG_NO_LICENSE_DOWNLOAD
    LicenseDownloadUI * licDownload;
#endif
    bool               readyToLoad;
    QString            pendingOpen;
    TaoEdition         edition;
    QString            startDir;

private:
    QStringList  pathList;
    QStringList  urlList;
    QPointer<SplashScreen>
                 splash;
    Window *     win;
    XL::source_names contextFiles;
    XL::Main *   xlr;
    QString      savedUri;
    bool         screenSaverBlocked;
#if defined (CONFIG_LINUX)
    Display *    xDisplay;
    QString      ssHeartBeatCommand;
#endif
    ModuleManager * moduleManager;
    QTranslator  translator, qtTranslator, qtHelpTranslator;
    double       startTime;
    ulong        qtVersion;
#if defined (Q_OS_WIN32)
    DDEWidget    dde;
#endif
    QtLocalPeer *peer;
};

#define DEBUG_TRACES_SETTING_NAME "DebugTraces"
}

#endif // APPLICATION_H
