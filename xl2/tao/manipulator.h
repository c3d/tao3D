#ifndef MANIPULATOR_H
#define MANIPULATOR_H
// ****************************************************************************
//  manipulator.h                                                   Tao project
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

#include "widget.h"
#include "tree.h"
#include "coords3d.h"
#include "drawing.h"

#include <vector>
#include <map>


TAO_BEGIN

struct Manipulator
// ----------------------------------------------------------------------------
//   Structure used simply to assign shape IDs during selection
// ----------------------------------------------------------------------------
{
    Manipulator(Widget *w, Box3 box = Box3(), text selector = "selection");
    ~Manipulator();

    typedef XL::Tree                    Tree, *tree_p;
    typedef std::map<text, tree_p>      tree_map;

    // Specifying where arguments are
    Manipulator&  x(Tree &xr, text s = "move")    { xp[s] = &xr; return *this; }
    Manipulator&  y(Tree &yr, text s = "move")    { yp[s] = &yr; return *this; }
    Manipulator&  w(Tree &xr)                     { return x(xr, "size"); }
    Manipulator&  h(Tree &yr)                     { return y(yr, "size"); }

protected:
    void        updateArg(tree_p param, coord delta);
    Vector3     dragDelta();

protected:
    Widget      *widget;
    Box3        box;
    text        selector;
    tree_map    xp, yp;
};

TAO_END

XL_BEGIN

// ============================================================================
// 
//    Specialized variant of 'TreeClone' that normalizes --x into x
// 
// ============================================================================

struct NormalizedCloneMode;
template<>
inline Tree *TreeCloneTemplate<NormalizedCloneMode>::DoPrefix(Prefix *what)
// ----------------------------------------------------------------------------
//   Specialization of TreeClone that changes --x into x
// ----------------------------------------------------------------------------
{
    if (Name *n = what->left->AsName())
    {
        if (n->value == "-")
        {
            if (Real *rv = what->right->AsReal())
                if (rv->value < 0)
                    return new Real(-rv->value, rv->Position());
            if (Integer *iv = what->right->AsInteger())
                if (iv->value < 0)
                    return new Integer(-iv->value, iv->Position());
        }
    }
    return new Prefix(Clone(what->left), Clone(what->right),
                      what->Position());
}


typedef XL::TreeCloneTemplate<NormalizedCloneMode> NormalizedClone;

XL_END

#endif // MANIPULATOR_H
