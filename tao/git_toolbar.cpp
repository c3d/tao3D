// ****************************************************************************
//  git_toolbar.cpp                                                Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

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
    branchSelection->setStatusTip(tr("Show/hide the branch selection tool"));
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
    historyPlayback->setStatusTip(tr("Show/hide the history playback tool"));
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
