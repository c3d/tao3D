// *****************************************************************************
// undo.cpp                                                        Tao3D project
// *****************************************************************************
//
// File description:
//
//     The command manipulated by the undo stack. Represents a commit in the
//     repository.
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

#include "undo.h"
#include "options.h"
#include "tao_utf8.h"

#include <QDebug>
#include <iostream>

namespace Tao {

UndoCommand::UndoCommand(Repository *repo, QString id, QString msg)
// ----------------------------------------------------------------------------
//    Create an object that represents a repository commit
// ----------------------------------------------------------------------------
    : QUndoCommand(), repo(repo), id(id), msg(msg), firstRedo(true)
{
    setText(msg);
}


void UndoCommand::redo()
// ----------------------------------------------------------------------------
//    Applies a change to the document.
// ----------------------------------------------------------------------------
{
    if (firstRedo)
    {
        // Kludge: The action was already applied when UndoCommand object is
        // created. Since Qt always calls redo() when the command is pushed
        // onto the undo stack, we just ignore the first invocation.
        firstRedo = false;
        return;
    }
    IFTRACE(undo)
        std::cerr << "Redo " << +id << " " << +text() << "\n";
    if (repo)
        repo->cherryPick(+id);
}


void UndoCommand::undo()
// ----------------------------------------------------------------------------
//    Reverts a change to the document.
// ----------------------------------------------------------------------------
{
    IFTRACE(undo)
        std::cerr << "Undo " << +id << " " << +text() << "\n";
    if (repo)
        repo->revert(+id);
}


}
