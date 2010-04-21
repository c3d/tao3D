// ****************************************************************************
//  window.cpp                                                     Tao project
// ****************************************************************************
//
//   File Description:
//
//     The main Tao output window
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

#include <QtGui>
#include "window.h"
#include "widget.h"
#include "apply_changes.h"
#include "git_backend.h"
#include "application.h"
#include "tao_utf8.h"
#include "pull_from_dialog.h"
#include "publish_to_dialog.h"
#include "undo.h"

#include <iostream>
#include <sstream>
#include <string>

#include <menuinfo.h>
#include <bfs.h>
#include <QList>
#include <QRegExp>

TAO_BEGIN

Window::Window(XL::Main *xlr, XL::SourceFile *sf)
// ----------------------------------------------------------------------------
//    Create a Tao window with default parameters
// ----------------------------------------------------------------------------
    : isUntitled(sf == NULL), xlRuntime(xlr), repo(NULL),
      textEdit(NULL), taoWidget(NULL), curFile(),
      fileCheckTimer(this)
{
    // Define the icon
    setWindowIcon(QIcon(":/images/tao.png"));

    // Create the widgets
    dock = new QDockWidget(tr("Source"));
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    textEdit = new QTextEdit(dock);
    dock->setWidget(textEdit);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    taoWidget = new Widget(this, sf);
    setCentralWidget(taoWidget);

    // Undo/redo management
    undoStack = new QUndoStack();
    createUndoView();

    // Create menus, actions, stuff
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    connect(textEdit->document(), SIGNAL(contentsChanged()),
            this, SLOT(documentWasModified()));

    // Set the window attributes
    setAttribute(Qt::WA_DeleteOnClose);
    readSettings();
    setUnifiedTitleAndToolBarOnMac(true);
    bool loaded = false;
    if (sf)
    {
        QString fileName(+sf->name);
        if (loadFile(fileName, true))
            loaded = true;
    }
    if (!loaded)
        setCurrentFile("");

    // Fire a timer to check if files changed
    fileCheckTimer.start(500);
    connect(&fileCheckTimer, SIGNAL(timeout()), this, SLOT(checkFiles()));
}


void Window::setText(QString txt)
// ----------------------------------------------------------------------------
//   Update the text edit widget with updates we made
// ----------------------------------------------------------------------------
{
    textEdit->document()->setPlainText(txt);
}


void Window::closeEvent(QCloseEvent *event)
// ----------------------------------------------------------------------------
//   Close the window - Save settings
// ----------------------------------------------------------------------------
{
    if (maybeSave())
    {
        writeSettings();
        event->accept();
    }
    else
    {
        event->ignore();
    }
}


void Window::checkFiles()
// ----------------------------------------------------------------------------
//   Check if any of the open files associated with the widget changed
// ----------------------------------------------------------------------------
{
    if (taoWidget)
    {
        XL::SourceFile *prog = taoWidget->xlProgram;
        if (prog && prog->tree.tree)
        {
            import_set done;
            if (ImportedFilesChanged(prog->tree.tree, done, false))
                loadFile(+prog->name);
        }
    }
}


void Window::toggleFullScreen()
// ----------------------------------------------------------------------------
//   Toggle between full-screen and normal mode
// ----------------------------------------------------------------------------
{
    switchToFullScreen(!isFullScreen());
}


void Window::newFile()
// ----------------------------------------------------------------------------
//   Create a new window
// ----------------------------------------------------------------------------
{
    Window *other = new Window(xlRuntime, NULL);
    other->move(x() + 40, y() + 40);
    other->show();
}


