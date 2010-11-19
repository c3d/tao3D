#ifndef SHAPES3D_H
#define SHAPES3D_H
// ****************************************************************************
//  shapes3d.h                                                      Tao project
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

#include "shapes.h"

TAO_BEGIN

struct Shape3 : Shape
// ----------------------------------------------------------------------------
//   Base class for all 3D shapes, just in case
// ----------------------------------------------------------------------------
{
    Shape3(): Shape() {}
    virtual void        DrawSelection(Layout *layout);

protected:
    bool                setFillColor(Layout *where);
    bool                setLineColor(Layout *where);
};


struct Cube : Shape3
// ----------------------------------------------------------------------------
//   Draw a cube (OK, not really a cube, but the name is short)
// ----------------------------------------------------------------------------
{
    Cube(const Box3 &bounds): Shape3(), bounds(bounds) {}
    virtual void        Draw(Layout *where);
    virtual Box3        Bounds(Layout *);
    Box3 bounds;
};


struct Sphere : Cube
// ----------------------------------------------------------------------------
//   Draw a sphere or ellipsoid
// ----------------------------------------------------------------------------
{
    Sphere(Box3 bounds, uint sl, uint st)
        : Cube(bounds), slices(sl), stacks(st) {}
    virtual void        Draw(Layout *where);
    uint    slices, stacks;
};


struct Cone : Cube
// ----------------------------------------------------------------------------
//   Draw a (possibly truncated) cone - Limit case is a cylinder
// ----------------------------------------------------------------------------
{
    Cone(Box3 bounds, double tipRatio = 0.0) : Cube(bounds), ratio(tipRatio) {}
    virtual void Draw(Layout *where);
    double ratio;
};


TAO_END

#endif // SHAPES3D_H
