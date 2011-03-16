// ****************************************************************************
//  branch_selection_tool.h                                        Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

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
