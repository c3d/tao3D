// *****************************************************************************
// branch_selection_tool.cpp                                       Tao3D project
// *****************************************************************************
//
// File description:
//
//    BranchSelectionTool implementation
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

#include "branch_selection_tool.h"
#include "branch_selection_combobox.h"
#include "repository.h"
#include "tao_utf8.h"

#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QFormLayout>


namespace Tao {

BranchSelectionTool::BranchSelectionTool(const QString & title,
                                         QWidget * parent,
                                         const QString & objName)
// ----------------------------------------------------------------------------
//    Create a branch selection tool window
// ----------------------------------------------------------------------------
    : ToolWindow(title, parent, objName)
{
    branchSelector = new BranchSelectionComboBox;
    branchSelector->setFilter(BranchSelectionComboBox::BSF_All);
    connect(branchSelector, SIGNAL(branchSelected(QString)),
            this, SLOT(checkout(QString)));
    branchSelector->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    branchSelector->setSizePolicy(QSizePolicy::MinimumExpanding,
                                  QSizePolicy::Fixed);

    taskLineEdit = new QLineEdit;
    taskLineEdit->setEnabled(false);

    projectUrl = new QLabel(tr("None"));
    projectUrl->setTextInteractionFlags(Qt::TextSelectableByMouse);
    projectUrl->setToolTip(tr("Shows the URL of the project for the current "
                              "document"));

    QFormLayout *formLayout = new QFormLayout;
    formLayout->setSizeConstraint(QLayout::SetMinimumSize);
    formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    formLayout->addRow(tr("Branch:"), branchSelector);
    formLayout->addRow(tr("Task:"), taskLineEdit);
    formLayout->addRow(tr("Project URL:"), projectUrl);
    setLayout(formLayout);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setMinimumWidth(250);
}


void BranchSelectionTool::setRepository(Repository *repo)
// ----------------------------------------------------------------------------
//    Select a repository and update the UI accordingly
// ----------------------------------------------------------------------------
{
    this->repo = repo;
    branchSelector->setRepository(repo);
    taskLineEdit->setEnabled(true);
    connect(taskLineEdit, SIGNAL(editingFinished()),
            this, SLOT(setRepoTaskDescription()));
    connect(this, SIGNAL(taskDescriptionSet(QString)),
            repo, SLOT(setTaskDescription(QString)));
}


void BranchSelectionTool::setRepoTaskDescription()
// ----------------------------------------------------------------------------
//    Emit signal when new task description is ready
// ----------------------------------------------------------------------------
{
    emit taskDescriptionSet(taskLineEdit->text());
}


void BranchSelectionTool::refresh()
// ----------------------------------------------------------------------------
//    Update the UI when current branch was changed externally
// ----------------------------------------------------------------------------
{
    branchSelector->refresh();
}


void BranchSelectionTool::checkout(QString branch)
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


void BranchSelectionTool::showProjectUrl(QString url)
// ----------------------------------------------------------------------------
//    Update the project URL area
// ----------------------------------------------------------------------------
{
    projectUrl->setText(url);
}

}
