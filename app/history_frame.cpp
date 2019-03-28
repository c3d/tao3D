// *****************************************************************************
// history_frame.cpp                                               Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
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
