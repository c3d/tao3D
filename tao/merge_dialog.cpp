// ****************************************************************************
//  merge_dialog.cpp                                           Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

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
