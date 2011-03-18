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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao.h"
#include "tao_utf8.h"
#include "fetch_dialog.h"
#include "remote_selection_frame.h"
#include "repository.h"
#include <QInputDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>

namespace Tao {

FetchDialog::FetchDialog(Repository *repo, QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a "remote" selection box for the given repository
// ----------------------------------------------------------------------------
    : FetchPushDialogBase(repo, parent)
{
    setWindowTitle(tr("Fetch"));
    chooseRemoteLabel->setText(tr("Choose the remote project you want to "
                                  "fetch from:"));
    rsFrame->setRepository(repo, repo->lastFetchUrl);
    rsFrame->setRole(RemoteSelectionFrame::RSF_Fetch);
}


void FetchDialog::accept()
// ----------------------------------------------------------------------------
//    Fetch from the previously chosen remote
// ----------------------------------------------------------------------------
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    proc = repo->asyncFetch(repo->lastFetchUrl = Url());
    if (!proc)
        return;
    connectSignalsAndSlots();
    (void)repo->dispatch(proc);
}

}
