#ifndef NAG_SCREEN_H
#define NAG_SCREEN_H
// ****************************************************************************
//  nag_screen.h                                                   Tao project
// ****************************************************************************
//
//   File Description:
//
//    A dialog box to remind the user that the application is not free and
//    propose a "Buy" option.
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
//  (C) 2013 Jerome Forissier <jerome@taodyne.com>
//  (C) 2013 Taodyne SAS
// ****************************************************************************

#include <QMessageBox>

namespace Tao {

class NagScreen : public QMessageBox
// ----------------------------------------------------------------------------
//   Remind user that Tao is not free
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    NagScreen(QWidget *parent = 0);
    ~NagScreen() {}

protected slots:
    virtual void processDialogStatus(int st);
};

}

#endif // NAG_SCREEN_H
