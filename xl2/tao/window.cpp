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
#include "fetch_dialog.h"
#include "merge_dialog.h"
#include "revert_to_dialog.h"
#include "selective_undo_dialog.h"
#include "clone_dialog.h"
#include "branch_selection_toolbar.h"
#include "undo.h"
#include "resource_mgt.h"
#include "splash_screen.h"

#include <iostream>
#include <sstream>
#include <string>

#include <menuinfo.h>
#include <bfs.h>
#include <QList>
#include <QRegExp>

#define TAO_FILESPECS "Tao documents (*.ddd);;XL programs (*.xl);;" \
                      "Headers (*.dds *.xs);;All files (*.*)"

TAO_BEGIN

Window::Window(XL::Main *xlr, XL::source_names context, QString sourceFile,
               bool ro)
// ----------------------------------------------------------------------------
//    Create a Tao window and optionnally open a document
// ----------------------------------------------------------------------------
    : isUntitled(sourceFile.isEmpty()), isReadOnly(ro),
      loadInProgress(false),
      contextFileNames(context), xlRuntime(xlr),
      repo(NULL), textEdit(NULL), errorMessages(NULL),
      dock(NULL), errorDock(NULL),
      taoWidget(NULL), curFile(),
      fileCheckTimer(this), splashScreen(NULL), aboutSplash(NULL)
{
    // Define the icon
    setWindowIcon(QIcon(":/images/tao.png"));

    // Create the text edit widget
    dock = new QDockWidget(tr("Document Source"));
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    textEdit = new QTextEdit(dock);
    dock->setWidget(textEdit);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    connect(dock, SIGNAL(visibilityChanged(bool)),
            this, SLOT(sourceViewBecameVisible(bool)));

    // Create the error reporting widget
    errorDock = new QDockWidget(tr("Errors"));
    errorDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    errorMessages = new QTextEdit(errorDock);
    errorMessages->setReadOnly(true);
    errorDock->setWidget(errorMessages);
    errorDock->hide();
    addDockWidget(Qt::BottomDockWidgetArea, errorDock);

    // Create the main widget for displaying Tao stuff
    taoWidget = new Widget(this);
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

    // Set current document
    if (sourceFile.isEmpty())
    {
        // Create a new file
        sourceFile = findUnusedUntitledFile();
        xlr->NewFile(+sourceFile);
        setCurrentFile(sourceFile);
    }
    else
    {
        // Load existing file
        if (loadFile(sourceFile, !isReadOnly))
            return;
    }

    // Cut/Copy/Paste actions management
    connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)),
            this, SLOT(onFocusWidgetChanged(QWidget*,QWidget*)));
    connect(qApp->clipboard(), SIGNAL(dataChanged()),
            this, SLOT(checkClipboard()));
    checkClipboard();

    // Create the main widget to display Tao stuff
    XL::SourceFile &sf = xlRuntime->files[+sourceFile];
    taoWidget->xlProgram = &sf;

    // Fire a timer to check if files changed
    fileCheckTimer.start(500);
    connect(&fileCheckTimer, SIGNAL(timeout()), this, SLOT(checkFiles()));
}


Window::~Window()
// ----------------------------------------------------------------------------
//   Destroy a document window and free associated resources
// ----------------------------------------------------------------------------
{
    FontFileManager::UnloadEmbeddedFonts(appFontIds);
}


void Window::setHtml(QString txt)
// ----------------------------------------------------------------------------
//   Update the text edit widget with updates we made
// ----------------------------------------------------------------------------
//   We try not to change the scrollbars and cursor positions
{
    QScrollBar * hsb = textEdit->horizontalScrollBar();
    QScrollBar * vsb = textEdit->verticalScrollBar();
    int x = hsb->value();
    int y = vsb->value();
    int pos = textEdit->textCursor().position();

    textEdit->setHtml(txt);

    hsb->setValue(x);
    vsb->setValue(y);
    QTextCursor cursor(textEdit->document());
    int last = textEdit->document()->characterCount();
    if (pos < last)
    {
        cursor.setPosition(pos);
        textEdit->setTextCursor(cursor);
    }
    textEdit->update();
}


