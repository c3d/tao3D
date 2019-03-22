#ifndef GIT_TOOLBAR_H
#define GIT_TOOLBAR_H
// *****************************************************************************
// git_toolbar.h                                                   Tao3D project
// *****************************************************************************
//
// File description:
//
//    Toolbar with buttons to show/hide git-related tool windows
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
// (C) 2010,2014-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
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
#include "history_playback_tool.h"
#include <QToolBar>

QT_BEGIN_NAMESPACE
class QToolButton;
class QWidget;
QT_END_NAMESPACE

namespace Tao {

struct Repository;

struct GitToolBar : public QToolBar
{
    Q_OBJECT

public:
    GitToolBar(QWidget *parent = 0);
    GitToolBar(const QString &title, QWidget *parent = 0);

public slots:
    void    setRepository(Repository *repo);
    void    refresh();
    void    showProjectUrl(QString url);

signals:
    void    checkedOut(QString branch);
    void    taskDescriptionSet(QString desc);
    void    documentChanged();

private:
    void    init(QWidget *parent);
    void    attachToolWindowToButton(ToolWindow *w, QToolButton* b);

private:
    BranchSelectionTool * branchSelectionTool;
    HistoryPlaybackTool * historyPlaybackTool;
    QToolButton *         branchSelection;
    QToolButton *         historyPlayback;
};

}

#endif // GIT_TOOLBAR_H
