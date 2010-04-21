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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
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
#include <iostream>

TAO_BEGIN

MenuInfo::MenuInfo(QString name, QAction *act)
// ----------------------------------------------------------------------------
//   Constructor taking an action, e.g. when creating a menu
// ----------------------------------------------------------------------------
    : fullname(name), p_action(act), p_toolbar(NULL)
{
    connect(p_action, SIGNAL(destroyed(QObject *)),
            this, SLOT(actionDestroyed()));
}


MenuInfo::MenuInfo(QString name, QToolBar *bar)
// ----------------------------------------------------------------------------
//   Constructor taking a toolbar, e.g. when adding items in a toolbar
// ----------------------------------------------------------------------------
    : fullname(name), p_action(NULL), p_toolbar(bar)
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
//   Set to NULL all the information because the action or
//     the tool bar will be deleted.
// ----------------------------------------------------------------------------
{
    IFTRACE(menus)
    {
        std::cerr << fullname.toStdString() << " actionDestroyed \n";
    }
    p_action   = NULL;
    p_toolbar  = NULL;
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

    // We override name "isChecked" in the input tree
    struct ClickTreeClone : XL::TreeClone
    {
        ClickTreeClone(text c) : name(c){}
        XL::Tree *DoName(XL::Name *what)
        {
            if (what->value == "button_name")
            {
                return new XL::Text(name);
            }
            return new XL::Name(what->value, what->Position());
        }
        text name;
    } replacer(+button->objectName());


    // The tree to be evaluated needs its own symbol table before evaluation
    XL::Tree *toBeEvaluated = action->tree;
    XL::Symbols *syms = toBeEvaluated->Get<XL::SymbolsInfo>();
    if (!syms)
        syms = XL::Symbols::symbols;
    syms = new XL::Symbols(syms);
    toBeEvaluated = toBeEvaluated->Do(replacer);
    toBeEvaluated->Set<XL::SymbolsInfo>(syms);

    // Evaluate the input tree
    xl_evaluate(toBeEvaluated);
}

TAO_END
