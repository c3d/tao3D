// *****************************************************************************
// menuinfo.cpp                                                    Tao3D project
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
// (C) 2010,2012, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
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

#include "tao.h"
#include "menuinfo.h"
#include "options.h"
#include "context.h"
#include "runtime.h"
#include "tao_utf8.h"
#include "tree_cloning.h"
#include "main.h"
#include <iostream>

RECORDER(menus, 16, "Tao3D menus");


TAO_BEGIN

MenuInfo::MenuInfo(QString name, QAction *action)
// ----------------------------------------------------------------------------
//   Constructor taking an action, e.g. when creating a menu
// ----------------------------------------------------------------------------
    : fullname(name), action(action), toolbar(NULL), title(""), icon("")
{
    record(menus, "Creating menuinfo %p for action %s", this, name);
    connect(action, SIGNAL(destroyed(QObject *)),
            this, SLOT(actionDestroyed()));
}


MenuInfo::MenuInfo(QString name, QToolBar *bar)
// ----------------------------------------------------------------------------
//   Constructor taking a toolbar, e.g. when adding items in a toolbar
// ----------------------------------------------------------------------------
    : fullname(name), action(NULL), toolbar(bar), title(""), icon("")
{
    record(menus, "Creating menuinfo %p for toolbar %s", this, name);
    connect(toolbar, SIGNAL(destroyed(QObject *)),
            this, SLOT(actionDestroyed()));
}


MenuInfo::~MenuInfo()
// ----------------------------------------------------------------------------
//   Delete a menu entry
// ----------------------------------------------------------------------------
{
    delete action;
    delete toolbar;
    actionDestroyed();
    fullname = "";
}


void MenuInfo::actionDestroyed()
// ----------------------------------------------------------------------------
//   Action or toolbar going away: clear everything
// ----------------------------------------------------------------------------
{
    record(menus, "Action %s destroyed", fullname);
    action  = NULL;
    toolbar = NULL;
    title   = "";
    icon    = "";
}


void GroupInfo::bClicked(QAbstractButton *button)
// ----------------------------------------------------------------------------
// Slot invoked when a button from the group is selected.
// ----------------------------------------------------------------------------
// The Name "button_name" is replaced by a Text containing the name
//   of the clicked button.
{
    if (!action)
        return;

    // Adjust tree to evaluate
    ClickTreeClone replacer(+button->objectName());
    XL::Tree *toRun = action->Do(replacer);

    // Evaluate the action
    xl_evaluate(scope, toRun);
}

TAO_END
