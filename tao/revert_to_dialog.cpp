// ****************************************************************************
//  revert_to_dialog.Cpp                                           Tao project
// ****************************************************************************
//
//   File Description:
//
//    RevertToDialog implementation.
//
//
//
//
//
//
//
//
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao.h"
#include "revert_to_dialog.h"
#include "repository.h"
#include "tao_utf8.h"

namespace Tao {

RevertToDialog::RevertToDialog(Repository *repo, QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a history dialog with a checkout button
// ----------------------------------------------------------------------------
    : HistoryDialog(repo, parent)
{
    setWindowTitle(tr("Revert to"));
    msg->setText(tr("Please select a version of the document and click "
                    "Checkout to bring a copy into a temporary branch"));
    QPushButton *checkoutButton;
    checkoutButton = new QPushButton(tr("&Checkout"));
    buttonBox->addButton(checkoutButton, QDialogButtonBox::ApplyRole);
    connect(checkoutButton, SIGNAL(clicked()),
            this, SLOT(checkoutButton_clicked()));
}


void RevertToDialog::checkoutButton_clicked()
// ----------------------------------------------------------------------------
//    Action for checkout button
// ----------------------------------------------------------------------------
{
    QString id = revEdit->text();
    bool ok = repo->checkout(+id);
    if (ok)
        emit checkedOut(id);
}

}
