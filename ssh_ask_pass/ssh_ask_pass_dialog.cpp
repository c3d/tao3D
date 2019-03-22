// *****************************************************************************
// ssh_ask_pass_dialog.cpp                                         Tao3D project
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
#include "ssh_ask_pass_dialog.h"
#include "ui_ssh_ask_pass_dialog.h"
#include <iostream>

SshAskPassDialog::SshAskPassDialog(QString text, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SshAskPassDialog)
{
    ui->setupUi(this);
    ui->label->setText(text);
    ui->label->adjustSize();
    connect(ui->buttonBox, SIGNAL(rejected()),
            this, SLOT(reject()));
    connect(ui->buttonBox, SIGNAL(accepted()),
            this, SLOT(accept()));
}


SshAskPassDialog::~SshAskPassDialog()
{
    delete ui;
}


void SshAskPassDialog::changeEvent(QEvent *e)
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


void SshAskPassDialog::accept()
{
    std::cout << ui->lineEdit->text().toStdString() << "\n";
    QDialog::accept();
}