void Window::addError(QString txt)
// ----------------------------------------------------------------------------
//   Update the text edit widget with updates we made
// ----------------------------------------------------------------------------
{
    QTextCursor cursor = errorMessages->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(txt + "\n");
    errorDock->show();
    statusBar()->showMessage(txt);
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
        if (!isUntitled && !isReadOnly && prog->tree)
        {
            import_set done;
            if (ImportedFilesChanged(prog->tree, done, false))
                loadFile(+prog->name, !prog->readOnly);
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


void Window::toggleAnimations()
// ----------------------------------------------------------------------------
//   Toggle between full-screen and normal mode
// ----------------------------------------------------------------------------
{
    taoWidget->enableAnimations(!taoWidget->hasAnimations());
}


void Window::toggleStereoscopy()
// ----------------------------------------------------------------------------
//   Toggle between full-screen and normal mode
// ----------------------------------------------------------------------------
{
    taoWidget->enableStereoscopy(!taoWidget->hasStereoscopy());
}


void Window::sourceViewBecameVisible(bool visible)
// ----------------------------------------------------------------------------
//   Source code view is shown or hidden
// ----------------------------------------------------------------------------
{
    if (visible)
    {
        bool modified = textEdit->document()->isModified();
        if (!taoWidget->inError)
            taoWidget->updateProgramSource();
        else
            loadFileIntoSourceFileView(curFile);
        markChanged(modified);
    }
}


void Window::newFile()
// ----------------------------------------------------------------------------
//   Create a new document (either in a new window or in the current one)
// ----------------------------------------------------------------------------
{
    if (!needNewWindow())
    {
        QString fileName = findUnusedUntitledFile();
        XL::SourceFile *sf = xlRuntime->NewFile(+fileName);
        isUntitled = true;
        isReadOnly = false;
        setCurrentFile(fileName);
        setHtml("");
        markChanged(false);
        taoWidget->updateProgram(sf);
        taoWidget->refresh();
    }
    else
    {
        Window *other = new Window(xlRuntime, contextFileNames);
        other->move(x() + 40, y() + 40);
        other->show();
    }
}


void Window::open(QString fileName, bool readOnly)
// ----------------------------------------------------------------------------
//   Open a file or a directory
// ----------------------------------------------------------------------------
{
    bool  isDir = false;
    QString dir = currentProjectFolderPath();
    if (!fileName.isEmpty())
    {
        if (QFileInfo(fileName).isDir())
        {
            isDir = true;
            dir = fileName;
        }
    }
    bool showDialog = fileName.isEmpty() || isDir;
    if (showDialog)
    {
        fileName = QFileDialog::getOpenFileName
                           (this,
                            tr("Open Tao Document"),
                            dir,
                            tr(TAO_FILESPECS));

        if (fileName.isEmpty())
            return;
    }

    Window *existing = findWindow(fileName);
    if (existing)
    {
        existing->show();
        existing->raise();
        existing->activateWindow();
        return;
    }

    if (!needNewWindow())
    {
        if (readOnly)
            isReadOnly = true;
        else
            isReadOnly = !QFileInfo(fileName).isWritable();
        if (!loadFile(fileName, !isReadOnly))
            return;
    }
    else
    {
        Window *other = new Window(xlRuntime, contextFileNames, "",
                                   readOnly);
        other->move(x() + 40, y() + 40);
        other->show();
        other->loadFile(fileName, true);

        if (other->isUntitled)
        {
            other->hide();
            delete other;
            return;
        }
    }
}


void Window::removeSplashScreen()
// ----------------------------------------------------------------------------
//    Do not use the splash screen anymore
// ----------------------------------------------------------------------------
{
    splashScreen = NULL;
}


bool Window::save()
// ----------------------------------------------------------------------------
//    Save the current window
// ----------------------------------------------------------------------------
{
    if (isUntitled || isReadOnly)
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
        QFileDialog::getSaveFileName(this, tr("Save As"), curFile,
                                     tr(TAO_FILESPECS));
    if (fileName.isEmpty())
        return false;
    QString projpath = QFileInfo(fileName).absolutePath();
    QString fileNameOnly = QFileInfo(fileName).fileName();
    if (!openProject(projpath, fileNameOnly, false))
        return false;
    updateContext(projpath);

    return saveFile(fileName);
}


bool Window::saveFonts()
// ----------------------------------------------------------------------------
//    Like saveAs() but also embed currently used fonts into the document
// ----------------------------------------------------------------------------
{
    bool ok;

    ok = saveAs();
    if (!ok)
        return ok;

    struct SOC
    {
         SOC() { QApplication::setOverrideCursor(Qt::WaitCursor); }
        ~SOC() { QApplication::restoreOverrideCursor(); }
    } soc;

    // Get list of font files
    QStringList files;
    files = taoWidget->fontFiles();
    ok = !files.empty();
    if (!ok)
        return ok;

    // Create font directory
    bool fontDirCreated = false;
    QString fontPath;
    fontPath = FontFileManager::FontPathFor(curFile);
    if (!QDir().exists(fontPath))
    {
        ok = QDir().mkdir(fontPath);
        if (!ok)
            return ok;
        fontDirCreated = true;
    }

    // Check if there are unused files in the font directory
    // (Some fonts may have been previously embedded and not used anymore)
    if (!fontDirCreated)
    {
        QDir fontDir(fontPath);
        QFileInfoList contents = fontDir.entryInfoList();
        foreach (QFileInfo f, contents)
        {
            if (f.isFile())
            {
                QString path = f.absoluteFilePath();
                QString name = f.fileName();
                bool found = false;
                foreach (QString s, files)
                {
                    if (s.endsWith(name))
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    IFTRACE(fonts)
                        std::cerr << "Removing  '" << +path << "'\n";
                    if (repo)
                    {
                        QString relPath = QString("fonts/%1").arg(name);
                        repo->remove(+relPath);
                    }
                    else
                    {
                        QDir().remove(path);
                    }
                }
            }
        }
    }

    // Copy font files into font directory
    QString fileName, newFile;
    foreach (QString file, files)
    {
        fileName = QFileInfo(file).fileName();
        newFile = QString("%1/%2").arg(fontPath).arg(fileName);
        IFTRACE(fonts)
        {
            std::cerr << "Copying '" << +file << "' as '" << +newFile << "'\n";
        }
        if (newFile == file)
            continue;
        if (QFile::exists(newFile))
            QFile::remove(newFile);
        ok |= QFile::copy(file, newFile);
    }

    if (repo)
    {
        repo->markChanged("Embed fonts");
        repo->change(+fontPath);
        repo->state = Repository::RS_NotClean;
        repo->commit();
    }

    statusBar()->showMessage(tr("File saved"), 2000);
    return ok;
}


void Window::openRecentFile()
// ----------------------------------------------------------------------------
//    Open a file from the recent file list
// ----------------------------------------------------------------------------
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        open(action->data().toString());
}


void Window::clearRecentFileList()
// ----------------------------------------------------------------------------
//    Clear the list of recently opened files
// ----------------------------------------------------------------------------
{
    QSettings settings;
    settings.setValue("recentFileList", QStringList());
    updateRecentFileActions();
}


void Window::cut()
// ----------------------------------------------------------------------------
//    Cut the current selection into the clipboard
// ----------------------------------------------------------------------------
{
    if (textEdit->hasFocus())
        return textEdit->cut();

    if (taoWidget->hasFocus())
        return taoWidget->cut();
}


void Window::copy()
// ----------------------------------------------------------------------------
//    Copy the current selection to the clipboard
// ----------------------------------------------------------------------------
{
    if (textEdit->hasFocus())
        return textEdit->copy();

    if (taoWidget->hasFocus())
        return taoWidget->copy();
}


void Window::paste()
// ----------------------------------------------------------------------------
//    Paste the clipboard content into the current document or source
// ----------------------------------------------------------------------------
{
    if (textEdit->hasFocus())
        return textEdit->paste();

    if (taoWidget->hasFocus())
        return taoWidget->paste();
}


void Window::onFocusWidgetChanged(QWidget *old, QWidget *now)
// ----------------------------------------------------------------------------
//    Enable or disable copy/cut/paste actions when current widget changes
// ----------------------------------------------------------------------------
{
    (void)old;    // Silence warning

    bool enable;
    if (now == textEdit)
        enable = textEdit->textCursor().hasSelection();
    else if (now == taoWidget)
        enable = taoWidget->hasSelection();
    else
        return;

    copyAct->setEnabled(enable);
    cutAct->setEnabled(enable);

    checkClipboard();
}


void Window::checkClipboard()
// ----------------------------------------------------------------------------
//    Enable/disable Paste action if paste is possible
// ----------------------------------------------------------------------------
{
    QWidget *now = QApplication::focusWidget();
    bool enable;
    if (now == textEdit)
        enable = textEdit->canPaste();
    else if (now == taoWidget)
        enable = taoWidget->canPaste();
    else
        return;

    pasteAct->setEnabled(enable);
}


void Window::warnNoRepo()
// ----------------------------------------------------------------------------
//    Display a warning box
// ----------------------------------------------------------------------------
{
    QMessageBox::warning(this, tr("No project"),
                         tr("This feature is not available because the "
                            "current document is not in a project."));
}

void Window::setPullUrl()
// ----------------------------------------------------------------------------
//    Prompt user for address of remote repository to pull from
// ----------------------------------------------------------------------------
{
    if (!repo)
        warnNoRepo();

    PullFromDialog dialog(repo.data());
    if (dialog.exec())
        taoWidget->nextPull = taoWidget->now();
}


void Window::fetch()
// ----------------------------------------------------------------------------
//    Prompt user for address of remote repository to fetch
// ----------------------------------------------------------------------------
{
    if (!repo)
        return warnNoRepo();

    FetchDialog dialog(repo.data());
    connect(&dialog, SIGNAL(fetched()), branchToolBar, SLOT(refresh()));
    dialog.exec();
}


void Window::publish()
// ----------------------------------------------------------------------------
//    Prompt user for address of remote repository to publish to
// ----------------------------------------------------------------------------
{
    if (!repo)
        return warnNoRepo();

    PublishToDialog(repo.data()).exec();
}


void Window::merge()
// ----------------------------------------------------------------------------
//    Show a "Merge" dialog
// ----------------------------------------------------------------------------
{
    if (!repo)
        return warnNoRepo();

    MergeDialog(repo.data()).exec();
}


void Window::revertTo()
// ----------------------------------------------------------------------------
//    Show a "Revert to" dialog
// ----------------------------------------------------------------------------
{
    if (!repo)
        return warnNoRepo();

    RevertToDialog *dialog = new RevertToDialog(repo.data(), this);
    connect(dialog, SIGNAL(checkedOut(QString)),
            this, SLOT(reloadCurrentFile()));
    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}


void Window::selectiveUndo()
// ----------------------------------------------------------------------------
//    Show a "Selective undo" dialog
// ----------------------------------------------------------------------------
{
    if (!repo)
        return warnNoRepo();

    SelectiveUndoDialog *dialog = new SelectiveUndoDialog(repo.data(), this);
    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}


void Window::clone()
// ----------------------------------------------------------------------------
//    Prompt user for address of remote repository and clone it locally
// ----------------------------------------------------------------------------
{
    CloneDialog dialog(this);
    int ret = dialog.exec();
    if (ret != QDialog::Accepted)
        return;
    QString path = dialog.projectPath;
    if (path.isEmpty())
        return;
    open(path);
}


void Window::about()
// ----------------------------------------------------------------------------
//    About Box
// ----------------------------------------------------------------------------
{
    if (aboutSplash)
        return;
    aboutSplash = new SplashScreen(Qt::WindowStaysOnTopHint);
    connect(aboutSplash, SIGNAL(dismissed()), this, SLOT(deleteAboutSplash()));
    aboutSplash->show();
}


void Window::deleteAboutSplash()
// ----------------------------------------------------------------------------
//    Delete the SplashScreen object allocated by the about() method
// ----------------------------------------------------------------------------
{
    delete aboutSplash;
    aboutSplash = NULL;
}


void Window::showProjectUrl(QString url)
// ----------------------------------------------------------------------------
//    Update the project URL in the status bar
// ----------------------------------------------------------------------------
{
    QString msg = tr("Project: %1").arg(url);
    projectUrl->setText(msg);
}


void Window::documentWasModified()
// ----------------------------------------------------------------------------
//   Record when the document was modified
// ----------------------------------------------------------------------------
{
    // If we're called because we're loading a file, don't set modified state.
    // It is useless, and moreover it triggers an error message on Linux:
    //   "The window title does not contain a '[*]' placeholder"
    if (!loadInProgress)
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
    newAct->setIconVisibleInMenu(false);
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

    openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    openAct->setIconVisibleInMenu(false);
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    saveAct->setIconVisibleInMenu(false);
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    recAct = new QAction(tr("Consolidate"), this);
    recAct->setStatusTip(tr("Make the document self contained"));
    connect(recAct, SIGNAL(triggered()), this, SLOT(consolidate()));

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    saveFontsAct = new QAction(tr("Save with fonts..."), this);
    saveFontsAct->setStatusTip(tr("Save the document and store all the "
                                  "needed fonts alongside the document"));
    connect(saveFontsAct, SIGNAL(triggered()), this, SLOT(saveFonts()));

    clearRecentAct = new QAction(tr("Clear list"), this);
    connect(clearRecentAct, SIGNAL(triggered()),
            this, SLOT(clearRecentFileList()));
    for (int i = 0; i < MaxRecentFiles; ++i)
    {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }

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
    cutAct->setIconVisibleInMenu(false);
    connect(cutAct, SIGNAL(triggered()), this, SLOT(cut()));

    copyAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    copyAct->setIconVisibleInMenu(false);
    connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));

    pasteAct = new QAction(QIcon(":/images/paste.png"), tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    pasteAct->setIconVisibleInMenu(false);
    connect(pasteAct, SIGNAL(triggered()), this, SLOT(paste()));

    setPullUrlAct = new QAction(tr("Synchronize..."), this);
    setPullUrlAct->setStatusTip(tr("Set the remote address to \"pull\" from "
                                   "when synchronizing the current "
                                   "document with a remote one"));
    setPullUrlAct->setEnabled(false);
    connect(setPullUrlAct, SIGNAL(triggered()), this, SLOT(setPullUrl()));

    publishAct = new QAction(tr("Publish..."), this);
    publishAct->setStatusTip(tr("Publish the current project to "
                                "a specific path or URL"));
    publishAct->setEnabled(false);
    connect(publishAct, SIGNAL(triggered()), this, SLOT(publish()));

    fetchAct = new QAction(tr("Fetch..."), this);
    fetchAct->setStatusTip(tr("Fetch data from a remote Tao project "
                              "(path or URL)"));
    fetchAct->setEnabled(false);
    connect(fetchAct, SIGNAL(triggered()), this, SLOT(fetch()));

    cloneAct = new QAction(tr("Clone..."), this);
    cloneAct->setStatusTip(tr("Clone (download) a Tao project "
                              "and make a local copy"));
    connect(cloneAct, SIGNAL(triggered()), this, SLOT(clone()));

    mergeAct = new QAction(tr("Merge..."), this);
    mergeAct->setStatusTip(tr("Apply the changes made in one branch into "
                              "another branch"));
    mergeAct->setEnabled(false);
    connect(mergeAct, SIGNAL(triggered()), this, SLOT(merge()));

    revertToAct = new QAction(tr("Revert to..."), this);
    revertToAct->setStatusTip(tr("Checkout a previous version of the document "
                                 "into a temporary branch"));
    revertToAct->setEnabled(false);
    connect(revertToAct, SIGNAL(triggered()), this, SLOT(revertTo()));

    selectiveUndoAct = new QAction(tr("Selective undo..."), this);
    selectiveUndoAct->setStatusTip(tr("Pick a previous change, revert it and "
                                      "apply it to the current document"));
    selectiveUndoAct->setEnabled(false);
    connect(selectiveUndoAct, SIGNAL(triggered()),
            this, SLOT(selectiveUndo()));

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

    viewAnimationsAct = new QAction(tr("Animations"), this);
    viewAnimationsAct->setStatusTip(tr("Switch animations on or off"));
    viewAnimationsAct->setCheckable(true);
    viewAnimationsAct->setChecked(taoWidget->hasAnimations());
    connect(viewAnimationsAct, SIGNAL(triggered()),
            this, SLOT(toggleAnimations()));

    viewStereoscopyAct = new QAction(tr("Stereoscopy"), this);
    viewStereoscopyAct->setStatusTip(tr("Switch stereoscopy on or off"));
    viewStereoscopyAct->setCheckable(true);
    viewStereoscopyAct->setChecked(taoWidget->hasStereoscopy());
    connect(viewStereoscopyAct, SIGNAL(triggered()),
            this, SLOT(toggleStereoscopy()));

    cutAct->setEnabled(false);
    copyAct->setEnabled(false);
    connect(textEdit, SIGNAL(copyAvailable(bool)),
            cutAct, SLOT(setEnabled(bool)));
    connect(textEdit, SIGNAL(copyAvailable(bool)),
            copyAct, SLOT(setEnabled(bool)));
    connect(taoWidget, SIGNAL(copyAvailable(bool)),
            cutAct, SLOT(setEnabled(bool)));
    connect(taoWidget, SIGNAL(copyAvailable(bool)),
            copyAct, SLOT(setEnabled(bool)));

    undoAction = undoStack->createUndoAction(this, tr("&Undo"));
    undoAction->setShortcuts(QKeySequence::Undo);

    redoAction = undoStack->createRedoAction(this, tr("&Redo"));
    redoAction->setShortcuts(QKeySequence::Redo);

    // Icon copied from:
    // /Developer/Documentation/Qt/html/images/cursor-openhand.png
    handCursorAct = new QAction(QIcon(":/images/cursor-openhand.png"),
                                    tr("Hand cursor"), this);
    handCursorAct->setStatusTip(tr("Select hand cursor to pan around screen"));
    handCursorAct->setCheckable(true);
    connect(handCursorAct, SIGNAL(toggled(bool)), taoWidget,
            SLOT(showHandCursor(bool)));
    // Icon copied from:
    // /opt/local/share/icons/gnome/32x32/actions/view-restore.png
    resetViewAct = new QAction(QIcon(":/images/view-restore.png"),
                                    tr("Restore default view"), this);
    resetViewAct->setStatusTip(tr("Restore default view (zoom and position)"));
    connect(resetViewAct, SIGNAL(triggered()), taoWidget,
            SLOT(resetView()));
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
    openRecentMenu = fileMenu->addMenu(tr("Open Recent"));
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addAction(saveFontsAct);
    fileMenu->addAction(recAct);
    fileMenu->addSeparator();
    fileMenu->addAction(closeAct);
    fileMenu->addAction(exitAct);
    for (int i = 0; i < MaxRecentFiles; ++i)
        openRecentMenu->addAction(recentFileActs[i]);
    openRecentMenu->addSeparator();
    openRecentMenu->addAction(clearRecentAct);
    clearRecentAct->setEnabled(false);
    updateRecentFileActions();

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
    shareMenu->addAction(cloneAct);
    shareMenu->addAction(fetchAct);
    shareMenu->addAction(setPullUrlAct);
    shareMenu->addAction(publishAct);
    shareMenu->addAction(mergeAct);
    shareMenu->addAction(revertToAct);
    shareMenu->addAction(selectiveUndoAct);

    viewMenu = menuBar()->addMenu(tr("&View"));
//    viewMenu->setObjectName(VIEW_MENU_NAME);
    viewMenu->addAction(dock->toggleViewAction());
    viewMenu->addAction(errorDock->toggleViewAction());
    viewMenu->addAction(fullScreenAct);
    viewMenu->addAction(viewAnimationsAct);
    viewMenu->addAction(viewStereoscopyAct);
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

    viewToolBar = addToolBar(tr("View"));
    viewToolBar->addAction(handCursorAct);
    viewToolBar->addAction(resetViewAct);
    if (view)
        view->addAction(viewToolBar->toggleViewAction());

    branchToolBar = new BranchSelectionToolBar(tr("Branch selection"));
    connect(this, SIGNAL(projectChanged(Repository*)),
            branchToolBar, SLOT(setRepository(Repository*)));
    connect(branchToolBar, SIGNAL(checkedOut(QString)),
            this, SLOT(reloadCurrentFile()));
    addToolBar(branchToolBar);
    if (view)
        view->addAction(branchToolBar->toggleViewAction());
}


