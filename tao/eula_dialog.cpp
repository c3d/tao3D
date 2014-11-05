// ****************************************************************************
//  eula_dialog.cpp                                                Tao project
// ****************************************************************************
//
//   File Description:
//
//    Display the End-User License Agreement dialog.
//
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
//  (C) 2012 Jerome Forissier <jerome@taodyne.com>
//  (C) 2012 Taodyne SAS
// ****************************************************************************

#include "eula_dialog.h"
#include "base.h"
#include "tao_utf8.h"

#include <QPushButton>
#include <QSettings>
#include <QVariant>

#include <iostream> // DEBUG

namespace Tao {

extern const char *GITREV_;

EulaDialog::EulaDialog(QWidget *parent)
// ----------------------------------------------------------------------------
//    Create dialog
// ----------------------------------------------------------------------------
    : QMessageBox(parent)
{
    QString title = tr("Tao3D");
    QString caption;
    caption = tr("<center><h3>"
                 "Tao3D End-User License Agreement"
                 "</h3></center>");

    QString msg;
#ifdef CFG_LIBRE_EDITION
    msg += tr( "<p>Thank you for your interest in Tao3D.</p>"
               "<p>This software is licensed to you under the terms "
               "of the GNU General Public License v3. Please make "
               "sure that you agree with these terms before using "
               "the software. You can read the license by clicking "
               "on the following link:"
               "<center><a href=\"%1\">%1</a></center>"
               "<p>If the link does not open, please copy the "
               "address in a web browser.</p>"
               "<p><b>By clicking on the \"I Accept\" button, you express your "
               "consent to the above terms and conditions.</b></p>"
               "<p>You must accept the agreement to use Tao3D.</p>"
               ).arg(tr("http://www.gnu.org/copyleft/gpl.html"));
#else // !CFG_LIBRE_EDITION
    msg += tr( "<p>Thank you for your interest in Tao3D.</p>"
               "<p>Please read the terms of the license agreement "
               "by clicking the following link: </p>"
               "<center><a href=\"%1\">%1</a></center>"
               "<p>If the link does not open, please copy the "
               "address in a web browser.</p>"
               "<p><b>By clicking on the \"I Accept\" button, you express your "
               "consent to the above terms and conditions.</b></p>"
               "<p>You must accept the agreement to use Tao3D.</p>"
               ).arg(tr("http://taodyne.com/eula"));
#endif // CFG_LIBRE_EDITION

    setWindowTitle(title);
    setText(caption);
    setInformativeText(msg);

    setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

    button(QMessageBox::Ok)->setText(tr("I Accept"));
    QPushButton *rej = dynamic_cast<QPushButton*>(button(QMessageBox::Cancel));
    Q_ASSERT(rej);
    setDefaultButton(rej);

    // Icons by LazyCrazy http://lazycrazy.deviantart.com/
    // License: free for commercial use
    // Downloaded from:
    // http://www.iconfinder.com/icondetails/33993/128/document_contract_signature_icon
    QPixmap pm(":/images/contract.png");
    QPixmap icon(pm.scaled(64, 64, Qt::IgnoreAspectRatio,
                                        Qt::SmoothTransformation));
    setIconPixmap(icon);
}


EulaDialog::~EulaDialog()
// ----------------------------------------------------------------------------
//    Destroy dialog
// ----------------------------------------------------------------------------
{
}


void EulaDialog::done(int r)
// ----------------------------------------------------------------------------
//    Button was clicked or dialog was dismissed somehow
// ----------------------------------------------------------------------------
{
    if (r == QMessageBox::Ok)
    {
        QString accepted(GITREV_);
        IFTRACE(settings)
            debug() << "EULA accepted, saving version: '"
                    << +accepted << "'\n";
        QSettings().setValue(EULA_SETTING_NAME, QVariant(accepted));
    }
    QMessageBox::done(r);
}


bool EulaDialog::previouslyAccepted()
// ----------------------------------------------------------------------------
//    True if user has accepted EULA on a previous run
// ----------------------------------------------------------------------------
{
    QString accepted = QSettings().value(EULA_SETTING_NAME).toString();
    QString current = QString(GITREV_);
    IFTRACE(settings)
        debug() << "Tao versions: current '" << GITREV_ << "', "
                << "EULA previously accepted: '" << +accepted << "'\n";
    return (accepted == current);
}


void EulaDialog::resetAccepted()
// ----------------------------------------------------------------------------
//   Forget that user has accepted EULA for any version
// ----------------------------------------------------------------------------
{
    IFTRACE(settings)
        debug() << "Clearing saved version\n";
    QSettings().remove(EULA_SETTING_NAME);
}


std::ostream & EulaDialog::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[EulaDialog] ";
    return std::cerr;
}

}