void Window::open()
// ----------------------------------------------------------------------------
//   Openg a file
// ----------------------------------------------------------------------------
{
    QString fileName = QFileDialog::getOpenFileName
        (this,
         tr("Open Tao Document"),
         currentProjectFolderPath(),
         tr("Tao documents (*.ddd);;XL programs (*.xl);;"
            "Headers (*.dds *.xs);;All files (*.*)"));

    if (fileName.isEmpty())
        return;

    Window *existing = findWindow(fileName);
    if (existing)
    {
        existing->show();
        existing->raise();
        existing->activateWindow();
        return;
    }

    if (isUntitled &&
        textEdit->document()->isEmpty() &&
        !isWindowModified())
    {
        if (!loadFile(fileName, true))
            return;
    }
    else
    {
        QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
        text fn = canonicalFilePath.toStdString();
        xlRuntime->LoadFile(fn);
        XL::SourceFile &sf = xlRuntime->files[fn];
        Window *other = new Window(xlRuntime, &sf);
        if (other->isUntitled ||
            !other->openProject(QFileInfo(+sf.name).canonicalPath(),
                                QFileInfo(+sf.name).fileName()))
        {
            delete other;
            return;
        }


        other->move(x() + 40, y() + 40);
        other->show();
    }

}


bool Window::save()
// ----------------------------------------------------------------------------
//    Save the current window
// ----------------------------------------------------------------------------
{
    if (isUntitled)
        return saveAs();
    return saveFile(curFile);
}


bool Window::saveAs()
// ----------------------------------------------------------------------------
//   Select file name and save under that name
// ----------------------------------------------------------------------------
{
    // REVISIT: create custom dialog to have the last part of the directory
    // path be the basename of file, as the user types it, while still
    // allowing override of directory name.
    QString fileName =
        QFileDialog::getSaveFileName(this, tr("Save As"), curFile);
    if (fileName.isEmpty())
        return false;
    QString projpath = QFileInfo(fileName).absolutePath();
    QString fileNameOnly = QFileInfo(fileName).fileName();
    if (!openProject(projpath, fileNameOnly, false))
        return false;

    return saveFile(fileName);
}


void Window::setPullUrl()
// ----------------------------------------------------------------------------
//    Prompt user for address of remote repository to pull from
// ----------------------------------------------------------------------------
{
    if (!repo)
    {
        QMessageBox::warning(this, tr("No project"),
                             tr("This feature is not available because the "
                                "current document is not in a project."));
        return;
    }

    PullFromDialog dialog(repo.data());
    if (dialog.exec())
        taoWidget->nextPull = taoWidget->now();
}


void Window::publish()
// ----------------------------------------------------------------------------
//    Prompt user for address of remote repository to publish to
// ----------------------------------------------------------------------------
{
    if (!repo)
    {
        QMessageBox::warning(this, tr("No project"),
                             tr("This feature is not available because the "
                                "current document is not in a project."));
        return;
    }

    PublishToDialog(repo.data()).exec();
}


void Window::about()
// ----------------------------------------------------------------------------
//    About Box
// ----------------------------------------------------------------------------
{
    kstring txt =
        "<b>Tao</b>, an interactive collaboration tool.<br/>"
        "Brought to you by Taodyne SAS:<br/>"
        "<center>"
        "Anne Lempereur<br/>"
        "Catherine Burvelle<br/>"
        "J\303\251r\303\264me Forissier<br/>"
        "Lionel Schaffhauser<br/>"
        "Christophe de Dinechin"
        "</center>";
   QMessageBox::about (this, tr("About Tao"), trUtf8(txt));
}


void Window::documentWasModified()
// ----------------------------------------------------------------------------
//   Record when the document was modified
// ----------------------------------------------------------------------------
{
    setWindowModified(true);
}


