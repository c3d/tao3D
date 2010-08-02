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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
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
