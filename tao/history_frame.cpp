// ****************************************************************************
//  history_frame.cpp                                             Tao project
// ****************************************************************************
//
//   File Description:
//
//    HistoryFrame implementation.
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
#include "history_frame.h"
#include "repository.h"
#include "ui_history_frame.h"
#include "commit_table_widget.h"
#include "tao_utf8.h"
#include <QMessageBox>
#include <QLineEdit>

namespace Tao {

HistoryFrame::HistoryFrame(QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a "History" dialog box with "Checkout" and "Undo" buttons
// ----------------------------------------------------------------------------
    : QFrame(parent)
{
    setupUi(this);

    connect(branchCombo, SIGNAL(branchSelected(QString)),
            tableView, SLOT(setBranch(QString)));
    connect(tableView, SIGNAL(commitSelected(Repository::Commit)),
            this, SLOT(tableView_commitSelected(Repository::Commit)));
    connect(revEdit, SIGNAL(editingFinished()),
            this,    SLOT(emitRev()));
}


void HistoryFrame::tableView_commitSelected(Repository::Commit commit)
// ----------------------------------------------------------------------------
//    Update QLineEdit widget when a commit is selected through table view
// ----------------------------------------------------------------------------
{
    revEdit->setText(commit.id);
    emit revSet(revEdit->text());
}


void HistoryFrame::setMessage(QString text)
// ----------------------------------------------------------------------------
//    Set the message that describes the contents of the frame
// ----------------------------------------------------------------------------
{
    msg->setText(text);
}


void HistoryFrame::setRepository(Repository *repo)
// ----------------------------------------------------------------------------
//    Set repository and update UI accordingly
// ----------------------------------------------------------------------------
{
    tableView->setRepository(repo);
    branchCombo->setRepository(repo);
}


QString HistoryFrame::rev()
// ----------------------------------------------------------------------------
//    Return ID of currently selected commit
// ----------------------------------------------------------------------------
{
    return revEdit->text();
}


void HistoryFrame::emitRev()
// ----------------------------------------------------------------------------
//    Send signal with current text in commit ID line edit
// ----------------------------------------------------------------------------
{
    emit revSet(revEdit->text());
}

}
