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

namespace Tao {

CloneDialog::CloneDialog(Repository *repo, QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a "clone" dialog
// ----------------------------------------------------------------------------
    : QDialog(parent), repo(repo), done(false), proc(NULL)
{
    setupUi(this);
    okButton = buttonBox->button(QDialogButtonBox::Ok);
    cancelButton = buttonBox->button(QDialogButtonBox::Cancel);
    folderEdit->setText(Application::defaultProjectFolderPath());
}


void CloneDialog::accept()
// ----------------------------------------------------------------------------
//    Ok/dismiss button was clicked
// ----------------------------------------------------------------------------
{
    if (done)
        return QDialog::accept();

    QString url = urlEdit->text();
    QString folder = folderEdit->text();
    if (url.isEmpty() || folder.isEmpty())
        return;
    okButton->setEnabled(false);
    cloneOutput->append(tr("Starting...\n"));
    connect(repo, SIGNAL(asyncProcessComplete(void *)),
            this, SLOT(endClone(void *)));
    proc = repo->asyncClone(url, folder, cloneOutput, this);
}


void CloneDialog::reject()
// ----------------------------------------------------------------------------
//    Cancel button was clicked
// ----------------------------------------------------------------------------
{
    if (done)
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
    QString text;
    if (proc)
        text = tr("Done.\n");
    else
        text = tr("Canceled.\n");
    cloneOutput->append(text);
    done = true;
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

}
