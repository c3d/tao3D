// *****************************************************************************
// commit_table_widget.cpp                                         Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2014-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
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

#include "commit_table_widget.h"
#include "commit_table_model.h"
#include <QHeaderView>

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
