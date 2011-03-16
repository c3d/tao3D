#ifndef FETCH_PUSH_DIALOG_BASE_H
#define FETCH_PUSH_DIALOG_BASE_H
// ****************************************************************************
//  fetch_push_dialog_base.h                                       Tao project
// ****************************************************************************
//
//   File Description:
//
//    Abstract base class for fetch and push dialogs
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
#include "ui_fetch_push_dialog.h"
#include "repository.h"
#include <QDialog>
#include <QWidget>
#include <QProcess>


namespace Tao {

class FetchPushDialogBase : public QDialog, protected Ui::FetchPushDialog
{
    Q_OBJECT

public:
    FetchPushDialogBase(Repository *repo, QWidget *parent = 0);

public:
    QString      Url();

public slots:
    virtual void accept() = 0;
    virtual void reject();
    void         on_rsFrame_noneSelected();
    void         on_rsFrame_nameSelected();

protected:
    void         dismissShortly();
    void         connectSignalsAndSlots();

protected slots:
    void         onFinished(int code, QProcess::ExitStatus status);
    void         onError(QProcess::ProcessError error);
    void         dismiss();

protected:
    Repository * repo;
    process_p    proc;
    bool         aborted;
};

}

#endif // FETCH_PUSH_DIALOG_BASE_H
