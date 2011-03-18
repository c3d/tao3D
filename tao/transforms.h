#ifndef TRANSFORMS_H
#define TRANSFORMS_H
// ****************************************************************************
//  transforms.h                                                    Tao project
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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
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
    virtual void Draw(Layout *where);
    virtual void DrawSelection(Layout *where);
    virtual void Identify(Layout *where);
};


struct ResetTransform : Transform
// ----------------------------------------------------------------------------
//   Reset transform to what it was initially
// ----------------------------------------------------------------------------
{
    ResetTransform() {}
    virtual void Draw(Layout *where);
};


struct Rotation : Transform
// ----------------------------------------------------------------------------
//    Record a rotation
// ----------------------------------------------------------------------------
{
    Rotation(coord a, coord x, coord y, coord z):
        Transform(), amount(a), xaxis(x), yaxis(y), zaxis(z) {}
    virtual void Draw(Layout *where);
    coord amount, xaxis, yaxis, zaxis;
};


struct Translation : Transform
// ----------------------------------------------------------------------------
//    Record a translation
// ----------------------------------------------------------------------------
{
    Translation(coord x, coord y, coord z):
        Transform(), xaxis(x), yaxis(y), zaxis(z) {}
    virtual void Draw(Layout *where);
    coord xaxis, yaxis, zaxis;
};


struct Scale : Transform
// ----------------------------------------------------------------------------
//    Record a scale change
// ----------------------------------------------------------------------------
{
    Scale(scale x, scale y, scale z):
        Transform(), xaxis(x), yaxis(y), zaxis(z) {}
    virtual void Draw(Layout *where);
    scale xaxis, yaxis, zaxis;
};


struct MoveTo : Transform
// ----------------------------------------------------------------------------
//    Record a move-to operation
// ----------------------------------------------------------------------------
{
    MoveTo(coord x, coord y, coord z):
        Transform(), xaxis(x), yaxis(y), zaxis(z) {}
    virtual void Draw(Layout *where);
    scale xaxis, yaxis, zaxis;
};


struct MoveToRel : MoveTo
// ----------------------------------------------------------------------------
//    Record a move-to operation
// ----------------------------------------------------------------------------
{
    MoveToRel(coord x, coord y, coord z): MoveTo(x, y, z) {}
    virtual void Draw(Layout *where);
};

TAO_END

#endif // TRANSFORMS_H
