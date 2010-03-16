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


TAO_BEGIN

Window::Window(XL::Main *xlr, XL::SourceFile *sf)
// ----------------------------------------------------------------------------
//    Create a Tao window with default parameters
// ----------------------------------------------------------------------------
    : xlRuntime(xlr), xlProgram(sf),
      textEdit(NULL), taoWidget(NULL),
      isUntitled(sf == NULL)
{
    // Create the widgets
    QDockWidget *dock = new QDockWidget(tr("Source"));
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    textEdit = new QTextEdit(dock);
    dock->setWidget(textEdit);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    taoWidget = new Widget(this, sf);
    setCentralWidget(taoWidget);

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
    if (sf)
        loadSource(QString::fromStdString(sf->name));    // untested
    else
        setCurrentFile("");
}


void Window::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        writeSettings();
        QDir().remove(tmpDocPath());
        // FIXME: Qt does not support recursive deletion!
        QDir().rename(repoPath(),
                      repoPath() + QString(".to_delete_%1")
                      .arg(QDateTime::currentDateTime().toTime_t()));

        event->accept();
    } else {
        event->ignore();
    }
}

void Window::newFile()
{
    Window *other = new Window(xlRuntime, NULL);
    other->move(x() + 40, y() + 40);
    other->show();
}

/*
TODO ADD IMPORT FUNCTION
    QString fileName = QFileDialog::getOpenFileName
        (this,
         tr("Open Tao Document"),
         tr(""),
         tr("Tao documents (*.ddd);;XL programs (*.xl);;"
            "Headers (*.dds *.xs);;All files (*.*)"));
*/

void Window::open()
{
    QString fileName = QFileDialog::getOpenFileName
        (this,
         tr("Open Tao Document"),
         tr(""),
         tr("Tao documents (*.tao);;All files (*.*)"));
    if (!fileName.isEmpty()) {

        Window *existing = findWindow(fileName);
        if (existing && !existing->isUntitled) {
            existing->show();
            existing->raise();
            existing->activateWindow();
            return;
        }

        if (isUntitled && textEdit->document()->isEmpty()
                && !isWindowModified()) {
            loadFile(fileName);
        } else {
            Window *other = new Window(xlRuntime, NULL);
            other->loadFile(fileName);
            if (other->isUntitled) {
                delete other;
                return;
            }
            other->move(x() + 40, y() + 40);
            other->show();
        }
    }
}

bool Window::save()
{
    if (isUntitled) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool Window::saveAs()
{
    // FIXME: clean tmp files when saving under a new name
    // (save as doc1, then save as doc2: doc1.git and doc1.git.tmp not deleted)
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
                                                    curFile);
    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

bool Window::saveToGit()
{
    switch (gitRepo.SaveDocument(xlProgram->tree))
    {
    case GitRepo::savedNewVersionCreated:
    case GitRepo::notSavedNullTree:
        statusBar()->showMessage(tr("Saved"), 2000);
        return true;
    case GitRepo::notSavedNoChange:
        statusBar()->showMessage(tr("No change"), 2000);
        break;
    case GitRepo::notSavedSaveError:
        statusBar()->showMessage(tr("Save error"), 2000);
        break;
    default:
        Q_ASSERT(!"Unexpected save status");
    }
    return false;
}

void Window::about()
{
    kstring txt =
        "<b>Tao</b>, an interactive collaboration tool.<br/>"
        "Brought to you by Taodyne SAS:<br/>"
        "Anne Lempereur<br/>"
        "Catherine Burvelle<br/>"
        "Jérôme Forissier<br/>"
        "Lionel Schaffhauser<br/>"
        "Christophe de Dinechin.";
   QMessageBox::about (this, tr("About Tao"), tr(txt));
}

void Window::documentWasModified()
{
    setWindowModified(true);
}

void Window::createActions()
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

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));


    cutAct->setEnabled(false);
    copyAct->setEnabled(false);
    connect(textEdit, SIGNAL(copyAvailable(bool)),
            cutAct, SLOT(setEnabled(bool)));
    connect(textEdit, SIGNAL(copyAvailable(bool)),
            copyAct, SLOT(setEnabled(bool)));
}

