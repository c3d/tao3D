// *****************************************************************************
// license_dialog.cpp                                              Tao3D project
// *****************************************************************************
//
// File description:
//
//     Show a dialog box with license-related information, and (optionally)
//     a configurable message.
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2012-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
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

#include "license_dialog.h"

#include "application.h"
#include "license.h"
#include "tao_utf8.h"

#include <QAbstractButton>
#include <QClipboard>
#include <QPixmap>

namespace Tao {

// First element is the one currently shown, each new dialog is shown when the
// previous one is closed.
QList<LicenseDialog *> LicenseDialog::dialogs;


LicenseDialog::LicenseDialog(QWidget *parent)
    : QMessageBox(parent)
// ----------------------------------------------------------------------------
//   Create a license dialog with general information about licenses
// ----------------------------------------------------------------------------
{
    init();
}


LicenseDialog::LicenseDialog(const QString &message, QWidget *parent)
    : QMessageBox(parent),  message(message)
// ----------------------------------------------------------------------------
//   Create the license dialog with general info and a custom (HTML) message
// ----------------------------------------------------------------------------
{
    init();
}


void LicenseDialog::showDialog()
// ----------------------------------------------------------------------------
//   Show dialog to the user (possibly after other similar dialogs are closed)
// ----------------------------------------------------------------------------
{
    setWindowModality(Qt::NonModal);
    if (dialogs.isEmpty())
    {
        show();
        raise();
    }
    dialogs.append(this);
}


void LicenseDialog::done(int r)
// ----------------------------------------------------------------------------
//   Maybe close this dialog and show next one, if any
// ----------------------------------------------------------------------------
{
    if (r == QMessageBox::Apply)
    {
#ifndef CFG_NO_LICENSE
        // Copy host ID into clipboard and keep dialog open
        QApplication::clipboard()->setText(+Licenses::hostID());
#endif // CFG_NO_LICENSE
        return;
    }

    QMessageBox::done(r);
    // NB: A dialog shown by exec() rather than showDialog() is NOT in dialogs
    if (dialogs.contains(this))
    {
        XL_ASSERT(dialogs.first() == this);
        dialogs.removeFirst();
        if (!dialogs.isEmpty())
        {
            LicenseDialog *next = dialogs.first();
            next->show();
            next->raise();
        }
    }
}


void LicenseDialog::init()
// ----------------------------------------------------------------------------
//   Create dialog contents
// ----------------------------------------------------------------------------
{
    QString title = tr("Licensing");
    QString caption;
    caption = tr("<h3>Tao3D Licenses</h3>");

    QString prefix;
#if defined(Q_OS_WIN)
    prefix = "file:///";
#else
    prefix = "file://";
#endif
    QString msg = message;

#ifndef CFG_NO_LICENSE
    msg += tr(
                "<h3>To add new licenses</h3>"
                "<p>If you received license files (with the .taokey "
                " or .taokey.src "
                "extension), copy them into the license folder and "
                "restart the application. Your new licenses will be "
                "loaded automatically.</p>"
                "<center><a href=\"%1%2\">"
                "Open the license folder</a></center>"
                "<h3>Your host identifier:</h3>"
                "<center>%3</center>"
                ).arg(prefix).arg(Application::userLicenseFolderPath())
                 .arg(+Licenses::hostID());
#else // CFG_NO_LICENSE
#ifndef CFG_LIBRE_EDITION
#warning "The GPL version should be compiled with CFG_LIBRE_EDITION set"
#endif
    msg += tr("<h3>GNU General Public License v3</h3>"
              "<p>This program is licensed to you under the GNU "
              "General Public License (GPL) version 3.<br/>"
              "A copy of this license should have been provided to you "
              "with the software.<br/>"
              "It is also available "
              "<a href=\"http://www.gnu.org/copyleft/gpl.html\">on-line</a>.");
#endif // CFG_NO_LICENSE

    setWindowTitle(title);
    setText(caption);
    setInformativeText(msg);

    // The padlock icon is a merge of:
    // http://www.openclipart.org/detail/17931 (public domain)
    // and our Tao pictogram
    // NB: Don't create pixmap from SVG here. See #1891.
    QPixmap *pm = Application::padlockIcon;
    if (pm && !pm->isNull())
        setIconPixmap(*pm);

#ifndef CFG_NO_LICENSE
    addButton(QMessageBox::Apply);
    button(QMessageBox::Apply)->setText(tr("Copy host ID to clipboard"));
#endif

    addButton(QMessageBox::Ok);
    setDefaultButton(QMessageBox::Ok);
    setEscapeButton(QMessageBox::Ok);
}

}
