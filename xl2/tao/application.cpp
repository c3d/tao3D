// ****************************************************************************
//  application.h                                                  Tao project
// ****************************************************************************
//
//   File Description:
//
//    The Tao application
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
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "application.h"
#include "repository.h"
#include "git_backend.h"

#include <QString>
#include <QSettings>
#include <QInputDialog>
#include <QMessageBox>
#include <QLineEdit>
#include <QDir>
#include <QDebug>

TAO_BEGIN


Application::Application(int & argc, char ** argv)
// ----------------------------------------------------------------------------
//    Build the Tao application
// ----------------------------------------------------------------------------
    : QApplication(argc, argv), repository(NULL)
{}


Application::~Application()
// ----------------------------------------------------------------------------
//    Delete application's repository object
// ----------------------------------------------------------------------------
{
    delete repository;
}


bool Application::OpenLibrary()
// ----------------------------------------------------------------------------
//   Open the default library, or ask the user which one to choose
// ----------------------------------------------------------------------------
{
    QString path = QSettings().value("defaultlibrary").toString();
    return OpenLibrary(path, false);
}


bool Application::OpenLibrary(QString path, bool confirm)
// ----------------------------------------------------------------------------
//   Open the default library, or ask the user which one to choose
// ----------------------------------------------------------------------------
{
    bool ok = true;
    bool saveSettings = !confirm;

    do
    {
        if (path.isNull())
        {
            QDir dir(QDir::homePath());
            dir.filePath("Tao Library.tao");
            ok = false;
        }
        path = QDir::cleanPath(path);

        if (!ok)
            path = QInputDialog::getText(NULL, tr("Welcome to Tao!"),
                                         tr("Please choose a directory "
                                             "for your project:"),
                                         QLineEdit::Normal, path, &ok);
        if (path.isNull() || !ok)
            break;
 
        delete repository;
        repository = Repository::repository(path);
        if (!repository)
        {
            QMessageBox::StandardButton b = QMessageBox::question
                (0, tr("Tao Library"),
                 tr("The project '%1' does not appear to exist. "
                    "Do you want to create it?\n\n"
                    "Select 'Yes' to create the project, "
                    "'No' to continue without a valid project, and"
                    "'Retry' to choose another location.")
                 .arg(path),
                 QMessageBox::Yes | QMessageBox::No | QMessageBox::Retry);

            switch(b)
            {
            case QMessageBox::Retry:
                ok = false;
                break;
                
            case QMessageBox::Yes:
                repository = new GitRepository(path);
                repository->initialize();
                ok = repository->valid();
                if (!ok)
                {
                    delete repository;
                    repository = NULL;
                }
                break;

            case QMessageBox::No:
                break;

            defaut:
                QMessageBox::question(0, tr("Puzzled"),
                                      tr("How did you do that?"),
                                      QMessageBox::No);
            }
        }

        // Select the task branch, either current branch or without _tao_undo
        if (ok && repository && repository->valid())
        {
            text task = repository->branch();
            size_t len = task.length() - (sizeof (TAO_UNDO_SUFFIX) - 1);
            text currentBranch = task;
            bool onUndoBranch = task.find(TAO_UNDO_SUFFIX) == len;
            bool setTask = false;
            if (onUndoBranch)
                task = task.substr(0, len);

            if (confirm)
            {
                QMessageBox::StandardButton b;
                if (onUndoBranch)
                    b = QMessageBox::question
                        (0, tr("Tao Library"),
                         tr("The library '%1' appears to be in use by Tao, "
                            "because the current branch is named '%2'.\n"
                            "Do you want Tao to use it for this session? "
                            "Select 'Yes' to use it for this session only, "
                            "'No' to leave it unmodified, "
                            "'Save' to use it for this session and later ones, "
                            "and 'Retry' to select another location.").
                         arg(path).arg(+currentBranch),
                         QMessageBox::Yes | QMessageBox::No |
                         QMessageBox::Save | QMessageBox::Retry);
                else
                    b = QMessageBox::question
                        (0, tr("Tao Library"),
                         tr("The library '%1' is a valid repository, but "
                            "is not currently used for Tao, because the "
                            "current branch is named '%2'. If you use it "
                            "for Tao, the current branch will be changed "
                            "to '%3'.\n"
                            "Select 'Yes' to use it for this session only, "
                            "'No' to leave it unmodified, "
                            "'Save' to use it for this session and later ones, "
                            "and 'Retry' to select another location.").
                         arg(path).arg(+currentBranch)
                         .arg(+currentBranch + TAO_UNDO_SUFFIX),
                         QMessageBox::Yes | QMessageBox::No |
                         QMessageBox::Save | QMessageBox::Retry);

                switch (b)
                {
                case QMessageBox::Yes:
                    setTask = true;
                    break;
                case QMessageBox::No:
                    setTask = false;
                    break;
                case QMessageBox::Retry:
                    ok = false;
                    setTask = false;
                    break;
                case QMessageBox::Save:
                    setTask = true;
                    saveSettings = true;
                    break;
                default:
                    QMessageBox::question(0, tr("Coin?"),
                                          tr("How did you do that?"),
                                          QMessageBox::Discard);
                }
            }

            if (setTask)
                repository->setTask(task);
        }

    } while (!ok);

    if (ok && saveSettings)
        QSettings().setValue("defaultlibrary", path);

    return ok;
}


QDir Application::LibraryDirectory()
// ----------------------------------------------------------------------------
//   Return the best path candidate for the library
// ----------------------------------------------------------------------------
{
    QDir result(QDir::current());
    if (repository)
    {
        result.cd(repository->path);
    }
    else
    {
        QString path = QSettings().value("defaultlibrary").toString();
        if (!path.isNull())
            result.cd(path);
    }
    return result;    
}


void pqs(const QString &qs)
// ----------------------------------------------------------------------------
//   Print a QString for debug purpose
// ----------------------------------------------------------------------------
{
    qDebug() << qs << "\n";
}

TAO_END
