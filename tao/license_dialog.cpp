// ****************************************************************************
//  license_dialog.cpp                                             Tao project
// ****************************************************************************
//
//   File Description:
//
//     Show a dialog box with license-related information, and (optionally)
//     a configurable message.
//
//
//
//
//
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2012 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2012 Taodyne SAS
// ****************************************************************************

#include "license_dialog.h"

#include "application.h"
#include "licence.h"
#include "tao_utf8.h"

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
    : QMessageBox(parent), message(message)
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
    if (dialogs.isEmpty())
    {
        show();
        raise();
    }
    dialogs.append(this);
}


void LicenseDialog::done(int r)
// ----------------------------------------------------------------------------
//   Close this dialog and show next one, if any
// ----------------------------------------------------------------------------
{
    QMessageBox::done(r);
    // NB: A dialog shown by exec() rather than showDialog() is NOT in dialogs
    if (dialogs.contains(this))
    {
        Q_ASSERT(dialogs.first() == this);
        LicenseDialog *next = dialogs.takeFirst();
        next->show();
        next->raise();
    }
}


void LicenseDialog::init()
// ----------------------------------------------------------------------------
//   Create dialog contents
// ----------------------------------------------------------------------------
{
    QString title = tr("Licensing");
    QString caption;
    caption = tr("<h3>Tao Presentation Licenses</h3>");

    QString prefix;
#if defined(Q_OS_WIN)
    prefix = "file:///";
#else
    prefix = "file://";
#endif
    QString msg = message;
    msg += tr(
                "<h3>To add new licenses</h3>"
                "<p>If you received license files (with the .taokey "
                "extension), copy them into the license folder and "
                "restart the application. Your new licenses will be "
                "loaded automatically.</p>"
                "<center><a href=\"%1%2\">"
                "Open the license folder</a></center>"
                "<h3>Your host identifier:</h3>"
                "<center>%3</center>"
                ).arg(prefix).arg(Application::userLicenseFolderPath())
                 .arg(+Licences::hostID());

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
}

}
