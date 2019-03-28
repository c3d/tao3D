#ifndef NAG_SCREEN_H
#define NAG_SCREEN_H
// *****************************************************************************
// nag_screen.h                                                    Tao3D project
// *****************************************************************************
//
// File description:
//
//    A dialog box to remind the user that the application is not free and
//    propose a "Buy" option.
//
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2013-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2012-2013, Jérôme Forissier <jerome@taodyne.com>
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

#include <QMessageBox>

namespace Tao {

class NagScreen : public QMessageBox
// ----------------------------------------------------------------------------
//   Remind user that Tao is not free
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    NagScreen(QWidget *parent = 0);
    ~NagScreen() {}

protected slots:
    virtual void processDialogStatus(int st);
};

}

#endif // NAG_SCREEN_H
