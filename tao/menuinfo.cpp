// ****************************************************************************
//  menuinfo.cpp                                                    Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

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
    toBeEvaluated = toBeEvaluated->Do(replacer);

    // Evaluate the input tree
    XL::MAIN->context->Evaluate(toBeEvaluated);
}

TAO_END
