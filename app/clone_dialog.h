#ifndef CLONE_DIALOG_H
#define CLONE_DIALOG_H
// *****************************************************************************
// clone_dialog.h                                                  Tao3D project
// *****************************************************************************
//
// File description:
//
//    The class to display the "Clone" dialog box
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
// Tao3D is free software: you can redistribute it and/or modify
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

#include "ui_clone_dialog.h"
#include "repository.h"
#include <QDialog>
#include <QPushButton>

namespace Tao {

struct Repository;

class CloneDialog : public QDialog, private Ui::CloneDialog
{
    Q_OBJECT

public:
    CloneDialog(QWidget *parent = 0);

public slots:
    virtual void accept();
    virtual void reject();
    virtual void on_browseButton_clicked();
    virtual void on_folderEdit_textEdited();
    virtual void on_urlEdit_textEdited();
    virtual void endClone(void *id, QString projPath);
    virtual void dismiss();

public:
    QString      projectPath;

private:
    void         enableOkCancel();
    void         dismissShortly();

private:
    repository_ptr repo;
    bool           okToDismiss;
    QPushButton   *okButton, *cancelButton, *detailsButton;
    process_p      proc;
};

}

#endif // CLONE_DIALOG_H
