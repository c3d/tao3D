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
#include "main.h"
#include "tao.h"
#include "git_backend.h"

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

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void newFile();
    void open();
    bool save();
    bool saveAs();
    void about();
    void documentWasModified();

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
    Window *findWindow(const QString &fileName);
    QString tmpDir();         // /tmp
    QString docFileName();    // document1.tao
    QString tmpDocPath();     // /tmp/document1.tao.tmp
    QString repoFileName();   // document1.tao.git
    QString repoPath();       // /tmp/document1.tao.git
    bool saveToGit();
    void loadSource(const QString &fileName);


private:
    XL::Main *        xlRuntime;
    XL::SourceFile *  xlProgram;

    QTextEdit        *textEdit;
    Widget           *taoWidget;
    QString           curFile;
    bool              isUntitled;
    GitRepo           gitRepo;

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

}

#endif // WINDOW_H
