// ****************************************************************************
//  clone_dialog.cpp                                               Tao project
// ****************************************************************************
//
//   File Description:
//
//    The class to display the "Clone" dialog box
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

#include "clone_dialog.h"
#include "repository.h"
#include "application.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QCompleter>

namespace Tao {

CloneDialog::CloneDialog(QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a "clone" dialog
// ----------------------------------------------------------------------------
    : QDialog(parent), repo(NULL), okToDismiss(false), proc(NULL)
{
    setupUi(this);
    okButton = buttonBox->button(QDialogButtonBox::Ok);
    cancelButton = buttonBox->button(QDialogButtonBox::Cancel);
    folderEdit->setText(Application::defaultProjectFolderPath());
    QCompleter *pc = new QCompleter(TaoApp->pathCompletions(), this);
    QCompleter *uc = new QCompleter(TaoApp->urlCompletions(), this);
    folderEdit->setCompleter(pc);
    urlEdit->setCompleter(uc);
}


void CloneDialog::accept()
// ----------------------------------------------------------------------------
//    Ok/dismiss button was clicked
// ----------------------------------------------------------------------------
{
    if (okToDismiss)
        return QDialog::accept();

    QString url = urlEdit->text();
    QString folder = folderEdit->text();
    QString newFolder = newFolderEdit->text();
    if (url.isEmpty() || folder.isEmpty())
        return;
    TaoApp->addUrlCompletion(url);
    TaoApp->addPathCompletion(folder);
    repo = RepositoryFactory::repository(folder, RepositoryFactory::Clone);
    if (!repo)
    {
        QMessageBox::warning(this, tr("Invalid folder"),
                             tr("You can't clone into a folder that is "
                                "already under a valid Tao project.<br>"
                                "Please select a different folder."));
        return;
    }
    okButton->setEnabled(false);
    cancelButton->setText("Abort");
    cloneOutput->append(tr("Starting...\n"));
    setCursor(Qt::BusyCursor);
    connect(repo.data(), SIGNAL(asyncCloneComplete(void *, QString)),
            this, SLOT(endClone(void *, QString)));
    proc = repo->asyncClone(url, newFolder);
    connect(proc.data(), SIGNAL(standardErrorUpdated(QByteArray)),
            cloneOutput, SLOT(insertAnsiText(QByteArray)));
    connect(proc.data(), SIGNAL(standardOutputUpdated(QByteArray)),
            cloneOutput, SLOT(insertAnsiText(QByteArray)));
    repo->dispatch(proc, this);
}


void CloneDialog::reject()
// ----------------------------------------------------------------------------
//    Cancel button was clicked
// ----------------------------------------------------------------------------
{
    if (!proc)
        return QDialog::reject();

    process_p p = proc;
    proc.clear();
    if (p)
        repo->abort(p);
}


void CloneDialog::endClone(void *id, QString projPath)
// ----------------------------------------------------------------------------
//    The clone operation has completed or has been canceled
// ----------------------------------------------------------------------------
{
    if (id != this)
        return;
    setCursor(Qt::ArrowCursor);
    QString text;
    if (proc)
        text = tr("Done.\n");
    else
        text = tr("Aborted.\n");
    proc.clear();
    okToDismiss = true;
    okButton->setText("Close");
    okButton->setEnabled(true);
    cancelButton->setEnabled(false);
    cloneOutput->insertPlainText(text);
    if (!projPath.isEmpty())
    {
        repository_ptr repo;
        repo = RepositoryFactory::repository(projPath,
                                             RepositoryFactory::OpenExisting);
        if (repo)
        {
            bool ok;
            cloneOutput->insertPlainText(tr("Checking out files...\n"));
            ok = repo->setTask("master");
            if (ok)
            {
                this->projectPath = projPath;
                cloneOutput->insertPlainText(tr("Done\n"));
            }
            else
                cloneOutput->insertPlainText(tr("Failed.\n"));
        }
    }
    cloneOutput->moveCursor(QTextCursor::End);
}

void CloneDialog::on_browseButton_clicked()
// ----------------------------------------------------------------------------
//    The "Browse..." button was clicked
// ----------------------------------------------------------------------------
{
    QString folder = QFileDialog::getExistingDirectory(this,
                         tr("Select destination folder"),
                         folderEdit->text());
    if (!folder.isEmpty())
        folderEdit->setText(folder);
}


void CloneDialog::on_folderEdit_textEdited()
// ----------------------------------------------------------------------------
//    The folder path was changed
// ----------------------------------------------------------------------------
{
    enableOkCancel();
}


void CloneDialog::on_urlEdit_textEdited()
// ----------------------------------------------------------------------------
//    The URL was changed
// ----------------------------------------------------------------------------
{
    enableOkCancel();
}


void CloneDialog::enableOkCancel()
// ----------------------------------------------------------------------------
//    Re-enable OK/Cancel after user has changed some parameter (to try again)
// ----------------------------------------------------------------------------
{
    if (!okToDismiss)
        return;

    okButton->setText(tr("OK"));
    cancelButton->setText("Cancel");
    cancelButton->setEnabled(true);
    okToDismiss = false;
}

}
