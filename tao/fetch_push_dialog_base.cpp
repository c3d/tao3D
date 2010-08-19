// ****************************************************************************
//  fetch_push_dialog_base.h                                       Tao project
// ****************************************************************************
//
//   File Description:
//
//    Abstract base class for fetch and push dialogs
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

#include "tao.h"
#include "tao_utf8.h"
#include "fetch_push_dialog_base.h"
#include "remote_selection_frame.h"
#include "repository.h"
#include <QInputDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>

namespace Tao {

FetchPushDialogBase::FetchPushDialogBase(Repository *repo, QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a "remote" selection box for the given repository
// ----------------------------------------------------------------------------
    : QDialog(parent), repo(repo), proc(NULL), aborted(false)
{
    setupUi(this);
    QPushButton *detailsBtn = new QPushButton(tr("Show/Hide details"), NULL);
    detailsBtn->setCheckable(true);
    connect(detailsBtn, SIGNAL(toggled(bool)),
            textPane, SLOT(setVisible(bool)));
    buttonBox->addButton(detailsBtn, QDialogButtonBox::ActionRole);
    textPane->hide();
}


QString FetchPushDialogBase::Url()
// ----------------------------------------------------------------------------
//    The name of the currently selected remote (empty string if "<None>")
// ----------------------------------------------------------------------------
{
    return rsFrame->remote();
}


void FetchPushDialogBase::reject()
// ----------------------------------------------------------------------------
//    Abort current fetch process (if running) and dismiss dialog
// ----------------------------------------------------------------------------
{
    if (proc)
    {
        aborted = true;
        repo->abort(proc);
    }
    QDialog::reject();
}


void FetchPushDialogBase::dismissShortly()
// ----------------------------------------------------------------------------
//    Close dialog after a short period of time (let user see 100% completion)
// ----------------------------------------------------------------------------
{
    QTimer::singleShot(200, this, SLOT(dismiss()));
}


void FetchPushDialogBase::dismiss()
// ----------------------------------------------------------------------------
//    Immediately close dialog (on success)
// ----------------------------------------------------------------------------
{
    QDialog::accept();
}


void FetchPushDialogBase::connectSignalsAndSlots()
// ----------------------------------------------------------------------------
//    Setup connections with Git process.
// ----------------------------------------------------------------------------
{
    connect(proc.data(), SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(onFinished(int,QProcess::ExitStatus)));
    connect(proc.data(), SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(onError(QProcess::ProcessError)));
    connect(proc.data(), SIGNAL(percentComplete(int)),
            progressBar, SLOT(setValue(int)));
    connect(proc.data(), SIGNAL(standardErrorUpdated(QByteArray)),
            textPane, SLOT(insertAnsiText(QByteArray)));
    connect(proc.data(), SIGNAL(standardOutputUpdated(QByteArray)),
            textPane, SLOT(insertAnsiText(QByteArray)));
}


void FetchPushDialogBase::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
// ----------------------------------------------------------------------------
//    Check status, handle some errors, close dialog
// ----------------------------------------------------------------------------
{
    QApplication::restoreOverrideCursor();

    // Error #1: process was aborted
    if (aborted)
        return dismiss();

    // Error #2: process crashed
    if (exitStatus != QProcess::NormalExit)
        return;    // let onError handle this case

    // Error #3: process returned an error
    if (exitCode)
    {
        QString msg = tr("Operation failed.\nExit code: %1\n%2")
                         .arg(exitCode).arg(proc->err);
        QMessageBox::warning(NULL, tr("Error"), msg);
        return dismiss();
    }

    // Success
    progressBar->setValue(100);
    dismissShortly();
}


void FetchPushDialogBase::onError(QProcess::ProcessError error)
// ----------------------------------------------------------------------------
//    Display error and close dialog
// ----------------------------------------------------------------------------
{
    if (!aborted)
    {
        QString err = Process::processErrorToString(error);
        QMessageBox::warning(NULL, tr("Error"), err);
    }
    dismiss();
}


void FetchPushDialogBase::on_rsFrame_noneSelected()
// ----------------------------------------------------------------------------
//    Disable the OK button if remote is not set
// ----------------------------------------------------------------------------
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}


void FetchPushDialogBase::on_rsFrame_nameSelected()
// ----------------------------------------------------------------------------
//    Disable the OK button if remote is not set
// ----------------------------------------------------------------------------
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

}
