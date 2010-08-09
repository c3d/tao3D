#ifndef OPEN_URI_DIALOG_H
#define OPEN_URI_DIALOG_H
// ****************************************************************************
//  open_uri_dialog.h                                              Tao project
// ****************************************************************************
//
//   File Description:
//
//    The class to display the "Open URI..." dialog box
//
//
//
//
//
//
//
//
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "ui_open_uri_dialog.h"
#include <QDialog>
#include <QWidget>

namespace Tao {

class OpenUriDialog : public QDialog, private Ui::OpenUriDialog
{
    Q_OBJECT

public:
    OpenUriDialog(QWidget *parent = 0);

public slots:
    virtual void accept();

public:
    QString        uri;
};

}

#endif // OPEN_URI_DIALOG_H
