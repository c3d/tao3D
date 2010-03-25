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
#include <GL/glew.h>
#include <QPainterPath>
#include <iostream>

TAO_BEGIN

void GraphicPath::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the graphic path using curves
// ----------------------------------------------------------------------------
{
    typedef std::vector<Point3> Point3V;

    Vector3 offset = where->Offset();
    Point3V vdata;   // Vertex coordinates
    Point3V tdata;   // Texture coordinates
    Point3V vctrl;   // Vertex control points
    Point3V tctrl;   // Texture control points
    path_elements::iterator i, begin = elements.begin(), end = elements.end();

    for (path_elements::iterator i = begin; i != end; i++)
    {
        Element &e          = *i;
        Kind     kind       = e.kind;
        Point3   pos        = e.position + offset; // Geometry coordinates
        Point3   tex        = e.position / bounds; // Texture coordinates
        uint     size;
        double   dt;

        switch (kind)
        {
        default:
            std::cerr << "GraphicPath::Draw: Unexpected element kind\n";

        case MOVE_TO:
        case LINE_TO:
            vdata.push_back(pos);
            tdata.push_back(tex);
            vctrl.clear();
            tctrl.clear();
            // Fallback

        case CURVE_CONTROL:
            vctrl.push_back(pos);
            tctrl.push_back(tex);
            break;

        case CURVE_TO:
            vctrl.push_back(pos);
            tctrl.push_back(tex);

            // Optimize common sizes, give up for the rest:
            size = vctrl.size();
            dt = 1.0 / resolution;
            switch (size)
            {
            case 1:
            case 2:
                // Simple line, just push the last point
                vdata.push_back(pos);
                vdata.push_back(tex);
                break;

            case 3:
            {
                // Quadratic Bezier curve
                Vector3 v0(vctrl[0]);
                Vector3 v1(vctrl[1]);
                Vector3 v2(vctrl[2]);
                Vector3 t0(tctrl[0]);
                Vector3 t1(tctrl[1]);
                Vector3 t2(tctrl[2]);

                for (double t = 0.0; t < 1.0; t += dt)
                {
                    double tt = t*t;
                    double m = 1-t;
                    double mm = m*m;
                    Vector3 vd = mm * v0 + 2*m*t * v1 + tt * v2;
                    Vector3 td = mm * t0 + 2*m*t * t1 + tt * t2;
                    vdata.push_back(vd);
                    tdata.push_back(td);
                }
                break;
                }

            default:
                std::cerr << "GraphicPath::Draw: Unimplemented Bezier"
                          << " (order " << size << ")\n";
            case 4:
            {
                // Cubic Bezier curve
                Vector3 v0(vctrl[0]);
                Vector3 v1(vctrl[1]);
                Vector3 v2(vctrl[2]);
                Vector3 v3(vctrl[3]);
                Vector3 t0(tctrl[0]);
                Vector3 t1(tctrl[1]);
                Vector3 t2(tctrl[2]);
                Vector3 t3(tctrl[3]);
                
                for (double t = 0.0; t < 1.0; t += dt)
                {
                    double tt = t*t;
                    double ttt = t*tt;
                    double m = 1-t;
                    double mm = m*m;
                    double mmm = m*mm;
                    
                    Vector3 vd = mmm*v0 + 3*mm*t*v1 + 3*m*tt*v2 + ttt*v3;
                    Vector3 td = mmm*t0 + 3*mm*t*t1 + 3*m*tt*t2 + ttt*t3;
                    vdata.push_back(vd);
                    tdata.push_back(td);
                }
                break;
            }
            } // switch(size)
            
            vctrl.clear();
            tctrl.clear();
        } // switch(kind)

        // Check if we need to emit what existed before
        if (i+1 == end || kind == MOVE_TO)
        {
            // Get size of previous elements
            if (uint size = vdata.size())
            {
                // Pass the data we had so far to OpenGL and clear it
                glVertexPointer(3,GL_DOUBLE,sizeof(Point3), &vdata[0].x);
                glTexCoordPointer(3,GL_DOUBLE,sizeof(Point3), &tdata[0].x);
                glEnableClientState(GL_VERTEX_ARRAY);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glDrawArrays(mode, 0, size);
                glDisableClientState(GL_VERTEX_ARRAY);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);

                vdata.clear();
                tdata.clear();
            }

            vdata.push_back(pos);
            tdata.push_back(tex);
        }

    } // Loop on all elements
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


Box3 GraphicPath::Bounds()
// ----------------------------------------------------------------------------
//   Return the bounding box, computed from all path elements
// ----------------------------------------------------------------------------
{
    return bounds;
}



TAO_END
