#ifndef SHAPES_DRAWING_H
#define SHAPES_DRAWING_H
// ****************************************************************************
//  shapes_drawing.h                                                Tao project
// ****************************************************************************
//
//   File Description:
//
//     Drawing of elementary geometry shapes on a 2D layout
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

TAO_BEGIN

struct Rectangle : Drawing
// ----------------------------------------------------------------------------
//    A rectangle that can be placed in a layout
// ----------------------------------------------------------------------------
{
    Rectangle(const Box &b): Drawing(), bounds(b) {}
    virtual void        Draw(const Point &where);
    virtual Box         Bounds()        { return bounds; }
    Box                 bounds;
};


struct RoundedRectangle : Rectangle
// ----------------------------------------------------------------------------
//   A rectangle with rounded corners
// ----------------------------------------------------------------------------
{
    RoundedRectangle(const Box &b, coord r): Rectangle(b), radius(r) {}
    virtual void        Draw(const Point &where);
    coord               radius;
};    


struct Circle : Drawing
// ----------------------------------------------------------------------------
//    A circle that can be placed in a layout
// ----------------------------------------------------------------------------
{
    Circle(Point c, coord r): Drawing(), center(c), radius(r) {}
    virtual void        Draw(const Point &where);
    virtual Box         Bounds();
    Point               center;
    coord               radius;
};

TAO_END

#endif // SHAP
