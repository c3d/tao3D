// ****************************************************************************
//  history_dialog.cpp                                             Tao project
// ****************************************************************************
//
//   File Description:
//
//    HistoryDialog implementation.
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
#include "history_dialog.h"
#include "repository.h"
#include "ui_history_dialog.h"
#include "commit_table_widget.h"
#include "tao_utf8.h"
#include <QMessageBox>
#include <QLineEdit>

namespace Tao {

HistoryDialog::HistoryDialog(Repository *repo, QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a "History" dialog box with "Checkout" and "Undo" buttons
// ----------------------------------------------------------------------------
    : QDialog(parent), repo(repo)
{
    Q_ASSERT(repo);

    setupUi(this);

    connect(branchCombo, SIGNAL(branchSelected(QString)),
            tableView, SLOT(setBranch(QString)));
    connect(tableView, SIGNAL(commitSelected(Repository::Commit)),
            this, SLOT(tableView_commitSelected(Repository::Commit)));

    tableView->setRepository(repo);
    branchCombo->setRepository(repo);
}


void HistoryDialog::tableView_commitSelected(Repository::Commit commit)
// ----------------------------------------------------------------------------
//    Update QLineEdit widget when a commit is selected through table view
// ----------------------------------------------------------------------------
{
    revEdit->setText(commit.id);
}

}
