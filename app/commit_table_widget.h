#ifndef COMMIT_TABLE_WIDGET_H
#define COMMIT_TABLE_WIDGET_H
// *****************************************************************************
// commit_table_widget.h                                           Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2014-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
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
#include <QtGui>
#include <QTableView>


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
