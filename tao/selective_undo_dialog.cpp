// ****************************************************************************
//  selective_undo_dialog.cpp                                      Tao project
// ****************************************************************************
//
//   File Description:
//
//    SelectiveUndoDialog implementation.
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
#include "selective_undo_dialog.h"
#include "repository.h"
#include "tao_utf8.h"
#include <QMessageBox>

namespace Tao {

SelectiveUndoDialog::SelectiveUndoDialog(Repository *repo, QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a history dialog with a checkout button
// ----------------------------------------------------------------------------
    : HistoryDialog(repo, parent)
{
    setWindowTitle(tr("Selective undo"));
    msg->setText(tr("Please select a change and click "
                    "Undo to undo this change in the current document"));
    QPushButton *undoButton;
    undoButton = new QPushButton(tr("&Undo"));
    buttonBox->addButton(undoButton, QDialogButtonBox::ApplyRole);
    connect(undoButton, SIGNAL(clicked()),
            this, SLOT(undoButton_clicked()));
}


void SelectiveUndoDialog::undoButton_clicked()
// ----------------------------------------------------------------------------
//    Action for undo button
// ----------------------------------------------------------------------------
{
    QString id = revEdit->text();
    bool ok = repo->revert(+id);
    if (!ok)
    {
        int ret;
        ret = QMessageBox::warning(this, tr("Undo error"),
                                   tr("Selective undo failed.\n"
                                      "Select OK to restore the project to "
                                      "its previous state.\n"
                                      "Select Cancel to leave the project "
                                      "as is and resolve conflicts "
                                      "manually."),
                                   QMessageBox::Ok | QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel)
            return;

        repo->reset();
    }
}

}
