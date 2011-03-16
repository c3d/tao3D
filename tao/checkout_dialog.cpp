// ****************************************************************************
//  checkout_dialog.cpp                                            Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

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
