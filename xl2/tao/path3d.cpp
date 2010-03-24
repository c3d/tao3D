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
    path_elements::iterator i, begin = elements.begin(), end = elements.end();
    bool hadCurve = false;
    bool began = false;
    Point3 lastPos, lastTex;

    for (path_elements::iterator i = begin; i != end; i++)
    {
        Element &e          = *i;
        Kind     kind       = e.kind;
        Point3   pos        = e.position + offset; // Geometry coordinates
        Point3   tex        = e.position / bounds; // Texture coordinates
        bool     emit       = false;

        // Check if we need to emit what existed before
        if (i+1 == end)
            emit = true;

        // Select what operation to perform
        switch (kind)
        {
        case MOVE_TO:
            emit = true;
            break;
        case LINE_TO:
            emit |= hadCurve;
            break;
        case CURVE_TO:
            emit = true;
            break;
        case CURVE_CONTROL:
            emit |= !hadCurve;
            break;
        }

        // Check if we need to draw what existed so far
        if (emit)
        {
            // Get size of previous elements
            uint size = vdata.size();

            // Select where we will clear
            Point3V::iterator lastV = vdata.end();
            Point3V::iterator lastT = tdata.end();

            switch(kind)
            {
            case CURVE_CONTROL:
                // For control points, also keep previous MOVE_TO / LINE_TO
                if (lastV != vdata.begin())     lastV--;
                if (lastT != tdata.begin())     lastT--;

            case CURVE_TO:
                // For CURVE_TO and CURVE_CONTROL, draw previous lines if any
                if (!hadCurve)
                {
                    // We will be emitting a curve, but only had lines so far
                    // Can't use array mode, so we emit individual vertices
                    if (!began)
                    {
                        glBegin(mode);
                        began = true;
                    }

                    // Emit the texture and vertex coordinates for the lines
                    Point3V::iterator v = vdata.begin();
                    Point3V::iterator t = tdata.begin();
                    for (uint j = 0; j < size; j++, v++, t++)
                    {
                        glTexCoord3d(t->x, t->y, t->z);
                        glVertex3d(v->x, v->y, v->z);
                    }

                    // We saw a curve on this path segment
                    hadCurve = true;
                }

                if (kind == CURVE_TO)
                {
                    // We are at the end of a subpath path, draw it
                    if (!began)
                    {
                        glBegin(mode);
                        began = true;
                    }

                    // We need to add the last vertex and texture coordinate
                    vdata.push_back(pos);
                    tdata.push_back(tex);

                    // Use a GL evaluator to generate coordinates
                    glMap1d(GL_MAP1_VERTEX_3, 0.0,1.0,3,size,&vdata[0].x);
                    glMap1d(GL_MAP1_TEXTURE_COORD_3,0.0,1.0,3,size,&tdata[0].x);
                    glEnable(GL_MAP1_VERTEX_3);
                    glEnable(GL_MAP1_TEXTURE_COORD_3);
                    for (uint i = 0; i <= resolution; i++)
                        glEvalCoord1f(GLfloat(i) / resolution);
                    glDisable(GL_MAP1_VERTEX_3);
                    glDisable(GL_MAP1_TEXTURE_COORD_3);
                }

                break;

            case LINE_TO:
                if (hadCurve)
                {
                    // We had a curve before, we can't use arrays
                    // Make sure we had a glBegin()
                    if (!began)
                    {
                        glBegin(mode);
                        began = true;
                    }

                    // Emit more vertex / texture coordinates
                    glTexCoord3d (tex.x, tex.y, tex.z);
                    glVertex3d (pos.x, pos.y, pos.z);
                    break;
                }

                // If we are still able to use an array, add last vertex
                vdata.push_back(pos);
                tdata.push_back(tex);
                size++;

            case MOVE_TO:
                if (!hadCurve)
                {
                    // We didn't have any line before, can use arrays
                    if (began)
                    {
                        glEnd();
                        began = false;
                    }
                    glVertexPointer(3,GL_DOUBLE,sizeof(Point3), &vdata[0].x);
                    glTexCoordPointer(3,GL_DOUBLE,sizeof(Point3), &tdata[0].x);
                    glEnableClientState(GL_VERTEX_ARRAY);
                    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                    glDrawArrays(mode, 0, size);
                    glDisableClientState(GL_VERTEX_ARRAY);
                    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                }

                // After a MOVE_TO, we can use arrays again
                hadCurve = false;

            } // switch(kind)

            // Clear all the vertex / coord data we already drew
            vdata.erase(vdata.begin(), lastV);
            tdata.erase(tdata.begin(), lastT);
        } // if (emit)

        // Insert the last element
        vdata.push_back(pos);
        tdata.push_back(tex);
    } // Loop on all elements

    // Make sure we have a glEnd() matching any glBegin() we did
    if (began)
        glEnd();
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


Box3 GraphicPath::Bounds()
// ----------------------------------------------------------------------------
//   Return the bounding box, computed from all path elements
// ----------------------------------------------------------------------------
{
    return bounds;
}

TAO_END
