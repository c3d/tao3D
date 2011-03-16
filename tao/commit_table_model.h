#ifndef COMMIT_TABLE_MODEL_H
#define COMMIT_TABLE_MODEL_H

// ****************************************************************************
//  commit_table_model.h                                          Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

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