void Window::createActions()
// ----------------------------------------------------------------------------
//   Create the various menus and icons on the toolbar
// ----------------------------------------------------------------------------
{
    newAct = new QAction(QIcon(":/images/new.png"), tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

    openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    closeAct = new QAction(tr("&Close"), this);
    closeAct->setShortcut(tr("Ctrl+W"));
    closeAct->setStatusTip(tr("Close this window"));
    connect(closeAct, SIGNAL(triggered()), this, SLOT(close()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

    cutAct = new QAction(QIcon(":/images/cut.png"), tr("Cu&t"), this);
    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(cutAct, SIGNAL(triggered()), textEdit, SLOT(cut()));

    copyAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copyAct, SIGNAL(triggered()), textEdit, SLOT(copy()));

    pasteAct = new QAction(QIcon(":/images/paste.png"), tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(pasteAct, SIGNAL(triggered()), textEdit, SLOT(paste()));

    setPullUrlAct = new QAction(tr("Synchronize..."), this);
    setPullUrlAct->setStatusTip(tr("Set the remote address to \"pull\" from "
                                   "when synchronizing the current "
                                   "document with a remote one"));
    connect(setPullUrlAct, SIGNAL(triggered()), this, SLOT(setPullUrl()));

    publishAct = new QAction(tr("Publish..."), this);
    publishAct->setStatusTip(tr("Publish the current project to "
                                "a specific path or URL"));
    connect(publishAct, SIGNAL(triggered()), this, SLOT(publish()));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    //aboutQtAct = new QAction(tr("About &Qt"), this);
    //aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    //connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    fullScreenAct = new QAction(tr("Full Screen"), this);
    fullScreenAct->setStatusTip(tr("Toggle full screen mode"));
    fullScreenAct->setCheckable(true);
    connect(fullScreenAct, SIGNAL(triggered()), this, SLOT(toggleFullScreen()));

    cutAct->setEnabled(false);
    copyAct->setEnabled(false);
    connect(textEdit, SIGNAL(copyAvailable(bool)),
            cutAct, SLOT(setEnabled(bool)));
    connect(textEdit, SIGNAL(copyAvailable(bool)),
            copyAct, SLOT(setEnabled(bool)));

    undoAction = undoStack->createUndoAction(this, tr("&Undo"));
    undoAction->setShortcuts(QKeySequence::Undo);

    redoAction = undoStack->createRedoAction(this, tr("&Redo"));
    redoAction->setShortcuts(QKeySequence::Redo);
}


void Window::createMenus()
// ----------------------------------------------------------------------------
//    Create all Tao menus
// ----------------------------------------------------------------------------
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->setObjectName(FILE_MENU_NAME);
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(closeAct);
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->setObjectName(EDIT_MENU_NAME);
    editMenu->addAction(undoAction);
    editMenu->addAction(redoAction);
    editMenu->addSeparator();
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);

    shareMenu = menuBar()->addMenu(tr("&Share"));
    shareMenu->setObjectName(SHARE_MENU_NAME);
    shareMenu->addAction(setPullUrlAct);
    shareMenu->addAction(publishAct);

    viewMenu = menuBar()->addMenu(tr("&View"));
//    viewMenu->setObjectName(VIEW_MENU_NAME);
    viewMenu->addAction(dock->toggleViewAction());
    viewMenu->addAction(fullScreenAct);
    viewMenu->addMenu(tr("&Toolbars"))->setObjectName(VIEW_MENU_NAME);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->setObjectName(HELP_MENU_NAME);
    helpMenu->addAction(aboutAct);
}


void Window::createToolBars()
// ----------------------------------------------------------------------------
//   Create the application tool bars
// ----------------------------------------------------------------------------
{
    QMenu *view = findChild<QMenu*>(VIEW_MENU_NAME);
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(newAct);
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);
    if (view)
        view->addAction(fileToolBar->toggleViewAction());

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    editToolBar->addAction(pasteAct);
    if (view)
        view->addAction(editToolBar->toggleViewAction());
}


void Window::createStatusBar()
// ----------------------------------------------------------------------------
//    Create the status bar for the window
// ----------------------------------------------------------------------------
{
    statusBar()->showMessage(tr("Ready"));
}


void Window::createUndoView()
// ----------------------------------------------------------------------------
//    Create the 'undo view' widget
// ----------------------------------------------------------------------------
{
    undoView = NULL;
    IFTRACE(undo)
    {
        undoView = new QUndoView(undoStack);
        undoView->setWindowTitle(tr("Change History"));
        undoView->show();  // REVISIT: add "Change history" to "View" menu?
        undoView->setAttribute(Qt::WA_QuitOnClose, false);
    }
}


void Window::readSettings()
// ----------------------------------------------------------------------------
//   Load the settings from persistent user preference
// ----------------------------------------------------------------------------
{
    QSettings settings;
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    move(pos);
    resize(size);
}


void Window::writeSettings()
// ----------------------------------------------------------------------------
//   Write settings to persistent user preferences
// ----------------------------------------------------------------------------
{
    QSettings settings;
    settings.setValue("pos", pos());
    settings.setValue("size", size());
}


