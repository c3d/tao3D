// ****************************************************************************
//  commit_table_widget.cpp                                        Tao project
// ****************************************************************************
//
//   File Description:
//
//    A class to display a table view showing the commit log for a branch
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

#include "commit_table_widget.h"
#include "commit_table_model.h"

namespace Tao {

CommitTableWidget::CommitTableWidget(QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a CommitTableWidget
// ----------------------------------------------------------------------------
    : QTableView(parent)
{
    table = new CommitTableModel(this);

    proxyModel = new CommitSortFilterProxyModel(this);
    proxyModel->setSourceModel(table);

    setModel(proxyModel);
    setSortingEnabled(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    horizontalHeader()->setStretchLastSection(true);
    verticalHeader()->hide();
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionMode(QAbstractItemView::SingleSelection);

    sortByColumn(1, Qt::DescendingOrder);
    horizontalHeader()->setSortIndicator(1, Qt::DescendingOrder);

    connect(selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(onSelectionChanged()));
}


void  CommitTableWidget::refresh()
// ----------------------------------------------------------------------------
//    Reload the commit history from the model
// ----------------------------------------------------------------------------
{
    table->refresh();

    resizeColumnToContents(0);  // ID
    resizeColumnToContents(1);  // Date
    resizeColumnToContents(2);  // Author
}


void CommitTableWidget::setRepository(Repository *repo)
// ----------------------------------------------------------------------------
//    Set the repository to use as a data source
// ----------------------------------------------------------------------------
{
    table->setRepository(repo);
}


void CommitTableWidget::setBranch(QString branch)
// ----------------------------------------------------------------------------
//    Choose the branch to display. Select latest commit.
// ----------------------------------------------------------------------------
{
    table->setBranch(branch);
    refresh();
    selectRow(0);
}


void CommitTableWidget::onSelectionChanged()
// ----------------------------------------------------------------------------
//    Emit a signal with a Commit structure when a line is selected
// ----------------------------------------------------------------------------
{
    QModelIndexList indexes = selectionModel()->selectedRows();
    if (indexes.size() > 1)
        std::cerr << "Warning: Multiple selection not supported\n";

    QModelIndex index = indexes.at(0);
    int row = proxyModel->mapToSource(index).row();
    Repository::Commit commit = table->commit(row);

   emit commitSelected(commit);
}

}
