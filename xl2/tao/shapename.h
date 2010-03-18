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
    enum
    {
        none   = 0,
        xy     = 0x000021,
        xywh   = 0x043021,
        xyr    = 0x003021,
        xyzr   = 0x004321,
        xyzwhd = 0x654321
    };
    typedef std::vector<XL::Tree **> tree_ptrs;

    ShapeName(Widget *w, XL::Tree *t, uint argsPos = xywh, Box3 box = Box3());
    ~ShapeName();

    bool        args(tree_ptrs &list);
    void        updateArg(tree_ptrs &list, uint index, coord delta);
    Vector3     dragDelta();

public:
    Box3        box;
    Widget      *widget;
    XL::Tree    *tree;
    uint        argPos;
};

TAO_END

#endif // SHAPENAME_H
