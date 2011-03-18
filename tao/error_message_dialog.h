// ****************************************************************************
//  error_message_dialog.h                                         Tao project
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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#ifndef ERROR_MESSAGE_DIALOG_H
#define ERROR_MESSAGE_DIALOG_H

#include "ui_error_message_dialog.h"
#include <QDialog>
#include <QStringList>
#include <QString>

namespace Tao {

class ErrorMessageDialog : public QDialog {
    Q_OBJECT
public:
    ErrorMessageDialog(QWidget *parent = 0);
    ~ErrorMessageDialog();

    bool showMessage(const QString & message, bool dontShowAgain = true);

public slots:
    void accept();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ErrorMessageDialog *ui;
    QString                 message;
    QStringList             prefs;
#   define                  EMD_SETTING_NAME "DoNotShowThisMessageAgain"
};

}

#endif // ERROR_MESSAGE_DIALOG_H
