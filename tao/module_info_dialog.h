#ifndef MODULE_INFO_DIALOG_H
#define MODULE_INFO_DIALOG_H
// ****************************************************************************
//  module_info_dialog.h                                           Tao project
// ****************************************************************************
//
//   File Description:
//
//    Display a dialog box with information about a Tao module
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
//  (C) 2011 Jerome Forissier <jerome@taodyne.com>
//  (C) 2011 Taodyne SAS
// ****************************************************************************

#include "module_info.h"
#include <QDialog>
#include <QWidget>
#include <QTextBrowser>
#include <QDialogButtonBox>
#include <QVBoxLayout>

namespace Tao {

class ModuleInfoDialog : public QDialog
{
    Q_OBJECT

public:
    ModuleInfoDialog(const ModuleInfo &info, QWidget *parent = 0);

protected:
    void    updateInfo(const ModuleInfo &info);
    QString autoLinkUrls(QString str);

protected slots:
    virtual void openUrl(QUrl url);

private:
    QVBoxLayout      *layout;
    QTextBrowser     *textArea;
    QDialogButtonBox *buttonBox;
};

}

#endif // MODULE_INFO_DIALOG_H
