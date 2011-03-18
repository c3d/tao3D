#ifndef SELECTIVE_UNDO_DIALOG_H
#define SELECTIVE_UNDO_DIALOG_H
// ****************************************************************************
//  selective_undo_dialog.cpp                                      Tao project
// ****************************************************************************
//
//   File Description:
//
//    The class to display the Selective Undo dialog box. This is a modeless
//    dialog which enables to pick any past change and revert it in the current
//    document.
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

#include "history_dialog.h"

namespace Tao {

struct Repository;

class SelectiveUndoDialog : public HistoryDialog
{
    Q_OBJECT

public:
    SelectiveUndoDialog(Repository *repo, QWidget *parent = 0);

private slots:
    void    undo();
};

}

#endif // SELECTIVE_UNDO_DIALOG_H