bool Window::maybeSave()
// ----------------------------------------------------------------------------
//   Check if we need to save the document
// ----------------------------------------------------------------------------
{
    if (textEdit->document()->isModified())
    {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning
            (this, tr("Save changes?"),
             tr("The document has been modified.\n"
                "Do you want to save your changes?"),
             QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}


bool Window::loadFile(const QString &fileName, bool openProj)
// ----------------------------------------------------------------------------
//    Load a specific file (and optionally, open project repository)
// ----------------------------------------------------------------------------
{
    if ( openProj &&
        !openProject(QFileInfo(fileName).canonicalPath(),
                     QFileInfo(fileName).fileName()))
        return false;

    if (!loadFileIntoSourceFileView(fileName, openProj))
        return false;

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
    updateProgram(fileName);
    return true;
}

bool Window::loadFileIntoSourceFileView(const QString &fileName, bool box)
// ----------------------------------------------------------------------------
//    Update the source file view with the contents of a specific file
// ----------------------------------------------------------------------------
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        if (box)
            QMessageBox::warning(this, tr("Cannot read file"),
                                 tr("Cannot read file %1:\n%2.")
                                 .arg(fileName)
                                 .arg(file.errorString()));
        textEdit->clear();
        return false;
    }

    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    textEdit->setPlainText(in.readAll());
    QApplication::restoreOverrideCursor();
    return true;
}

void Window::updateProgram(const QString &fileName)
// ----------------------------------------------------------------------------
//   When a file has changed, reload corresponding XL program
// ----------------------------------------------------------------------------
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    text fn = canonicalFilePath.toStdString();
    XL::SourceFile *sf = &xlRuntime->files[fn];

    // Clean menus and reload XL program
    resetTaoMenus();
    if (!sf->tree.tree)
        xlRuntime->LoadFile(fn);

    taoWidget->updateProgram(sf);
    taoWidget->updateGL();
}


