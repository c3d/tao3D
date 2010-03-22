#ifndef SHAPENAME_H
#define SHAPENAME_H
// ****************************************************************************
//  shapename.h                                                     XLR project
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
#include "opcodes.h"

#include <vector>
#include <map>


TAO_BEGIN

struct ShapeName
// ----------------------------------------------------------------------------
//   Structure used simply to assign shape IDs during selection
// ----------------------------------------------------------------------------
{
    ShapeName(Widget *w, Box3 box = Box3(), text selector = "selection");
    ~ShapeName();

    typedef XL::Tree                    Tree, *tree_p;
    typedef std::map<text, tree_p>      tree_map;

    // Specifying where arguments are
    ShapeName&  x(Tree &xr, text s = "move")    { xp[s] = &xr; return *this; }
    ShapeName&  y(Tree &yr, text s = "move")    { yp[s] = &yr; return *this; }
    ShapeName&  w(Tree &xr)                     { return x(xr, "size"); }
    ShapeName&  h(Tree &yr)                     { return y(yr, "size"); }

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

#endif // SHAPENAME_H
