#ifndef COMMIT_TABLE_WIDGET_H
#define COMMIT_TABLE_WIDGET_H

// ****************************************************************************
//  commit_table_widget.h                                          Tao project
// ****************************************************************************
//
//   File Description:
//
//    A class to display a table view that shows the commit log for a branch
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

#include "repository.h"
#include <QtGui>


namespace Tao {

class CommitTableModel;

class CommitTableWidget : public QTableView
{
    Q_OBJECT

public:
    CommitTableWidget(QWidget *parent = 0);

public:
    void    refresh();

public slots:
    void    setRepository(Repository *repo);
    void    setBranch(QString branch);

signals:
    void    commitSelected(Repository::Commit commit);

private slots:
    void    onSelectionChanged();

private:
    CommitTableModel      * table;
    QSortFilterProxyModel * proxyModel;
};

}

#endif // COMMIT_TABLE_WIDGET_H
