#ifndef RENDER_TO_FILE_DIALOG_H
#define RENDER_TO_FILE_DIALOG_H
// *****************************************************************************
// render_to_file_dialog.h                                         Tao3D project
// *****************************************************************************
//
// File description:
//
//    Display a dialog box to get offline rendering parameters (ouput
//    directory, time range, frames per second, etc.)
//
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2013-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011,2013, Jérôme Forissier <jerome@taodyne.com>
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

#include "ui_render_to_file_dialog.h"
#include <QDialog>

QT_BEGIN_NAMESPACE
class QPushButton;
QT_END_NAMESPACE

namespace Tao {

struct Widget;

class RenderToFileDialog : public QDialog, private Ui::RenderToFileDialog
{
    Q_OBJECT

public:
    RenderToFileDialog(Widget *widget, QWidget *parent = 0);

public slots:
    virtual void on_browseButton_clicked();
    virtual void accept();
    virtual void reject();
    virtual void dismiss();
    virtual void done(int x) { QDialog::done(x); }
    virtual void done();
    virtual void failed();

public:
    QString      folder, fileName;
    int          x, y;
    double       start;      // Unix time ; -1 = current date/time
    double       duration;   // in seconds
    double       fps;
    int          startPage;  // 0 = current page
    double       timeOffset; // initial value of page_time in startPage

private:
    void         dismissShortly();

private:
    Widget        *widget;
    bool           okToDismiss;
    QPushButton   *okButton;
    bool           rendering;
};

}

#endif // RENDER_TO_FILE_DIALOG_H
