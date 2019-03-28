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

TAO_BEGIN

MenuInfo::MenuInfo(QString name, QAction *act)
// ----------------------------------------------------------------------------
//   Constructor taking an action, e.g. when creating a menu
// ----------------------------------------------------------------------------
    : fullname(name), p_action(act), p_toolbar(NULL), title(""), icon("")
{
    connect(p_action, SIGNAL(destroyed(QObject *)),
            this, SLOT(actionDestroyed()));
}


MenuInfo::MenuInfo(QString name, QToolBar *bar)
// ----------------------------------------------------------------------------
//   Constructor taking a toolbar, e.g. when adding items in a toolbar
// ----------------------------------------------------------------------------
    : fullname(name), p_action(NULL), p_toolbar(bar), title(""), icon("")
{
    connect(p_toolbar, SIGNAL(destroyed(QObject *)),
            this, SLOT(actionDestroyed()));
}


MenuInfo::~MenuInfo()
// ----------------------------------------------------------------------------
//   Delete a menu entry
// ----------------------------------------------------------------------------
{
    if (p_action)
    {
        delete p_action;
        p_action = NULL;
    }
    if (p_toolbar)
    {
        delete p_toolbar;
        p_toolbar = NULL;
    }

    actionDestroyed();
    fullname = "";
}


void MenuInfo::actionDestroyed()
// ----------------------------------------------------------------------------
//   Action or toolbar going away: clear everything
// ----------------------------------------------------------------------------
{
    IFTRACE(menus)
    {
        std::cerr << fullname.toStdString() << " actionDestroyed \n";
    }
    p_action   = NULL;
    p_toolbar  = NULL;
    title = "";
    icon = "";
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


    // The tree to be evaluated needs its own symbol table before evaluation
    ClickTreeClone replacer(+button->objectName());
    XL::Tree *  toBeEvaluated = action;
    XL::Symbols * sym = action->Symbols();
    toBeEvaluated = toBeEvaluated->Do(replacer);
    toBeEvaluated->SetSymbols(sym);

    // Evaluate the input tree
    XL::MAIN->context->Evaluate(toBeEvaluated);
}

TAO_END
