#ifndef APPLICATION_H
#define APPLICATION_H
// *****************************************************************************
// application.h                                                   Tao3D project
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
// Tao3D is free software: you can r redistribute it and/or modify
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
#ifdef CFG_LICENSE_DOWNLOAD
struct LicenseDownloadUI;
#endif


#define TaoApp  ((Application *) qApp)

class Application : public QApplication
// ----------------------------------------------------------------------------
//    The main Tao application
// ----------------------------------------------------------------------------
{
public:
    enum TaoEdition
    {
        Unknown,
        Studio,StudioPro,
        Player,PlayerPro,
        Libre
    };

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
        return (TaoApp->edition == Player || TaoApp->edition == Studio);
    }
    static QString editionStr()
    {
        switch (TaoApp->edition)
        {
        case Application::Studio:
            return "Studio (demo)";
        case Application::StudioPro:
            return "Studio";
        case Application::Player:
            return "Player (demo)";
        case Application::PlayerPro:
            return "Player";
        case Application::Libre:
            return "Libre";
        case Application::Unknown:
            XL_ASSERT(!"Edition not set");
            break;
        default:
            XL_ASSERT(!"Unexpected edition value");
            break;
        }
        return "Unknown";
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
#ifdef CFG_LICENSE_DOWNLOAD
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
#ifdef CFG_LICENSE_DOWNLOAD
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
