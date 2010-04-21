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
    QCompleter *pathCompleter = new QCompleter(TaoApp->pathCompletions, this);
    QCompleter *urlCompleter = new QCompleter(TaoApp->urlCompletions, this);
    folderEdit->setCompleter(pathCompleter);
    urlEdit->setCompleter(urlCompleter);
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
    if (url.isEmpty() || folder.isEmpty())
        return;
    TaoApp->urlCompletions.append(url);
    TaoApp->pathCompletions.append(folder);
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
    cloneOutput->append(tr("Starting...\n"));
    setCursor(Qt::BusyCursor);
    connect(repo.data(), SIGNAL(asyncProcessComplete(void *)),
            this, SLOT(endClone(void *)));
    proc = repo->asyncClone(url, folder, cloneOutput, this);
}


void CloneDialog::reject()
// ----------------------------------------------------------------------------
//    Cancel button was clicked
// ----------------------------------------------------------------------------
{
    if (!proc)
        return QDialog::reject();

    Process *p = proc;
    proc = NULL;
    if (p)
        repo->abort(p);
}


void CloneDialog::endClone(void *id)
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
        text = tr("Canceled.\n");
    cloneOutput->append(text);
    okToDismiss = true;
    okButton->setText(tr("Dismiss"));
    okButton->setEnabled(true);
    cancelButton->setEnabled(false);
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
    cancelButton->setEnabled(true);
    okToDismiss = false;
}

}
