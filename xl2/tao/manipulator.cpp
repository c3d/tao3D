// ****************************************************************************
//  manipulator.cpp                                                 Tao project
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

#include "manipulator.h"
#include "drag.h"
#include "runtime.h"

TAO_BEGIN

// ============================================================================
// 
//    Shape name (providing an 
// 
// ============================================================================

Manipulator::Manipulator(Widget *w, Box3 box, text selector)
// ----------------------------------------------------------------------------
//   Record the GL name for a given tree
// ----------------------------------------------------------------------------
    : widget(w), box(box), selector(selector), xp(), yp()
{
    widget->loadName(true);
}


Manipulator::~Manipulator()
// ----------------------------------------------------------------------------
//    Maintain the selection for the given tree
// ----------------------------------------------------------------------------
{
    widget->loadName(false);
    if (widget->selected())
    {
        Vector3 v = widget->dragDelta();
        text selName = widget->dragSelector();
        tree_p x = xp.count(selName) ? xp[selName] : NULL;
        tree_p y = yp.count(selName) ? yp[selName] : NULL;

        v.z = 0;
        if (x)  updateArg(x,  v.x);
        if (y)  updateArg(y,  v.y);
        if ((x || y) && (v.x != 0 || v.y != 0))
            widget->markChanged("Shape " + selName);

        widget->drawSelection(box, selector);
    }
}


void Manipulator::updateArg(tree_p arg, coord delta)
// ----------------------------------------------------------------------------
//   Update the given argument by the given offset
// ----------------------------------------------------------------------------
{
    // Defensive coding against bad callers...
    if (!arg || delta == 0.0)
        return;

    Tree *source = xl_source(arg);       // Find the source expression
    tree_p    *ptr       = &source;
    bool       more      = true;
    bool       negative  = false;
    tree_p    *pptr      = NULL;
    tree_p    *ppptr     = NULL;

    // Check if we have an Infix +, if so walk down the left side
    arg = source;
    while (more)
    {
        more = false;
        ppptr = pptr;
        pptr = NULL;
        if (XL::Infix *infix = (*ptr)->AsInfix())
        {
            if (infix->name == "-")
            {
                ptr = &infix->left;
                more = true;
            }
            else if (infix->name == "+")
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
                    pptr = ptr;
                    ptr = &prefix->right;
                    more = true;
                    negative = !negative;
                    delta = -delta;
                }
            }
        }
    }

    // Test the simple cases where the argument is directly an Integer or Real
    if (XL::Integer *ival = (*ptr)->AsInteger())
    {
        ival->value += delta;
        if (ppptr && ival->value < 0)
            widget->reloadProgram = true;
    }
    else if (XL::Real *rval = (*ptr)->AsReal())
    {
        rval->value += delta;
        if (ppptr && rval->value < 0)
            widget->reloadProgram = true;
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
