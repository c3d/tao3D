// ****************************************************************************
//  path3d.cpp                                                      Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "path3d.h"
#include "layout.h"
#include "gl_keepers.h"
#include <GL/glew.h>
#include <QtOpenGL>
#include <QPainterPath>
#include <iostream>

TAO_BEGIN

typedef GraphicPath::VertexData         VertexData;
typedef GraphicPath::PolygonData        PolygonData;
typedef GraphicPath::Vertices           Vertices;
typedef GraphicPath::DynamicVertices    DynamicVertices;


GraphicPath::PolygonData::~PolygonData()
// ----------------------------------------------------------------------------
//   Delete all the intersections we created
// ----------------------------------------------------------------------------
{
    DynamicVertices::iterator i;
    for (i = allocated.begin(); i != allocated.end(); i++)
        delete *i;
}


static void tessBegin(GLenum mode, PolygonData *poly)
// ----------------------------------------------------------------------------
//   Callback for beginning of rendering
// ----------------------------------------------------------------------------
{
    (void) poly;
    glBegin(mode);
}


static void tessEnd(PolygonData *poly)
// ----------------------------------------------------------------------------
//   Callback for end of rendering
// ----------------------------------------------------------------------------
{
    (void) poly;
    glEnd();
}


static void tessVertex(VertexData *vertex, PolygonData *poly)
// ----------------------------------------------------------------------------
//   Emit a vertex during tesselation
// ----------------------------------------------------------------------------
{
    (void) poly;
    glVertex3dv(&vertex->vertex.x);
    glTexCoord3dv(&vertex->texture.x);
}


static void tessCombine(GLdouble coords[3],
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
    Point3 tex = weight[0] * vertex[0]->texture;
    if (vertex[1]) tex += weight[1] * vertex[1]->texture;
    if (vertex[2]) tex += weight[2] * vertex[2]->texture;
    if (vertex[3]) tex += weight[3] * vertex[3]->texture;
    VertexData *result = new VertexData(pos, tex);
    polygon->allocated.push_back(result);
    *dataOut = result;
}


void GraphicPath::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the graphic path using the current texture, fill and line color
// ----------------------------------------------------------------------------
{
    setTexture(where);
    if (setFillColor(where))
        Draw(where, GL_POLYGON);
    if (setLineColor(where))
        Draw(where, GL_LINE_STRIP);
}


