// *****************************************************************************
// pull_from_dialog.cpp                                            Tao3D project
// *****************************************************************************
//
// File description:
//
//    The class to display the "Pull from" dialog box
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
#include "pull_from_dialog.h"
#include "remote_selection_frame.h"
#include "repository.h"
#include <QInputDialog>

namespace Tao {

PullFromDialog::PullFromDialog(Repository *repo, QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a "remote" selection box for the given repository
// ----------------------------------------------------------------------------
    : QDialog(parent), repo(repo)
{
    setupUi(this);
    rsFrame->setRepository(repo, repo->pullFrom);
    rsFrame->setRole(RemoteSelectionFrame::RSF_Fetch);
    QString interval = QString("%1").arg(repo->pullInterval/1000);
    updateInterval->setText(interval);
}


QString PullFromDialog::pullFrom()
// ----------------------------------------------------------------------------
//    The name of the currently selected remote (empty string if "<None>")
// ----------------------------------------------------------------------------
{
    return rsFrame->remote();
}


Repository::ConflictResolution PullFromDialog::conflictResolution()
// ----------------------------------------------------------------------------
//    The currently selected conflict resolution mode
// ----------------------------------------------------------------------------
{
    if (ours->isChecked())
        return Repository::CR_Ours;
    return Repository::CR_Theirs;
}


int PullFromDialog::pullInterval()
// ----------------------------------------------------------------------------
//    The interval (in seconds) between each synchronization
// ----------------------------------------------------------------------------
{
    return updateInterval->text().toInt();
}


void PullFromDialog::accept()
// ----------------------------------------------------------------------------
//    Update the repository synchronization settings (URL, conflict mode)
// ----------------------------------------------------------------------------
{
    repo->pullFrom           = pullFrom();
    repo->conflictResolution = conflictResolution();
    repo->pullInterval       = pullInterval() * 1000;
    QDialog::accept();
}

}
