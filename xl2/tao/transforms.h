#ifndef TRANSFORMS_H
#define TRANSFORMS_H
// ****************************************************************************
//  transforms.h                                                    XLR project
// ****************************************************************************
//
//   File Description:
//
//    Record transformations being applied in a layout
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

#include "attributes.h"

TAO_BEGIN

struct Transform : Attribute
// ----------------------------------------------------------------------------
//   Base class for all transforms
// ----------------------------------------------------------------------------
{
    Transform() : Attribute() {}
};


struct Rotation : Transform
// ----------------------------------------------------------------------------
//    Record a rotation
// ----------------------------------------------------------------------------
{
    Rotation(float a, float x, float y, float z):
        Transform(), amount(a), xaxis(x), yaxis(y), zaxis(z) {}
    virtual void Draw(Layout *where);
    float amount, xaxis, yaxis, zaxis;
};


struct Translation : Transform
// ----------------------------------------------------------------------------
//    Record a translation
// ----------------------------------------------------------------------------
{
    Translation(float x, float y, float z):
        Transform(), xaxis(x), yaxis(y), zaxis(z) {}
    virtual void Draw(Layout *where);
    float xaxis, yaxis, zaxis;
};


struct Scale : Transform
// ----------------------------------------------------------------------------
//    Record a scale change
// ----------------------------------------------------------------------------
{
    Scale(float x, float y, float z):
        Transform(), xaxis(x), yaxis(y), zaxis(z) {}
    virtual void Draw(Layout *where);
    float xaxis, yaxis, zaxis;
};

TAO_END

#endif // TRANSFORMS_H