bool Window::saveFile(const QString &fileName)
// ----------------------------------------------------------------------------
//   Save a file with a given name
// ----------------------------------------------------------------------------
{
    QFile file(fileName);
    text fn = +fileName;

    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("Error saving file"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    do
    {
        QTextStream out(&file);
        QApplication::setOverrideCursor(Qt::WaitCursor);
        out << textEdit->toPlainText();
        QApplication::restoreOverrideCursor();
    } while (0); // Flush

    setCurrentFile(fileName);
    xlRuntime->LoadFile(fn);
    statusBar()->showMessage(tr("File saved"), 2000);
    updateProgram(fileName);

    // Trigger immediate commit to repository
    XL::SourceFile &sf = xlRuntime->files[fn];

    if (taoWidget->writeIfChanged(sf))
    {
        taoWidget->markChanged("Manual save");
        taoWidget->doCommit();
    }
    return true;
}


void Window::markChanged(bool changed)
// ----------------------------------------------------------------------------
//   Someone else tells us that the window is changed or not
// ----------------------------------------------------------------------------
{
    textEdit->document()->setModified(changed);
    setWindowModified(changed);
}


bool Window::openProject(QString path, QString fileName, bool confirm)
// ----------------------------------------------------------------------------
//   Find and open a project (= SCM repository)
// ----------------------------------------------------------------------------
{
    // If project does not exist and 'confirm' is true, user will be asked to
    // confirm project creation. User is always prompted before re-using a
    // valid repository not currently used by Tao.
    // Returns:
    // - true if project is open succesfully, or
    //        user has chosen to proceed without a project, or
    //        no repository management tool is available;
    // - false if user cancelled.

    if (!Repository::available())
        return true;

    bool created = false;
    QSharedPointer<Repository> repo = Repository::repository(path);
    if (!repo)
    {
        bool docreate = !confirm;
        if (confirm)
        {
            QMessageBox box;
            box.setWindowTitle("No Tao Project");
            box.setText
                    (tr("The file '%1' is not associated with a Tao project.")
                     .arg(fileName));
            box.setInformativeText
                    (tr("Do you want to create a new project in %1, or skip "
                        "and continue without a project (version control and "
                        "sharing will be disabled)?").arg(path));
            box.setIcon(QMessageBox::Question);
            QPushButton *cancel = box.addButton(tr("Cancel"),
                                                QMessageBox::RejectRole);
            QPushButton *skip = box.addButton(tr("Skip"),
                                              QMessageBox::RejectRole);
            QPushButton *create = box.addButton(tr("Create"),
                                                QMessageBox::AcceptRole);
            box.setDefaultButton(create);
            int index = box.exec(); (void) index;
            QAbstractButton *which = box.clickedButton();

            if (which == cancel)
            {
                return false;
            }
            else if (which == create)
            {
                docreate = true;
            }
            else if (which == skip)
            {
                // Continue with repo == NULL
            }
            else
            {
                QMessageBox::question(NULL, tr("Puzzled"),
                                      tr("How did you do that?"),
                                      QMessageBox::No);
            }
        }
        if (docreate)
        {
            repo = Repository::repository(path,true);
            created = (repo != NULL);
        }
    }

    // Select the task branch, either current branch or without _tao_undo
    if (repo && repo->valid())
    {
        text task = repo->branch();
        size_t pos = task.rfind(TAO_UNDO_SUFFIX);
        size_t len = task.length() - (sizeof(TAO_UNDO_SUFFIX) - 1);
        text currentBranch = task;
        bool onUndoBranch = pos != task.npos && pos == len;
        if (onUndoBranch)
        {
            task = task.substr(0, len);
        }
        else if (!created)
        {
            QMessageBox box;
            QString rep = repo->userVisibleName();
            box.setIcon(QMessageBox::Question);
            box.setWindowTitle
                    (tr("Existing %1 repository").arg(rep));
            box.setText
                    (tr("The folder '%1' looks like a valid "
                        "%2 repository, but is not currently used by Tao.")
                     .arg(path).arg(rep));
            box.setInformativeText
                    (tr("This repository appears to not be currently "
                        "used by Tao, because the current branch, "
                        "'%1', is not a Tao working branch. "
                        "Do you want to use this repository (Tao will "
                        "use the '%2' branch and make it the active one) "
                        "or skip and use '%3' without a project (version "
                        "control and sharing will be disabled)?")
                     .arg(+currentBranch)
                     .arg(+currentBranch + TAO_UNDO_SUFFIX)
                     .arg(fileName));
            // REVISIT: this info text is not very well suited to the
            // "Save as..." case.

            QPushButton *cancel = box.addButton(tr("Cancel"),
                                                QMessageBox::RejectRole);
            QPushButton *skip = box.addButton(tr("Skip"),
                                              QMessageBox::NoRole);
            QPushButton *use = box.addButton(tr("Use"),
                                             QMessageBox::YesRole);
            box.setDefaultButton(use);
            int index = box.exec(); (void) index;
            QAbstractButton *which = box.clickedButton();

            if (which == cancel)
            {
                return false;
            }
            else if (which == use)
            {
                // Continue with current repo
            }
            else if (which == skip)
            {
                repo.clear();  // Drop shared pointer reference -> repo == NULL
            }
            else
            {
                QMessageBox::question(NULL, tr("Coin?"),
                                      tr("How did you do that?"),
                                      QMessageBox::Discard);
            }
        }

        if (repo)
        {
            if (!repo->setTask(task))
            {
                QMessageBox::information
                        (NULL, tr("Task selection"),
                         tr("An error occured setting the task:\n%1")
                         .arg(+repo->errors),
                         QMessageBox::Ok);
            }
            else
            {
                this->repo = repo;

                // For undo/redo: widget has to be notified when document
                // is succesfully committed into repository
                connect(repo.data(), SIGNAL(asyncCommitSuccess(QString, QString)),
                        taoWidget,   SLOT(commitSuccess(QString, QString)));
                populateUndoStack();
            }
        }
    }

    return true;
}


void Window::switchToFullScreen(bool fs)
// ----------------------------------------------------------------------------
//   Switch a window to full screen mode, hiding children
// ----------------------------------------------------------------------------
{
    if (fs == isFullScreen())
        return;

    if (fs)
    {
        setUnifiedTitleAndToolBarOnMac(false);
        removeToolBar(fileToolBar);
        removeToolBar(editToolBar);
        showFullScreen();
        taoWidget->showFullScreen();
    }
    else
    {
        showNormal();
        taoWidget->showNormal();
        addToolBar(fileToolBar);
        addToolBar(editToolBar);
        fileToolBar->show();
        editToolBar->show();
        setUnifiedTitleAndToolBarOnMac(true);
    }
}


QString Window::currentProjectFolderPath()
// ----------------------------------------------------------------------------
//    The folder to use in the "Save as..." dialog
// ----------------------------------------------------------------------------
{
    if (repo)
        return repo->path;
    return Application::defaultProjectFolderPath();
}


void Window::resetTaoMenus()
// ----------------------------------------------------------------------------
//   Clean added menus (from menu bar and contextual menus)
// ----------------------------------------------------------------------------
{
    // Removes top menu from the menu bar
//    QRegExp reg("^"+ QString(TOPMENU) +".*", Qt::CaseSensitive);
//    QList<QMenu *> menu_list = menuBar()->findChildren<QMenu *>(reg);
//    QList<QMenu *>::iterator it;
//    for(it = menu_list.begin(); it!=menu_list.end(); ++it)
//    {
//        QMenu *menu = *it;
//        IFTRACE(menus)
//        {
//            std::cout << menu->objectName().toStdString()
//                    << " removed from menu bar \n";
//            std::cout.flush();
//        }
//
//        menuBar()->removeAction(menu->menuAction());
//        delete menu;
//    }

    // Reset currentMenu and currentMenuBar
    taoWidget->currentMenu = NULL;
    taoWidget->currentMenuBar = this->menuBar();
    taoWidget->currentToolBar = NULL;

    // Removes contextual menus
    QRegExp reg("^"+QString(CONTEXT_MENU)+".*", Qt::CaseSensitive);
    QList<QMenu *> menu_list = taoWidget->findChildren<QMenu *>(reg);
    QList<QMenu *>::iterator it;
    for(it = menu_list.begin(); it!=menu_list.end(); ++it)
    {
        QMenu *menu = *it;
        IFTRACE(menus)
        {
            std::cout << menu->objectName().toStdString()
                    << " Contextual menu removed\n";
            std::cout.flush();
        }
        delete menu;
    }

    // Cleanup all menus defined in the current file and all imports
//    CleanMenuInfo cmi;
//    taoWidget->applyAction(cmi);
}



void Window::setCurrentFile(const QString &fileName)
// ----------------------------------------------------------------------------
//   Set the current file name, create one for empty documents
// ----------------------------------------------------------------------------
{
    QString name = fileName;
    isUntitled = name.isEmpty();

    // If the document name doesn't exist, loop until we find an unused one
    // in the current project
    if (isUntitled)
    {
        static int sequenceNumber = 1;
        bool       exists;
        do
        {
            name = QString(tr("%1/Untitled%2.ddd"))
                   .arg(currentProjectFolderPath())
                   .arg(sequenceNumber++);
            QFile file(name);
            exists = file.open(QFile::ReadOnly | QFile::Text);
        } while (exists);
    }

    curFile = QFileInfo(name).absoluteFilePath();

    markChanged(false);
    setWindowFilePath(curFile);
}


QString Window::strippedName(const QString &fullFileName)
// ----------------------------------------------------------------------------
//   Return the short name for a document (no directory name)
// ----------------------------------------------------------------------------
{
    return QFileInfo(fullFileName).fileName();
}


Window *Window::findWindow(const QString &fileName)
// ----------------------------------------------------------------------------
//   Find a window given its file name
// ----------------------------------------------------------------------------
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    foreach (QWidget *widget, qApp->topLevelWidgets())
    {
        Window *mainWin = qobject_cast<Window *>(widget);
        if (mainWin && mainWin->curFile == canonicalFilePath)
            return mainWin;
    }
    return NULL;
}


bool Window::populateUndoStack()
// ----------------------------------------------------------------------------
//    Fill the undo stack with the latest commits from the project
// ----------------------------------------------------------------------------
{
    if (!repo)
        return false;

    QList<Repository::Commit>         commits = repo->history();
    QListIterator<Repository::Commit> it(commits);
    while (it.hasNext())
    {
        Repository::Commit c = it.next();
        undoStack->push(new UndoCommand(repo.data(), c.id, c.msg));
    }
    return true;
}

TAO_END

