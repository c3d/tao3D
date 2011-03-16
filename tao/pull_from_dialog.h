#ifndef PULL_FROM_DIALOG_H
#define PULL_FROM_DIALOG_H
// ****************************************************************************
//  pull_from_dialog.h                                             Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "remote_selection_frame.h"
#include "ui_pull_from_dialog.h"
#include "repository.h"
#include <QDialog>
#include <QWidget>

namespace Tao {

struct Repository;

class PullFromDialog : public QDialog, private Ui::PullFromDialog
{
    Q_OBJECT

public:
    PullFromDialog(Repository *repo, QWidget *parent = 0);

public:
    QString                        pullFrom();
    Repository::ConflictResolution conflictResolution();
    int                            pullInterval();

public slots:
    virtual void accept();

private:
    Repository           *repo;
};

}

#endif // PULL_FROM_DIALOG_H
