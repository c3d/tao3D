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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
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
#include "repository.h"

QT_BEGIN_NAMESPACE
class QAction;
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

    void setHtml(QString txt);
    void addError(QString txt);
    bool openProject(QString path, QString filename, bool confirm = true);
    Repository * repository() { return repo.data(); }
    void switchToFullScreen(bool fs = true);
    bool switchToSlideShow(bool ss = true);
    void setWindowAlwaysOnTop(bool alwaysOnTop);
    bool showSourceView(bool fs);
    bool loadFileIntoSourceFileView(const QString &fileName, bool box=false);
    QString  currentProjectFolderPath();
    bool     needNewWindow();

    bool isUntitled;
    bool isReadOnly;
    bool loadInProgress;


public:
    QUndoStack       * undoStack;
    XL::source_names   contextFileNames; // Extra context file names

public slots:
    void markChanged(bool changed = true);
    void toggleAnimations();
    void toggleStereoscopy();
    bool toggleSlideShow();
    void sourceViewBecameVisible(bool visible);
    int  open(QString fileName = "", bool readOnly = false);
    void openUri();
    void pageSetup();
    void print();
    void removeSplashScreen();
    void deleteAboutSplash();
    void showMessage(QString message)  { showMessage(message, 2000); }
    void setReadOnly(bool ro);

signals:
    void projectUrlChanged(QString url);
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

    void setPullUrl();
    void fetch();
    void push();
    void merge();
    void diff();
    void checkout();
    void selectiveUndo();
    void clone();
    void about();
    void preferences();
    void documentWasModified();
    void checkFiles();
    void toggleFullScreen();
    void clearUndoStack();
    void reloadCurrentFile();
    void onUriGetFailed();
    void onDocReady(QString path);
    void checkDetachedHead();

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
    bool     populateUndoStack();
    void     warnNoRepo();
    void     enableProjectSharingMenus();
    void     updateRecentFileActions();
    void     updateContext(QString docPath);
    void     loadSrcViewStyleSheet();
    void     showMessage(QString message, int timeout);
    void     closeToolWindows();


private:
    XL::Main *        xlRuntime;
    QSharedPointer<Repository> repo;
    QList<int>        appFontIds;

    QTextEdit        *textEdit;
    QTextEdit        *errorMessages;
    // currentProjectFolder : Used if repo is not used.
    QString          currentProjectFolder;
public:
    QDockWidget      *dock;
private:
    QDockWidget      *errorDock;
    Widget           *taoWidget;
    QString           curFile;
    Uri              *uri;
    bool              slideShowMode;

    QTimer            fileCheckTimer;
    QMenu            *fileMenu;
    QMenu            *openRecentMenu;
    QMenu            *editMenu;
    QMenu            *viewMenu;
    QMenu            *helpMenu;
    QToolBar         *fileToolBar;
    QToolBar         *editToolBar;
    QToolBar         *viewToolBar;
    GitToolBar       *gitToolBar;
    QAction          *newDocAct;
    QAction          *newAct;
    QAction          *openAct;
    QAction          *openUriAct;
    QAction          *saveAct;
    QAction          *saveAsAct;
    QAction          *consolidateAct;
    QAction          *saveFontsAct;
    QAction          *printAct;
    QAction          *pageSetupAct;
    QAction          *closeAct;
    QAction          *exitAct;
    QAction          *cutAct;
    QAction          *copyAct;
    QAction          *pasteAct;
    QAction          *setPullUrlAct;
    QAction          *pushAct;
    QAction          *fetchAct;
    QAction          *cloneAct;
    QAction          *mergeAct;
    QAction          *checkoutAct;
    QAction          *selectiveUndoAct;
    QAction          *diffAct;
    QAction          *aboutAct;
    QAction          *preferencesAct;
    QAction          *aboutQtAct;
    QAction          *fullScreenAct;
    QAction          *slideShowAct;
    QAction          *viewAnimationsAct;
    QAction          *viewStereoscopyAct;
    QUndoView        *undoView;
    QAction          *undoAction;
    QAction          *redoAction;
    QAction          *recentFileActs[MaxRecentFiles];
    QAction          *clearRecentAct;
    QAction          *handCursorAct;
    QAction          *resetViewAct;
    struct
    {
      QByteArray         geometry;
      QByteArray         state;
      QList<QToolBar *>  visibleToolBars;
      QMap<ToolWindow *, QByteArray> visibleTools;
    }                 savedState;

public:
    QPrinter         *printer;
    QMenu            *shareMenu;
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
