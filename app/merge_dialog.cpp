// *****************************************************************************
// merge_dialog.cpp                                                Tao3D project
// *****************************************************************************
//
// File description:
//
//    The class to display the "Merge" dialog box
//
//
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

#include "tao.h"
#include "merge_dialog.h"
#include "repository.h"
#include "branch_selection_combobox.h"
#include "ui_merge_dialog.h"
#include "tao_utf8.h"
#include <QMessageBox>


namespace Tao {

MergeDialog::MergeDialog(Repository *repo, QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a "Merge" dialog box
// ----------------------------------------------------------------------------
    : QDialog(parent), repo(repo)
{
    setupUi(this);
    connect(srcBranch, SIGNAL(branchSelected(QString)),
            srcCommit, SLOT(setBranch(QString)));
    srcCommit->setRepository(repo);
    srcBranch->setRepository(repo);
    dstBranch->setFilter(BranchSelectionComboBox::BSF_LocalBranches);
    dstBranch->setRepository(repo);
}


Repository::ConflictResolution MergeDialog::conflictResolution()
// ----------------------------------------------------------------------------
//    The currently selected conflict resolution mode
// ----------------------------------------------------------------------------
{
    if (manual->isChecked())
        return Repository::CR_Manual;
    if (ours->isChecked())
        return Repository::CR_Ours;
    if (theirs->isChecked())
        return Repository::CR_Theirs;
    return Repository::CR_Unknown;
}


void MergeDialog::accept()
// ----------------------------------------------------------------------------
//    Do the merge
// ----------------------------------------------------------------------------
{
    text current = repo->branch();
    text from    = +(srcBranch->branch());
    text to      = +(dstBranch->branch());
    Repository::Commit             commit = srcCommit->currentCommit();
    Repository::ConflictResolution how    = conflictResolution();

    bool needswitch = (current != to);
    if (needswitch)
        repo->checkout(to);

    text what;
    if (commit != Repository::HeadCommit)
        what = +(commit.id);
    else
        what = from;

    bool ok = repo->merge(what, how);
    if (!ok)
    {
        int ret;
        ret = QMessageBox::warning(this, tr("Merge error"),
                                   tr("Automatic merge failed.\n"
                                      "Select OK to restore the project to "
                                      "its previous state.\n"
                                      "Select Cancel to leave the project "
                                      "half-merged and resolve conflicts "
                                      "manually."),
                                   QMessageBox::Ok | QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel)
            return QDialog::accept();

        repo->reset();
    }

    if (needswitch)
        repo->checkout(current);

    QDialog::accept();
}

}
