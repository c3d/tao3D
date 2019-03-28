#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H
// *****************************************************************************
// login_dialog.h                                                  Tao3D project
// *****************************************************************************
//
// File description:
//
//    A dialog box that allows the user to enter his username and password.
//    Adapted from http://qt-project.org/wiki/UserLoginDialogExample
//    License: CC BY-SA 2.5 (http://creativecommons.org/licenses/by-sa/2.5/)
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2013, Jérôme Forissier <jerome@taodyne.com>
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
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QComboBox>
#include <QGridLayout>
#include <QStringList>

namespace Tao {

class LoginDialog : public QDialog
{
    Q_OBJECT

private:
    QLabel *            labelDescription;
    QLabel *            labelUsername;
    QLabel *            labelPassword;

    QComboBox *         comboUsername;
    QLineEdit *         editPassword;
    QDialogButtonBox *  buttons;

    void setUpGUI();

public:
    LoginDialog(QWidget *parent = 0);

    void setDescription(QString description);
    void setUsername(QString username);
    void setPassword(QString password);
    void setUsernamesList(const QStringList &usernames);

signals:
    void acceptLogin(QString &username, QString &password, int &index);

public slots:
    void slotAcceptLogin();
};

}

#endif // LOGIN_DIALOG_H