void Window::createStatusBar()
// ----------------------------------------------------------------------------
//    Create the status bar for the window
// ----------------------------------------------------------------------------
{
    // Set up project URL area
    projectUrl = new QLabel;
    projectUrl->setTextInteractionFlags(Qt::TextSelectableByMouse);
    projectUrl->setToolTip(tr("Shows the URL of the project for the current "
                              "document"));
    statusBar()->addPermanentWidget(projectUrl);
    connect(this, SIGNAL(projectUrlChanged(QString)),
            this, SLOT(showProjectUrl(QString)));
    showProjectUrl(tr("None"));

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


bool Window::needNewWindow()
// ----------------------------------------------------------------------------
//   Check if we need a new window or if we can recycle the old one
// ----------------------------------------------------------------------------
//   We need a new window if:
//   - A document is currently open (i.e., current doc is not 'Untitled')
//     or
//   - Current doc is untitled and has been modified.
{
    return isWindowModified() || !isUntitled;
}


void Window::loadSrcViewStyleSheet()
// ----------------------------------------------------------------------------
//    Load the XL and CSS stylesheet to use for syntax highlighting
// ----------------------------------------------------------------------------
{
    taoWidget->setSourceRenderer();

    QFileInfo info("xl:srcview.css");
    QString path = info.canonicalFilePath();
    IFTRACE2(srcview, paths)
       std::cerr << "Reading syntax highlighting CSS from '" << +path << "'\n";
    QFile file(path);
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream css(&file);
    QString srcViewStyleSheet = css.readAll();
    textEdit->document()->setDefaultStyleSheet(srcViewStyleSheet);
}


void Window::showMessage(QString message, int timeout)
// ----------------------------------------------------------------------------
//    Show a status message, either in the status bar on on the splash screen
// ----------------------------------------------------------------------------
{
    if (splashScreen)
    {
        QColor gray60(102, 102, 102);
        splashScreen->showMessage(message, Qt::AlignBottom, gray60);
    }
    else
    {
        statusBar()->showMessage(message, timeout);
    }
    QApplication::processEvents();
}


bool Window::loadFile(const QString &fileName, bool openProj)
// ----------------------------------------------------------------------------
//    Load a specific file (and optionally, open project repository)
// ----------------------------------------------------------------------------
{
    if (XL::MAIN->options.fileLoad)
            std::cerr << "Opening document: " << +fileName << "\n";

    QString msg = QString(tr("Loading %1 [%2]...")).arg(fileName);

    QString docPath = QFileInfo(fileName).canonicalPath();
    if (openProj &&
        !openProject(docPath,
                     QFileInfo(fileName).fileName()))
        return false;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (openProj && repo)
    {
        showMessage(msg.arg(tr("GC")));
        repo->gc();
    }

    showMessage(msg.arg(tr("Fonts")));
    FontFileManager ffm;
    appFontIds = ffm.LoadEmbeddedFonts(fileName);
    if (!appFontIds.empty())
        taoWidget->glyphs().Clear();
    foreach (QString e, ffm.errors)
        addError(e);
    showMessage(msg.arg(tr("Document")));

    // FIXME: the whole search path stuff is broken when multiple documents
    // are open. There is no way to make "xl:" have a different meaning in
    // two Window instances. And yet it's what we need!
    // As a result, opening a new document from a different directory *will*
    // make the current doc fail to execute if it uses e.g., [load "stuff.xl"]
    // to load a file from the project's directory.
    updateContext(docPath);
    xlRuntime->LoadContextFiles(contextFileNames);
    bool hadError = updateProgram(fileName);

    QApplication::restoreOverrideCursor();

    if (hadError)
    {
        // File not found, or parse error
        showMessage(tr("Load error"), 2000);
        // Try to show source as plain text
        if (!loadFileIntoSourceFileView(fileName, openProj))
            return false;
    }
    else
    if (taoWidget->inError)
    {
        // Runtime error. Already handled by Widget::runtimeError().
    }
    else
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);

        showMessage(msg.arg(tr("Caching code")));
        taoWidget->preloadSelectionCode();

        loadSrcViewStyleSheet();
        loadInProgress = true;
        taoWidget->updateProgramSource();
        loadInProgress = false;
        QApplication::restoreOverrideCursor();
        showMessage(tr("File loaded"), 2000);
    }
    setCurrentFile(fileName);
    isUntitled = false;
    return true;
}


