// *****************************************************************************
// path3d.cpp                                                      Tao3D project
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
// (C) 2011-2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011,2013-2014, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010-2011, Lionel Schaffhauser <lionel@taodyne.com>
// (C) 2011-2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2010, Christophe de Dinechin <christophe@dinechin.org>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
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

#include "path3d.h"
#include "layout.h"
#include "manipulator.h"
#include "gl_keepers.h"
#include "tao_gl.h"
#include "tao_glu.h"
#include <QPainterPath>
#include <QPainterPathStroker>
#include <iostream>

TAO_BEGIN

typedef GraphicPath::VertexData         VertexData;
typedef GraphicPath::PolygonData        PolygonData;
typedef GraphicPath::Vertices           Vertices;
typedef GraphicPath::DynamicVertices    DynamicVertices;
typedef GraphicPath::EndpointStyle      EndpointStyle;

scale GraphicPath::steps_min = 0;
scale GraphicPath::steps_increase = 2;
scale GraphicPath::steps_max = 25;

inline int pathSteps(scale length)
// ----------------------------------------------------------------------------
//   Compute the number of polygon sides when converting a path
// ----------------------------------------------------------------------------
{
    scale order = log2(length);
    scale steps = GraphicPath::steps_min
                + order * GraphicPath::steps_increase;
    if (steps > GraphicPath::steps_max)
        steps = GraphicPath::steps_max;
    if (steps < 1)
        steps = 1;
    return (int) ceil(steps);
}


#ifndef CALLBACK // Needed for Windows
#define CALLBACK
#endif

GraphicPath::GraphicPath()
// ----------------------------------------------------------------------------
//   Constructor
// ----------------------------------------------------------------------------
    : Shape(), startStyle(NONE), endStyle(NONE), lineStyle(Qt::SolidLine),
      invert(false)
{}


GraphicPath::~GraphicPath()
// ----------------------------------------------------------------------------
//   Destructor deletes all control points
// ----------------------------------------------------------------------------
{
    clear();
}


GraphicPath::PolygonData::~PolygonData()
// ----------------------------------------------------------------------------
//   Delete all the intersections we created
// ----------------------------------------------------------------------------
{
    DynamicVertices::iterator i;
    for (i = allocated.begin(); i != allocated.end(); i++)
        delete *i;
}


