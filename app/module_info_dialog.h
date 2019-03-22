#ifndef MODULE_INFO_DIALOG_H
#define MODULE_INFO_DIALOG_H
// *****************************************************************************
// module_info_dialog.h                                            Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
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
