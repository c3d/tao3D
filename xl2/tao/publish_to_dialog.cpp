// ****************************************************************************
//  publish_to_dialog.cpp                                          Tao project
// ****************************************************************************
//
//   File Description:
//
//    The class to display the "Publish to" dialog box
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
#include "publish_to_dialog.h"
#include "remote_selection_frame.h"
#include "repository.h"
#include <QInputDialog>
#include <QDialogButtonBox>
#include <QPushButton>

namespace Tao {

PublishToDialog::PublishToDialog(Repository *repo, QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a "remote" selection box for the given repository
// ----------------------------------------------------------------------------
    : QDialog(parent), repo(repo)
{
    setupUi(this);
    rsFrame->setRepository(repo, repo->lastPublishTo);
}


QString PublishToDialog::pushTo()
// ----------------------------------------------------------------------------
//    The name of the currently selected remote (empty string if "<None>")
// ----------------------------------------------------------------------------
{
    return rsFrame->remote();
}


void PublishToDialog::accept()
// ----------------------------------------------------------------------------
//    Publish the current project to the previously chosen remote
// ----------------------------------------------------------------------------
{
    repo->push(repo->lastPublishTo = pushTo());
    QDialog::accept();
}

void PublishToDialog::on_rsFrame_noneSelected()
// ----------------------------------------------------------------------------
//    Disable the OK button if remote is not set
// ----------------------------------------------------------------------------
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}


void PublishToDialog::on_rsFrame_nameSelected()
// ----------------------------------------------------------------------------
//    Disable the OK button if remote is not set
// ----------------------------------------------------------------------------
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

}
