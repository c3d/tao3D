#ifndef MERGE_DIALOG_H
#define MERGE_DIALOG_H
// ****************************************************************************
//  merge_dialog.h                                                 Tao project
// ****************************************************************************
//
//   File Description:
//
//    The class to display the "Merge" dialog box
//
//
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

#include "ui_merge_dialog.h"
#include "repository.h"
#include <QDialog>
#include <QWidget>

namespace Tao {

struct Repository;

class MergeDialog : public QDialog, private Ui::MergeDialog
{
    Q_OBJECT

public:
    MergeDialog(Repository *repo, QWidget *parent = 0);

public:
    Repository::ConflictResolution conflictResolution();

public slots:
    virtual void accept();

private:
    Repository           *repo;
};

}

#endif // MERGE_DIALOG_H
