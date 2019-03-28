#ifndef SHAPES_H
#define SHAPES_H
// *****************************************************************************
// shapes.h                                                        Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2011,2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2010-2012, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2011,2013-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2013, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
// (C) 2011,2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

#include "drawing.h"
#include "tao_gl.h"
#include "coords3d.h"
#include <vector>

TAO_BEGIN

struct GraphicPath;
struct WidgetSurface;
struct Shape3;

struct Shape : Drawing
// ----------------------------------------------------------------------------
//   Common base class, just in case
// ----------------------------------------------------------------------------
{
    Shape(): Drawing() {}

public:
    // Shape parameters
    static void         enableTexCoord(double *texCoord, uint64 mask = ~0UL);
    static void         disableTexCoord(uint64 mask);
    static bool         setTexture(Layout *where);
    static bool         setShader(Layout *where);
    static bool         setFillColor(Layout *where);
    static bool         setLineColor(Layout *where);
};

struct Shape2 : Shape
// ----------------------------------------------------------------------------
//   Common base class, just in case
// ----------------------------------------------------------------------------
{
    Shape2(): Shape() {}

    virtual void        Draw(Layout *where);
    virtual void        Identify(Layout *);
    virtual void        Draw(GraphicPath &path);
};


struct Rectangle : Shape2
// ----------------------------------------------------------------------------
//    A rectangle that can be placed in a layout
// ----------------------------------------------------------------------------
{
    Rectangle(const Box &b): Shape2(), bounds(b) {}
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
//    An arc of ellipse or ellipse sector that fits within the given bounds
// ----------------------------------------------------------------------------
{
    EllipseArc(const Box &b, float start, float sweep, bool sector = false)
        : Ellipse(b), start(start), sweep(sweep), sector(sector) {}
    virtual void        Draw(GraphicPath &path);
    float start, sweep;
    bool  sector;
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


struct FixedSizePoint : Shape2
// ----------------------------------------------------------------------------
//    An OpenGL point
// ----------------------------------------------------------------------------
{
    FixedSizePoint(Point3 c, scale r): Shape2(), center(c), radius(r) {}
    virtual void        Draw(Layout *where);
    virtual void        DrawSelection(Layout *where) { Draw(where); }
    Point3 center;
    scale  radius;
};


struct PlaneMesh
// ----------------------------------------------------------------------------
//   Common drawing code for mesh-based shapes
// ----------------------------------------------------------------------------
{
    PlaneMesh(int lines, int columns);

    std::vector<Point3> vertices;
    std::vector<GLuint> indices;
    std::vector<Point>  textures;
};

struct Plane : Shape2
// ----------------------------------------------------------------------------
//   Define a subdivided plane
// ----------------------------------------------------------------------------
{
    // Constructor and destructor
    Plane(float x, float y, float w, float h, int lines, int columns);

    // Draw plane
    virtual void Draw(Layout *where);
    virtual void Draw(GraphicPath &path) { return Shape2::Draw(path); }
    void Draw(PlaneMesh* plane, Layout *where);

private:
    // Plane parameters
    Tao::Vector3 center;
    float width, height;
    int   slices, stacks;

    enum { MAX_PLANES = 10 };
    typedef std::pair<uint, uint> Key;
    typedef std::map<Key, PlaneMesh *> PlaneCache;

    static PlaneCache cache;
};


// ============================================================================
//
//   Entering shapes in the symbols table
//
// ============================================================================

extern void EnterShapes();
extern void DeleteShapes();

TAO_END

#endif // SHAPES_H
