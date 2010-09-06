// ****************************************************************************
//  branch_selection_toolbar.cpp                                   Tao project
// ****************************************************************************
//
//   File Description:
//
//    A class to display a toolbar that contains:
//      * a label
//      * a branch selection combo box; select a branch to check it out
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

#include "branch_selection_toolbar.h"
#include "repository.h"
#include "tao_utf8.h"
#include <QLabel>


namespace Tao {

BranchSelectionToolBar::BranchSelectionToolBar(QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a branch selection combo box with given parent
// ----------------------------------------------------------------------------
    : QToolBar(parent)
{
    init();
}


BranchSelectionToolBar::BranchSelectionToolBar(const QString &title, QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a branch selection combo box with given title and parent
// ----------------------------------------------------------------------------
    : QToolBar(title, parent)
{
    init();
}


void BranchSelectionToolBar::init()
// ----------------------------------------------------------------------------
//    Initialize the widgets in the toolbar
// ----------------------------------------------------------------------------
{
    addWidget(new QLabel(tr("Branch:")));

    branchSelector = new BranchSelectionComboBox;
    connect(branchSelector, SIGNAL(branchSelected(QString)),
            this, SLOT(checkout(QString)));
    branchSelector->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    addWidget(branchSelector);
}


void BranchSelectionToolBar::setRepository(Repository *repo)
// ----------------------------------------------------------------------------
//    Select a repository and update the UI accordingly
// ----------------------------------------------------------------------------
{
    this->repo = repo;
    branchSelector->setRepository(repo);
}


void BranchSelectionToolBar::refresh()
// ----------------------------------------------------------------------------
//    Update the UI when current branch was changed externally
// ----------------------------------------------------------------------------
{
    branchSelector->refresh();
}


void BranchSelectionToolBar::checkout(QString branch)
// ----------------------------------------------------------------------------
//    Checkout a branch
// ----------------------------------------------------------------------------
{
    if (+branch == repo->branch())
        return;
    bool ok = repo->checkout(+branch);
    if (ok)
    {
        branchSelector->refresh();
        emit checkedOut(branch);
    }
}

}
