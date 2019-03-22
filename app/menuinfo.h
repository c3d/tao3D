#ifndef MENUINFO_H
#define MENUINFO_H
// *****************************************************************************
// menuinfo.h                                                      Tao3D project
// *****************************************************************************
//
// File description:
//
//    Associate Qt menu data to XL trees
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
// (C) 2010,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
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

#include <QAbstractButton>
#include <string>
#include <QButtonGroup>
#include <QMenu>
#include <QToolBar>
#include <QAction>
#include <QMainWindow>
#include "tree.h"


namespace Tao {

struct MenuInfo : QObject
// ----------------------------------------------------------------------------
//  Qt menu data.
//  This objects holds either a menu/submenu/menuitem using p_parent and
//    p_action or a toolbar using p_window and p_toolbar.
// ----------------------------------------------------------------------------
{
    Q_OBJECT;
public:
    MenuInfo(QString name, QAction *act);
    MenuInfo(QString name, QToolBar *bar);
    ~MenuInfo();

public:
    QString        fullname; // the widget full name.
    QAction      * p_action; // The action associated with the widget
    QToolBar     * p_toolbar; // The toolbar
    text           title;
    text           icon;

public slots:
    void actionDestroyed();

};


struct GroupInfo : QButtonGroup, XL::Info
// ----------------------------------------------------------------------------
// QGroupButton associated to an XL tree
// ----------------------------------------------------------------------------
{
    Q_OBJECT;

public:
    typedef GroupInfo * data_t;

    GroupInfo(XL::Tree *t, QWidget * parent) :
            QButtonGroup(parent), XL::Info(), tree(t), action(NULL)
    {
        connect(this, SIGNAL(buttonClicked(QAbstractButton*)),
                this, SLOT(bClicked(QAbstractButton*)));
    }
    ~GroupInfo()
    {
        if (tree)
            tree->Remove(this);
    }

    operator data_t() { return this; }

public slots:
    void bClicked(QAbstractButton* button);

public:
    XL::Tree_p tree;
    XL::Tree_p action;
};

}

#endif // MENUINFO_H
