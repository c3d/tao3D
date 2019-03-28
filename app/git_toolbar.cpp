// *****************************************************************************
// git_toolbar.cpp                                                 Tao3D project
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
// (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2012, Jérôme Forissier <jerome@taodyne.com>
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

#include "git_toolbar.h"
#include "branch_selection_tool.h"
#include "history_playback_tool.h"
#include "tao_utf8.h"
#include <QToolButton>
#include <QIcon>


namespace Tao {

GitToolBar::GitToolBar(QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a toolbar with buttons to show/hide git-related tools
// ----------------------------------------------------------------------------
    : QToolBar(parent), branchSelectionTool(NULL), historyPlaybackTool(NULL)
{
    init(parent);
}


GitToolBar::GitToolBar(const QString &title, QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a branch selection combo box with given title and parent
// ----------------------------------------------------------------------------
    : QToolBar(title, parent), branchSelectionTool(NULL),
    historyPlaybackTool(NULL)
{
    init(parent);
}


void GitToolBar::init(QWidget *parent)
// ----------------------------------------------------------------------------
//    Initialize the toolbar: buttons and tool windows
// ----------------------------------------------------------------------------
{
#define TGTB(x) "Tao::GitToolBar::" #x

    branchSelectionTool = new BranchSelectionTool(tr("Branch selection"),
                                                  parent,
                                                  TGTB("branchSelectionTool"));

    connect(branchSelectionTool, SIGNAL(checkedOut(QString)),
            this,                SIGNAL(checkedOut(QString)));
    connect(branchSelectionTool, SIGNAL(taskDescriptionSet(QString)),
            this,                SIGNAL(taskDescriptionSet(QString)));

    branchSelection = new QToolButton;
    branchSelection->setToolTip(tr("Show/hide the branch selection tool"));
    branchSelection->setIcon(QIcon(":/images/branch_tool.png"));
    branchSelection->setText(tr("Branch"));
    branchSelection->setCheckable(true);

    attachToolWindowToButton(branchSelectionTool, branchSelection);


    historyPlaybackTool = new HistoryPlaybackTool(tr("History Playback"),
                                                  parent,
                                                  TGTB("historyPlaybackTool"));

    connect(historyPlaybackTool, SIGNAL(documentChanged()),
            this,                SIGNAL(documentChanged()));

    historyPlayback = new QToolButton;
    historyPlayback->setToolTip(tr("Show/hide the history playback tool"));
    historyPlayback->setIcon(QIcon(":/images/playback_tool.png"));
    historyPlayback->setText(tr("Playback"));
    historyPlayback->setCheckable(true);

    attachToolWindowToButton(historyPlaybackTool, historyPlayback);

#undef GT

    addWidget(branchSelection);
    addWidget(historyPlayback);
}


void GitToolBar::attachToolWindowToButton(ToolWindow *w, QToolButton* b)
// ----------------------------------------------------------------------------
//    Associate a tool window with a toolbar button
// ----------------------------------------------------------------------------
{
    connect(b, SIGNAL(toggled(bool)),           w, SLOT(setVisible(bool)));
    connect(w, SIGNAL(visibilityChanged(bool)), b, SLOT(setChecked(bool)));

    if (w->createVisible())
        w->show();
}


void GitToolBar::setRepository(Repository *repo)
// ----------------------------------------------------------------------------
//    Set repository
// ----------------------------------------------------------------------------
{
    branchSelectionTool->setRepository(repo);
    historyPlaybackTool->setRepository(repo);
}


void GitToolBar::refresh()
// ----------------------------------------------------------------------------
//    Refresh tool windows managed by this toolbar
// ----------------------------------------------------------------------------
{
    branchSelectionTool->refresh();
    historyPlaybackTool->refresh();
}


void GitToolBar::showProjectUrl(QString url)
// ----------------------------------------------------------------------------
//    Show project URL
// ----------------------------------------------------------------------------
{
    branchSelectionTool->showProjectUrl(url);
}

}
