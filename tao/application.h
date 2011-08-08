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

enum Constructor {
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
    static text constructorsList[LAST];

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

public:
    QStringList    pathCompletions();
    QStringList    urlCompletions();
    void           addPathCompletion(QString path);
    void           addUrlCompletion(QString url);
    bool           processCommandLine();
    Window *       findFirstTaoWindow();
    void           loadUri(QString uri);
    void           blockScreenSaver(bool block);

signals:
    void           allWindowsReady();

public slots:
    void           saveDebugTraceSettings();
    void           checkingModule(QString name);

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
    static bool    recursiveDelete(QString path);
    static QString defaultUserDocumentsFolderPath();
    static QString appDataPath();
    static bool    createDefaultProjectFolder();

public:
    void         updateSearchPaths(QString path = "");
    static bool  createDefaultTaoPrefFolder();

public:
    bool         hasGLMultisample, hasFBOMultisample;
    bool         hasGLStereoBuffers;
    Constructor  constructor;
    uint         maxTextureCoords;
    uint         maxTextureUnits;    
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
};

#define TaoApp  ((Application *) qApp)

#define DEBUG_TRACES_SETTING_NAME "DebugTraces"
}

#endif // APPLICATION_H
