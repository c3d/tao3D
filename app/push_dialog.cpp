// *****************************************************************************
// push_dialog.cpp                                                 Tao3D project
// *****************************************************************************
//
// File description:
//
//    The class to display the "Publish to" dialog box
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
// (C) 2010-2011,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
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
#include "tao_utf8.h"
#include "push_dialog.h"
#include "remote_selection_frame.h"
#include "repository.h"
#include <QInputDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>

namespace Tao {

PushDialog::PushDialog(Repository *repo, QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a "remote" selection box for the given repository
// ----------------------------------------------------------------------------
    : FetchPushDialogBase(repo, parent)
{
    setWindowTitle(tr("Push"));
    chooseRemoteLabel->setText(tr("Choose the remote project you want to "
                                  "push to:"));
    rsFrame->setRepository(repo, repo->lastPublishTo);
    rsFrame->setRole(RemoteSelectionFrame::RSF_Push);
}


void PushDialog::accept()
// ----------------------------------------------------------------------------
//    Publish the current project to the previously chosen remote
// ----------------------------------------------------------------------------
{
    QApplication::setOverrideCursor(Qt::BusyCursor);

    proc = repo->asyncPush(repo->lastPublishTo = Url());
    if (!proc)
        return;
    connectSignalsAndSlots();
    (void)repo->dispatch(proc);
}

}
