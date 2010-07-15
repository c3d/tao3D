// ****************************************************************************
//  pull_from_dialog.cpp                                           Tao project
// ****************************************************************************
//
//   File Description:
//
//    The class to display the "Pull from" dialog box
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
#include "pull_from_dialog.h"
#include "remote_selection_frame.h"
#include "repository.h"
#include <QInputDialog>

namespace Tao {

PullFromDialog::PullFromDialog(Repository *repo, QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a "remote" selection box for the given repository
// ----------------------------------------------------------------------------
    : QDialog(parent), repo(repo)
{
    setupUi(this);
    rsFrame->setRepository(repo, repo->pullFrom);
    rsFrame->setRole(RemoteSelectionFrame::RSF_Fetch);
    QString interval = QString("%1").arg(repo->pullInterval/1000);
    updateInterval->setText(interval);
}


QString PullFromDialog::pullFrom()
// ----------------------------------------------------------------------------
//    The name of the currently selected remote (empty string if "<None>")
// ----------------------------------------------------------------------------
{
    return rsFrame->remote();
}


Repository::ConflictResolution PullFromDialog::conflictResolution()
// ----------------------------------------------------------------------------
//    The currently selected conflict resolution mode
// ----------------------------------------------------------------------------
{
    if (ours->isChecked())
        return Repository::CR_Ours;
    return Repository::CR_Theirs;
}


int PullFromDialog::pullInterval()
// ----------------------------------------------------------------------------
//    The interval (in seconds) between each synchronization
// ----------------------------------------------------------------------------
{
    return updateInterval->text().toInt();
}


void PullFromDialog::accept()
// ----------------------------------------------------------------------------
//    Update the repository synchronization settings (URL, conflict mode)
// ----------------------------------------------------------------------------
{
    repo->pullFrom           = pullFrom();
    repo->conflictResolution = conflictResolution();
    repo->pullInterval       = pullInterval() * 1000;
    QDialog::accept();
}

}
