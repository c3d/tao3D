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
#include "tao_utf8.h"
#include "publish_to_dialog.h"
#include "remote_selection_frame.h"
#include "repository.h"
#include <QInputDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>

namespace Tao {

PublishToDialog::PublishToDialog(Repository *repo, QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a "remote" selection box for the given repository
// ----------------------------------------------------------------------------
    : FetchPushDialogBase(repo, parent)
{
    setWindowTitle(tr("Publish"));
    chooseRemoteLabel->setText(tr("Choose the remote project you want to "
                                  "publish to:"));
    rsFrame->setRepository(repo, repo->lastPublishTo);
    rsFrame->setRole(RemoteSelectionFrame::RSF_Push);
}


void PublishToDialog::accept()
// ----------------------------------------------------------------------------
//    Publish the current project to the previously chosen remote
// ----------------------------------------------------------------------------
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    proc = repo->asyncPush(repo->lastPublishTo = Url());
    if (!proc)
        return;
    connectSignalsAndSlots();
    (void)repo->dispatch(proc);
}

}
