#ifndef BRANCH_SELECTION_COMBOBOX_H
#define BRANCH_SELECTION_COMBOBOX_H
// *****************************************************************************
// branch_selection_combobox.h                                     Tao3D project
// *****************************************************************************
//
// File description:
//
//    A class to display a combobox that allows users to:
//      * select an existing repository branch
//      * rename a branch
//      * delete a branch
//      * create a new branch
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
// Tao3D is free software: you can redistribute it and/or modify
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

#include <QComboBox>
#include <QWidget>
#include <QString>

namespace Tao {

struct Repository;

class BranchSelectionComboBox : public QComboBox
{
    Q_OBJECT

public:
    enum Filter
    // ------------------------------------------------------------------------
    //   Branch selection filter: which branches will appear in the combobox
    // ------------------------------------------------------------------------
    {
        BSF_LocalBranches  = 1,
        BSF_RemoteBranches = 2,
        BSF_AllBranches    = (BSF_LocalBranches | BSF_RemoteBranches),
        BSF_Commands       = 4,
        BSF_All            = (BSF_AllBranches | BSF_Commands),
    };

public:
    BranchSelectionComboBox(QWidget *parent = 0);

public:
    QString branch();
    void    refresh();
    void    setFilter(unsigned int filter);

public slots:
    void    setRepository(Repository *repo, QString preferredBranch = "");

signals:
    void    noneSelected();
    void    branchSelected(QString name);

protected slots:
    void    on_activated(QString selected);

private:
    bool    populate();
    bool    populateAndSelect(QString sel = "", bool sig = true);
    QString addNewBranch();
    QString renameBranch();

private:
    enum ComboItemKind
    // ------------------------------------------------------------------------
    //    Values to identify each entry of the name combo box
    // ------------------------------------------------------------------------
    {
        CIK_None,   // The "<None>" item
        CIK_Name,   // Any valid branch name
        CIK_AddNew, // The "New branch..." item
        CIK_Delete, // The "Delete <name>" item
        CIK_Rename, // The "Rename <name>" item
    };

private:
    Repository * repo;
    QString      prevSelected;
    unsigned int filter;
};

}

#endif // BRANCH_SELECTION_COMBOBOX_H
