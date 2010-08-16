#ifndef UNDO_H
#define UNDO_H
// ****************************************************************************
//  undo.h                                                         Tao project
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

#include "repository.h"
#include <QUndoCommand>

namespace Tao {

class UndoCommand : public QUndoCommand
// ----------------------------------------------------------------------------
//   A command that can be undone and redone
// ----------------------------------------------------------------------------
{

public:
    UndoCommand(Repository *repo, QString commitId, QString msg = "");
    virtual ~UndoCommand() {}

public:
    virtual void undo();
    virtual void redo();

private:
    Repository *repo;
    QString     id;         // commit ID
    QString     msg;        // commit message
    bool        firstRedo;
};

}

#endif // UNDO_H
