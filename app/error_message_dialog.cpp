// *****************************************************************************
// error_message_dialog.cpp                                        Tao3D project
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
// (C) 2010-2011,2013, Jérôme Forissier <jerome@taodyne.com>
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

#include "error_message_dialog.h"
#include "ui_error_message_dialog.h"
#include <QSettings>
#include <QVariant>

namespace Tao {

ErrorMessageDialog::ErrorMessageDialog(QWidget *parent) :
// ----------------------------------------------------------------------------
//    Create an error message dialog
// ----------------------------------------------------------------------------
    QDialog(parent),
    ui(new Ui::ErrorMessageDialog)
{
    ui->setupUi(this);
}


ErrorMessageDialog::~ErrorMessageDialog()
// ----------------------------------------------------------------------------
//    Destroy error message dialog
// ----------------------------------------------------------------------------
{
    delete ui;
}


void ErrorMessageDialog::changeEvent(QEvent *e)
// ----------------------------------------------------------------------------
//    Handle language change notification
// ----------------------------------------------------------------------------
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


bool ErrorMessageDialog::showMessage(const QString & message,
                                     bool doNotShowAgain)
// ----------------------------------------------------------------------------
//    Display given message if not forbidden by saved setting
// ----------------------------------------------------------------------------
{
    this->message = message;
    QVariant var(QSettings().value(EMD_SETTING_NAME));
    prefs = var.toStringList();
    if (prefs.contains(message))
        return false;
    ui->message->setText(message);
    ui->doNotShowAgain->setChecked(doNotShowAgain);
    adjustSize();
    exec();
    return true;
}


void ErrorMessageDialog::accept()
// ----------------------------------------------------------------------------
//    OK button was clicked
// ----------------------------------------------------------------------------
{
    bool nomore = ui->doNotShowAgain->isChecked();
    if (nomore)
    {
        prefs.append(message);
        QVariant var(prefs);
        QSettings().setValue(EMD_SETTING_NAME, var);
    }
    QDialog::accept();
}

}
