// ****************************************************************************
//  nag_screen.cpp                                                 Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This software is licensed under the GNU General Public License v3.
// See file COPYING for details.
//  (C) 2013 Jerome Forissier <jerome@taodyne.com>
//  (C) 2013 Taodyne SAS
// ****************************************************************************

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
        QUrl url(tr("http://taodyne.com/taopresentations/buynow"));
        QDesktopServices::openUrl(url);
    }
}

}
