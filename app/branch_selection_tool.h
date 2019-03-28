#ifndef BRANCH_SELECTION_TOOL_H
#define BRANCH_SELECTION_TOOL_H
// *****************************************************************************
// branch_selection_tool.h                                         Tao3D project
// *****************************************************************************
//
// File description:
//
//    A floating tool window with a branch selection combobox, and other
//    widgets to show/control the current state of the document.
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

#include "tool_window.h"
#include <QObject>
#include <QString>

QT_BEGIN_NAMESPACE
class QWidget;
class QLineEdit;
class QLabel;
QT_END_NAMESPACE

namespace Tao {

class  BranchSelectionComboBox;
struct Repository;

class BranchSelectionTool : public ToolWindow
// ----------------------------------------------------------------------------
//    Tool for branch selection, task description, project URL
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    BranchSelectionTool(const QString &title, QWidget *parent = 0,
                        const QString &objName = "");

public slots:
    void    setRepository(Repository *repo);
    void    refresh();
    void    showProjectUrl(QString url);

signals:
    void    checkedOut(QString branch);
    void    taskDescriptionSet(QString desc);

protected slots:
    void    checkout(QString branch);
    void    setRepoTaskDescription();

private:
    BranchSelectionComboBox * branchSelector;
    Repository              * repo;
    QLineEdit               * taskLineEdit;
    QLabel                  * projectUrl;
};

}

#endif // BRANCH_SELECTION_TOOL_H
