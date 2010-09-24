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
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "drawing.h"
#include "coords3d.h"
#include <vector>

TAO_BEGIN

struct GraphicPath;
struct WidgetSurface;

struct Shape : Drawing
// ----------------------------------------------------------------------------
//   Common base class, just in case
// ----------------------------------------------------------------------------
{
    Shape(): Drawing() {}

    virtual void        Draw(Layout *where);
    virtual void        Draw(GraphicPath &path);

protected:
    bool                setTexture(Layout *where);
    bool                setFillColor(Layout *where);
    bool                setLineColor(Layout *where);
};


struct Rectangle : Shape
// ----------------------------------------------------------------------------
//    A rectangle that can be placed in a layout
// ----------------------------------------------------------------------------
{
    Rectangle(const Box &b): Shape(), bounds(b) {}
    virtual void        Draw(GraphicPath &path);
    virtual Box3        Bounds(Layout *where);
    Box                 bounds;
};


struct PlaceholderRectangle : Rectangle
// ----------------------------------------------------------------------------
//    A placeholder for empty items (e.g. empty layouts)
// ----------------------------------------------------------------------------
{
    PlaceholderRectangle(const Box &b): Rectangle(b) {}
    virtual void        Draw(Layout *where);
    virtual void        Draw(GraphicPath &path);
};


struct ClickThroughRectangle : Rectangle
// ----------------------------------------------------------------------------
//    A rectangle that cannot be clicked into
// ----------------------------------------------------------------------------
{
    ClickThroughRectangle(const Box &b, WidgetSurface *s)
        : Rectangle(b), surface(s) {}
    virtual void        DrawSelection(Layout *where);
    WidgetSurface *surface;
};


struct IsoscelesTriangle : Rectangle
// ----------------------------------------------------------------------------
//    A isosceles triangle that can be placed in a layout
// ----------------------------------------------------------------------------
{
    IsoscelesTriangle(const Box &b): Rectangle(b) {}
    virtual void        Draw(GraphicPath &path);
};


struct RightTriangle : Rectangle
// ----------------------------------------------------------------------------
//    A right triangle that can be placed in a layout
// ----------------------------------------------------------------------------
{
    RightTriangle(const Box &b): Rectangle(b) {}
    virtual void        Draw(GraphicPath &path);
};


struct RoundedRectangle : Rectangle
// ----------------------------------------------------------------------------
//   A rectangle with rounded corners
// ----------------------------------------------------------------------------
{
    RoundedRectangle(const Box &b, coord r)
        : Rectangle(b), r(r) {}
    virtual void        Draw(GraphicPath &path);
    coord r;
};


struct EllipticalRectangle : Rectangle
// ----------------------------------------------------------------------------
//   A rectangle with elliptical sides
// ----------------------------------------------------------------------------
{
    EllipticalRectangle(const Box &b, double r)
        : Rectangle(b), r(r) {}
    virtual void        Draw(Layout *where);
    virtual void        Draw(GraphicPath &path);
    double r;
};


struct Ellipse : Rectangle
// ----------------------------------------------------------------------------
//    An ellipse that fits within the given bounds
// ----------------------------------------------------------------------------
{
    Ellipse(const Box &b): Rectangle(b) {}
    virtual void        Draw(GraphicPath &path);
};


struct EllipseArc : Ellipse
// ----------------------------------------------------------------------------
//    An arc of ellipse that fits within the given bounds
// ----------------------------------------------------------------------------
{
    EllipseArc(const Box &b, float start, float sweep)
        : Ellipse(b), start(start), sweep(sweep) {}
    virtual void        Draw(GraphicPath &path);
    float start, sweep;
};


struct Arrow : Rectangle
// ----------------------------------------------------------------------------
//   An arrow
// ----------------------------------------------------------------------------
{
    Arrow(const Box &b, double ax, double ary)
        : Rectangle(b), ax(ax), ary(ary) {}
    virtual void        Draw(Layout *where);
    virtual void        Draw(GraphicPath &path);
    double ax, ary;
};


struct DoubleArrow : Rectangle
// ----------------------------------------------------------------------------
//   A double arrow
// ----------------------------------------------------------------------------
{
    DoubleArrow(const Box &b, double ax, double ary)
        : Rectangle(b), ax(ax), ary(ary) {}
    virtual void        Draw(Layout *where);
    virtual void        Draw(GraphicPath &path);
    double ax, ary;
};


struct StarPolygon : Rectangle
// ----------------------------------------------------------------------------
//   A star or a regular polygon
// ----------------------------------------------------------------------------
{
    StarPolygon(const Box &b, int p, int q): Rectangle(b), p(p), q(q) {}
    virtual void        Draw(Layout *where);
    virtual void        Draw(GraphicPath &path);
    int p,q;
};


struct Star : Rectangle
// ----------------------------------------------------------------------------
//   A star with arbitrary inner circle
// ----------------------------------------------------------------------------
{
    Star(const Box &b, int p, float r): Rectangle(b), p(p), r(r) {}
    virtual void        Draw(Layout *where);
    virtual void        Draw(GraphicPath &path);
    int p;
    float r;
};


struct SpeechBalloon : Rectangle
// ----------------------------------------------------------------------------
//   A Speech ballon with rounded corners and a tail
// ----------------------------------------------------------------------------
{
    SpeechBalloon(const Box &b, coord r, coord ax, coord ay)
        : Rectangle(b), r(r), a(ax, ay) {}
    virtual void        Draw(Layout *where);
    virtual void        Draw(GraphicPath &path);
    double r;
    Point a;
};


struct Callout : Rectangle
// ----------------------------------------------------------------------------
//   A callout with rounded corners and a tail
// ----------------------------------------------------------------------------
{
    Callout(const Box &b, coord r, coord ax, coord ay, coord d)
        : Rectangle(b), r(r), a(ax, ay), d(d) {}
    virtual void        Draw(Layout *where);
    virtual void        Draw(GraphicPath &path);
    coord r;
    Point a;
    coord d;
};


struct FixedSizePoint : Shape
// ----------------------------------------------------------------------------
//    An OpenGL point
// ----------------------------------------------------------------------------
{
    FixedSizePoint(Point3 c, scale r): Shape(), center(c), radius(r) {}
    virtual void        Draw(Layout *where);
    virtual void        DrawSelection(Layout *where) { Draw(where); }
    Point3 center;
    scale  radius;
};

TAO_END

#endif // SHAPES_H