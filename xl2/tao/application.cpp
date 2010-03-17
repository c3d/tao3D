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

void Application::OpenLibrary()
// ----------------------------------------------------------------------------
//   Open the default library, or ask the user which one to choose
// ----------------------------------------------------------------------------
{
    QString path = QSettings().value("defaultlibrary").toString();
    OpenLibrary(path);
}


void Application::OpenLibrary(QString path)
// ----------------------------------------------------------------------------
//   Open the default library, or ask the user which one to choose
// ----------------------------------------------------------------------------
{
    bool ok = true;

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
                                         tr("Please choose a folder "
                                             "for your project:"),
                                         QLineEdit::Normal, path, &ok);
        if (path.isNull() || !ok)
            break;
 
        repository = Repository::repository(path);
        if (!repository)
        {
            QMessageBox::StandardButton b = QMessageBox::
                question (0, tr("Tao Library"),
                          tr("The project '%1' does not appear to exist. "
                             "Do you want to create it?\n\n"
                             "'Yes' will create the project.\n"
                             "'No' will continue without a valid project.\n"
                             "'Retry' will let you select another location.")
                          .arg(path),
                          QMessageBox::Yes |
                          QMessageBox::No |
                          QMessageBox::Retry);
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
    } while (!ok);

    if (ok)
        QSettings().setValue("defaultlibrary", path);
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
