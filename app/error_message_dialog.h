// *****************************************************************************
// error_message_dialog.h                                          Tao3D project
// *****************************************************************************
//
// File description:
//
//    Display a message dialog with a "Do not show this message again" option.
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

#ifndef ERROR_MESSAGE_DIALOG_H
#define ERROR_MESSAGE_DIALOG_H

#include "ui_error_message_dialog.h"
#include <QDialog>
#include <QStringList>
#include <QString>

namespace Tao {

class ErrorMessageDialog : public QDialog {
    Q_OBJECT
public:
    ErrorMessageDialog(QWidget *parent = 0);
    ~ErrorMessageDialog();

    bool showMessage(const QString & message, bool dontShowAgain = false);

public slots:
    void accept();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ErrorMessageDialog *ui;
    QString                 message;
    QStringList             prefs;
#   define                  EMD_SETTING_NAME "DoNotShowThisMessageAgain"
};

}

#endif // ERROR_MESSAGE_DIALOG_H
