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


TAO_BEGIN

struct ShapeName
// ----------------------------------------------------------------------------
//   Structure used simply to assign shape IDs during selection
// ----------------------------------------------------------------------------
{
    ShapeName(Widget *w, Box3 box = Box3());
    ~ShapeName();

    typedef XL::Tree Tree, *tree_p;

    // Specifying where arguments are
    ShapeName&  x(Tree &xr)     { xp = &xr; }
    ShapeName&  y(Tree &yr)     { yp = &yr; }
    ShapeName&  z(Tree &zr)     { zp = &zr; }
    ShapeName&  w(Tree &wr)     { wp = &wr; }
    ShapeName&  h(Tree &hr)     { hp = &hr; }
    ShapeName&  d(Tree &dr)     { dp = &dr; }

protected:
    void        updateArg(tree_p param, coord delta);
    Vector3     dragDelta();

protected:
    Widget      *widget;
    Box3        box;
    tree_p      xp, yp, zp, wp, hp, dp;
};

TAO_END

#endif // SHAPENAME_H
