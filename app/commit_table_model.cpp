// *****************************************************************************
// commit_table_model.cpp                                          Tao3D project
// *****************************************************************************
//
// File description:
//
//    The data model for CommitTableWidget. Basically, it holds a list of
//    repository commits.
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

#include "commit_table_model.h"

namespace Tao {

CommitTableModel::CommitTableModel(QObject *parent)
// ----------------------------------------------------------------------------
//    Create a CommitTableModel
// ----------------------------------------------------------------------------
    : QAbstractTableModel(parent)
{
}


int CommitTableModel::rowCount(const QModelIndex &parent) const
// ----------------------------------------------------------------------------
//    Return the number of rows in the model
// ----------------------------------------------------------------------------
{
    (void)parent;
    return commits.size();
}


int CommitTableModel::columnCount(const QModelIndex &parent) const
// ----------------------------------------------------------------------------
//    Return the number of columns in the model
// ----------------------------------------------------------------------------
{
    (void)parent;
    return 4;
}


QVariant CommitTableModel::data(const QModelIndex &index, int role) const
// ----------------------------------------------------------------------------
//    Return the model data for a given element of the table
// ----------------------------------------------------------------------------
{
    if (!index.isValid() ||
        index.row() >= commits.size() ||
        index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        Repository::Commit commit = commits.at(index.row());

        switch (index.column())
        {
        case 0:    return commit.id;
        case 1:    return commit.date;
        case 2:    return commit.author;
        case 3:    return commit.msg;
        default:   return QVariant();
        }
    }
    return QVariant();
}


QVariant CommitTableModel::headerData(int section, Qt::Orientation orientation,
                                      int role) const
// ----------------------------------------------------------------------------
//    Return the headers of the table
// ----------------------------------------------------------------------------
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case 0:     return tr("ID");
        case 1:     return tr("Date");
        case 2:     return tr("Author");
        case 3:     return tr("Message");
        default:    return QVariant();
        }
    }
    return QVariant();
}


Repository::Commit CommitTableModel::commit(int row)
// ----------------------------------------------------------------------------
//    Return the commit at position 'row' in the commit list
// ----------------------------------------------------------------------------
{
    return commits[row];
}


void CommitTableModel::refresh()
// ----------------------------------------------------------------------------
//    Refresh commit list
// ----------------------------------------------------------------------------
{
    if (!repo)
        return;

    beginResetModel();

    commits.clear();
    // Put commits in reverse chronological order
    QList<Repository::Commit> tmp = repo->history(branch);
    foreach (Repository::Commit commit, tmp)
        commits.prepend(commit);

    endResetModel();
}


void CommitTableModel::setRepository(Repository *repo)
// ----------------------------------------------------------------------------
//    Associate a repository to the model
// ----------------------------------------------------------------------------
{
    if (!repo)
        return;

    this->repo = repo;
}


void CommitTableModel::setBranch(QString branch)
// ----------------------------------------------------------------------------
//    Select a branch
// ----------------------------------------------------------------------------
{
    this->branch = branch;
}



bool CommitSortFilterProxyModel::lessThan(const QModelIndex &left,
                                          const QModelIndex &right) const
// ----------------------------------------------------------------------------
//    Compare to cells in the model
// ----------------------------------------------------------------------------
{
    // We want to preserve exactly the history order when sorting by date
    if (left.column() == 2)
        return left.row() < right.row();

    return QSortFilterProxyModel::lessThan(left, right);
}

}
