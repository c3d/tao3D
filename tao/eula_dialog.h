// ****************************************************************************
//  eula_dialog.h                                                  Tao project
// ****************************************************************************
//
//   File Description:
//
//    Display the End-User License Agreement dialog.
//
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

#ifndef EULA_DIALOG_H
#define EULA_DIALOG_H

#include <QMessageBox>
#include <iostream>

namespace Tao {

class EulaDialog : public QMessageBox
{
    Q_OBJECT

public:
    EulaDialog(QWidget *parent = 0);
    virtual ~EulaDialog();

public:
    static bool     previouslyAccepted();
    static void     resetAccepted();

public slots:
    virtual void    done(int);

protected:
    static
    std::ostream &  debug();

#   define          EULA_SETTING_NAME  "EulaAccepted"
};

}

#endif // EULA_DIALOG_H
