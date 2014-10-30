#ifndef RENDER_TO_FILE_DIALOG_H
#define RENDER_TO_FILE_DIALOG_H
// ****************************************************************************
//  render_to_file_dialog.h                                        Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This software is licensed under the GNU General Public License v3.
// See file COPYING for details.
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

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
