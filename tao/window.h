#ifndef WINDOW_H
#define WINDOW_H
// ****************************************************************************
//  window.h                                                       Tao project
// ****************************************************************************
//
//   File Description:
//
//    The main Tao window
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
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include <QMainWindow>
#include <QTimer>
#include <QSharedPointer>
#include <QUndoStack>
#include <QUndoView>
#include <QPointer>
#include <QStackedWidget>
#include "main.h"
#include "tao.h"
#ifndef CFG_NOGIT
#include "repository.h"
#include "update_application.h"
#endif

QT_BEGIN_NAMESPACE
class QAction;
class QActionGroup;
class QMenu;
class QTextEdit;
class QSplashScreen;
class QLabel;
QT_END_NAMESPACE

namespace Tao {

struct Widget;
class SplashScreen;
class GitToolBar;
class Uri;
class ToolWindow;
class XLSourceEdit;
class Repository;
class Assistant;


class Window : public QMainWindow
// ----------------------------------------------------------------------------
//    The main window where we display our stuff
// ----------------------------------------------------------------------------
{
    Q_OBJECT

    // Number of items to show in the "File/Open Recent" list
    enum { MaxRecentFiles = 5 };

public:
    Window(XL::Main *xlr, XL::source_names context = XL::source_names(),
           QString sourceFile = "", bool ro = false);
    ~Window();

    void addError(QString txt);
#ifndef CFG_NOGIT
    bool openProject(QString path, QString filename, bool confirm = true);
    Repository * repository() { return repo.data(); }
#else
    Repository * repository() { return NULL; }
#endif
    void switchToFullScreen(bool fs = true);
    bool switchToSlideShow(bool ss = true);
    void setWindowAlwaysOnTop(bool alwaysOnTop);
#ifndef CFG_NOSRCEDIT
    void setHtml(QString txt);
    bool showSourceView(bool fs);
    bool loadFileIntoSourceFileView(const QString &fileName, bool box=false);
#endif
    QString  currentProjectFolderPath();
    bool     needNewWindow();
    bool     setStereo(bool on);
    void     addDisplayModeMenu(QString mode, QString label);
    bool     hasStackedWidget() { return (stackedWidget->count() > 1); }
    QString  welcomePath();

    bool isUntitled;
    bool isReadOnly;
    bool loadInProgress;

public:
    static void        addWidget(void * w);
    static void        removeWidget(void * w);
    static void        setCurrentWidget(void * w);

public:
    QUndoStack       * undoStack;
    XL::source_names   contextFileNames; // Extra context file names

public slots:
    void markChanged(bool changed = true);
    void toggleAnimations();
    bool toggleSlideShow();
    void toggleStereoIdent();
#ifndef CFG_NOSRCEDIT
    void sourceViewBecameVisible(bool visible);
#endif
    int  open(QString fileName = "", bool readOnly = false);
    int  openReadOnly(QString fileName = "")  { return open(fileName, true); }
#ifndef CFG_NONETWORK
    void openUri();
#endif
    void pageSetup();
    void print();
    void showMessage(QString message)  { showMessage(message, 2000); }
    void setReadOnly(bool ro);
    void clearErrors();
    void renderToFile();
    void adjustToScreenResolution(int screen);
    void updateDisplayModeCheckMark(QString mode);
    void closeDocument();
#ifndef TAO_PLAYER
    void signDocument();
#endif

signals:
#ifndef CFG_NOGIT
    void projectUrlChanged(QString url);
#endif
    void projectChanged(Repository *repo);
    void openFinished(bool success);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
#ifndef CFG_NO_NEW_FROM_TEMPLATE
    void newDocument();
#endif
    void newFile();
    void openRecentFile();
    void clearRecentFileList();
#ifndef CFG_NOEDIT
    bool save();
    bool saveAs();
    bool saveFonts();
    void consolidate();
    void cut();
    void copy();
    void paste();
    void onFocusWidgetChanged(QWidget *old, QWidget *now);
    void checkClipboard();
    void updateCopyMenuName(bool hasSelection);
#endif
#ifndef CFG_NOGIT
    void setPullUrl();
    void fetch();
    void push();
    void merge();
    void diff();
    void checkout();
    void selectiveUndo();
    void clone();
    void checkDetachedHead();
    void reloadCurrentFile();
#endif
#ifndef CFG_NONETWORK
    void onDocReady(QString path);
    void onNewTemplateInstalled(QString path);
    void onTemplateUpToDate(QString path);
    void onTemplateUpdated(QString path);
    void onNewModuleInstalled(QString path);
    void onModuleUpToDate(QString path);
    void onModuleUpdated(QString path);
    void onUriGetFailed();
#endif
#if !defined(CFG_NOGIT) && !defined(CFG_NOEDIT)
    void clearUndoStack();
#endif
    void about();
    void update();
    void preferences();
    void licenses();
    void onlineDoc();
    void tutorialsPage();
    void documentWasModified();
    void displayModeTriggered(bool on);
#ifdef CFG_TIMED_FULLSCREEN
    void leaveFullScreen();
    void restartFullScreenTimer();
#endif

private:
    void     createActions();
    void     createMenus();
    void     createToolBars();
    void     createStatusBar();
    void     createUndoView();

    void     readSettings();
    void     writeSettings();
    bool     maybeSave();
    bool     loadFile(const QString &fileName, bool openProj = false);
    bool     saveFile(const QString &fileName);
    bool     isTutorial(const QString &filePath);
    void     setCurrentFile(const QString &fileName);
    QString  findUnusedUntitledFile();
    Window  *findWindow(const QString &fileName);
    bool     updateProgram(const QString &filename);
    void     resetTaoMenus();
#if !defined(CFG_NOGIT) && !defined(CFG_NOEDIT)
    bool     populateUndoStack();
    void     enableProjectSharingMenus();
#endif
#ifndef CFG_NOGIT
    void     warnNoRepo();
#endif
    void     updateRecentFileActions();
    void     updateContext(QString docPath);
    void     showMessage(QString message, int timeout);
    void     showInfoDialog(QString title, QString msg, QString info = "");
    void     closeToolWindows();


private:
    XL::Main *        xlRuntime;
    QSharedPointer<Repository> repo;
    QList<int>        docFontIds;
    // currentProjectFolder : Used if repo is not used.
    QString          currentProjectFolder;
    QTextEdit        *errorMessages;
    QDockWidget      *errorDock;
public:
#ifndef CFG_NOSRCEDIT
    XLSourceEdit     *srcEdit;
    ToolWindow       *src;
#endif
    QStackedWidget   *stackedWidget;
    Widget           *taoWidget;
private:
    QString           curFile;
    Uri              *uri;
#ifndef CFG_NOFULLSCREEN
    bool              slideShowMode;
#ifdef CFG_TIMED_FULLSCREEN
    QTimer            fullScreenTimer;
#endif
#endif
    bool              unifiedTitleAndToolBarOnMac;

#ifndef CFG_NORELOAD
    QTimer            fileCheckTimer;
#endif
    QMenu            *fileMenu;
    QMenu            *openRecentMenu;
#ifndef CFG_NOEDIT
    QMenu            *editMenu;
#endif
    QMenu            *viewMenu;
    QMenu            *displayModeMenu;
    QActionGroup     *displayModes;
    QMap<QString, QAction *> displayModeToAction;
    QToolBar         *fileToolBar;
#ifndef CFG_NOEDIT
    QToolBar         *editToolBar;
#endif
    QToolBar         *viewToolBar;
    GitToolBar       *gitToolBar;
#ifndef CFG_NO_NEW_FROM_TEMPLATE
    QAction          *newDocAct;
#endif
    QAction          *newAct;
    QAction          *openAct;
#ifndef CFG_NOEDIT
    QAction          *saveAct;
    QAction          *saveAsAct;
    QAction          *saveFontsAct;
    QAction          *consolidateAct;
#ifndef TAO_PLAYER
    QAction          *signDocumentAct;
#endif
#endif
    QAction          *renderToFileAct;
    QAction          *printAct;
    QAction          *pageSetupAct;
    QAction          *closeAct;
    QAction          *exitAct;
#ifndef CFG_NONETWORK
    QAction          *openUriAct;
#endif
#if !defined(CFG_NOGIT) && !defined(CFG_NOEDIT)
    QAction          *setPullUrlAct;
    QAction          *pushAct;
    QAction          *fetchAct;
    QAction          *cloneAct;
    QAction          *mergeAct;
    QAction          *checkoutAct;
    QAction          *selectiveUndoAct;
    QAction          *diffAct;
#endif
    QAction          *aboutAct;
    QAction          *updateAct;
    QAction          *preferencesAct;
    QAction          *licensesAct;
    QAction          *onlineDocAct;
    QAction          *tutorialsPageAct;
#ifndef CFG_NOFULLSCREEN
    QAction          *slideShowAct;
#endif
    QAction          *viewAnimationsAct;
    QAction          *stereoIdentAct;
    QUndoView        *undoView;
#ifndef CFG_NOEDIT
    QAction          *cutAct;
    QAction          *copyAct;
    QAction          *pasteAct;
    QAction          *undoAction;
    QAction          *redoAction;
#endif
    QAction          *recentFileActs[MaxRecentFiles];
    QAction          *clearRecentAct;
    QAction          *handCursorAct;
    QAction          *zoomInAct;
    QAction          *zoomOutAct;
    QAction          *resetViewAct;
    struct
    {
      QByteArray         geometry;
      QByteArray         state;
      QList<QToolBar *>  visibleToolBars;
      QMap<ToolWindow *, QByteArray> visibleTools;

      void clear()
      {
          geometry.clear(); state.clear(); visibleToolBars.clear();
          visibleTools.clear();
      }
    }                 savedState;

public:
    QPrinter         *printer;
#if !defined(CFG_NOGIT) && !defined(CFG_NOEDIT)
    QMenu            *shareMenu;
#endif
    QMenu            *helpMenu;
    QPointer<SplashScreen>
                      splashScreen, aboutSplash;

};

// Prefixes for the created menus and sub menus
#define TOPMENU "TAO_TOP_MENU_"
#define SUBMENU "TAO_SUB_MENU_"

// Name of the availables contextual menus
#define         CONTEXT_MENU  "TAO_CONTEXT_MENU"
#define   SHIFT_CONTEXT_MENU  "TAO_CONTEXT_MENU_SHIFT"
#define CONTROL_CONTEXT_MENU  "TAO_CONTEXT_MENU_CONTROL"
#define     ALT_CONTEXT_MENU  "TAO_CONTEXT_MENU_ALT"
#define    META_CONTEXT_MENU  "TAO_CONTEXT_MENU_META"

}

#endif // WINDOW_H
