#ifndef FETCH_PUSH_DIALOG_BASE_H
#define FETCH_PUSH_DIALOG_BASE_H
// *****************************************************************************
// fetch_push_dialog_base.h                                        Tao3D project
// *****************************************************************************
//
// File description:
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
