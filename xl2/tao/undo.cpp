// ****************************************************************************
//  undo.cpp                                                       Tao project
// ****************************************************************************
//
//   File Description:
//
//     The command manipulated by the undo stack. Represents a commit in the
//     repository.
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
        repo->asyncCherryPick(+id);
}


void UndoCommand::undo()
// ----------------------------------------------------------------------------
//    Reverts a change to the document.
// ----------------------------------------------------------------------------
{
    IFTRACE(undo)
        std::cerr << "Undo " << +id << " " << +text() << "\n";
    if (repo)
        repo->asyncRevert(+id);
}


}