bool Window::loadFileIntoSourceFileView(const QString &fileName, bool box)
// ----------------------------------------------------------------------------
//    Update the source file view with the plain contents of a specific file
// ----------------------------------------------------------------------------
{
    if (isUntitled)
        return true;

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
    loadInProgress = true;
    textEdit->setPlainText(in.readAll());
    loadInProgress = false;
    QApplication::restoreOverrideCursor();
    markChanged(false);
    return true;
}


bool Window::updateProgram(const QString &fileName)
// ----------------------------------------------------------------------------
//   When a file has changed, reload corresponding XL program
// ----------------------------------------------------------------------------
{
    bool hadError = false;
    QFileInfo fileInfo(fileName);
    QString canonicalFilePath = fileInfo.canonicalFilePath();
    text fn = +canonicalFilePath;
    XL::SourceFile *sf = NULL;

    if (!isUntitled)
    {
        sf = &xlRuntime->files[fn];
        // Clean menus and reload XL program
        resetTaoMenus();
        if (!sf->tree)
            if (xlRuntime->LoadFile(fn))
                hadError = true;

        // Check if we can access the file
        if (!fileInfo.isWritable())
            sf->readOnly = true;
    }
    else
    {
        if (xlRuntime->LoadFile(fn))
            return true;
        sf = &xlRuntime->files[fn];
    }

    taoWidget->updateProgram(sf);
    taoWidget->updateGL();
    return hadError;
}


