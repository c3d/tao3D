#ifndef LICENSE_DIALOG_H
#define LICENSE_DIALOG_H
// ****************************************************************************
//  license_dialog.h                                               Tao project
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
// This software is licensed under the GNU General Public License v3.
// See file COPYING for details.
//  (C) 2012 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2012 Taodyne SAS
// ****************************************************************************

#include <QMessageBox>
#include <QList>

namespace Tao {

class LicenseDialog : public QMessageBox
{
    Q_OBJECT

public:
    LicenseDialog(QWidget *parent = NULL);
    LicenseDialog(const QString &message, QWidget *parent = NULL);

    void           showDialog();

public slots:
    void           done(int r);

private:
    void           init();

private:
    QString        message;

private:
    static QList<LicenseDialog *> dialogs;
};

}

#endif
