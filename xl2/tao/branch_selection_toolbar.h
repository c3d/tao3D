#ifndef BRANCH_SELECTION_TOOLBAR_H
#define BRANCH_SELECTION_TOOLBAR_H

// ****************************************************************************
//  branch_selection_toolbar.h                                     Tao project
// ****************************************************************************
//
//   File Description:
//
//    A class to display a toolbar that contains:
//      * a label
//      * a branch selection combo box; select a branch to check it out
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

#include "branch_selection_combobox.h"
#include <QToolBar>
#include <QWidget>
#include <QPushButton>

namespace Tao {

struct Repository;

class BranchSelectionToolBar : public QToolBar
{
    Q_OBJECT

public:
    BranchSelectionToolBar(QWidget *parent = 0);
    BranchSelectionToolBar(const QString &title, QWidget *parent = 0);

public slots:
    void    setRepository(Repository *repo);

signals:
    void    checkedOut(QString branch);

protected slots:
    void    checkout(QString branch);

private:
    void    init();

private:
    BranchSelectionComboBox * branchSelector;
    Repository              * repo;
};

}

#endif // BRANCH_SELECTION_TOOLBAR_H