void Window::consolidate()
// ----------------------------------------------------------------------------
//   Menu entry for the resource management activities.
// ----------------------------------------------------------------------------
{
    text fn = +curFile;
    IFTRACE(resources)
    {
        std::cerr << "fn is "<< fn << std::endl;
    }

    ResourceMgt checkFiles(taoWidget);
    xlRuntime->files[fn].tree->Do(checkFiles);
    checkFiles.cleanUpRepo();
    // Reload the program and mark the changes
    taoWidget->reloadProgram();
    taoWidget->markChanged("Related files included in the project");

}

bool Window::saveFile(const QString &fileName)
// ----------------------------------------------------------------------------
//   Save a file with a given name
// ----------------------------------------------------------------------------
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("Error saving file"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    statusBar()->showMessage(tr("Saving..."));
    // FIXME: can't call processEvent here, or the "Save with fonts..."
    // function fails to save all the fonts of a multi-page doc
    // QApplication::processEvents();

    do
    {
        QTextStream out(&file);
        QApplication::setOverrideCursor(Qt::WaitCursor);
        out << textEdit->toPlainText();
        QApplication::restoreOverrideCursor();
    } while (0); // Flush

    text fn = +fileName;

    setCurrentFile(fileName);
    xlRuntime->LoadFile(fn);

    showMessage(tr("File saved"), 2000);
    updateProgram(fileName);
    isReadOnly = false;

    if (repo)
    {
        // Trigger immediate commit to repository
        // FIXME: shouldn't create an empty commit
        XL::SourceFile &sf = xlRuntime->files[fn];
        sf.changed = true;
        taoWidget->markChanged("Manual save");
        taoWidget->writeIfChanged(sf);
        taoWidget->doCommit(true);
        sf.changed = false;
    }
    markChanged(false);
    isUntitled = false;

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


void Window::enableProjectSharingMenus()
// ----------------------------------------------------------------------------
//   Activate the Git-related actions
// ----------------------------------------------------------------------------
{
    setPullUrlAct->setEnabled(true);
    publishAct->setEnabled(true);
    fetchAct->setEnabled(true);
    mergeAct->setEnabled(true);
    revertToAct->setEnabled(true);
    selectiveUndoAct->setEnabled(true);
}


bool Window::openProject(QString path, QString fileName, bool confirm)
// ----------------------------------------------------------------------------
//   Find and open a project (= SCM repository)
// ----------------------------------------------------------------------------
// If project does not exist and 'confirm' is true, user will be asked to
// confirm project creation. User is always prompted before re-using a
// valid repository not currently used by Tao.
// Returns:
// - true if project is open succesfully, or
//        user has chosen to proceed without a project, or
//        no repository management tool is available;
// - false if user cancelled.
{
    repository_ptr oldRepo = repo;
    QString oldUrl;
    if (repo)
        oldUrl = repo->url();

    bool created = false;
    repository_ptr repo = RepositoryFactory::repository(path);
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
            repo = RepositoryFactory::repository(path,
                                                 RepositoryFactory::Create);
            created = (repo != NULL);
        }
    }

    // Select current branch, make sure it can be used
    if (repo && repo->valid())
    {
        text task = repo->branch();
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
            // REVISIT: should slot be in Window rather than Widget?
            connect(repo.data(),SIGNAL(commitSuccess(QString,QString)),
                    taoWidget,  SLOT(commitSuccess(QString, QString)));
            // Also be notified when changes come from remote sync (pull)
            connect(repo.data(), SIGNAL(asyncPullComplete()),
                    this, SLOT(clearUndoStack()));
            // REVISIT
            // Do not populate undo stack with current Git history to avoid
            // making it possible to undo some operations like document
            // creation... (these commits are not easy to identify
            // currently)
            // populateUndoStack();

            enableProjectSharingMenus();

            QString url = repo->url();
            if (url != oldUrl)
                emit projectUrlChanged(url);
            if (repo != oldRepo)
                emit projectChanged(repo.data());   // REVISIT projectUrlChanged
            connect(repo.data(), SIGNAL(branchChanged(QString)),
                    branchToolBar, SLOT(refresh()));
        }
    }

    return true;
}


