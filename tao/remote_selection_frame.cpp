// ****************************************************************************
//  remote_selection_frame.cpp                                     Tao project
// ****************************************************************************
//
//   File Description:
//
//    A class to display a frame that allows users to select a remote
//    repository.
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

#include "remote_selection_frame.h"
#include "repository.h"
#include "application.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QCompleter>

namespace Tao {

RemoteSelectionFrame::RemoteSelectionFrame(QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a "remote" selection frame
// ----------------------------------------------------------------------------
    : QFrame(parent), repo(NULL), whatFor(RSF_Unknown)
{
    setupUi(this);
    urlEdit->setCompleter(new QCompleter(TaoApp->urlCompletions(), this));
}


void RemoteSelectionFrame::setRepository(Repository *repo, QString sel)
// ----------------------------------------------------------------------------
//    Associate a repository to the frame and update the UI accordingly
// ----------------------------------------------------------------------------
{
    this->repo = repo;
    populateNameComboAndSelect(sel);
}


void RemoteSelectionFrame::setRole(PushOrFetch whatFor)
// ----------------------------------------------------------------------------
//    Do we select a remote and an URL to push or to pull/fetch?
// ----------------------------------------------------------------------------
{
    this->whatFor = whatFor;
}


QString RemoteSelectionFrame::remote()
// ----------------------------------------------------------------------------
//    The name of the currently selected remote (empty string if "<None>")
// ----------------------------------------------------------------------------
{
    QString result;
    if (nameCombo->currentIndex())
        result = nameCombo->currentText();
    return result;
}


bool RemoteSelectionFrame::populateNameCombo()
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


bool RemoteSelectionFrame::populateNameComboAndSelect(QString sel)
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
        urlEdit->setEnabled(false);
        emit noneSelected();
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
        QString url;
        if (whatFor == RSF_Fetch)
            url = repo->remoteFetchUrl(sel);
        else
            url = repo->remotePushUrl(sel);
        urlEdit->setText(url);
        urlEdit->setEnabled(true);
        emit nameSelected();
    }
    else if (kind == CIK_None)
    {
        emit noneSelected();
    }

    return true;
}


void RemoteSelectionFrame::on_nameCombo_activated(QString selected)
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


void RemoteSelectionFrame::on_urlEdit_editingFinished()
// ----------------------------------------------------------------------------
//    Update the remote URL in the repository to reflect the URL field.
// ----------------------------------------------------------------------------
{
    QString url = urlEdit->text();
    repo->setRemote(nameCombo->currentText(), url);
    TaoApp->addUrlCompletion(url);
}


QString RemoteSelectionFrame::addNewRemote()
// ----------------------------------------------------------------------------
//    Prompt user for name of a new remote and create it with an empty URL
// ----------------------------------------------------------------------------
{
    QString name;

again:
    QString result = QInputDialog::getText(this, tr("New remote"),
                                           tr("Enter the name of the new "
                                              "remote:"), QLineEdit::Normal,
                                           name);
    name = result;
    if (name.replace(" ", "") != result)
    {
        QMessageBox::warning(this, tr("Invalid name"),
                             tr("Remote name cannot contain spaces"));
        goto again;
    }

    if (!result.isEmpty())
        repo->addRemote(result, "");

    return result;
}


QString RemoteSelectionFrame::renameRemote()
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
