#ifndef CHECKOUT_DIALOG_H
#define CHECKOUT_DIALOG_H
// ****************************************************************************
//  checkout_dialog.h                                              Tao project
// ****************************************************************************
//
//   File Description:
//
//    The class to display the "Revert to" dialog box. This is a modeless
//    dialog which enables to checkout any past version of the document into a
//    temporary branch
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

#include "history_dialog.h"
#include "repository.h"
#include <QWidget>
#include <QPushButton>

namespace Tao {

struct Repository;

class CheckoutDialog : public HistoryDialog
{
    Q_OBJECT

public:
    CheckoutDialog(Repository *repo, QWidget *parent = 0);

signals:
    void   checkedOut(QString id);

private slots:
    void   checkoutButton_clicked();
};

}

#endif // CHECKOUT_DIALOG_H