static void drawArrays(GLenum mode, uint64 textureUnits, Vertices &data)
// ----------------------------------------------------------------------------
//   Draw arrays
// ----------------------------------------------------------------------------
{
    double *vdata = &data[0].vertex.x;
    double *tdata = &data[0].texture.x;
    double *ndata = &data[0].normal.x;
    GLuint size = data.size();

    GL.VertexPointer(3,GL_DOUBLE,sizeof(VertexData), vdata);
    GL.NormalPointer(GL_DOUBLE, sizeof(VertexData), ndata);
    GL.EnableClientState(GL_VERTEX_ARRAY);
    GL.EnableClientState(GL_NORMAL_ARRAY);

    // Activate texture coordinates for all used units
    for(uint i = 0; i < GL.MaxTextureCoords() ; i++)
    {
        if(textureUnits & (1 << i))
        {
            GL.ClientActiveTexture( GL_TEXTURE0 + i );
            GL.EnableClientState(GL_TEXTURE_COORD_ARRAY);
            GL.TexCoordPointer(3, GL_DOUBLE, sizeof(VertexData), tdata);
        }
    }

    GL.DrawArrays(mode, 0, size);
    GL.DisableClientState(GL_VERTEX_ARRAY);
    GL.DisableClientState(GL_NORMAL_ARRAY);

    for(uint i = 0; i < GL.MaxTextureCoords() ; i++)
    {
        if(textureUnits & (1 << i))
        {
            GL.ClientActiveTexture( GL_TEXTURE0 + i );
            GL.DisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
    }

    // Restore the client active texture
    GL.ClientActiveTexture(GL_TEXTURE0);
}


static inline Vector3 swapXY(Vector3 v)
// ----------------------------------------------------------------------------
//   Swap X and Y coordinates (rotate 90 degrees)
// ----------------------------------------------------------------------------
{
    coord y = v.y;
    v.y = -v.x;
    v.x = y;
    v.Normalize();
    return v;
}


static inline void extrudeFacet(Vertices &side, VertexData &v,
                                Vector3 &orig, Vector3 &normal,
                                double r, double z, double sa, double ca)
// ----------------------------------------------------------------------------
//    Push a facet during extrusion
// ----------------------------------------------------------------------------
//    In practice, 'invert' is set for glyphs, which we have rendered with
//    inverted y. So we also need to invert the normal here.
{
    Vector3 vertex = orig + r * normal;
    vertex.z = z;

    Vector3 fn = normal;
    fn.z  = ca;
    fn.x *= sa;
    fn.y *= sa;

    v.vertex = vertex;
    v.normal = fn;
    side.push_back(v);
}


#define ROUNDING_EPSILON        0.001
#define BACKSTEP_VALUE          0.9

static void extrudeSide(Vertices &data, bool invert, uint64 textureUnits,
                        double r1, double z1, double sa1, double ca1,
                        double r2, double z2, double sa2, double ca2)
// ----------------------------------------------------------------------------
//   Extrude a side range
// ----------------------------------------------------------------------------
{
    uint       size = data.size();
    Vertices   side;
    VertexData v;
    Vector3    normal;

    side.reserve(2*size+2);
    for (uint s = 0; s <= size + 2; s++)
    {
        v = data[s%size];
        uint n = (s+1)%size;
        Vector3 delta = data[n].vertex - v.vertex;
        if (delta.Dot(delta) <= ROUNDING_EPSILON)
            continue;
        Vector3 newNormal = swapXY(delta);
        if (invert)
            newNormal *= -1.0;
        if (!s)
        {
            normal = newNormal;
            continue;
        }

        Vector3 orig = v.vertex;
        float dotProduct = newNormal.Dot(normal);
        if (dotProduct < BACKSTEP_VALUE)
        {
            Vector3 oldPos = v.vertex + r2 * normal;
            Vector3 newPos = v.vertex + r2 * newNormal;
            float backstep = (newPos-oldPos).Dot(delta);
            if (backstep < 0.0)
            {
                Vector3 mid = normal + newNormal;
                mid.Normalize();
                extrudeFacet(side, v, orig, mid, r1, z1, sa1, ca1);
                extrudeFacet(side, v, orig, mid, r2, z2, sa2, ca2);
                v.vertex = orig;
                normal = newNormal;
                continue;
            }
            else
            {
                // If we have a sharp angle, create additional face
                // so that OpenGL does not interpolate normals
                const uint MAX = 5;
                for (uint a = 0; a <= MAX; a++)
                {
                    Vector3 mid = normal * (MAX-a) + newNormal * a;
                    mid.Normalize();
                    extrudeFacet(side, v, orig, mid, r1, z1, sa1, ca1);
                    extrudeFacet(side, v, orig, mid, r2, z2, sa2, ca2);
                    v.vertex = orig;
                }
            }
        }

        normal = newNormal;
        extrudeFacet(side, v, orig, normal, r1, z1, sa1, ca1);
        extrudeFacet(side, v, orig, normal, r2, z2, sa2, ca2);
    }

    drawArrays(GL_TRIANGLE_STRIP, textureUnits, side);
}


static void extrude(PolygonData &poly, Vertices &data, scale depth)
// ----------------------------------------------------------------------------
//   Extrude a given set of vertices
// ----------------------------------------------------------------------------
{
    uint size = data.size();
    if (!size)
        return;

    Layout *layout = poly.layout;
    uint64 textureUnits = GL.ActiveTextureUnits();

    scale radius = layout->extrudeRadius;
    int count = layout->extrudeCount;

    bool sharpEdges = count < 0;
    if (sharpEdges)
        count = -count;

    // Check if we need to invert the normals on this contour
    bool invert = poly.path->invert;

    // Check if there is an extrude radius
    if (radius != 0 && count > 0)
    {
        if (depth < 2 * radius)
            radius = depth / 2;

        // Loop on the number of extrusion facets
        for (int c = 0; c < count; c++)
        {
            double a1  = M_PI/2 * c / count;
            double ca1 = cos (a1);
            double sa1 = sin (a1);
            double a2  = M_PI/2 * (c+1) / count;
            double ca2 = cos (a2);
            double sa2 = sin (a2);
            double z1 = -radius * (1 - ca1);
            double z2 = -radius * (1 - ca2);
            double r1 = radius * sa1;
            double r2 = radius * sa2;

            if (sharpEdges)
                extrudeSide(data, invert, textureUnits,
                            r1, z1, sa2, ca2, r2, z2, sa2, ca2);
            else
                extrudeSide(data, invert, textureUnits,
                            r1, z1, sa1, ca1, r2, z2, sa2, ca2);
            z1 = -depth - z1;
            z2 = -depth - z2;
            if (sharpEdges)
                extrudeSide(data, invert, textureUnits,
                            r2, z2, sa2, -ca2, r1, z1, sa2, -ca2);
            else
                extrudeSide(data, invert, textureUnits,
                            r2, z2, sa2, -ca2, r1, z1, sa1, -ca1);
        }

        if (depth > 2 * radius)
            extrudeSide(data, invert, textureUnits,
                        radius, -radius, 1, 0, radius, radius - depth, 1, 0);
    }
    else // Optimized case for 0 radius
    {
        Vertices side;
        VertexData v;
        Vector3 normal;

        side.reserve(2*size);
        for (uint s = 0; s <= size + 2; s++)
        {
            v = data[s%size];
            uint n = (s+1) % size;
            Vector3 delta = data[n].vertex - v.vertex;
            if (delta.Dot(delta) <= ROUNDING_EPSILON)
                continue;
            Vector3 newNormal = swapXY(delta);
            if (invert)
                newNormal *= -1.0;
            if (!s)
            {
                normal = newNormal;
                continue;
            }

            coord origZ = v.vertex.z;
            float dotProduct = newNormal.Dot(normal);
            if (dotProduct < BACKSTEP_VALUE)
            {
                // If we have a sharp angle, create additional face
                // so that OpenGL does not interpolate normals
                v.normal = normal;
                side.push_back(v);
                v.vertex.z -= depth;
                side.push_back(v);
                v.vertex.z = origZ;
            }
            normal = newNormal;

            v.normal = normal;
            side.push_back(v);
            v.vertex.z -= depth;
            side.push_back(v);
        }

        drawArrays(GL_TRIANGLE_STRIP, textureUnits, side);
    }
}


static void CALLBACK tessBegin(GLenum mode, PolygonData *poly)
// ----------------------------------------------------------------------------
//   Callback for beginning of rendering
// ----------------------------------------------------------------------------
{
    poly->mode = mode;
    poly->vertices.clear();
}


static void CALLBACK tessVertex(VertexData *vertex, PolygonData *poly)
// ----------------------------------------------------------------------------
//   Emit a vertex during tesselation
// ----------------------------------------------------------------------------
{
    poly->vertices.push_back(*vertex);
    VertexData &back = poly->vertices.back();
    back.normal = Vector3(0, 0, 1);
}


static void CALLBACK tessEnd(PolygonData *poly)
// ----------------------------------------------------------------------------
//   Callback for end of rendering
// ----------------------------------------------------------------------------
{
    Vertices &data = poly->vertices;
    uint size = data.size();
    if (size)
    {
        Layout *layout = poly->layout;
        uint64 textureUnits = GL.ActiveTextureUnits();
        double depth = layout->extrudeDepth;
        if (depth > 0.0)
        {
            bool invert = poly->path->invert;
            GL.Sync();
            // REVISIT: Replace pushMatrix/popMatrix by
            // GL.Save() (Find why it occurs a bug with outline). Refs #3040.
            glPushMatrix();
            glTranslatef(0.0, 0.0, -depth);
            glScalef(1, 1, -1);
            GL.FrontFace(invert ? GL_CCW : GL_CW);
            drawArrays(poly->mode, textureUnits, data);
            GL.FrontFace(invert ? GL_CW : GL_CCW);
            glPopMatrix();
        }
        drawArrays(poly->mode, textureUnits, data);
        data.clear();
     }
}


static void CALLBACK tessCombine(GLdouble coords[3],
                                 VertexData *vertex[4],
                                 GLfloat weight[4], VertexData **dataOut,
                                 PolygonData *polygon)
// ----------------------------------------------------------------------------
//   Combine vertex data when the polygon self-intersects
// ----------------------------------------------------------------------------
{
    Point3 pos(coords[0], coords[1], coords[2]);

    // The documentation states that all pointers are "valid", but valid
    // seems to mean they can be NULL if we come from SpliceMergeVertices
    Point3 tex =          weight[0] * vertex[0]->texture;
    if (vertex[1]) tex += weight[1] * vertex[1]->texture;
    if (vertex[2]) tex += weight[2] * vertex[2]->texture;
    if (vertex[3]) tex += weight[3] * vertex[3]->texture;
    VertexData *result = new VertexData(pos, tex, -1);
    polygon->allocated.push_back(result);
    *dataOut = result;
}


void GraphicPath::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the graphic path using the current texture, fill and line color
// ----------------------------------------------------------------------------
{
    Draw(where, 0);
}


static scale getShortenByStyle(EndpointStyle style, scale lw)
// ----------------------------------------------------------------------------
//   Returns the length to remove from a path for a particular endpoint style
// ----------------------------------------------------------------------------
{
    switch (style)
    {
        case GraphicPath::HOLLOW_CIRCLE:
        case GraphicPath::HOLLOW_SQUARE:
            return 3.5 * lw;
        case GraphicPath::FLETCHING:
            return 3.0 * lw;
        case GraphicPath::ARROWHEAD:
        case GraphicPath::TRIANGLE:
        case GraphicPath::POINTER:
        case GraphicPath::DIAMOND:
            return 2.0 * lw;
        case GraphicPath::CIRCLE:
        case GraphicPath::SQUARE:
            return 1.5 * lw;
        case GraphicPath::CUP:
            return 1.0 * lw;
        case GraphicPath::ROUNDED:
        case GraphicPath::BAR:
            return 0.5 * lw;
        case GraphicPath::NONE:
        default:
            return 0.0;
    }
}


static void addEndpointToPath(EndpointStyle style,
                              Point3 endpoint, Vector3 heading,
                              GraphicPath &outline)
// ----------------------------------------------------------------------------
//   Adds the slyle decoration to the outline path
// ----------------------------------------------------------------------------
{
    switch (style)
    {
    case GraphicPath::ARROWHEAD:
    {
        Point3 p2 = endpoint + 2.0 * heading;
        Point3 p3 = endpoint + 1.5 * heading;
        Point3 p4 = p2;
        p2.x += heading.y;
        p2.y -= heading.x;
        p4.x -= heading.y;
        p4.y += heading.x;
        outline.moveTo(endpoint);
        outline.lineTo(p2);
        outline.lineTo(p3);
        outline.lineTo(p4);
        outline.lineTo(endpoint);
    }
    break;
    case GraphicPath::TRIANGLE:
    {
        Point3 p2 = endpoint + 2.0 * heading;
        Point3 p3 = p2;
        p2.x += heading.y;
        p2.y -= heading.x;
        p3.x -= heading.y;
        p3.y += heading.x;
        outline.moveTo(endpoint);
        outline.lineTo(p2);
        outline.lineTo(p3);
        outline.lineTo(endpoint);
    }
    break;
    case GraphicPath::POINTER:
    {
        Point3 p1 = endpoint + heading / 4.0;
        Point3 p2 = endpoint + 7.0 / 4.0 * heading;
        Point3 p3 = p2;
        Point3 p4 = endpoint + heading;
        p2.x += 3.0 / 4.0 * heading.y;
        p2.y -= 3.0 / 4.0 * heading.x;
        p3.x -= 3.0 / 4.0 * heading.y;
        p3.y += 3.0 / 4.0 * heading.x;

        QPainterPath pointer;
        pointer.moveTo(p1.x, p1.y);
        pointer.lineTo(p4.x, p4.y);
        pointer.moveTo(p2.x, p2.y);
        pointer.lineTo(p1.x, p1.y);
        pointer.lineTo(p3.x, p3.y);

        QPainterPathStroker stroker;
        stroker.setWidth(heading.Length() / 2.0);
        stroker.setCapStyle(Qt::FlatCap);
        stroker.setJoinStyle(Qt::RoundJoin);
        stroker.setDashPattern(Qt::SolidLine);

        QPainterPath path = stroker.createStroke(pointer);
        outline.addQtPath(path);
    }
    break;
    case GraphicPath::DIAMOND:
    {
        Point3 p2 = endpoint + 1.0 * heading;
        Point3 p3 = endpoint + 2.0 * heading;
        Point3 p4 = p2;
        p2.x += heading.y;
        p2.y -= heading.x;
        p4.x -= heading.y;
        p4.y += heading.x;
        outline.moveTo(endpoint);
        outline.lineTo(p2);
        outline.lineTo(p3);
        outline.lineTo(p4);
        outline.lineTo(endpoint);
    }
    break;
    case GraphicPath::CIRCLE:
    {
        Point3 c = endpoint + heading;
        scale r = heading.Length();
        QPainterPath circle;
        circle.addEllipse(c.x-r, c.y-r, 2*r, 2*r);
        outline.addQtPath(circle);
    }
    break;
    case GraphicPath::SQUARE:
    {
        Point3 p1 = endpoint;
        Point3 p2 = p1;
        Point3 p3 = endpoint + 2.0 * heading;
        Point3 p4 = p3;
        p2.x += heading.y;
        p2.y -= heading.x;
        p1.x -= heading.y;
        p1.y += heading.x;
        p3.x += heading.y;
        p3.y -= heading.x;
        p4.x -= heading.y;
        p4.y += heading.x;
        outline.moveTo(endpoint);
        outline.lineTo(p2);
        outline.lineTo(p3);
        outline.lineTo(p4);
        outline.lineTo(p1);
        outline.lineTo(endpoint);
    }
    break;
    case GraphicPath::BAR:
    {
        Point3 p1 = endpoint;
        Point3 p2 = p1;
        Point3 p3 = endpoint + 2.0 * heading;
        Point3 p4 = p3;
        p2.x += 4.0 * heading.y;
        p2.y -= 4.0 * heading.x;
        p1.x -= 4.0 * heading.y;
        p1.y += 4.0 * heading.x;
        p3.x += 4.0 * heading.y;
        p3.y -= 4.0 * heading.x;
        p4.x -= 4.0 * heading.y;
        p4.y += 4.0 * heading.x;
        outline.moveTo(endpoint);
        outline.lineTo(p2);
        outline.lineTo(p3);
        outline.lineTo(p4);
        outline.lineTo(p1);
        outline.lineTo(endpoint);
    }
    break;
    case GraphicPath::CUP:
    {
        Point3 p2 = endpoint;
        Point3 p3 = endpoint + 2.0 * heading;
        Point3 p4 = p2;
        p2.x += 2.0 * heading.y;
        p2.y -= 2.0 * heading.x;
        p4.x -= 2.0 * heading.y;
        p4.y += 2.0 * heading.x;
        outline.moveTo(endpoint);
        outline.lineTo(p2);
        outline.lineTo(p3);
        outline.lineTo(p4);
        outline.lineTo(endpoint);
    }
    break;
    case GraphicPath::FLETCHING:
    {
        Point3 p1 = endpoint + heading / 3.0;
        Point3 p2 = endpoint;
        Point3 p3 = endpoint + 4.0 / 3.0 * heading;
        Point3 p4 = endpoint + 2.0 * heading;
        Point3 p5 = p3;
        Point3 p6 = p2;
        p2.x += 2.0 / 3.0 * heading.y;
        p2.y -= 2.0 / 3.0 * heading.x;
        p3.x += 1.0 / 2.0 * heading.y;
        p3.y -= 1.0 / 2.0 * heading.x;
        p5.x -= 1.0 / 2.0 * heading.y;
        p5.y += 1.0 / 2.0 * heading.x;
        p6.x -= 2.0 / 3.0 * heading.y;
        p6.y += 2.0 / 3.0 * heading.x;
        outline.moveTo(p1);
        outline.lineTo(p2);
        outline.lineTo(p3);
        outline.lineTo(p4);
        outline.lineTo(p5);
        outline.lineTo(p6);
        outline.lineTo(p1);
    }
    break;
    case GraphicPath::HOLLOW_CIRCLE:
    {
        Point3 c = endpoint + 4.0 / 7.0 * heading;
        scale r = 3.0 / 7.0 * heading.Length();

        QPainterPath circle;
        circle.addEllipse(c.x-r, c.y-r, 2*r, 2*r);

        QPainterPathStroker stroker;
        stroker.setWidth(2.0 / 7.0 * heading.Length());
        stroker.setCapStyle(Qt::FlatCap);
        stroker.setJoinStyle(Qt::RoundJoin);
        stroker.setDashPattern(Qt::SolidLine);

        QPainterPath path = stroker.createStroke(circle);
        outline.addQtPath(path);
    }
    break;
    case GraphicPath::HOLLOW_SQUARE:
    {
        Point3 p1 = endpoint +  1.0 / 7.0 * heading;
        Point3 p2 = p1;
        Point3 p3 = endpoint + heading;
        Point3 p4 = p3;
        p2.x += 3.0 / 7.0 * heading.y;
        p2.y -= 3.0 / 7.0 * heading.x;
        p1.x -= 3.0 / 7.0 * heading.y;
        p1.y += 3.0 / 7.0 * heading.x;
        p3.x += 3.0 / 7.0 * heading.y;
        p3.y -= 3.0 / 7.0 * heading.x;
        p4.x -= 3.0 / 7.0 * heading.y;
        p4.y += 3.0 / 7.0 * heading.x;

        QPainterPath square;
        square.moveTo(p1.x, p1.y);
        square.lineTo(p2.x, p2.y);
        square.lineTo(p3.x, p3.y);
        square.lineTo(p4.x, p4.y);
        square.lineTo(p1.x, p1.y);

        QPainterPathStroker stroker;
        stroker.setWidth(2.0 / 7.0 * heading.Length());
        stroker.setCapStyle(Qt::FlatCap);
        stroker.setJoinStyle(Qt::RoundJoin);
        stroker.setDashPattern(Qt::SolidLine);

        QPainterPath path = stroker.createStroke(square);
        outline.addQtPath(path);
    }
    break;
    case GraphicPath::ROUNDED:
    {
        Point3 c = endpoint + heading;
        scale r = heading.Length();
        QPainterPath circle;
        circle.addEllipse(c.x-r, c.y-r, 2*r, 2*r);
        outline.addQtPath(circle);
    }
    break;
    case GraphicPath::NONE:
    default:
        break;
    }
}


void GraphicPath::Draw(Layout *where, GLenum tessel)
// ----------------------------------------------------------------------------
//   Draw the graphic path using the current texture, fill and line color
// ----------------------------------------------------------------------------
{
    GLAllStateKeeper save;

    // Sync lighting state only if we have lights or shaders
    if(GL.LightsMask() || where->programId)
        GL.Sync(STATE_lights);

    GL.LoadMatrix();

    // Do not bother setting up textures and programs if we are in selection
    if (tessel != GL_SELECT)
        setTexture(where);
    else
        tessel = 0;

    if (where->extrudeDepth > 0.0)
    {
        bool hasFill = setFillColor(where);
        if (hasFill)
            Draw(where, where->offset, GL_POLYGON, tessel);
        bool hasLine = setLineColor(where);
        if (hasLine)
            DrawOutline(where);
        if (hasFill || hasLine)
            Draw(where, where->offset, GL_POLYGON, GL_DEPTH);
    }
    else
    {
        if (setFillColor(where))
            Draw(where, where->offset, GL_POLYGON, tessel);
        if (setLineColor(where))
            DrawOutline(where);
    }
}


void GraphicPath::DrawOutline(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the outline for the current path
// ----------------------------------------------------------------------------
{
    GraphicPath outline;

    bool first = true;
    bool last = true;
    Kind endKind = MOVE_TO;
    Point3 startPoint;
    Point3 endPoint;
    Point3* endPointPtr;
    Vector3 startHeading;
    Vector3 endHeading;
    scale length;
    scale shortenBy;

    path_elements::iterator begin = elements.begin(), end = elements.end();
    path_elements::iterator p;
    for (path_elements::iterator i = begin; i != end; i++)
    {
        if (first)
        {
            shortenBy = getShortenByStyle(startStyle, where->lineWidth);

            switch ((*i).kind)
            {
            case MOVE_TO:
                startPoint = (*i).position;
                break;
            case LINE_TO:
            case CURVE_TO:
                // First line
                startHeading = (*i).position - startPoint;
                length = startHeading.Length();
                if (length == 0)
                {
                    startPoint = (*i).position;
                }
                else
                {
                    startHeading.Normalize();
                    startHeading *= shortenBy;
                    if(length > shortenBy)
                    {
                        outline.moveTo(startPoint + startHeading);
                    }
                    else
                    {
                        outline.moveTo(startPoint);
                        startPoint -= startHeading;
                    }
                    p=i;
                    first = false;
                }
                break;
            case CURVE_CONTROL:
                // First curve
                startHeading = (*i).position - startPoint;
                length = startHeading.Length();
                if (length == 0)
                {
                    startPoint = (*i).position;
                }
                else
                {
                    startHeading.Normalize();
                    startHeading *= shortenBy;
                    outline.moveTo(startPoint);
                    startPoint -= startHeading;
                    p = i;
                    first = false;
                }
                break;
            }

        }
        else
        {
            if ((*p).position == (*i).position)
            {
                if ((*i).kind == CURVE_TO)
                {
                    p=i;
                }
            }
            else
            {
                outline.elements.push_back(*p);
                p = i;
            }
        }
    }
    if (!first)
    {
        outline.elements.push_back(*p);
    }

    path_elements::reverse_iterator rbegin = outline.elements.rbegin();
    path_elements::reverse_iterator rend = outline.elements.rend();
    for (path_elements::reverse_iterator j, i = rbegin; i != rend; ++i)
    {
        j = i+1;
        if (last)
        {
            shortenBy = getShortenByStyle(endStyle, where->lineWidth);

            switch ((*i).kind)
            {
            case LINE_TO:
                // Last line
                endKind = LINE_TO;
                break;
            case CURVE_TO:
            case CURVE_CONTROL:
                // Last curve or line
                if ((*j).kind == CURVE_CONTROL)
                    endKind = CURVE_TO;
                else
                    endKind = LINE_TO;
                break;
            case MOVE_TO:
                endKind = MOVE_TO;
                break;
            }
            endPoint = (*i).position;
            endPointPtr = &(*i).position;
            endHeading = (*j).position - (*i).position;
            length = endHeading.Length();
            if (length == 0)
            {
                endKind = MOVE_TO;
            }
            else
            {
                endHeading.Normalize();
                endHeading *= shortenBy;
                if (endKind == LINE_TO && length > shortenBy)
                {
                    *endPointPtr += endHeading;
                }
                else
                {
                    endPoint -= endHeading;
                }
            }
            if (endKind != MOVE_TO)
            {
                last = false;
            }
        }
    }


    // Draw outline of the path.
    QPainterPath path;
    if (extractQtPath(outline, path))
    {
        // Path is flat: use a QPainterPathStroker
        QPainterPathStroker stroker;
        stroker.setWidth(where->lineWidth);
        stroker.setCapStyle(Qt::FlatCap);
        stroker.setJoinStyle(Qt::RoundJoin);
        stroker.setDashPattern(lineStyle);
        QPainterPath stroke = stroker.createStroke(path);
        outline.clear();
        outline.addQtPath(stroke);

        // Draw the endpoints
        if (!first)
            addEndpointToPath(startStyle,startPoint,startHeading,outline);
        if (!last)
            addEndpointToPath(endStyle, endPoint, endHeading, outline);

        outline.Draw(where, where->offset,
                     GL_POLYGON, GLU_TESS_WINDING_POSITIVE);
    }
    else
    {
        // Path is not flat: use GL lines (temporarily)
        Draw(where, where->offset, GL_LINE_STRIP, 0);
    }
}


void GraphicPath::Draw(Layout *layout,
                       const Vector3 &offset,
                       GLenum mode, GLenum tesselation)
// ----------------------------------------------------------------------------
//   Draw the graphic path using curves with the given mode and tesselation
// ----------------------------------------------------------------------------
{
    PolygonData polygon (this);        // Polygon information
    Vertices &data = polygon.vertices;
    Vertices control;           // Control points
    path_elements::iterator i, begin = elements.begin(), end = elements.end();
    polygon.layout = layout;
    uint textureUnits = GL.ActiveTextureUnits();
    uint vertexIndex = 0;
    scale depth = layout->extrudeDepth;

    // Check if we need to tesselate polygon
    static GLUtesselator *tess = 0;
    if (tesselation && tesselation != GL_DEPTH)
    {
        if (!tess)
        {
            tess = gluNewTess();
            typedef void CALLBACK (*fn)();
            gluTessCallback(tess, GLU_TESS_BEGIN_DATA,   fn(tessBegin));
            gluTessCallback(tess, GLU_TESS_END_DATA,     fn(tessEnd));
            gluTessCallback(tess, GLU_TESS_VERTEX_DATA,  fn(tessVertex));
            gluTessCallback(tess, GLU_TESS_COMBINE_DATA, fn(tessCombine));
        }

        gluTessProperty(tess, GLU_TESS_WINDING_RULE, tesselation);
        gluTessBeginPolygon(tess, &polygon);
    }

    for (i = begin; i != end; i++)
    {
        Element    &e    = *i;
        Kind        kind = e.kind;
        Point3      pos  = e.position + offset; // Geometry coordinates
        Point3      tex  = e.position / bounds; // Texture coordinates
        VertexData  here(pos, tex, vertexIndex++);
        uint        size;

        switch (kind)
        {
        default:
            std::cerr << "GraphicPath::Draw: Unexpected element kind\n";
            break;

        case LINE_TO:
            data.push_back(here);
            // Fall through

        case MOVE_TO:
            control.clear();
            // Fall through

        case CURVE_CONTROL:
            control.push_back(here);
            break;

        case CURVE_TO:
            control.push_back(here);

            // Optimize common sizes, give up for the rest:
            size = control.size();
            switch (size)
            {
            case 1:
            case 2:
                // Simple line, just push the last point
                std::cerr << "Got line in CURVE_TO\n";
                data.push_back(here);
                break;

            case 3:
            {
                // Quadratic Bezier curve
                Vector3& v0 = control[0].vertex;
                Vector3& v1 = control[1].vertex;
                Vector3& v2 = control[2].vertex;
                Vector3& t0 = control[0].texture;
                Vector3& t1 = control[1].texture;
                Vector3& t2 = control[2].texture;

                // REVISIT: Implement a true optimization of the interpolation
                // Compute a good number of points for approximating the curve
                scale length = (v2-v0).Length() + 1;
                uint steps = pathSteps(length);
                double dt = 1.0 / steps;
                double lt = 1.0 + dt/2;

                // Emit the points
                for (double t = 0.0; t < lt; t += dt)
                {
                    double tt = t*t;
                    double m = 1-t;
                    double mm = m*m;
                    Vector3 vd = mm * v0 + 2*m*t * v1 + tt * v2;
                    Vector3 td = mm * t0 + 2*m*t * t1 + tt * t2;
                    VertexData intermediate(vd, td, vertexIndex++);
                    data.push_back(intermediate);
                }
                break;
                }

            default:
                std::cerr << "GraphicPath::Draw: Unimplemented Bezier"
                          << " (order " << size << ")\n";
                break;
            case 4:
            {
                // Cubic Bezier curve
                Vector3& v0 = control[0].vertex;
                Vector3& v1 = control[1].vertex;
                Vector3& v2 = control[2].vertex;
                Vector3& v3 = control[3].vertex;
                Vector3& t0 = control[0].texture;
                Vector3& t1 = control[1].texture;
                Vector3& t2 = control[2].texture;
                Vector3& t3 = control[3].texture;

                // Compute a good number of points for approximating the curve
                scale length = (v2-v0).Length() + 1;
                uint steps = pathSteps(length);
                double dt = 1.0 / steps;
                double lt = 1.0 + dt/2;

                for (double t = 0.0; t < lt; t += dt)
                {
                    double tt = t*t;
                    double ttt = t*tt;
                    double m = 1-t;
                    double mm = m*m;
                    double mmm = m*mm;
                    Vector3 vd = mmm*v0 + 3*mm*t*v1 + 3*m*tt*v2 + ttt*v3;
                    Vector3 td = mmm*t0 + 3*mm*t*t1 + 3*m*tt*t2 + ttt*t3;
                    VertexData intermediate(vd, td, vertexIndex++);
                    data.push_back(intermediate);
                }
                break;
            }
            } // switch(size)

            control.clear();
            control.push_back(here);
        } // switch(kind)

        // Check if we need to emit what existed before
        if (i+1 == end || kind == MOVE_TO)
        {
            // Get size of previous elements
            if (uint size = data.size())
            {
                if (depth > 0.0)
                {
                    if (!tesselation)
                    {
                        // If no tesselation is required, draw back directly
                        GraphicSave* save = GL.Save();
                        GL.Translate(0.0, 0.0, -depth);
                        GL.Scale(1, 1, -1);
                        GL.FrontFace(GL_CW);
                        drawArrays(mode, textureUnits, data);
                        GL.FrontFace(GL_CCW);
                        GL.Restore(save);
                    }
                    extrude(polygon, data, depth);
                }

                // Pass the data we had so far to OpenGL and clear it
                if (tesselation != GL_DEPTH)
                {
                    if (tesselation)
                    {
                        gluTessBeginContour(tess);
                        for (uint j = 0; j < size; j++)
                        {
                            VertexData *dynv = new VertexData(data[j]);
                            polygon.allocated.push_back(dynv);
                            gluTessVertex(tess, &dynv->vertex.x, dynv);
                        }
                        gluTessEndContour(tess);
                    }
                    else
                    {
                        // If no tesselation is required, draw directly
                        drawArrays(mode, textureUnits, data);
                    }
                }

                data.clear();
            }

            data.push_back(here);
        }

    } // Loop on all elements

    // End of tesselation
    if (tesselation && tesselation != GL_DEPTH)
    {
        gluTessEndPolygon(tess);
    }
}


GraphicPath& GraphicPath::moveTo(Point3 dst)
// ----------------------------------------------------------------------------
//   Insert a 'move-to' into the path
// ----------------------------------------------------------------------------
{
    elements.push_back(Element(MOVE_TO, dst));
    start = dst;
    position = dst;
    bounds |= dst;
    return *this;
}


GraphicPath& GraphicPath::lineTo(Point3 dst)
// ----------------------------------------------------------------------------
//   Insert a 'line-to' into the path
// ----------------------------------------------------------------------------
{
    if (!elements.size())
        moveTo(dst);

    elements.push_back(Element(LINE_TO, dst));
    position = dst;
    bounds |= dst;
    return *this;
}


GraphicPath& GraphicPath::curveTo(Point3 control, Point3 dst)
// ----------------------------------------------------------------------------
//   Insert a quadratic curve
// ----------------------------------------------------------------------------
{
    if (!elements.size())
        moveTo(start);

    elements.push_back(Element(CURVE_CONTROL, control));
    elements.push_back(Element(CURVE_TO, dst));
    position = dst;
    bounds |= control;
    bounds |= dst;
    return *this;
}


GraphicPath& GraphicPath::curveTo(Point3 c1, Point3 c2, Point3 dst)
// ----------------------------------------------------------------------------
//   Insert a cubic curve
// ----------------------------------------------------------------------------
{
    if (!elements.size())
        moveTo(start);

    elements.push_back(Element(CURVE_CONTROL, c1));
    elements.push_back(Element(CURVE_CONTROL, c2));
    elements.push_back(Element(CURVE_TO, dst));
    position = dst;
    bounds |= c1;
    bounds |= c2;
    bounds |= dst;
    return *this;
}


GraphicPath& GraphicPath::close()
// ----------------------------------------------------------------------------
//   Close a path by returning to the starting point
// ----------------------------------------------------------------------------
{
    if (elements.size())
        lineTo(start);
    return *this;
}


GraphicPath& GraphicPath::addQtPath(QPainterPath &qt, scale sy)
// ----------------------------------------------------------------------------
//   Add a QT path
// ----------------------------------------------------------------------------
{
    // Check that Kind and QPainterPath::ElementType numerical values match
    XL_CASSERT (int(QPainterPath::MoveToElement) == int(MOVE_TO) &&
                int(QPainterPath::LineToElement) == int(LINE_TO) &&
                int(QPainterPath::CurveToElement) == int(CURVE_TO) &&
                int(QPainterPath::CurveToDataElement) == int(CURVE_CONTROL));

    if (sy < 0)
        invert = true;

    // Loop on the Qt Path and insert elements
    // Qt paths place CURVE_TO before control points, we place it last
    uint i, max = qt.elementCount();
    bool hadControl = false;
    for (i = 0; i < max; i++)
    {
        const QPainterPath::Element &e = qt.elementAt(i);
        Kind kind = Kind(e.type);
        position = Point3(e.x, sy * e.y, 0);
        if (kind == CURVE_CONTROL)
        {
            hadControl = true;
            if (i+1 == max)
                kind = CURVE_TO;
        }
        else
        {
            if (hadControl)
            {
                elements.back().kind = CURVE_TO;
                hadControl = false;
            }
            if (kind == CURVE_TO)
                kind = CURVE_CONTROL;
        }
        elements.push_back(Element(kind, position));
        bounds |= position;
        if (i == 0 && kind == MOVE_TO)
            start = position;
    }

    return *this;
}


bool GraphicPath::extractQtPath(GraphicPath &in, QPainterPath &out)
// ----------------------------------------------------------------------------
//   Extract a QT path from a graphic path. Returns true if path was flat
// ----------------------------------------------------------------------------
{
    QPainterPath path;
    bool flat = true;

    // Check that Kind and QPainterPath::ElementType numerical values match
    XL_CASSERT (int(QPainterPath::MoveToElement) == int(MOVE_TO) &&
                int(QPainterPath::LineToElement) == int(LINE_TO) &&
                int(QPainterPath::CurveToElement) == int(CURVE_TO) &&
                int(QPainterPath::CurveToDataElement) == int(CURVE_CONTROL));

    // Loop on the Graphic Path and insert elements in the QT Path
    // Qt paths place CURVE_TO before control points, we place it last
    uint i, max = in.elements.size();
    Point3 p, c1, c2;
    uint control = 0;
    for (i = 0; i < max; i++)
    {
        const GraphicPath::Element &e = in.elements[i];
        p = e.position;
        switch(e.kind)
        {
        case GraphicPath::MOVE_TO:
            path.moveTo(p.x, p.y);
            if (p.z != 0.0)
                flat = false;
            break;
        case GraphicPath::LINE_TO:
            path.lineTo(p.x, p.y);
            if (p.z != 0.0)
                flat = false;
            break;
        case GraphicPath::CURVE_TO:
            switch (control)
            {
                case 0:
                    path.lineTo(p.x, p.y);
                    break;
                case 1:
                    path.quadTo(c1.x, c1.y, p.x, p.y);
                    break;
                case 2:
                default:
                    path.cubicTo(c1.x, c1.y, c2.x, c2.y, p.x, p.y);
                    break;
            }
            control = 0;
            if (p.z != 0.0)
                flat = false;
            break;
        case GraphicPath::CURVE_CONTROL:
            switch (++control)
            {
                case 1:
                    c1 = p;
                    break;
                case 2:
                    c2 = p;
                    break;
                default:
                    assert(!"Curve should not exceed a cubic form");
            }
            break;
        }
    }

    out = path;
    return flat;
}


bool GraphicPath::extractQtPath(QPainterPath &out)
// ----------------------------------------------------------------------------
//   Extract a QT path from the graphic path. Returns true if path was flat
// ----------------------------------------------------------------------------
{
    return extractQtPath(*this, out);
}


void GraphicPath::Draw(Layout *where, QPainterPath &qtPath,
                       GLenum tessel, scale sy)
// ----------------------------------------------------------------------------
//   Draw the graphic path using the current texture, fill and line color
// ----------------------------------------------------------------------------
{
    GraphicPath path;
    path.addQtPath(qtPath, sy);
    path.Draw(where, tessel);
}


Box3 GraphicPath::Bounds(Layout *where)
// ----------------------------------------------------------------------------
//   Return the bounding box, computed from all path elements
// ----------------------------------------------------------------------------
{
    return bounds + where->offset;
}


void GraphicPath::clear()
// ----------------------------------------------------------------------------
//    Clear a path
// ----------------------------------------------------------------------------
{
    elements.clear();
    start = position = Point3();
    bounds = Box3();

    // Delete the control points
    control_points::iterator i;
    for (i = controls.begin(); i != controls.end(); i++)
        delete *i;
    controls.clear();
}


void GraphicPath::AddControl(XL::Tree *self, bool onCurve,
                             Real *x, Real *y, Real *z)
// ----------------------------------------------------------------------------
//   Add a control point to a path
// ----------------------------------------------------------------------------
{
    controls.push_back(new ControlPoint(self, onCurve,
                                        x, y, z, controls.size() + 1));
}


void GraphicPath::DrawSelection(Layout *layout)
// ----------------------------------------------------------------------------
//   Draw the control points
// ----------------------------------------------------------------------------
{
    Widget *widget = layout->Display();
    uint sel = widget->selected(layout);

    if (sel > 1)
    {
        // Show the control points
        widget->selectionContainerPush();
        control_points::iterator i;
        for (i = controls.begin(); i != controls.end(); i++)
        {
            ControlPoint *child = *i;
            child->DrawSelection(layout);
        }
        widget->selectionContainerPop();
    }
    else if (sel)
    {
        // Show bounding box
        Drawing::DrawSelection(layout);
    }
}


void GraphicPath::Identify(Layout *layout)
// ----------------------------------------------------------------------------
//   Identify the control points
// ----------------------------------------------------------------------------
{
    Draw(layout);
    Widget *widget = layout->Display();
    if (widget->selected(layout))
    {
        GL.PushName(layout->id);
        control_points::iterator i;
        for (i = controls.begin(); i != controls.end(); i++)
        {
            ControlPoint *child = *i;
            child->Identify(layout);
        }
        GL.PopName();
    }
}


GraphicPathInfo::GraphicPathInfo(GraphicPath *path)
// ----------------------------------------------------------------------------
//   Store information about a GraphicPath
// ----------------------------------------------------------------------------
{
    b0 = path->bounds;
    GraphicPath::control_points::iterator i;
    for (i = path->controls.begin(); i != path->controls.end(); i++)
    {
        ControlPoint *child = *i;
        controls.push_back(Point3(child->x, child->y, child->z));
    }
}


void TesselatedPath::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the graphic path using the current texture, fill and line color
// ----------------------------------------------------------------------------
{
    GraphicPath::Draw(where, tesselation);
}

TAO_END
