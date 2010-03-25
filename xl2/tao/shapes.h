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

protected:
    bool setTexture(Layout *where);
    bool setFillColor(Layout *where);
    bool setLineColor(Layout *where);

    // The current color
    static double red, green, blue, alpha;
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


struct Ellipse : Rectangle
// ----------------------------------------------------------------------------
//    An ellipse that fits within the given bounds
// ----------------------------------------------------------------------------
{
    Ellipse(const Box &b): Rectangle(b) {}
    virtual void        Draw(Layout *where);
};


struct EllipseArc : Ellipse
// ----------------------------------------------------------------------------
//    An arc of ellipse that fits within the given bounds
// ----------------------------------------------------------------------------
{
    EllipseArc(const Box &b, float start, float sweep)
        : Ellipse(b), start(start), sweep(sweep) {}
    virtual void        Draw(Layout *where);
    float start, sweep;
};

TAO_END

#endif // SHAP
