#ifndef SHAPES_H
#define SHAPES_H
// ****************************************************************************
//  shapes.h                                                       Tao project
// ****************************************************************************
//
//   File Description:
//
//     Drawing of elementary 2D geometry shapes
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

#include "drawing.h"
#include "coords3d.h"
#include <vector>

TAO_BEGIN

struct Shape : Drawing
// ----------------------------------------------------------------------------
//   Common base class, just in case
// ----------------------------------------------------------------------------
{
    Shape(): Drawing() {}
};


struct Rectangle : Shape
// ----------------------------------------------------------------------------
//    A rectangle that can be placed in a layout
// ----------------------------------------------------------------------------
{
    Rectangle(const Box &b): Shape(), bounds(b) {}
    virtual void        Draw(Layout *where);
    virtual Box3        Bounds()        { return bounds; }
    Box                 bounds;
};


struct RoundedRectangle : Rectangle
// ----------------------------------------------------------------------------
//   A rectangle with rounded corners
// ----------------------------------------------------------------------------
{
    RoundedRectangle(const Box &b, coord rx, coord ry)
        : Rectangle(b), radiusX(rx), radiusY(ry) {}
    virtual void        Draw(Layout *where);
    coord radiusX, radiusY;
};    


struct Circle : Shape
// ----------------------------------------------------------------------------
//    A circle that can be placed in a layout
// ----------------------------------------------------------------------------
{
    Circle(Point c, coord r): Shape(), center(c), radius(r) {}
    virtual void        Draw(Layout *where);
    virtual Box3        Bounds();
    Point               center;
    coord               radius;
};

TAO_END

#endif // SHAP
