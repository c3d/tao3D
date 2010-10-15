#ifndef BRANCH_SELECTION_TOOL_H
#define BRANCH_SELECTION_TOOL_H

// ****************************************************************************
//  branch_selection_tool.h                                        Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

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
