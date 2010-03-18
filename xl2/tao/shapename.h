#ifndef SHAPENAME_H
#define SHAPENAME_H
// ****************************************************************************
//  shapename.h                                                     XLR project
// ****************************************************************************
// 
//   File Description:
// 
//    Helper class used to assign names to individual graphic shapes
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


TAO_BEGIN

struct ShapeName
// ----------------------------------------------------------------------------
//   Structure used simply to assign shape IDs during selection
// ----------------------------------------------------------------------------
{
    ShapeName(Widget *w, XL::Tree *t)
        : widget(w), tree(t)
    {
        widget->loadName(true);
    }
    ~ShapeName()
    {
        widget->loadName(false);
        if (widget->selected())
            widget->selectionTrees.insert(tree);
    }

public:
    Widget      *widget;
    XL::Tree    *tree;
};


struct ShapeSelection : ShapeName
// ----------------------------------------------------------------------------
//    Structure used to assigne shape IDs and draw a selection rectangle
// ----------------------------------------------------------------------------
{
    ShapeSelection(Widget *widget, XL::Tree *tree,
                   const Box3 &box, bool deep = true)
        : ShapeName(widget, tree), bounds(box), 
          x(NULL), y(NULL), w(NULL), h(NULL),
          deep(deep) {}
    ShapeSelection(Widget *widget, XL::Tree *tree,
                   XL::real_r x, XL::real_r y, XL::real_r w, XL::real_r h,
                   bool deep = true)
        : ShapeName(widget,tree),
          bounds(x,y,-(w+h)/4,w,h,(w+h)/2),
          x(&x), y(&y), w(&w), h(&h),
          deep(deep) {}

    ~ShapeSelection();

    Box3 bounds;
    XL::tree_p x, y, w, h;
    bool deep;
};

TAO_END

#endif // SHAPENAME_H
