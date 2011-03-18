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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
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
    historyFrame->setMessage(tr("Please select a change and click "
                    "Undo to undo this change in the current document"));
    QPushButton *undoButton;
    undoButton = new QPushButton(tr("&Undo"));
    buttonBox->addButton(undoButton, QDialogButtonBox::ApplyRole);
    connect(undoButton, SIGNAL(clicked()), this, SLOT(undo()));
}


void SelectiveUndoDialog::undo()
// ----------------------------------------------------------------------------
//    Action for undo button
// ----------------------------------------------------------------------------
{
    QString id = historyFrame->rev();
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
