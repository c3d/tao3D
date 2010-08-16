// ****************************************************************************
//  error_message_dialog.cpp                                       Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

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
