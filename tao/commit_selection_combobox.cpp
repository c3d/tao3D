// ****************************************************************************
//  commit_selection_combobox.cpp                                  Tao project
// ****************************************************************************
//
//   File Description:
//
//    A class to display a combobox that shows the commit log for a branch
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

#include "commit_selection_combobox.h"
#include "repository.h"
#include "application.h"
#include "tao_utf8.h"
#include <QInputDialog>
#include <QMessageBox>

namespace Tao {

CommitSelectionComboBox::CommitSelectionComboBox(QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a commit selection combo box
// ----------------------------------------------------------------------------
    : QComboBox(parent), repo(NULL), prevSelected(-1), mode(CSM_Default)
{
    connect(this, SIGNAL(activated(int)),
            this, SLOT(on_activated(int)));
    setEnabled(false);
}


void CommitSelectionComboBox::setRepository(Repository *repo)
// ----------------------------------------------------------------------------
//    Associate a repository to the combo box
// ----------------------------------------------------------------------------
{
    if (!repo)
        return;
    if (repo != this->repo)
    {
        clear();
        setEnabled(false);
    }
    this->repo = repo;
}


void CommitSelectionComboBox::setBranch(QString branch)
// ----------------------------------------------------------------------------
//    Select a branch and show the commit log for the branch
// ----------------------------------------------------------------------------
{
    this->branch = branch;
    populate();
    setEnabled(true);
}


Repository::Commit CommitSelectionComboBox::currentCommit()
// ----------------------------------------------------------------------------
//    The currently selected commit
// ----------------------------------------------------------------------------
{
    int index = currentIndex();
    Repository::Commit commit = itemData(index).value<Repository::Commit>();
    return commit;
}


void CommitSelectionComboBox::setMode(unsigned int mode)
// ----------------------------------------------------------------------------
//    Select display options 
// ----------------------------------------------------------------------------
{
    this->mode = mode;
}


void CommitSelectionComboBox::refresh()
// ----------------------------------------------------------------------------
//    Refresh commit list
// ----------------------------------------------------------------------------
{
    populate();
}


bool CommitSelectionComboBox::populate()
// ----------------------------------------------------------------------------
//    Read branches from repository, fill the combo box
// ----------------------------------------------------------------------------
{
    if (!repo || branch.isEmpty())
        return false;

    int i = 0;
    clear();
    prevSelected = -1;

    if (mode & CSM_HeadEntry)
    {
        Repository::Commit head = Repository::HeadCommit;
        addItem(head.toString());
        setItemData(i++, QVariant::fromValue(head));
    }

    QList<Repository::Commit> commits = repo->history(branch);
    // Commit list is in chronological order, we want latest first
    for (int n = commits.size() - 1; n >= 0; n--)
    {
        Repository::Commit commit = commits[n];
        QString txt;
        if (mode & CSM_CommitId)
        {
            txt += commit.id;
        }
        if (mode & CSM_CommitMsg)
        {
            if (!txt.isEmpty())
                txt += " ";
            txt += commit.msg;
        }
        if (!txt.isEmpty())
        {
            addItem(txt);
            setItemData(i++, QVariant::fromValue(commit));
        }
    }

    on_activated(currentIndex());
    return true;
}


void CommitSelectionComboBox::on_activated(int selected)
// ----------------------------------------------------------------------------
//    Process the selection by the user of an item of the combo box
// ----------------------------------------------------------------------------
{
    if (selected == prevSelected)
        return;

    prevSelected = selected;
    Repository::Commit commit = currentCommit();
    emit commitSelected(commit);
}


}
