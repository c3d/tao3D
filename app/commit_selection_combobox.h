#ifndef COMMIT_SELECTION_COMBOBOX_H
#define COMMIT_SELECTION_COMBOBOX_H
// *****************************************************************************
// commit_selection_combobox.h                                     Tao3D project
// *****************************************************************************
//
// File description:
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

#include "repository.h"
#include <QComboBox>
#include <QWidget>
#include <QString>

namespace Tao {

class CommitSelectionComboBox : public QComboBox
{
    Q_OBJECT

public:
    enum Mode
    // ------------------------------------------------------------------------
    //   Display options: select what to show in the combo box
    // ------------------------------------------------------------------------
    {
        CSM_HeadEntry  = 1,  // Show "HEAD" entry
        CSM_CommitId   = 2,  // Show commit identifier
        CSM_CommitMsg  = 4,  // Show commit message
        CSM_Default    = (CSM_HeadEntry | CSM_CommitId | CSM_CommitMsg),
    };

public:
    CommitSelectionComboBox(QWidget *parent = 0);

public:
    Repository::Commit currentCommit();
    void               refresh();
    void               setMode(unsigned int mode);

public slots:
    void    setRepository(Repository *repo);
    void    setBranch(QString branch);

signals:
    void    commitSelected(Repository::Commit commit);

protected slots:
    void    on_activated(int selected);

private:
    bool    populate();

private:
    Repository * repo;
    QString      branch;
    int          prevSelected;
    unsigned int mode;
};

}

#endif // COMMIT_SELECTION_COMBOBOX_H