void Window::updateContext(QString docPath)
// ----------------------------------------------------------------------------
// update the context of the window with the path of the current document,
// and the associated user.xl and theme.xl.
// ----------------------------------------------------------------------------
{
    currentProjectFolder = docPath;
    TaoApp->updateSearchPaths(currentProjectFolder);

    // Fetch info for XL files
    QFileInfo user      ("xl:user.xl");
    QFileInfo theme     ("xl:theme.xl");

    contextFileNames.clear();

    if (user.exists())
        contextFileNames.push_back(+user.canonicalFilePath());
    if (theme.exists())
        contextFileNames.push_back(+theme.canonicalFilePath());

    XL::MAIN->LoadContextFiles(contextFileNames);
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
        removeToolBar(viewToolBar);
        removeToolBar(branchToolBar);
        statusBar()->hide();
        showFullScreen();
        taoWidget->showFullScreen();
    }
    else
    {
        showNormal();
        taoWidget->showNormal();
        statusBar()->show();
        addToolBar(fileToolBar);
        addToolBar(editToolBar);
        addToolBar(viewToolBar);
        addToolBar(branchToolBar);
        fileToolBar->show();
        editToolBar->show();
        viewToolBar->show();
        branchToolBar->show();
        setUnifiedTitleAndToolBarOnMac(true);
    }
}


