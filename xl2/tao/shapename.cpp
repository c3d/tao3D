// ****************************************************************************
//  shapename.cpp                                                   XLR project
// ****************************************************************************
// 
//   File Description:
// 
// 
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

ShapeSelection::ShapeSelection(Widget *widget, XL::Tree *tree,
                               const Box3 &box, bool deep)
// ----------------------------------------------------------------------------
//   Initialize a selection based on a 3D bounding box
// ----------------------------------------------------------------------------
    : ShapeName(widget, tree), bounds(box), 
      x(NULL), y(NULL), w(NULL), h(NULL),
      deep(deep)
{}


ShapeSelection::ShapeSelection(Widget *widget, XL::Tree *tree,
                               XL::real_r x, XL::real_r y,
                               XL::real_r w, XL::real_r h,
                               bool deep)
// ----------------------------------------------------------------------------
//   Initialize a selection based on tree coordinates
// ----------------------------------------------------------------------------
    : ShapeName(widget,tree),
      bounds(x-w/2,y-h/2,-(w+h)/4,w,h,(w+h)/2),
      x(&x), y(&y), w(&w), h(&h),
      deep(deep)
{}


ShapeSelection::~ShapeSelection()
// ----------------------------------------------------------------------------
//   Destructor checks the various drag / resizing actions
// ----------------------------------------------------------------------------
{
    typedef XL::Tree Tree;
    typedef XL::Integer Integer;
    typedef XL::Real Real;

    if (widget->selected())
    {
        widget->requestFocus();
        if (Drag *d = dynamic_cast<Drag *>(widget->activities))
        {
            double x1 = d->x1;
            double y1 = d->y1;
            double x2 = d->x2;
            double y2 = d->y2;
            int hh = widget->height();

            Point3 u1 = widget->unproject(x1, hh-y1, 0);
            Point3 u2 = widget->unproject(x2, hh-y2, 0);
            Vector3 v = u2 - u1;

            bool resize = qApp->keyboardModifiers() & Qt::ControlModifier;

            if (resize)
            {
                if (Tree *cw = xl_pre_cast(w))
                {
                    if (Integer *iw = cw->AsInteger())
                        iw->value += v.x;
                    else if (Real *rw = cw->AsReal())
                        rw->value += v.x;
                }
                if (Tree *ch = xl_pre_cast(h))
                {
                    if (Integer *ih = ch->AsInteger())
                        ih->value += v.y;
                    else if (Real *rh = ch->AsReal())
                        rh->value += v.y;
                }
            }
            else
            {
                if (Tree *cx = xl_pre_cast(x))
                {
                    if (Integer *ix = cx->AsInteger())
                        ix->value += v.x;
                    else if (Real *rx = cx->AsReal())
                        rx->value += v.x;
                }
                if (Tree *cy = xl_pre_cast(y))
                {
                    if (Integer *iy = cy->AsInteger())
                        iy->value += v.y;
                    else if (Real *ry = cy->AsReal())
                        ry->value += v.y;
                }
            }

            v.z = 0;
            bounds += v;
        }
        widget->drawSelection(bounds, deep);
    }
}

TAO_END
