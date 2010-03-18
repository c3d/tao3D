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

TAO_BEGIN

ShapeSelection::~ShapeSelection()
// ----------------------------------------------------------------------------
//   Destructor checks the various drag / resizing actions
// ----------------------------------------------------------------------------
{
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
                if (w)
                {
                    if (Integer *iw = w->AsInteger())
                        iw->value += v.x;
                    else if (Real *rw = w->AsReal())
                        rw->value += v.x;
                }
                if (h)
                {
                    if (Integer *ih = h->AsInteger())
                        ih->value += v.y;
                    else if (Real *rh = h->AsReal())
                        rh->value += v.y;
                }
            }
            else
            {
                if (x)
                {
                    if (Integer *ix = x->AsInteger())
                        ix->value += v.x;
                    else if (Real *rx = x->AsReal())
                        rx->value += v.x;
                }
                if (y)
                {
                    if (Integer *iy = y->AsInteger())
                        iy->value += v.y;
                    else if (Real *ry = y->AsReal())
                        ry->value += v.y;
                }
            }

            bounds += v;
        }
        widget->drawSelection(bounds, deep);
    }
}

TAO_END
