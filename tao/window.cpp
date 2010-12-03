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
#include "push_dialog.h"
#include "fetch_dialog.h"
#include "merge_dialog.h"
#include "checkout_dialog.h"
#include "selective_undo_dialog.h"
#include "clone_dialog.h"
#include "diff_dialog.h"
#include "git_toolbar.h"
#include "undo.h"
#include "resource_mgt.h"
#include "splash_screen.h"
#include "uri.h"
#include "open_uri_dialog.h"
#include "new_document_wizard.h"
#include "preferences_dialog.h"
#include "tool_window.h"
#include "xl_source_edit.h"

#include <iostream>
#include <sstream>
#include <string>

#include <menuinfo.h>
#include <bfs.h>
#include <QList>
#include <QRegExp>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#define TAO_FILESPECS "Tao documents (*.ddd)"
/*                      ";;XL programs (*.xl)" \
 *                      ";;Headers (*.dds *.xs)"\
 *                      ";;All files (*.*)"
 */

TAO_BEGIN

Window::Window(XL::Main *xlr, XL::source_names context, QString sourceFile,
               bool ro)
// ----------------------------------------------------------------------------
//    Create a Tao window and optionnally open a document
// ----------------------------------------------------------------------------
    : isUntitled(sourceFile.isEmpty()), isReadOnly(ro),
      loadInProgress(false),
      contextFileNames(context), xlRuntime(xlr),
      repo(NULL), srcEdit(NULL), errorMessages(NULL),
      dock(NULL), errorDock(NULL),
      taoWidget(NULL), curFile(), uri(NULL), slideShowMode(false),
      fileCheckTimer(this), splashScreen(NULL), aboutSplash(NULL),
      deleteOnOpenFailed(false)
{
    // Define the icon
    setWindowIcon(QIcon(":/images/tao.png"));

    // Create the text edit widget
    dock = new QDockWidget(tr("Document Source"));
    dock->setObjectName("dock");
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    srcEdit = new XLSourceEdit(dock);
    dock->setWidget(srcEdit);
    dock->hide();
    addDockWidget(Qt::RightDockWidgetArea, dock);
    connect(dock, SIGNAL(visibilityChanged(bool)),
            this, SLOT(sourceViewBecameVisible(bool)));

    // Create the error reporting widget
    errorDock = new QDockWidget(tr("Errors"));
    errorDock->setObjectName("errorDock");
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
    connect(srcEdit->document(), SIGNAL(contentsChanged()),
            this, SLOT(documentWasModified()));
    statusBar()->show();

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
        closeToolWindows();
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
            if (ImportedFilesChanged(done, false))
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
    bool enable = !taoWidget->hasAnimations();
    taoWidget->enableAnimations(enable);
    viewAnimationsAct->setChecked(enable);
}


void Window::toggleStereoscopy()
// ----------------------------------------------------------------------------
//   Toggle between full-screen and normal mode
// ----------------------------------------------------------------------------
{
    bool enable = !taoWidget->hasStereoscopy();
    taoWidget->enableStereoscopy(enable);
    viewStereoscopyAct->setChecked(enable);
}


void Window::sourceViewBecameVisible(bool visible)
// ----------------------------------------------------------------------------
//   Source code view is shown or hidden
// ----------------------------------------------------------------------------
{
    if (visible)
    {
        bool modified = srcEdit->document()->isModified();
        if (!taoWidget->inError)
            taoWidget->updateProgramSource();
        else
            loadFileIntoSourceFileView(curFile);
        markChanged(modified);
    }
}


void Window::newDocument()
// ----------------------------------------------------------------------------
//   Create, save and open a new document from a wizard
// ----------------------------------------------------------------------------
{
    NewDocumentWizard wizard(this);
    wizard.exec();
}


