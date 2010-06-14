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
QT_END_NAMESPACE

namespace Tao {

struct Widget;


class Window : public QMainWindow
// ----------------------------------------------------------------------------
//    The main window where we display our stuff
// ----------------------------------------------------------------------------
{
    Q_OBJECT

    // Number of items to show in the "File/Open Recent" list
    enum { MaxRecentFiles = 5 };

public:
    Window(XL::Main *xlr, XL::source_names context, XL::SourceFile *sf = NULL);
    ~Window();

    void setHtml(QString txt);
    void addError(QString txt);
    bool openProject(QString path, QString filename, bool confirm = true);
    Repository * repository() { return repo.data(); }
    void switchToFullScreen(bool fs);
    bool showSourceView(bool fs);
    bool loadFileIntoSourceFileView(const QString &fileName, bool box=false);
    QString  currentProjectFolderPath();

    bool isUntitled;
    bool isReadOnly;
    bool loadInProgress;


public:
    QUndoStack       * undoStack;
    XL::source_names   contextFileNames; // Extra context file names

public slots:
    void markChanged(bool changed = true);
    void toggleAnimations();
    void sourceViewBecameVisible(bool visible);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void newFile();
    void open(QString fileName = "");
    bool save();
    bool saveAs();
    bool saveFonts();
    void openRecentFile();
    void clearRecentFileList();
    void cut();
    void copy();
    void paste();
    void onFocusWidgetChanged(QWidget *old, QWidget *now);
    void checkClipboard();

    void setPullUrl();
    void publish();
    void clone();
    void about();
    void documentWasModified();
    void checkFiles();
    void toggleFullScreen();

private:
    void     createActions();
    void     createMenus();
    void     createToolBars();
    void     createStatusBar();
    void     createUndoView();

    void     readSettings();
    void     writeSettings();
    bool     maybeSave();
    bool     needNewWindow();
    bool     loadFile(const QString &fileName, bool openProj = false);
    bool     saveFile(const QString &fileName);
    void     setCurrentFile(const QString &fileName);
    QString  findUnusedUntitledFile();
    QString  strippedName(const QString &fullFileName);
    Window  *findWindow(const QString &fileName);
    void     updateProgram(const QString &filename);
    void     resetTaoMenus();
    QString  fontPathFor(const QString &docPath);
    bool     populateUndoStack();
    void     warnNoRepo();
    void     enableProjectSharingMenus();
    void     updateRecentFileActions();
    void     updateContext(QString docPath);
    void     loadSrcViewStyleSheet();

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

    QTimer            fileCheckTimer;
    QMenu            *fileMenu;
    QMenu            *openRecentMenu;
    QMenu            *editMenu;
    QMenu            *viewMenu;
    QMenu            *helpMenu;
    QToolBar         *fileToolBar;
    QToolBar         *editToolBar;
    QAction          *newAct;
    QAction          *openAct;
    QAction          *saveAct;
    QAction          *saveAsAct;
    QAction          *saveFontsAct;
    QAction          *closeAct;
    QAction          *exitAct;
    QAction          *cutAct;
    QAction          *copyAct;
    QAction          *pasteAct;
    QAction          *setPullUrlAct;
    QAction          *publishAct;
    QAction          *cloneAct;
    QAction          *aboutAct;
    QAction          *aboutQtAct;
    QAction          *fullScreenAct;
    QAction          *viewAnimationsAct;
    QUndoView        *undoView;
    QAction          *undoAction;
    QAction          *redoAction;
    QAction          *recentFileActs[MaxRecentFiles];
    QAction          *clearRecentAct;

public:
    QMenu            *shareMenu;

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
