#ifndef SSH_ASK_PASS_DIALOG_H
#define SSH_ASK_PASS_DIALOG_H
// *****************************************************************************
// ssh_ask_pass_dialog.h                                           Tao3D project
// *****************************************************************************
//
// File description:
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
// (C) 2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010, Jérôme Forissier <jerome@taodyne.com>
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

#include <QDialog>

namespace Ui {
    class SshAskPassDialog;
}

class SshAskPassDialog : public QDialog {
    Q_OBJECT
public:
    SshAskPassDialog(QString text, QWidget *parent = 0);
    ~SshAskPassDialog();

    void setText(QString text);

protected:
    void changeEvent(QEvent *e);

protected slots:
    void accept();

private:
    Ui::SshAskPassDialog *ui;
};

#endif // SSH_ASK_PASS_DIALOG_H
