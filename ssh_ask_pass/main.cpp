// *****************************************************************************
// main.cpp                                                        Tao3D project
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
#include <QApplication>
#include <QFileInfo>
#include <QMessageBox>
#include "ssh_ask_pass_dialog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    if (argc != 2)
    {
        QFileInfo fi(argv[0]);
        QString usage =
                QCoreApplication::tr("Usage: %1 <message>")
                    .arg(fi.fileName());
        QMessageBox::critical(NULL, QCoreApplication::tr("Error"),
                              usage);
        return 1;
    }
    QString question(argv[1]);
    SshAskPassDialog w(question);
    w.show();
    return a.exec();
}
