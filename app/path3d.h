#ifndef PATH3D_H
#define PATH3D_H
// *****************************************************************************
// path3d.h                                                        Tao3D project
// *****************************************************************************
//
// File description:
//
//     Representation of paths in 3D
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
// (C) 2010-2011, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2012-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011,2014, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010-2011, Lionel Schaffhauser <lionel@taodyne.com>
// (C) 2011, Soulisse Baptiste <soulisse@polytech.unice.fr>
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

#include "coords3d.h"
#include "shapes.h"
#include "tree.h"
#include "tao_tree.h"
#include "tao_gl.h"
#include <QPen>

class QPainterPath;

TAO_BEGIN

struct ControlPoint;
struct FrameManipulator;
struct GraphicPath : Shape
// ----------------------------------------------------------------------------
//    An arbitrary graphic path
// ----------------------------------------------------------------------------
{
    GraphicPath();
    ~GraphicPath();

    virtual void        Draw(Layout *where);
    virtual void        DrawSelection(Layout *where);
    virtual void        Identify(Layout *where);
    virtual Box3        Bounds(Layout *layout);

    // Internal drawing routines
    void                Draw(Layout *where, GLenum tessel);
    void                Draw(Layout *where,
                             const Vector3 &offset, GLenum mode, GLenum tessel);
    void                DrawOutline(Layout *where);

    // Absolute coordinates
    GraphicPath&        moveTo(Point3 dst);
    GraphicPath&        lineTo(Point3 dst);
    GraphicPath&        curveTo(Point3 control, Point3 dst);
    GraphicPath&        curveTo(Point3 c1, Point3 c2, Point3 dst);
    GraphicPath&        close();

    // Relative coordinates
    GraphicPath&        moveTo(Vector3 dst)     { return moveTo(position+dst); }
    GraphicPath&        lineTo(Vector3 dst)     { return lineTo(position+dst); }

    // Qt path conversion
    GraphicPath&        addQtPath(QPainterPath &path, scale sy = 1);
    static bool         extractQtPath(GraphicPath &in, QPainterPath &out);
    bool                extractQtPath(QPainterPath &path);
    static void         Draw(Layout *where, QPainterPath &path,
                             GLenum tessel = 0, scale sy = 1);

    // Other operations
    void                clear();
    void                AddControl(XL::Tree *self, bool onCurve,
                                   Real *x,Real *y,Real *z);

public:
    enum Kind           { MOVE_TO, LINE_TO, CURVE_TO, CURVE_CONTROL };
    enum EndpointStyle  { NONE,
                          ARROWHEAD, TRIANGLE, POINTER, DIAMOND, CIRCLE,
                          SQUARE, BAR, CUP, FLETCHING,
                          HOLLOW_CIRCLE, HOLLOW_SQUARE, ROUNDED };
    struct Element
    {
        Element(Kind k, const Point3 &p): kind(k), position(p) {}
        Kind    kind;
        Point3  position;
    };
    typedef std::vector<Element> path_elements;
    typedef std::vector<ControlPoint *> control_points;
    struct VertexData
    {
        VertexData(const Point3& v, const Point3& t, int index)
            : vertex(v), texture(t), normal(0,0,1), index(index) {}
        VertexData()
            : vertex(), texture(), normal(), index(-1) {}
        Vector3     vertex;
        Vector3     texture;
        Vector3     normal;
        int         index;
    };
    typedef std::vector<VertexData>   Vertices;
    typedef std::vector<VertexData *> DynamicVertices;
    struct PolygonData
    {
        PolygonData(GraphicPath *path): path(path) {}
        ~PolygonData();

        Vertices        vertices;
        DynamicVertices allocated;
        Layout *        layout;
        GraphicPath *   path;
        GLenum          mode;
    };

public:
    path_elements       elements;
    control_points      controls;
    Point3              start, position;
    Box3                bounds;
    EndpointStyle       startStyle, endStyle;
    Qt::PenStyle        lineStyle;
    bool                invert;
    static scale        steps_min;
    static scale        steps_increase;
    static scale        steps_max;
};


struct GraphicPathInfo : XL::Info
// ----------------------------------------------------------------------------
//    Information about a given GraphicPath
// ----------------------------------------------------------------------------
{
    typedef GraphicPathInfo *data_t;

    GraphicPathInfo(GraphicPath *path);

    std::vector<Point3> controls;
    Box3                b0;
};


struct TesselatedPath : GraphicPath
// ----------------------------------------------------------------------------
//   Like a graphic path, but with explicit tesselation
// ----------------------------------------------------------------------------
{
    TesselatedPath(GLuint tesselation): tesselation(tesselation) {}
    void Draw(Layout *where);
    GLuint tesselation;
};

TAO_END

#endif // PATH3D_H
