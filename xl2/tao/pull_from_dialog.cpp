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
#include "main.h"
#include <QInputDialog>

namespace Tao {

PullFromDialog::PullFromDialog(Repository *repo, QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a "remote" selection box for the given repository
// ----------------------------------------------------------------------------
    : QDialog(parent), repo(repo)
{
    XL::Main *xlr = XL::MAIN;
    setupUi(this);
    rsFrame->setRepository(repo, repo->pullFrom);
    QString i = QString("%1").arg(xlr->options.pull_interval/1000);
    updateInterval->setText(i);
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


int PullFromDialog::syncInterval()
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
    XL::Main *xlr = XL::MAIN;
    repo->pullFrom           = pullFrom();
    repo->conflictResolution = conflictResolution();
    xlr->options.pull_interval = syncInterval() * 1000;
    QDialog::accept();
}

}
