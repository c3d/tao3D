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
#include "main.h"
#include "tao.h"

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

public:
    Window(XL::Main *xlr, XL::SourceFile *sf = NULL);
    void updateProgram(XL::Tree *tree);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void newFile();
    void open();
    bool save();
    bool saveAs();
    void about();
    void documentWasModified();
    void checkFiles();

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();

    void readSettings();
    void writeSettings();
    bool maybeSave();
    void loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);
    Window *findWindow(const QString &fileName);
    void updateProgram(const QString &filename);
    void resetTaoMenus(XL::Tree *a_tree);

private:
    XL::Main *        xlRuntime;

    QTextEdit        *textEdit;
    Widget           *taoWidget;
    QString           curFile;
    bool              isUntitled;

    QTimer            fileCheckTimer;
    QMenu            *fileMenu;
    QMenu            *editMenu;
    QMenu            *helpMenu;
    QToolBar         *fileToolBar;
    QToolBar         *editToolBar;
    QAction          *newAct;
    QAction          *openAct;
    QAction          *saveAct;
    QAction          *saveAsAct;
    QAction          *closeAct;
    QAction          *exitAct;
    QAction          *cutAct;
    QAction          *copyAct;
    QAction          *pasteAct;
    QAction          *aboutAct;
    QAction          *aboutQtAct;
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
