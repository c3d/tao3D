#ifndef UNDO_H
#define UNDO_H
// ****************************************************************************
//  undo.h                                                         Tao project
// ****************************************************************************
//
//   File Description:
//
//     The command manipulated by the undo stack.
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

#include <QUndoCommand>

namespace Tao {

class UndoCommand : public QUndoCommand
// ----------------------------------------------------------------------------
//   A command that can be undone and redone
// ----------------------------------------------------------------------------
{

public:
    UndoCommand(QString &commitId): QUndoCommand(), commitId(commitId) {}
    virtual ~UndoCommand() {}

public:
//    void    undo();
//    void    redo();

private:
    QString commitId;
};

}

#endif // UNDO_H
