// *****************************************************************************
// nag_screen.cpp                                                  Tao3D project
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
// (C) 2014-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
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

#include "nag_screen.h"
#include <QAbstractButton>
#include <QUrl>
#include <QDesktopServices>

namespace Tao {

NagScreen::NagScreen(QWidget *parent)
// ----------------------------------------------------------------------------
//   Constructor
// ----------------------------------------------------------------------------
    : QMessageBox(parent)
{
    setWindowTitle(tr("Tao3D"));
    setText(tr("<h3>Reminder</h3>"));
    setInformativeText(tr("<p>This is an evaluation copy of "
                          "Tao3D.<p>"));

    setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    button(QMessageBox::Ok)->setText(tr("Buy now"));
    button(QMessageBox::Cancel)->setText(tr("Buy later"));
    setDefaultButton(QMessageBox::Cancel);

    // Icon from:
    // http://www.iconfinder.com/icondetails/61809/64/buy_cart_ecommerce_shopping_webshop_icon
    // Author: Ivan M. (www.visual-blast.com)
    // License: free for commercial use, do not redistribute
    QPixmap pm(":/images/shopping_cart.png");
    setIconPixmap(pm);

    connect(this, SIGNAL(finished(int)), this, SLOT(processDialogStatus(int)));
}


void NagScreen::processDialogStatus(int st)
{
    if (st == QMessageBox::Ok)
    {
        QUrl url(tr("http://www.taodyne.com/taopresentations/buynow"));
        QDesktopServices::openUrl(url);
    }
}

}
