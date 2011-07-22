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
#include "main.h"
#include "tao.h"
#ifndef CFG_NOGIT
#include "repository.h"
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

    bool isUntitled;
    bool isReadOnly;
    bool loadInProgress;


public:
    QUndoStack       * undoStack;
    XL::source_names   contextFileNames; // Extra context file names

public slots:
    void markChanged(bool changed = true);
    void toggleAnimations();
    bool toggleSlideShow();
#ifndef CFG_NOSRCEDIT
    void sourceViewBecameVisible(bool visible);
#endif
    int  open(QString fileName = "", bool readOnly = false);
#ifndef CFG_NOGIT
    void openUri();
#endif
    void pageSetup();
    void print();
    void removeSplashScreen();
    void deleteAboutSplash();
    void showMessage(QString message)  { showMessage(message, 2000); }
    void setReadOnly(bool ro);
    void clearErrors();
    void renderToFile();
    void adjustToScreenResolution(int screen);
    void updateDisplayModeCheckMark(QString mode);

signals:
#ifndef CFG_NOGIT
    void projectUrlChanged(QString url);
#endif
    void projectChanged(Repository *repo);
    void openFinished(bool success);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void newDocument();
    void newFile();
    bool save();
    bool saveAs();
    bool saveFonts();
    void consolidate();
    void openRecentFile();
    void clearRecentFileList();
    void cut();
    void copy();
    void paste();
    void onFocusWidgetChanged(QWidget *old, QWidget *now);
    void checkClipboard();
#ifndef CFG_NOGIT
    void setPullUrl();
    void fetch();
    void push();
    void merge();
    void diff();
    void checkout();
    void selectiveUndo();
    void clone();
    void onDocReady(QString path);
    void onUriGetFailed();
    void checkDetachedHead();
    void reloadCurrentFile();
    void clearUndoStack();
#endif
    void about();
    void preferences();
    void onlineDoc();
    void documentWasModified();
    void checkFiles();
    void displayModeTriggered(bool on);

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
#ifndef CFG_NOGIT
    bool     populateUndoStack();
    void     warnNoRepo();
    void     enableProjectSharingMenus();
#endif
    void     updateRecentFileActions();
    void     updateContext(QString docPath);
    void     showMessage(QString message, int timeout);
    void     closeToolWindows();


private:
    XL::Main *        xlRuntime;
    QSharedPointer<Repository> repo;
    QList<int>        appFontIds;
    // currentProjectFolder : Used if repo is not used.
    QString          currentProjectFolder;
    QTextEdit        *errorMessages;
    QDockWidget      *errorDock;
public:
#ifndef CFG_NOSRCEDIT
    XLSourceEdit     *srcEdit;
    ToolWindow       *src;
#endif
    Widget           *taoWidget;
private:
    QString           curFile;
    Uri              *uri;
    bool              slideShowMode;
    bool              unifiedTitleAndToolBarOnMac;

    QTimer            fileCheckTimer;
    QMenu            *fileMenu;
    QMenu            *openRecentMenu;
    QMenu            *editMenu;
    QMenu            *viewMenu;
    QMenu            *displayModeMenu;
    QActionGroup     *displayModes;
    QMap<QString, QAction *> displayModeToAction;
    QToolBar         *fileToolBar;
    QToolBar         *editToolBar;
    QToolBar         *viewToolBar;
    GitToolBar       *gitToolBar;
    QAction          *newDocAct;
    QAction          *newAct;
    QAction          *openAct;
    QAction          *saveAct;
    QAction          *saveAsAct;
    QAction          *consolidateAct;
    QAction          *renderToFileAct;
    QAction          *saveFontsAct;
    QAction          *printAct;
    QAction          *pageSetupAct;
    QAction          *closeAct;
    QAction          *exitAct;
    QAction          *cutAct;
    QAction          *copyAct;
    QAction          *pasteAct;
#ifndef CFG_NOGIT
    QAction          *openUriAct;
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
    QAction          *preferencesAct;
    QAction          *onlineDocAct;
    QAction          *slideShowAct;
    QAction          *viewAnimationsAct;
    QUndoView        *undoView;
    QAction          *undoAction;
    QAction          *redoAction;
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
#ifndef CFG_NOGIT
    QMenu            *shareMenu;
#endif
    QMenu            *helpMenu;
    SplashScreen     *splashScreen;
    SplashScreen     *aboutSplash;
    bool              deleteOnOpenFailed;

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