void Window::newFile()
// ----------------------------------------------------------------------------
//   Create a new file (either in a new window or in the current one)
// ----------------------------------------------------------------------------
{
    if (!needNewWindow())
    {
        QString fileName = findUnusedUntitledFile();
        XL::SourceFile *sf = xlRuntime->NewFile(+fileName);
        isUntitled = true;
        isReadOnly = false;
        setCurrentFile(fileName);
        srcEdit->clear();
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


int Window::open(QString fileName, bool readOnly)
// ----------------------------------------------------------------------------
//   Open a file or a directory.
// ----------------------------------------------------------------------------
//   0: error
//   1: success
//   2: don't know yet (asynchronous opening of an URI)
{
    bool  isDir = false;
    QString dir = currentProjectFolderPath();
    if (!fileName.isEmpty())
    {
        bool fileExists = QFileInfo(fileName).exists();
        if (!fileExists && fileName.contains("://"))
        {
            // No local file with this name: try to parse as an URI
            uri = new Uri(fileName);
            if (uri->isValid())
            {
                connect(uri, SIGNAL(progressMessage(QString)),
                        this, SLOT(showMessage(QString)));
                connect(uri, SIGNAL(docReady(QString)),
                        this, SLOT(onDocReady(QString)));
                connect(uri, SIGNAL(getFailed()),
                        this, SLOT(onUriGetFailed()));
                bool ok = uri->get();  // Will emit a signal when done
                if (!ok && deleteOnOpenFailed)
                    deleteLater();
                return 2;
            }
        }
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
            return 0;
    }

    Window *existing = findWindow(fileName);
    if (existing)
    {
        existing->show();
        existing->raise();
        existing->activateWindow();
        return 0;
    }

    if (!QFileInfo(fileName).exists())
    {
        QMessageBox::warning(this->isVisible() ? this : NULL,
                             tr("Error"),
                             tr("%1: File not found").arg(fileName));
        return 0;
    }
    if (!needNewWindow())
    {
        if (readOnly)
            isReadOnly = true;
        else
            isReadOnly = !QFileInfo(fileName).isWritable();

        if (!loadFile(fileName, !isReadOnly))
            return 0;

        taoWidget->refresh();
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
        }
        return 0;
    }
    deleteOnOpenFailed = 0;
    return 1;
}


void Window::print()
// ----------------------------------------------------------------------------
//   Print a document
// ----------------------------------------------------------------------------
{
    // Get print parameters
    QPrintDialog dialog(printer, this);
    if (dialog.exec() != QDialog::Accepted)
        return;
    taoWidget->print(printer);
}


void Window::pageSetup()
// ----------------------------------------------------------------------------
//   Setup parameters for printing a document
// ----------------------------------------------------------------------------
{
    // Get print parameters
    QPageSetupDialog dialog(printer, this);
    if (dialog.exec() != QDialog::Accepted)
        return;
}


void Window::onUriGetFailed()
// ----------------------------------------------------------------------------
//    Called asynchronously when open() failed to open an URI
// ----------------------------------------------------------------------------
{
    if (deleteOnOpenFailed)
        deleteLater();
    emit openFinished(false);
}


void Window::onDocReady(QString path)
// ----------------------------------------------------------------------------
//    Called asynchronously when URI resolution is sucessful
// ----------------------------------------------------------------------------
{
    int st = open(path);
    bool ok = (st == 1);
    if (ok)
        show();
    emit openFinished(ok);
}


void Window::openUri()
// ----------------------------------------------------------------------------
//    Show a dialog box to enter URI and open it
// ----------------------------------------------------------------------------
{
    OpenUriDialog dialog(this);
    int ret = dialog.exec();
    if (ret != QDialog::Accepted)
        return;
    QString uri = dialog.uri;
    if (uri.isEmpty())
        return;
    open(uri);
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
    QString dir;
    if (isUntitled)
        dir = Application::defaultProjectFolderPath();
    else
        dir = curFile;
again:
    QString fileName =
        QFileDialog::getSaveFileName(this, tr("Save As"), dir,
                                     tr(TAO_FILESPECS));
    if (fileName.isEmpty())
        return false;

    // Avoid saving in the Tao folder (and thus creating a repository with all
    // the Tao documents inside...)
    // Note that on MacOSX, in some circumstances, user may involontarily
    // select the Tao folder as a target even though he/she wanted to create
    // a subfolder -- see http://bugreports.qt.nokia.com/browse/QTBUG-13526.
    {
        QDir dir = QDir(QFileInfo(fileName).absoluteDir());
        QDir taoDir = QDir(Application::defaultProjectFolderPath());
        if (dir == taoDir)
        {
            int r = QMessageBox::warning(this, tr("Confirm folder"),
                        tr("Saving in the Tao document folder is not "
                           "recommended. You should create a subfolder "
                           "instead.\n"
                           "Do you want to proceed anyway? Click No to "
                           "choose another location."),
                           QMessageBox::Yes | QMessageBox::No);
            if (r != QMessageBox::Yes)
                goto again;
        }
    }

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
    IFTRACE(settings)
        std::cerr << "Cleared recent file list\n";
    updateRecentFileActions();
}


void Window::cut()
// ----------------------------------------------------------------------------
//    Cut the current selection into the clipboard
// ----------------------------------------------------------------------------
{
    if (srcEdit->hasFocus())
        return srcEdit->cut();

    if (taoWidget->hasFocus())
        return taoWidget->cut();
}


void Window::copy()
// ----------------------------------------------------------------------------
//    Copy the current selection to the clipboard
// ----------------------------------------------------------------------------
{
    if (srcEdit->hasFocus())
        return srcEdit->copy();

    if (taoWidget->hasFocus())
        return taoWidget->copy();

}


void Window::paste()
// ----------------------------------------------------------------------------
//    Paste the clipboard content into the current document or source
// ----------------------------------------------------------------------------
{
    if (srcEdit->hasFocus())
        return srcEdit->paste();

    if (taoWidget->hasFocus())
        return taoWidget->paste();
}


void Window::onFocusWidgetChanged(QWidget */*old*/, QWidget *now)
// ----------------------------------------------------------------------------
//    Enable or disable copy/cut/paste actions when current widget changes
// ----------------------------------------------------------------------------
{
    bool enable;
    if (now == srcEdit)
        enable = srcEdit->textCursor().hasSelection();
    else if (now == taoWidget)
        enable = taoWidget->hasSelection();
    else
        return;

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
    if (now == srcEdit)
        enable = srcEdit->canPaste();
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

    PullFromDialog dialog(repo.data(), this);
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

    FetchDialog dialog(repo.data(), this);
    connect(&dialog, SIGNAL(fetched()), gitToolBar, SLOT(refresh()));
    dialog.exec();
}


void Window::push()
// ----------------------------------------------------------------------------
//    Prompt user for address of remote repository to push to
// ----------------------------------------------------------------------------
{
    if (!repo)
        return warnNoRepo();

    PushDialog(repo.data(), this).exec();
}


void Window::merge()
// ----------------------------------------------------------------------------
//    Show a "Merge" dialog
// ----------------------------------------------------------------------------
{
    if (!repo)
        return warnNoRepo();

    MergeDialog(repo.data(), this).exec();
}


void Window::diff()
// ----------------------------------------------------------------------------
//    Show a "Diff" dialog
// ----------------------------------------------------------------------------
{
    if (!repo)
        return warnNoRepo();

    DiffDialog *dialog = new DiffDialog(repo.data(), this);
    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}


void Window::checkout()
// ----------------------------------------------------------------------------
//    Show a "Checkout" dialog
// ----------------------------------------------------------------------------
{
    if (!repo)
        return warnNoRepo();

    CheckoutDialog *dialog = new CheckoutDialog(repo.data(), this);
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


void Window::preferences()
// ----------------------------------------------------------------------------
//    Show the Preferences dialog
// ----------------------------------------------------------------------------
{
    PreferencesDialog(this).exec();
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
    newDocAct = new QAction(QIcon(":/images/new.png"), tr("New &Document..."), this);
    newDocAct->setShortcut(QKeySequence(Qt::SHIFT + Qt::CTRL + Qt::Key_N));
    newDocAct->setStatusTip(tr("Create a new document"));
    newDocAct->setIconVisibleInMenu(false);
    newDocAct->setObjectName("newDocument");
    connect(newDocAct, SIGNAL(triggered()), this, SLOT(newDocument()));

    newAct = new QAction(QIcon(":/images/new.png"), tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    newAct->setIconVisibleInMenu(false);
    newAct->setObjectName("newFile");
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

    openAct = new QAction(QIcon(":/images/open.png"), tr("&Open File..."),
                          this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    openAct->setIconVisibleInMenu(false);
    openAct->setObjectName("open");
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    openUriAct = new QAction(tr("Open &URI..."), this);
    openUriAct->setStatusTip(tr("Open an URI"));
    openUriAct->setObjectName("openURI");
    connect(openUriAct, SIGNAL(triggered()), this, SLOT(openUri()));

    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    saveAct->setIconVisibleInMenu(false);
    saveAct->setObjectName("save");
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    consolidateAct = new QAction(tr("Consolidate"), this);
    consolidateAct->setStatusTip(tr("Make the document self contained"));
    consolidateAct->setObjectName("consolidate");
    connect(consolidateAct, SIGNAL(triggered()), this, SLOT(consolidate()));

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    saveAsAct->setObjectName("saveAs");
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    saveFontsAct = new QAction(tr("Save with fonts..."), this);
    saveFontsAct->setStatusTip(tr("Save the document with all required fonts"));
    saveFontsAct->setObjectName("saveFonts");
    connect(saveFontsAct, SIGNAL(triggered()), this, SLOT(saveFonts()));

    printAct = new QAction(tr("&Print..."), this);
    printAct->setStatusTip(tr("Print the document"));
    printAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
    connect(printAct, SIGNAL(triggered()), this, SLOT(print()));

    pageSetupAct = new QAction(tr("Page setup..."), this);
    pageSetupAct->setStatusTip(tr("Setup page parameters for this document"));
    connect(pageSetupAct, SIGNAL(triggered()), this, SLOT(pageSetup()));

    printer = new QPrinter;
    printer->setOrientation(QPrinter::Landscape);

    clearRecentAct = new QAction(tr("Clear list"), this);
    clearRecentAct->setObjectName("clearRecent");
    connect(clearRecentAct, SIGNAL(triggered()),
            this, SLOT(clearRecentFileList()));
    for (int i = 0; i < MaxRecentFiles; ++i)
    {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        recentFileActs[i]->setObjectName(QString("recentFile%1").arg(i));
        connect(recentFileActs[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }

    closeAct = new QAction(tr("&Close"), this);
    closeAct->setShortcut(tr("Ctrl+W"));
    closeAct->setStatusTip(tr("Close this window"));
    closeAct->setObjectName("close");
    connect(closeAct, SIGNAL(triggered()), this, SLOT(close()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    exitAct->setObjectName("exit");
    connect(exitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

    cutAct = new QAction(QIcon(":/images/cut.png"), tr("Cu&t"), this);
    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    cutAct->setIconVisibleInMenu(false);
    cutAct->setObjectName("cut");
    connect(cutAct, SIGNAL(triggered()), this, SLOT(cut()));

    copyAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    copyAct->setIconVisibleInMenu(false);
    copyAct->setObjectName("copy");
    connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));

    pasteAct = new QAction(QIcon(":/images/paste.png"), tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    pasteAct->setIconVisibleInMenu(false);
    pasteAct->setObjectName("paste");
    connect(pasteAct, SIGNAL(triggered()), this, SLOT(paste()));

    setPullUrlAct = new QAction(tr("Synchronize..."), this);
    setPullUrlAct->setStatusTip(tr("Set the remote address to \"pull\" from "
                                   "when synchronizing the current "
                                   "document with a remote one"));
    setPullUrlAct->setEnabled(false);
    setPullUrlAct->setObjectName("pullURL");
    connect(setPullUrlAct, SIGNAL(triggered()), this, SLOT(setPullUrl()));

    pushAct = new QAction(tr("Push..."), this);
    pushAct->setStatusTip(tr("Push the current project to "
                                "a specific path or URL"));
    pushAct->setEnabled(false);
    pushAct->setObjectName("pushURL");
    connect(pushAct, SIGNAL(triggered()), this, SLOT(push()));

    fetchAct = new QAction(tr("Fetch..."), this);
    fetchAct->setStatusTip(tr("Fetch data from a remote Tao project "
                              "(path or URL)"));
    fetchAct->setEnabled(false);
    fetchAct->setObjectName("fetchURL");
    connect(fetchAct, SIGNAL(triggered()), this, SLOT(fetch()));

    cloneAct = new QAction(tr("Clone..."), this);
    cloneAct->setStatusTip(tr("Clone (download) a Tao project "
                              "and make a local copy"));
    cloneAct->setObjectName("clone");
    connect(cloneAct, SIGNAL(triggered()), this, SLOT(clone()));

    mergeAct = new QAction(tr("Merge..."), this);
    mergeAct->setStatusTip(tr("Apply the changes made in one branch into "
                              "another branch"));
    mergeAct->setEnabled(false);
    mergeAct->setObjectName("merge");
    connect(mergeAct, SIGNAL(triggered()), this, SLOT(merge()));

    checkoutAct = new QAction(tr("Checkout..."), this);
    checkoutAct->setStatusTip(tr("Checkout a previous version of the document "
                                 "into a temporary branch"));
    checkoutAct->setEnabled(false);
    checkoutAct->setObjectName("checkout");
    connect(checkoutAct, SIGNAL(triggered()), this, SLOT(checkout()));

    selectiveUndoAct = new QAction(tr("Selective undo..."), this);
    selectiveUndoAct->setStatusTip(tr("Pick a previous change, revert it and "
                                      "apply it to the current document"));
    selectiveUndoAct->setEnabled(false);
    selectiveUndoAct->setObjectName("selectiveUndo");
    connect(selectiveUndoAct, SIGNAL(triggered()),
            this, SLOT(selectiveUndo()));

    diffAct = new QAction(tr("Diff..."), this);
    diffAct->setStatusTip(tr("View the source code difference between two "
                             "document versions"));
    diffAct->setEnabled(false);
    diffAct->setObjectName("diff");
    connect(diffAct, SIGNAL(triggered()), this, SLOT(diff()));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    aboutAct->setObjectName("about");
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    //aboutQtAct = new QAction(tr("About &Qt"), this);
    //aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    //connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    preferencesAct = new QAction(tr("&Preferences"), this);
    preferencesAct->setStatusTip(tr("Set application preferences"));
    preferencesAct->setObjectName("preferences");
    connect(preferencesAct, SIGNAL(triggered()), this, SLOT(preferences()));

    fullScreenAct = new QAction(tr("Full Screen"), this);
    fullScreenAct->setStatusTip(tr("Toggle full screen mode"));
    fullScreenAct->setCheckable(true);
    fullScreenAct->setObjectName("fullScreen");
    connect(fullScreenAct, SIGNAL(triggered()), this, SLOT(toggleFullScreen()));

    slideShowAct = new QAction(tr("Slide show"), this);
    slideShowAct->setStatusTip(tr("Toggle slide show mode"));
    slideShowAct->setCheckable(true);
    slideShowAct->setObjectName("slideShow");
    connect(slideShowAct, SIGNAL(triggered()), this, SLOT(toggleSlideShow()));

    viewAnimationsAct = new QAction(tr("Animations"), this);
    viewAnimationsAct->setStatusTip(tr("Switch animations on or off"));
    viewAnimationsAct->setCheckable(true);
    viewAnimationsAct->setChecked(taoWidget->hasAnimations());
    viewAnimationsAct->setObjectName("viewAnimations");
    connect(viewAnimationsAct, SIGNAL(triggered()),
            this, SLOT(toggleAnimations()));

    viewStereoscopyAct = new QAction(tr("Stereoscopy"), this);
    viewStereoscopyAct->setStatusTip(tr("Switch stereoscopy on or off"));
    viewStereoscopyAct->setCheckable(true);
    viewStereoscopyAct->setChecked(taoWidget->hasStereoscopy());
    viewStereoscopyAct->setObjectName("viewStereoscopy");
    connect(viewStereoscopyAct, SIGNAL(triggered()),
            this, SLOT(toggleStereoscopy()));

    cutAct->setEnabled(false);
    copyAct->setEnabled(true);
    connect(srcEdit, SIGNAL(copyAvailable(bool)),
            cutAct, SLOT(setEnabled(bool)));
    connect(taoWidget, SIGNAL(copyAvailable(bool)),
            cutAct, SLOT(setEnabled(bool)));

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
    handCursorAct->setObjectName("handCursor");
    connect(handCursorAct, SIGNAL(toggled(bool)), taoWidget,
            SLOT(showHandCursor(bool)));
    // Icon copied from:
    // /opt/local/share/icons/gnome/32x32/actions/view-restore.png
    resetViewAct = new QAction(QIcon(":/images/view-restore.png"),
                                    tr("Restore default view"), this);
    resetViewAct->setStatusTip(tr("Restore default view (zoom and position)"));
    resetViewAct->setObjectName("resetView");
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
    fileMenu->addAction(newDocAct);
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(openUriAct);
    openRecentMenu = fileMenu->addMenu(tr("Open &Recent"));
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addAction(saveFontsAct);
    fileMenu->addAction(consolidateAct);
    fileMenu->addSeparator();
    fileMenu->addAction(pageSetupAct);
    fileMenu->addAction(printAct);
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
    shareMenu->addAction(pushAct);
    shareMenu->addAction(mergeAct);
    shareMenu->addAction(checkoutAct);
    shareMenu->addAction(selectiveUndoAct);
    shareMenu->addAction(diffAct);

    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(dock->toggleViewAction());
    viewMenu->addAction(errorDock->toggleViewAction());
    viewMenu->addAction(fullScreenAct);
    viewMenu->addAction(slideShowAct);
    viewMenu->addAction(viewAnimationsAct);
    if (XL::MAIN->options.enable_stereoscopy)
        viewMenu->addAction(viewStereoscopyAct);
    viewMenu->addMenu(tr("&Toolbars"))->setObjectName(VIEW_MENU_NAME);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->setObjectName(HELP_MENU_NAME);
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(preferencesAct);
}


void Window::createToolBars()
// ----------------------------------------------------------------------------
//   Create the application tool bars
// ----------------------------------------------------------------------------
{
    QMenu *view = findChild<QMenu*>(VIEW_MENU_NAME);
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->setObjectName("fileToolBar");
    fileToolBar->addAction(newAct);
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);
    if (view)
        view->addAction(fileToolBar->toggleViewAction());

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->setObjectName("editToolBar");
    editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    editToolBar->addAction(pasteAct);
    if (view)
        view->addAction(editToolBar->toggleViewAction());

    viewToolBar = addToolBar(tr("View"));
    viewToolBar->setObjectName("viewToolBar");
    viewToolBar->addAction(handCursorAct);
    viewToolBar->addAction(resetViewAct);
    if (view)
        view->addAction(viewToolBar->toggleViewAction());

    gitToolBar = new GitToolBar(tr("Git Tools"), this);
    gitToolBar->setObjectName("gitToolbar");
    connect(this, SIGNAL(projectChanged(Repository*)),
            gitToolBar, SLOT(setRepository(Repository*)));
    connect(this, SIGNAL(projectChanged(Repository*)),
            this, SLOT(checkDetachedHead()));
    connect(gitToolBar, SIGNAL(checkedOut(QString)),
            this, SLOT(reloadCurrentFile()));
    connect(this, SIGNAL(projectUrlChanged(QString)),
            gitToolBar, SLOT(showProjectUrl(QString)));
    addToolBar(gitToolBar);
    if (view)
        view->addAction(gitToolBar->toggleViewAction());
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
    // By default, the application's main window is centered and proportional
    // to the screen size, p being the scaling factor
    const float p = 0.7;
    QRect avail = TaoApp->desktop()->availableGeometry(this);
    int w = avail.width(), h = avail.height();
    QSettings settings;
    QPoint pos = settings.value("pos", QPoint((w*(1-p))/2, (h*(1-p))/2)).toPoint();
    QSize size = settings.value("size", QSize(w*p, h*p)).toSize();
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


void Window::closeToolWindows()
// ----------------------------------------------------------------------------
//   Close tool
// ----------------------------------------------------------------------------
{
    // Call ToolWindow::doClose() because close() simply hides the window
    QList<ToolWindow *> floats = findChildren<ToolWindow *>();
    foreach (ToolWindow *f, floats)
        f->doClose();
}

bool Window::maybeSave()
// ----------------------------------------------------------------------------
//   Check if we need to save the document
// ----------------------------------------------------------------------------
{
    if (srcEdit->document()->isModified())
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
//    Load the XL stylesheet to use for syntax highlighting
// ----------------------------------------------------------------------------
{
    taoWidget->setSourceRenderer();
}


void Window::showMessage(QString message, int timeout)
// ----------------------------------------------------------------------------
//    Show a status message, either in the status bar on on the splash screen
// ----------------------------------------------------------------------------
{
    if (splashScreen)
        splashScreen->showMessage(message);
    else
        statusBar()->showMessage(message, timeout);
    QApplication::processEvents();
}


void Window::setReadOnly(bool ro)
// ----------------------------------------------------------------------------
//    Switch document to read-only or read-write mode
// ----------------------------------------------------------------------------
{
    isReadOnly = ro;
    srcEdit->setReadOnly(ro);
    pushAct->setEnabled(!ro);
    mergeAct->setEnabled(!ro);
    selectiveUndoAct->setEnabled(!ro);
}


bool Window::loadFile(const QString &fileName, bool openProj)
// ----------------------------------------------------------------------------
//    Load a specific file (and optionally, open project repository)
// ----------------------------------------------------------------------------
{
    IFTRACE(fileload)
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
        showMessage(msg.arg(tr("Repository cleanup")));
        repo->gc();
    }

    showMessage(msg.arg(tr("Fonts")));
    FontFileManager ffm;
    QList<int> prev = appFontIds;
    appFontIds = ffm.LoadEmbeddedFonts(fileName);
    if (!appFontIds.empty())
        taoWidget->glyphs().Clear();
    foreach (QString e, ffm.errors)
        addError(e);
    ffm.UnloadEmbeddedFonts(prev);
    showMessage(msg.arg(tr("Document")));

    // FIXME: the whole search path stuff is broken when multiple documents
    // are open. There is no way to make "xl:" have a different meaning in
    // two Window instances. And yet it's what we need!
    // As a result, opening a new document from a different directory *will*
    // make the current doc fail to execute if it uses e.g., [load "stuff.xl"]
    // to load a file from the project's directory.
    updateContext(docPath);
    bool hadError = updateProgram(fileName);
    srcEdit->setXLNames(taoWidget->listNames());

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
    isUntitled = false;
    setCurrentFile(fileName);
    if (XL::MAIN->options.slideshow)
        switchToSlideShow();
    return true;
}


bool Window::toggleSlideShow()
// ----------------------------------------------------------------------------
//    Toggle between slide show and normal mode
// ----------------------------------------------------------------------------
{
    return switchToSlideShow(!slideShowMode);
}


bool Window::switchToSlideShow(bool ss)
// ----------------------------------------------------------------------------
//    Enter or leave slide show mode
// ----------------------------------------------------------------------------
{
    bool oldMode = slideShowMode;
    switchToFullScreen(ss);
    setWindowAlwaysOnTop(ss);
    showSourceView(!ss);
    taoWidget->autoHideCursor(NULL, ss);
    TaoApp->blockScreenSaver(ss);
    slideShowAct->setChecked(ss);
    slideShowMode = ss;
    return oldMode;
}

void Window::setWindowAlwaysOnTop(bool alwaysOnTop)
// ----------------------------------------------------------------------------
//    Set 'always on top' flag for the current window
// ----------------------------------------------------------------------------
{
#ifdef Q_OS_WIN
    HWND flag = alwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST;
    SetWindowPos(winId(), flag, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
#else
    Qt::WindowFlags flags = windowFlags();
    bool prev = ((flags & Qt::WindowStaysOnTopHint) ==
                          Qt::WindowStaysOnTopHint);
    if (prev != alwaysOnTop)
    {
        if (alwaysOnTop)
            flags |= Qt::WindowStaysOnTopHint;
        else
            flags &= ~Qt::WindowStaysOnTopHint;
        setWindowFlags(flags);
        show();
    }
#endif
}


bool Window::loadFileIntoSourceFileView(const QString &fileName, bool box)
// ----------------------------------------------------------------------------
//    Update the source file view with the plain contents of a specific file
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
        srcEdit->clear();
        return false;
    }

    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    loadInProgress = true;
    srcEdit->setPlainText(in.readAll());
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
            if (xlRuntime->LoadFile(fn, true))
                hadError = true;

        // Check if we can access the file
        if (!fileInfo.isWritable())
            sf->readOnly = true;
    }
    else
    {
        if (xlRuntime->LoadFile(fn, true))
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

    isUntitled = false;
    setCurrentFile(fileName);
    statusBar()->showMessage(tr("Saving..."));
    // FIXME: can't call processEvent here, or the "Save with fonts..."
    // function fails to save all the fonts of a multi-page doc
    // QApplication::processEvents();

    do
    {
        QTextStream out(&file);
        QApplication::setOverrideCursor(Qt::WaitCursor);
        out << srcEdit->toPlainText();
        QApplication::restoreOverrideCursor();
    } while (0); // Flush

    text fn = +fileName;

    xlRuntime->LoadFile(fn);

    showMessage(tr("File saved"), 2000);
    updateProgram(fileName);
    srcEdit->setXLNames(taoWidget->listNames());
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

    return true;
}


void Window::markChanged(bool changed)
// ----------------------------------------------------------------------------
//   Someone else tells us that the window is changed or not
// ----------------------------------------------------------------------------
{
    srcEdit->document()->setModified(changed);
    setWindowModified(changed);
}


void Window::enableProjectSharingMenus()
// ----------------------------------------------------------------------------
//   Activate the Git-related actions
// ----------------------------------------------------------------------------
{
    setPullUrlAct->setEnabled(true);
    pushAct->setEnabled(true);
    fetchAct->setEnabled(true);
    mergeAct->setEnabled(true);
    checkoutAct->setEnabled(true);
    selectiveUndoAct->setEnabled(true);
    diffAct->setEnabled(true);
}


bool Window::openProject(QString path, QString fileName, bool confirm)
// ----------------------------------------------------------------------------
//   Find and open a project (= SCM repository)
// ----------------------------------------------------------------------------
// If project does not exist and 'confirm' is true, user will be asked to
// confirm project creation.
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
                    gitToolBar, SLOT(refresh()));
            connect(repo.data(), SIGNAL(branchChanged(QString)),
                    this, SLOT(checkDetachedHead()));
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
    QFileInfo tao       ("system:tao.xl");
    QFileInfo user      ("xl:user.xl");
    QFileInfo theme     ("xl:theme.xl");

    contextFileNames.clear();

    if (tao.exists())
        contextFileNames.push_back(+tao.canonicalFilePath());
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

        // Save state of main window and dock widgets that were added by
        // addDockWidget(). Toolbars should normally be saved, too, but see
        // QTBUG? comment below.
        savedState.geometry = saveGeometry();
        savedState.state = saveState();

        // Hide ToolWindows.
        // Remember which ones are open. Required because they are not added to
        // the main window with addDockWidget().
        QList<ToolWindow *> tools = findChildren<ToolWindow *>();
        foreach (ToolWindow *t, tools)
        {
            if (t->isVisible())
            {
                savedState.visibleTools[t] = t->saveGeometry();
                t->hide();
            }
        }

        // Hide all dockable widgets
        QList<QDockWidget *> docks = findChildren<QDockWidget *>();
        foreach(QDockWidget *d, docks)
            d->hide();

        // Hide toolbars
        savedState.visibleToolBars.clear();
        QList<QToolBar *> toolBars = findChildren<QToolBar *>();
        foreach (QToolBar *t, toolBars)
        {
            if (t->isVisible())
                savedState.visibleToolBars << t;
            // QTBUG?
            // Toolbars have to be removed, not just hidden, to avoid a
            // display glitch when the "unified title and toolbar look" is
            // enabled on MacOSX.
            // In this case, hiding a toolbar through its toggleViewAction
            // effectively makes it disappear from the toolbar area (i.e., it
            // does not leave an empty space). BUT, when a toolbar is hidden,
            // switching to fullscreen then back to normal view causes an empty
            // area to appear in place of the disabled toolbar.
            // Removing all toolbars, then adding back only the ones which were
            // visible solves the problem.
            removeToolBar(t);
        }

        statusBar()->hide();
        menuBar()->hide();
        showFullScreen();
    }
    else
    {
        showNormal();
        menuBar()->show();
        statusBar()->show();
        setUnifiedTitleAndToolBarOnMac(true);

        // Restore toolbars
        foreach (QToolBar *t, savedState.visibleToolBars)
        {
            addToolBar(t);
            t->show();
        }

        // Restore tool windows
        foreach (ToolWindow *t, savedState.visibleTools.keys())
        {
            t->restoreGeometry(savedState.visibleTools[t]);
            t->show();
        }

        // Restore dockable widgets, and window geometry
        restoreGeometry(savedState.geometry);
        restoreState(savedState.state);
    }
    fullScreenAct->setChecked(fs);
}


bool Window::showSourceView(bool show)
// ----------------------------------------------------------------------------
//   Show or hide source view
// ----------------------------------------------------------------------------
{
    bool old = dock->isVisible();
    dock->setVisible(show);
    dock->toggleViewAction()->setChecked(show);
    return old;
}


QString Window::currentProjectFolderPath()
// ----------------------------------------------------------------------------
//    The folder to use in the "Save as..."/"Open File..." dialogs
// ----------------------------------------------------------------------------
{
    if (repo)
        return repo->path;

    if ( !currentProjectFolder.isEmpty() &&
         !isTutorial(curFile))
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
    if (fi.exists())
        isReadOnly |= !fi.isWritable();

    markChanged(false);
    setWindowFilePath(curFile);

    // Update the recent file list
    if (!isUntitled && !isTutorial(curFile))
    {
        IFTRACE(settings)
            std::cerr << "Adding " << +fileName << " to recent file list\n";

        QSettings settings;
        QStringList files = settings.value("recentFileList").toStringList();
        files.removeAll(fileName);
        files.prepend(fileName);
        while (files.size() > MaxRecentFiles)
            files.removeLast();
        settings.setValue("recentFileList", files);

        IFTRACE(settings)
        {
            std::cerr << "Recent file list:\n";
            foreach (QString s, files)
                std::cerr << "  " << +s << "\n";
        }

        foreach (QWidget *widget, QApplication::topLevelWidgets())
        {
            Window *mainWin = qobject_cast<Window *>(widget);
            if (mainWin)
                mainWin->updateRecentFileActions();
        }
    }
}


bool Window::isTutorial(const QString &filePath)
// ----------------------------------------------------------------------------
//    Return true if the file currently loaded is the Tao tutorial
// ----------------------------------------------------------------------------
{
    static QFileInfo tutorial("system:tutorial.ddd");
    static QString tutoPath = tutorial.canonicalFilePath();
    return (filePath == tutoPath);
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
        QString nat = QDir::toNativeSeparators(files[i]);
#ifdef CONFIG_MACOSX
        QString text = nat;
#else
        QString text = tr("&%1 %2").arg(i + 1).arg(nat);
#endif
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setToolTip(files[i]);
        recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        recentFileActs[j]->setVisible(false);

    clearRecentAct->setEnabled(numRecentFiles > 0);
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


void Window::checkDetachedHead()
// ----------------------------------------------------------------------------
//    Prevent document changes when current head is detached
// ----------------------------------------------------------------------------
{
    if (!repo)
        return;
    setReadOnly(repo->branch() == "");
}

TAO_END
