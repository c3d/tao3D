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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
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

namespace Tao {

struct Widget;
struct Window;
struct SplashScreen;
struct ModuleManager;
struct GCThread;
struct UpdateApplication;


#define TaoApp  ((Application *) qApp)

class Application : public QApplication
// ----------------------------------------------------------------------------
//    The main Tao application
// ----------------------------------------------------------------------------
{
public:
    enum TaoEdition { Unknown, Discovery, Creativity, Impress, Other };
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
    static bool    isDiscovery() { return (TaoApp->edition == Discovery); }
    static bool    isImpress()   { return (TaoApp->edition == Impress); }
    static QString editionStr()
    {
#ifdef TAO_EDITION
        return QString(TAO_EDITION);
#else
        switch (TaoApp->edition)
        {
        case Application::Impress:
            return "Impress";
        case Application::Creativity:
            return "Creativity";
        case Application::Discovery:
            return "Discovery";
        case Application::Other:
            Q_ASSERT(!"Unexpected edition value (TAO_EDITION not empty)");
            return "Other";
        case Application::Unknown:
        default:
            Q_ASSERT(!"Edition not set");
            return "Unknown";
        }

#endif
    }

public:
    QStringList    pathCompletions();
    QStringList    urlCompletions();
    void           addPathCompletion(QString path);
    void           addUrlCompletion(QString url);
    void           processCommandLineFile();
    void           blockScreenSaver(bool block);
    void           enableVSync(bool on);

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
    bool           loadLicenses();
    bool           installTranslators();
    bool           checkGL();
    void           checkModules();
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
    Window *     window() { Q_ASSERT(win); return win; }
    QWidget *    windowWidget() { return (QWidget*)window(); }
    void         updateSearchPaths(QString path = "");
    static bool  createDefaultTaoPrefFolder();
    static bool  recursiveDelete(QString path);

public:
    bool               hasGLMultisample, hasFBOMultisample;
    bool               hasGLStereoBuffers;
    bool               useShaderLighting;
    QString            lang;
    GCThread *         gcThread;
    UpdateApplication* updateApp;
    bool               readyToLoad;
    QString            pendingOpen;
    TaoEdition         edition;

private:
    QStringList  pathList;
    QStringList  urlList;
    QString      startDir;
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
#if defined (Q_OS_WIN32)
    DDEWidget    dde;
#endif
    QtLocalPeer *peer;
    QSharedPointer<TextureCache> textureCache;
};

#define DEBUG_TRACES_SETTING_NAME "DebugTraces"
}

#endif // APPLICATION_H
