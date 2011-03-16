#ifndef GIT_TOOLBAR_H
#define GIT_TOOLBAR_H

// ****************************************************************************
//  branch_selection_toolbar.h                                     Tao project
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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "branch_selection_tool.h"
#include "history_playback_tool.h"
#include <QToolBar>

QT_BEGIN_NAMESPACE
class QToolButton;
class QWidget;
QT_END_NAMESPACE

namespace Tao {

struct Repository;

class GitToolBar : public QToolBar
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
