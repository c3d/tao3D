#ifndef SHAPES3D_H
#define SHAPES3D_H
// ****************************************************************************
//  shapes3d.h                                                      XLR project
// ****************************************************************************
//
//   File Description:
//
//     Drawing of elementary 3D geometry shapes
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

struct Shape3 : Drawing
// ----------------------------------------------------------------------------
//   Base class for all 3D shapes, just in case
// ----------------------------------------------------------------------------
{
    Shape3(): Drawing() {}
};


struct Sphere : Shape3
// ----------------------------------------------------------------------------
//   Draw a sphere
// ----------------------------------------------------------------------------
{
    Sphere(Point3 c, coord r): Shape3(), center(c), radius(r) {}
    virtual void        Draw(Layout *where);
    virtual Box3        Bounds();
    Point3  center;
    coord   radius;
};

TAO_END

#endif // SHAPES3D_H
