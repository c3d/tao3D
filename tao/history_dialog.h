#ifndef HISTORY_DIALOG_H
#define HISTORY_DIALOG_H
// ****************************************************************************
//  history_dialog.h                                               Tao project
// ****************************************************************************
//
//   File Description:
//
//    The class to display the "History" dialog box, which is used to select
//    a commit. Contains a branch selection combo box, a table widget to show
//    the commit log, and a free text edit area for current commit ID.
//    Used as a base class for dialogs like Checkout and Selective Undo.
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

#include "ui_history_dialog.h"
#include "repository.h"
#include <QDialog>
#include <QWidget>
#include <QPushButton>

namespace Tao {

struct Repository;

class HistoryDialog : public QDialog, protected Ui::HistoryDialog
{
    Q_OBJECT

public:
    HistoryDialog(Repository *repo, QWidget *parent = 0);
    QString rev();

protected:
    Repository  *repo;
};

}

#endif // HISTORY_DIALOG_H
