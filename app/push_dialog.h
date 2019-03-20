#ifndef PUSH_DIALOG_H
#define PUSH_DIALOG_H
// ****************************************************************************
//  push_dialog.h                                                  Tao project
// ****************************************************************************
//
//   File Description:
//
//    The class to display the "Push" dialog box
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

#include "fetch_push_dialog_base.h"

namespace Tao {

class PushDialog : public FetchPushDialogBase
{
    Q_OBJECT

public:
    PushDialog(Repository *repo, QWidget *parent = 0);

public slots:
    virtual void accept();
};

}

#endif // PUSH_DIALOG_H
