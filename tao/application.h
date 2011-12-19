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
#include <QApplication>
#include <QDir>
#include <QStringList>
#include <QTranslator>

namespace Tao {

struct Widget;
struct Window;
struct SplashScreen;
struct ModuleManager;
struct GCThread;

enum Vendor {
    ATI = 0,
    NVIDIA = 1,
    INTEL = 2,
    LAST = 3
};

class Application : public QApplication
// ----------------------------------------------------------------------------
//    The main Tao application
// ----------------------------------------------------------------------------
{
public:
    static text vendorsList[LAST];

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
    static QString defaultLicenseFolderPath();

public:
    QStringList    pathCompletions();
    QStringList    urlCompletions();
    void           addPathCompletion(QString path);
    void           addUrlCompletion(QString url);
    bool           processCommandLine();
    Window *       findFirstTaoWindow();
    void           loadUri(QString uri);
    void           blockScreenSaver(bool block);
    void           enableVSync(bool on);

signals:
    void           allWindowsReady();

public slots:
    void           saveDebugTraceSettings();
    void           checkingModule(QString name);
    void           updatingModule(QString name);

protected:
    void           saveSettings();
    void           loadSettings();
    void           loadDebugTraceSettings();
    void           loadFonts();
    void           checkModules();
    virtual bool   event(QEvent *e);

protected slots:
    void           cleanup();
    void           onOpenFinished(bool ok);
#if defined (CONFIG_MACOSX) || defined (CONFIG_LINUX)
    void           simulateUserActivity();
#endif
    void           checkOfflineRendering();
    void           printRenderingProgress(int percent);
    void           onRenderingDone();

protected:
    static QString defaultUserDocumentsFolderPath();
    static bool    createDefaultProjectFolder();

public:
    void         updateSearchPaths(QString path = "");
    static bool  createDefaultTaoPrefFolder();
    static bool  recursiveDelete(QString path);

public:
    bool         hasGLMultisample, hasFBOMultisample;
    bool         hasGLStereoBuffers;
    bool         useShaderLighting;
    int          tex2DMinFilter, tex2DMagFilter;
    Vendor       vendorID;
    uint         maxTextureCoords;
    uint         maxTextureUnits;
    text         GLVendor;
    text         GLRenderer;
    text         GLVersionAvailable;
    text         GLExtensionsAvailable;
    QString      lang;
    GCThread *   gcThread;

private:
    QStringList  pathList;
    QStringList  urlList;
    SplashScreen *splash;
    int          pendingOpen;
    bool         hadWin;
    XL::source_names contextFiles;
    XL::Main *   xlr;
    QString      savedUri;
    bool         screenSaverBlocked;
#if defined (CONFIG_LINUX)
    Display *    xDisplay;
    QString      ssHeartBeatCommand;
#endif
    ModuleManager * moduleManager;
    bool         doNotEnterEventLoop;
    QTranslator  translator, qtTranslator;
    bool         appInitialized;
#if defined (Q_OS_WIN32)
    DDEWidget    dde;
#endif
};

#define TaoApp  ((Application *) qApp)

#define DEBUG_TRACES_SETTING_NAME "DebugTraces"
}

#endif // APPLICATION_H