//! [implicit tr context]
void Window::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
//! [implicit tr context]
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(closeAct);
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void Window::createToolBars()
{
//! [0]
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(newAct);
    fileToolBar->addAction(openAct);
//! [0]
    fileToolBar->addAction(saveAct);

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    editToolBar->addAction(pasteAct);
}

void Window::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void Window::readSettings()
{
    QSettings settings;
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    move(pos);
    resize(size);
}

void Window::writeSettings()
{
    QSettings settings;
    settings.setValue("pos", pos());
    settings.setValue("size", size());
}

bool Window::maybeSave()
{
    if (textEdit->document()->isModified()) {
	QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("SDI"),
                     tr("The document has been modified.\n"
                        "Do you want to save your changes?"),
                     QMessageBox::Save | QMessageBox::Discard
		     | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void Window::loadFile(const QString &fileName)
{
    // Check document exists and is readable
    QFile docFile(fileName);
    if (!docFile.open(QFile::ReadOnly)) {
        QMessageBox::warning(this, tr("Cannot read file"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(docFile.errorString()));
        return;
    }
    docFile.close();
    setCurrentFile(fileName);

    qDebug() << "loadFile()" << fileName;

    // Extract document's Git repo to temporary directory
    QProcess tar;
    tar.setWorkingDirectory(tmpDir());
    tar.start("tar", QStringList() << "zxf" << fileName);
    if (!tar.waitForFinished())
        return;

    // Checkout main file to temporary path
    gitRepo.curPath = repoPath();
    gitRepo.CheckoutDocument(tmpDocPath());

    // Load source from temporary path
    loadSource(tmpDocPath());

    statusBar()->showMessage(tr("File loaded"), 2000);
}

void Window::loadSource(const QString &fileName)
{
    // Load doc source from temporary path
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this, tr("Cannot read file"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }
    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    textEdit->setPlainText(in.readAll());
    QApplication::restoreOverrideCursor();

    // Update program
    text fn = tmpDocPath().toStdString();
    xlRuntime->LoadFile(fn);
    xlProgram = &xlRuntime->files[fn];
    taoWidget->xlProgram = xlProgram;
    taoWidget->updateGL();
}

bool Window::saveFile(const QString &fileName)
{
    // Save current doc source to temporary path
    QFile file(tmpDocPath());
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("SDI"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(tmpDocPath())
                             .arg(file.errorString()));
        return false;
    }
    QApplication::setOverrideCursor(Qt::WaitCursor);

    do
    {
        QTextStream out(&file);
        out << textEdit->toPlainText();
    } while (0);                // Flush  // REVISIT (.flush())

    // Update XL tree from temporary doc source
    text fn = tmpDocPath().toStdString();
    xlRuntime->LoadFile(fn);
    xlProgram = &xlRuntime->files[fn];
    taoWidget->xlProgram = xlProgram;
    taoWidget->updateGL();

    // Save to Git
    gitRepo.curPath = repoPath();
    saveToGit();

    // Make archive of Git repository
    QProcess tar;
    tar.setWorkingDirectory(tmpDir());
    tar.start("tar", QStringList() << "zcf" << fileName << repoFileName());
    tar.waitForFinished();

    QApplication::restoreOverrideCursor();
    setCurrentFile(fileName);

    return true;
}

void Window::setCurrentFile(const QString &fileName)
{
    static int sequenceNumber = 1;

    isUntitled = fileName.isEmpty();
    if (isUntitled) {
        curFile = QDir().currentPath() + "/" +
                  tr("document%1.tao").arg(sequenceNumber++);
    } else {
        curFile = fileName;
    }

    textEdit->document()->setModified(false);
    setWindowModified(false);
    setWindowFilePath(curFile);
}

Window *Window::findWindow(const QString &fileName)
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();

    foreach (QWidget *widget, qApp->topLevelWidgets()) {
        Window *mainWin = qobject_cast<Window *>(widget);
        if (mainWin && mainWin->curFile == canonicalFilePath)
            return mainWin;
    }
    return 0;
}

QString Window::tmpDir()
{
    return QDir().tempPath();
}

QString Window::docFileName()
{
    return QFileInfo(curFile).fileName();
}

QString Window::repoFileName()
{
    return docFileName() + ".git";
}

QString Window::tmpDocPath()
{
    return tmpDir() + "/" + docFileName() + ".tmp";
}

QString Window::repoPath()
{
    return tmpDir() + "/" + repoFileName();
}

TAO_END

