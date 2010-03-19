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

ShapeName::ShapeName(Widget *w, XL::Tree *t, uint argPos, Box3 box)
// ----------------------------------------------------------------------------
//   Record the GL name for a given tree
// ----------------------------------------------------------------------------
    : widget(w), tree(t), argPos(argPos), box(box)
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
        Vector3 v = dragDelta();

        uint modifiers = qApp->keyboardModifiers();
        bool resize = modifiers & Qt::ShiftModifier;
        bool zaxis = modifiers & Qt::AltModifier;

        uint x = (argPos >> 0)  & 0xF;
        uint y = (argPos >> 4)  & 0xF;
        uint z = (argPos >> 8)  & 0xF;
        uint w = (argPos >> 12) & 0xF;
        uint h = (argPos >> 16) & 0xF;
        uint d = (argPos >> 20) & 0xF;
        uint yz = zaxis ? z : y;
        uint hd = zaxis ? d : h;

        tree_ptrs list;
        args(list);

        IFTRACE(drag)
        {
            tree_ptrs::iterator it;
            std::cerr << "Args (" << tree << ")= " << (void *) argPos << ": ";
            for (it = list.begin(); it != list.end(); it++)
                std::cerr << **it << "; ";
            std::cerr << "V=" << v.x << ", " << v.y << ", " << v.z
                      << " resize=" << resize << "\n";
        }

        v.z = 0;
        if (resize)
        {
            if (w)      updateArg(list, w,  v.x);
            if (hd)     updateArg(list, hd, v.y);
            box += v;
        }
        else
        {
            if (x)      updateArg(list, x,  v.x);
            if (yz)     updateArg(list, yz, v.y);
            v /= 2;
            box.upper += v;
            box.lower -= v;
        }
        widget->selectionTrees.insert(tree);
        widget->drawSelection(box);
    }
}


bool ShapeName::args(tree_ptrs &list)
// ----------------------------------------------------------------------------
//   Reconstruct the arg list for the given tree
// ----------------------------------------------------------------------------
//   The shape of the tree is assumed to be a Prefix with arguments
//   separated by Infix , trees
{
    // We need a valid input argument
    if (!tree)
        return false;

    // The input must be a prefix tree
    XL::Prefix *prefix = tree->AsPrefix();
    if (!prefix)
        return false;

    // Point to the arguments
    XL::Tree **ptr = &prefix->right;
 
    // Walk down the comma-separated arguments
    while (true)
    {
        // Check if we have more arguments
        XL::Infix *infix = (*ptr)->AsInfix();
        if (!infix || infix->name != ",")
            break;
        list.push_back(&infix->right);
        ptr = &infix->left;
    }
    list.push_back(ptr);
    return true;
}


Vector3 ShapeName::dragDelta()
// ----------------------------------------------------------------------------
//   Compute the drag delta based on the current rotation coordinates
// ----------------------------------------------------------------------------
{
    Vector3 result;

    if (widget->selected())
    {
        widget->recordProjection();
        if (Drag *d = dynamic_cast<Drag *>(widget->activities))
        {
            double x1 = d->x1;
            double y1 = d->y1;
            double x2 = d->x2;
            double y2 = d->y2;
            int hh = widget->height();

            Point3 u1 = widget->unproject(x1, hh-y1, 0);
            Point3 u2 = widget->unproject(x2, hh-y2, 0);
            result = u2 - u1;
        }
    }
    return result;
}


void ShapeName::updateArg(tree_ptrs &list, uint index, coord delta)
// ----------------------------------------------------------------------------
//   Update the given argument by the given offset
// ----------------------------------------------------------------------------
{
    // Check if we have the given argument
    if (index > list.size())
        return;

    XL::Tree **ptr = list[list.size() - index];

    // Check if we have an Infix +, if so walk down the left side
    while (XL::Infix *infix = (*ptr)->AsInfix())
    {
        if (infix->name == "+")
            ptr = &infix->left;
        else
            break;
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
        *ptr = new XL::Infix("+", new XL::Real(delta), *ptr);
        widget->reloadProgram = true;
    }
}

TAO_END