bool Window::showSourceView(bool show)
// ----------------------------------------------------------------------------
//   Show or hide source view
// ----------------------------------------------------------------------------
{
    bool old = dock->isVisible();
    dock->setVisible(show);
    return old;
}


QString Window::currentProjectFolderPath()
// ----------------------------------------------------------------------------
//    The folder to use in the "Save as..." dialog
// ----------------------------------------------------------------------------
{
    if (repo)
        return repo->path;

    if ( !currentProjectFolder.isEmpty())
        return currentProjectFolder;

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
    QFileInfo fi(name);
    curFile = fi.absoluteFilePath();
    isReadOnly |= !fi.isWritable();

    markChanged(false);
    setWindowFilePath(curFile);

    // Update the recent file list
    if (!isUntitled)
    {
        QSettings settings;
        QStringList files = settings.value("recentFileList").toStringList();
        files.removeAll(fileName);
        files.prepend(fileName);
        while (files.size() > MaxRecentFiles)
            files.removeLast();
        settings.setValue("recentFileList", files);

        foreach (QWidget *widget, QApplication::topLevelWidgets())
        {
            Window *mainWin = qobject_cast<Window *>(widget);
            if (mainWin)
                mainWin->updateRecentFileActions();
        }
    }
}


QString Window::findUnusedUntitledFile()
// ----------------------------------------------------------------------------
//    Find an untitled file that we have not used yet
// ----------------------------------------------------------------------------
{
    static int sequenceNumber = 1;
    while (true)
    {
        QString name = QString(tr("%1/Untitled%2.ddd"))
            .arg(currentProjectFolderPath())
            .arg(sequenceNumber++);
        QFile file(name);
        if (!file.open(QFile::ReadOnly | QFile::Text))
            return name;
    }

    return "";
}


void Window::updateRecentFileActions()
// ----------------------------------------------------------------------------
//   Update the recent files menu (file names are read from settings)
// ----------------------------------------------------------------------------
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();

    files.removeAll("");
    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setToolTip(files[i]);
        recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        recentFileActs[j]->setVisible(false);

    clearRecentAct->setEnabled(numRecentFiles > 0);
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


void Window::clearUndoStack()
// ----------------------------------------------------------------------------
//    Clear the undo stack
// ----------------------------------------------------------------------------
{
    undoStack->clear();
}


void Window::reloadCurrentFile()
// ----------------------------------------------------------------------------
//    Reload the current document when user has switched branches
// ----------------------------------------------------------------------------
{
    loadFile(curFile, false);
}

TAO_END

