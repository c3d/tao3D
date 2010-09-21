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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "main.h"
#include <QApplication>
#include <QDir>
#include <QStringList>


namespace Tao {

struct Widget;
struct Window;
struct SplashScreen;
struct ModuleManager;


class Application : public QApplication
// ----------------------------------------------------------------------------
//    The main Tao application
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    Application(int & argc, char ** argv);
    virtual ~Application() {}

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

public slots:
    void           saveDebugTraceSettings();

protected:
    void           saveSettings();
    void           loadSettings();
    void           loadDebugTraceSettings();
    void           loadFonts();
    void           loadModules();
    virtual bool   event(QEvent *e);

protected slots:
    void           cleanup();
    void           onOpenFinished(bool ok);
#if defined (CONFIG_MACOSX) || defined (CONFIG_LINUX)
    void           simulateUserActivity();
#endif

protected:
    static bool    recursiveDelete(QString path);
    static QString defaultUserDocumentsFolderPath();
    static QString defaultPreferencesFolderPath();
    static bool    createDefaultProjectFolder();

public:
    void         updateSearchPaths(QString path = "");
    static bool  createDefaultTaoPrefFolder();

public:
    bool         hasGLMultisample;

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
};

#define TaoApp  ((Application *) qApp)

#define DEBUG_TRACES_SETTING_NAME "DebugTraces"
}

#endif // APPLICATION_H