void GraphicPath::Draw(Layout *where, GLenum mode, GLenum tesselation)
// ----------------------------------------------------------------------------
//   Draw the graphic path using curves with the given mode and tesselation
// ----------------------------------------------------------------------------
{
    Vector3 offset = where->Offset();
    PolygonData polygon;        // Polygon information
    Vertices &data = polygon.vertices;
    Vertices control;           // Control points
    path_elements::iterator i, begin = elements.begin(), end = elements.end();

    // Check if we need to tesselate polygon
    static GLUtesselator *tess = 0;
    if (tesselation)
    {
        if (!tess)
        {
            tess = gluNewTess();
            typedef GLvoid (*fn)();
            gluTessCallback(tess, GLU_TESS_BEGIN_DATA,   fn(tessBegin));
            gluTessCallback(tess, GLU_TESS_END_DATA,     fn(tessEnd));
            gluTessCallback(tess, GLU_TESS_VERTEX_DATA,  fn(tessVertex));
            gluTessCallback(tess, GLU_TESS_COMBINE_DATA, fn(tessCombine));
        }

        gluTessProperty(tess, GLU_TESS_WINDING_RULE, tesselation);
        gluTessBeginPolygon(tess, &polygon);
    }

    for (path_elements::iterator i = begin; i != end; i++)
    {
        Element    &e    = *i;
        Kind        kind = e.kind;
        Point3      pos  = e.position + offset; // Geometry coordinates
        Point3      tex  = e.position / bounds; // Texture coordinates
        VertexData  here(pos, tex);
        uint        size;
        double      dt;

        switch (kind)
        {
        default:
            std::cerr << "GraphicPath::Draw: Unexpected element kind\n";

        case LINE_TO:
            data.push_back(here);
        case MOVE_TO:
            control.clear();
            // Fallback

        case CURVE_CONTROL:
            control.push_back(here);
            break;

        case CURVE_TO:
            control.push_back(here);

            // Optimize common sizes, give up for the rest:
            size = control.size();
            dt = 1.0 / resolution;
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

                for (double t = 0.0; t < 1.0; t += dt)
                {
                    double tt = t*t;
                    double m = 1-t;
                    double mm = m*m;
                    Vector3 vd = mm * v0 + 2*m*t * v1 + tt * v2;
                    Vector3 td = mm * t0 + 2*m*t * t1 + tt * t2;
                    VertexData intermediate(vd, td);
                    data.push_back(intermediate);
                }
                break;
                }

            default:
                std::cerr << "GraphicPath::Draw: Unimplemented Bezier"
                          << " (order " << size << ")\n";
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
                Vector3& t3 = control[2].texture;
                
                for (double t = 0.0; t < 1.0; t += dt)
                {
                    double tt = t*t;
                    double ttt = t*tt;
                    double m = 1-t;
                    double mm = m*m;
                    double mmm = m*mm;
                    Vector3 vd = mmm*v0 + 3*mm*t*v1 + 3*m*tt*v2 + ttt*v3;
                    Vector3 td = mmm*t0 + 3*mm*t*t1 + 3*m*tt*t2 + ttt*t3;
                    VertexData intermediate(vd, td);
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
                // Pass the data we had so far to OpenGL and clear it
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
                    double *vdata = &data[0].vertex.x;
                    double *tdata = &data[0].texture.x;
                    glVertexPointer(3,GL_DOUBLE,sizeof(VertexData), vdata);
                    glTexCoordPointer(3,GL_DOUBLE,sizeof(VertexData), tdata);
                    glEnableClientState(GL_VERTEX_ARRAY);
                    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                    glDrawArrays(mode, 0, size);
                    glDisableClientState(GL_VERTEX_ARRAY);
                    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                }

                data.clear();
            }

            data.push_back(here);
        }

    } // Loop on all elements

    // End of tesselation
    if (tesselation)
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
        moveTo(start);

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


GraphicPath& GraphicPath::addQtPath(QPainterPath &qt)
// ----------------------------------------------------------------------------
//   Add a QT path
// ----------------------------------------------------------------------------
{
    // Check that Kind and QPainterPath::ElementType numerical values match
    XL_CASSERT (int(QPainterPath::MoveToElement) == int(MOVE_TO) &&
                int(QPainterPath::LineToElement) == int(LINE_TO) &&
                int(QPainterPath::CurveToElement) == int(CURVE_TO) &&
                int(QPainterPath::CurveToDataElement) == int(CURVE_CONTROL));

    // Loop on the Qt Path and insert elements
    // Qt paths place CURVE_TO before control points, we place it last
    uint i, max = qt.elementCount();
    bool hadControl = false;
    for (i = 0; i < max; i++)
    {
        const QPainterPath::Element &e = qt.elementAt(i);
        Kind kind = Kind(e.type);
        position = Point3(e.x, -e.y, 0);
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
    }

    return *this;
}


void GraphicPath::Draw(Layout *where, QPainterPath &qtPath, GLenum tessel)
// ----------------------------------------------------------------------------
//   Draw the graphic path using the current texture, fill and line color
// ----------------------------------------------------------------------------
{
    GraphicPath path;
    path.addQtPath(qtPath);

    path.setTexture(where);
    if (path.setFillColor(where))
        path.Draw(where, GL_POLYGON, tessel);
    if (path.setLineColor(where))
        path.Draw(where, GL_LINE_STRIP);
}


Box3 GraphicPath::Bounds()
// ----------------------------------------------------------------------------
//   Return the bounding box, computed from all path elements
// ----------------------------------------------------------------------------
{
    return bounds;
}



TAO_END
