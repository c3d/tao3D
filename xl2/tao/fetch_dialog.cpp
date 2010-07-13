// ****************************************************************************
//  fetch_dialog.cpp                                               Tao project
// ****************************************************************************
//
//   File Description:
//
//    The class to display the "Fetch" dialog box
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
#include "fetch_dialog.h"
#include "remote_selection_frame.h"
#include "repository.h"
#include <QInputDialog>
#include <QDialogButtonBox>
#include <QPushButton>

namespace Tao {

FetchDialog::FetchDialog(Repository *repo, QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a "remote" selection box for the given repository
// ----------------------------------------------------------------------------
    : QDialog(parent), repo(repo)
{
    setupUi(this);
    rsFrame->setRepository(repo, repo->lastFetchUrl);
    rsFrame->setRole(RemoteSelectionFrame::RSF_Fetch);
}


QString FetchDialog::fetchUrl()
// ----------------------------------------------------------------------------
//    The name of the currently selected remote (empty string if "<None>")
// ----------------------------------------------------------------------------
{
    return rsFrame->remote();
}


void FetchDialog::accept()
// ----------------------------------------------------------------------------
//    Publish the current project to the previously chosen remote
// ----------------------------------------------------------------------------
{
    repo->fetch(repo->lastFetchUrl = fetchUrl());
    QDialog::accept();
}

void FetchDialog::on_rsFrame_noneSelected()
// ----------------------------------------------------------------------------
//    Disable the OK button if remote is not set
// ----------------------------------------------------------------------------
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}


void FetchDialog::on_rsFrame_nameSelected()
// ----------------------------------------------------------------------------
//    Disable the OK button if remote is not set
// ----------------------------------------------------------------------------
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

}
