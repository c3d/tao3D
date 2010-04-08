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
    populateNameComboAndSelect(repo->pullFrom);
}


QString PullFromDialog::pullFrom()
// ----------------------------------------------------------------------------
//    The name of the currently selected remote (empty string if "<None>")
// ----------------------------------------------------------------------------
{
    QString result;
    if (nameCombo->currentIndex())
        result = nameCombo->currentText();
    return result;
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

bool PullFromDialog::populateNameCombo()
// ----------------------------------------------------------------------------
//    Read remotes from repository, fill the combo box
// ----------------------------------------------------------------------------
{
    nameCombo->clear();
    nameCombo->addItem(tr("<None>"), CIK_None);
    nameCombo->addItems(repo->remotes());
    for (int i = 1; i < nameCombo->count(); i++)
        nameCombo->setItemData(i, CIK_Name);
    nameCombo->insertSeparator(nameCombo->count());
    nameCombo->addItem(tr("Add new..."), CIK_AddNew);

    return true;
}


bool PullFromDialog::populateNameComboAndSelect(QString sel)
// ----------------------------------------------------------------------------
//    Re-fill remotes combo box and select a name, or <None> if sel == ""
// ----------------------------------------------------------------------------
{
    // The contents of the combo box depends on the newly selected item.
    // The combo is split in two sections:
    // (1) the list of remotes, including "<None>", and
    // (2) a list of actions (add/rename/delete).
    // If "<None>" is selected, only the "Add" action is present ; if a name is
    // chosen, the delete and rename actions are added, too.

    if (!populateNameCombo())
        return false;

    if (sel.isEmpty())
    {
        urlEdit->clear();
        paramsWidget->setEnabled(false);
        return true;
    }

    int index = nameCombo->findText(sel);
    if (index == -1)
        return false;

    int kind = nameCombo->itemData(index).toInt();
    if (kind != CIK_None && kind != CIK_Name)
        return false;

    if (kind == CIK_Name)
    {
        prevSelected = sel;
        nameCombo->addItem(tr("Rename %1...").arg(sel), CIK_Rename);
        nameCombo->addItem(tr("Delete %1...").arg(sel), CIK_Delete);
        index = nameCombo->findText(sel);
        if (index == -1)
            return false;
        nameCombo->setCurrentIndex(index);
        // Also update URL field
        urlEdit->setText(repo->remotePullUrl(sel));
        paramsWidget->setEnabled(true);
    }

    return true;
}


void PullFromDialog::on_nameCombo_activated(QString selected)
// ----------------------------------------------------------------------------
//    Process the selection by the user of an item of the combo box
// ----------------------------------------------------------------------------
{
    int index = nameCombo->currentIndex();
    int kind  = nameCombo->itemData(index).toInt();

    switch (kind)
    {
    case CIK_None:
        populateNameComboAndSelect("");
        break;

    case CIK_Name:
        populateNameComboAndSelect(selected);
        break;

    case CIK_AddNew:
        populateNameComboAndSelect(addNewRemote());
        urlEdit->setFocus();
        break;

    case CIK_Rename:
        populateNameComboAndSelect(renameRemote());
        break;

    case CIK_Delete:
        repo->delRemote(prevSelected);
        populateNameComboAndSelect("");
        break;
    }
}


void PullFromDialog::on_urlEdit_editingFinished()
// ----------------------------------------------------------------------------
//    Update the remote URL in the repository to reflect the URL field.
// ----------------------------------------------------------------------------
{
    repo->setRemote(nameCombo->currentText(), urlEdit->text());
}


void PullFromDialog::accept()
// ----------------------------------------------------------------------------
//    Update the repository synchronization settings (URL, conflict mode)
// ----------------------------------------------------------------------------
{
    repo->pullFrom           = pullFrom();
    repo->conflictResolution = conflictResolution();
    QDialog::accept();
}


QString PullFromDialog::addNewRemote()
// ----------------------------------------------------------------------------
//    Prompt user for name of a new remote and create it with an empty URL
// ----------------------------------------------------------------------------
{
    QString result = QInputDialog::getText(this, tr("New remote"),
                                           tr("Enter the name of the new "
                                              "remote:"));
    if (!result.isEmpty())
        repo->addRemote(result, "");

    return result;
}


QString PullFromDialog::renameRemote()
// ----------------------------------------------------------------------------
//    Prompt user for a new name and rename previously selected remote
// ----------------------------------------------------------------------------
{
    if (prevSelected.isEmpty())
        return "";

    QString result;
    result = QInputDialog::getText(this, tr("Rename remote"),
                                   tr("Enter the new name for %1:")
                                   .arg(prevSelected));
    if (!result.isEmpty())
        repo->renRemote(prevSelected, result);

    return result;
}

}
