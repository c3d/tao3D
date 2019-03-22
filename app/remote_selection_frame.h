#ifndef REMOTE_SELECTION_FRAME_H
#define REMOTE_SELECTION_FRAME_H
// *****************************************************************************
// remote_selection_frame.h                                        Tao3D project
// *****************************************************************************
//
// File description:
//
//    A class to display a frame that allows users to select a remote
//    repository. This frame can be used ina dialog box.
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

#include "ui_remote_selection_frame.h"
#include <QFrame>
#include <QWidget>
#include <QString>

namespace Tao {

struct Repository;

class RemoteSelectionFrame : public QFrame, private Ui::RemoteSelectionFrame
{
    Q_OBJECT

public:
    enum PushOrFetch
    // ------------------------------------------------------------------------
    //   Should we display the push or the fetch URL of the remotes?
    // ------------------------------------------------------------------------
    {
        RSF_Unknown,
        RSF_Push,
        RSF_Fetch
    };

public:
    RemoteSelectionFrame(QWidget *parent = 0);

public:
    void    setRepository(Repository *repo, QString preferredRemote = "");
    void    setRole(PushOrFetch whatFor);
    QString remote();

signals:
    void    noneSelected();
    void    nameSelected();

private slots:
    void    on_nameCombo_activated(QString selected);
    void    on_urlEdit_editingFinished();

private:
    bool    populateNameCombo();
    bool    populateNameComboAndSelect(QString sel);
    QString addNewRemote();
    QString renameRemote();

private:
    enum ComboItemKind
    // ------------------------------------------------------------------------
    //    Values to identify each entry of the name combo box
    // ------------------------------------------------------------------------
    {
        CIK_None,   // The "<None>" item
        CIK_Name,   // Any valid remote name
        CIK_AddNew, // The "Add new..." item
        CIK_Delete, // The "Delete <name>" item
        CIK_Rename, // The "Rename <name>" item
    };

private:
    Repository * repo;
    QString      prevSelected;
    PushOrFetch  whatFor;
};

}

#endif // REMOTE_SELECTION_FRAME_H
