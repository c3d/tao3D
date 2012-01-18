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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2012 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2012 Taodyne SAS
// ****************************************************************************

#include <QMessageBox>

namespace Tao {

class LicenseDialog : public QMessageBox
{
    Q_OBJECT

public:
    LicenseDialog(QWidget *parent = NULL);
    LicenseDialog(const QString &message, QWidget *parent = NULL);

private:
    void           init();

private:
    QString        message;
};

}

#endif
