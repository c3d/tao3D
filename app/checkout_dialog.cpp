// *****************************************************************************
// checkout_dialog.cpp                                             Tao3D project
// *****************************************************************************
//
// File description:
//
//    CheckoutDialog implementation.
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

#include "tao.h"
#include "checkout_dialog.h"
#include "repository.h"
#include "tao_utf8.h"

namespace Tao {

CheckoutDialog::CheckoutDialog(Repository *repo, QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a history dialog with a checkout button
// ----------------------------------------------------------------------------
    : HistoryDialog(repo, parent)
{
    setWindowTitle(tr("Checkout"));
    historyFrame->setMessage(tr("Please select a version of the document and "
                    "click Checkout to bring a copy into a temporary branch"));
    QPushButton *checkoutButton;
    checkoutButton = new QPushButton(tr("&Checkout"));
    buttonBox->addButton(checkoutButton, QDialogButtonBox::ApplyRole);
    connect(checkoutButton, SIGNAL(clicked()), this, SLOT(checkout()));
}


void CheckoutDialog::checkout()
// ----------------------------------------------------------------------------
//    Action for checkout button
// ----------------------------------------------------------------------------
{
    QString id = historyFrame->rev();
    bool ok = repo->checkout(+id);
    if (ok)
        emit checkedOut(id);
}

}
