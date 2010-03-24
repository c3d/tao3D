#ifndef PATH3D_H
#define PATH3D_H
// ****************************************************************************
//  path3d.h                                                        Tao project
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

#include "coords3d.h"
#include "shapes.h"
#include <GL/glew.h>

TAO_BEGIN

struct GraphicPath : Shape
// ----------------------------------------------------------------------------
//    An arbitrary graphic path
// ----------------------------------------------------------------------------
{
    GraphicPath(GLenum m = GL_POLYGON, uint r = 20)
        : Shape(), mode(m), resolution(r) {}
    virtual void        Draw(Layout *where);
    virtual Box3        Bounds();

    // Absolute coordinates
    GraphicPath&        moveTo(Point3 dst);
    GraphicPath&        lineTo(Point3 dst);
    GraphicPath&        curveTo(Point3 control, Point3 dst);
    GraphicPath&        curveTo(Point3 c1, Point3 c2, Point3 dst);
    GraphicPath&        close();

    // Relative coordinates
    GraphicPath&        moveTo(Vector3 dst)     { return moveTo(position+dst); }
    GraphicPath&        lineTo(Vector3 dst)     { return lineTo(position+dst); }

public:
    enum Kind { MOVE_TO, LINE_TO, CURVE_TO, CURVE_CONTROL };
    struct Element
    {
        Element(Kind k, Point3 p): kind(k), position(p) {}
        Kind    kind;
        Point3  position;
    };
    typedef std::vector<Element> path_elements;

public:
    path_elements       elements;
    Point3              start, position;
    Box3                bounds;
    GLenum              mode;
    uint                resolution;
};

TAO_END

#endif // PATH3D_H
