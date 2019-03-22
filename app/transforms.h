#ifndef TRANSFORMS_H
#define TRANSFORMS_H
// *****************************************************************************
// transforms.h                                                    Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2010-2011, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2011,2013-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
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


struct ClipPlane : Transform
// ----------------------------------------------------------------------------
//    Record a clip plane change
// ----------------------------------------------------------------------------
{
    ClipPlane(int plane, scale a, scale b, scale c, scale d):
        Transform(), plane(plane), a(a), b(b), c(c), d(d) {}
    virtual void Draw(Layout *where);
    virtual void Evaluate(Layout *) {}
    int plane;
    scale a, b, c, d;
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


void printMatrix(GLint model = -1);
// ----------------------------------------------------------------------------
//    Print GL matrix on stderr
// ----------------------------------------------------------------------------



// ============================================================================
//
//   Entering transforms in the symbols table
//
// ============================================================================

extern void EnterTransforms();
extern void DeleteTransforms();

TAO_END

#endif // TRANSFORMS_H
