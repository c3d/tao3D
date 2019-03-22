#ifndef COMMIT_TABLE_MODEL_H
#define COMMIT_TABLE_MODEL_H
// *****************************************************************************
// commit_table_model.h                                            Tao3D project
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

#include "repository.h"
#include <QAbstractTableModel>
#include <QModelIndex>
#include <QVariant>
#include <QList>
#include <QSortFilterProxyModel>

namespace Tao {

class CommitTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    CommitTableModel(QObject *parent = 0);

    int      rowCount(const QModelIndex &parent) const;
    int      columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orient, int role) const;

    void     refresh();
    Repository::Commit commit(int row);

public slots:
    void    setRepository(Repository *repo);
    void    setBranch(QString branch);

private:
    Repository *              repo;
    QString                   branch;
    QList<Repository::Commit> commits;
};

class CommitSortFilterProxyModel : public QSortFilterProxyModel
// ----------------------------------------------------------------------------
//    Custom sort/filter model to re-implement sorting by date
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    CommitSortFilterProxyModel(QObject *parent)
        : QSortFilterProxyModel(parent) {}

    bool    lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

}

#endif // COMMIT_TABLE_MODEL_H
