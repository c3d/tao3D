// ****************************************************************************
//  shapename.cpp                                                   XLR project
// ****************************************************************************
// 
//   File Description:
// 
//    Helper class used to assign GL names to individual graphic shapes
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
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "shapename.h"
#include "drag.h"
#include "runtime.h"

TAO_BEGIN

// ============================================================================
// 
//    Shape name (providing an 
// 
// ============================================================================

ShapeName::ShapeName(Widget *w, Box3 box)
// ----------------------------------------------------------------------------
//   Record the GL name for a given tree
// ----------------------------------------------------------------------------
    : widget(w), box(box), xp(0), yp(0), zp(0), wp(0), hp(0), dp(0)
{
    widget->loadName(true);
}


ShapeName::~ShapeName()
// ----------------------------------------------------------------------------
//    Maintain the selection for the given tree
// ----------------------------------------------------------------------------
{
    widget->loadName(false);
    if (widget->selected())
    {
        Vector3 v = widget->dragDelta();

        uint modifiers = qApp->keyboardModifiers();
        bool resize = modifiers & Qt::ShiftModifier;
        bool zaxis = modifiers & Qt::AltModifier;
        tree_p yzp = zaxis ? zp : yp;
        tree_p hdp = zaxis ? dp : hp;

        v.z = 0;
        if (resize)
        {
            if (wp)     updateArg(wp,  v.x);
            if (hdp)    updateArg(hdp, v.y);
            if (v.x != 0 || v.y != 0.0)
                widget->markChanged("Moved shape");
        }
        else
        {
            if (xp)     updateArg(xp,  v.x);
            if (yzp)    updateArg(yzp, v.y);
            if (v.x != 0 || v.y != 0.0)
                widget->markChanged("Resized shape");
        }
        box.Normalize();
        widget->drawSelection(box);
    }
}


void ShapeName::updateArg(tree_p arg, coord delta)
// ----------------------------------------------------------------------------
//   Update the given argument by the given offset
// ----------------------------------------------------------------------------
{
    // Defensive coding against bad callers...
    if (!arg || delta == 0.0)
        return;

    Tree *source = xl_source(arg);       // Find the source expression
    arg = source;

    tree_p *ptr = &arg;
    bool more = true;
    bool negative = false;

    // Check if we have an Infix +, if so walk down the left side
    while (more)
    {
        more = false;
        if (XL::Infix *infix = (*ptr)->AsInfix())
        {
            if (infix->name == "+")
            {
                ptr = &infix->left;
                more = true;
            }
        }
        if (XL::Prefix *prefix = (*ptr)->AsPrefix())
        {
            if (XL::Name *name = prefix->left->AsName())
            {
                if (name->value == "-")
                {
                    ptr = &prefix->right;
                    more = true;
                    negative = !negative;
                    delta = -delta;
                }
                else if (name->value == "+")
                {
                    ptr = &prefix->right;
                    more = true;
                }
            }
        }
    }

    // Test the simple cases where the argument is directly an Integer or Real
    if (XL::Integer *ival = (*ptr)->AsInteger())
    {
        ival->value += delta;
    }
    else if (XL::Real *rval = (*ptr)->AsReal())
    {
        rval->value += delta;
    }
    else
    {
        // Create an Infix + with the delta we add
        if (ptr != &arg)
        {
            *ptr = new XL::Infix("+", new XL::Real(delta), *ptr);
            widget->reloadProgram = true;
        }
    }
}

TAO_END
