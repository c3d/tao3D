// ****************************************************************************
//  open_uri_dialog.h                                              Tao project
// ****************************************************************************
//
//   File Description:
//
//    Implementation of OpenUriDialog
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

#include "tao.h"
#include "open_uri_dialog.h"
#include "application.h"

#include <QCompleter>

namespace Tao {

OpenUriDialog::OpenUriDialog(QWidget *parent)
// ----------------------------------------------------------------------------
//    Create an "Open URI" dialog box
// ----------------------------------------------------------------------------
    : QDialog(parent)
{
    setupUi(this);
    uriEdit->setCompleter(new QCompleter(TaoApp->urlCompletions(), this));
}


void OpenUriDialog::accept()
// ----------------------------------------------------------------------------
//    Make URI string accessible, add it to the completion list, and close
// ----------------------------------------------------------------------------
{
    uri = uriEdit->text();
    TaoApp->addUrlCompletion(uri);
    QDialog::accept();
}

}
