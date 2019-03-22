// *****************************************************************************
// selective_undo_dialog.cpp                                       Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

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
