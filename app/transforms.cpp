// *****************************************************************************
// transforms.cpp                                                  Tao3D project
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
// (C) 2012-2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2010-2011, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
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

#include "transforms.h"
#include "layout.h"
#include "shapes3d.h"
#include "context.h"
#include "widget.h"


TAO_BEGIN

void Transform::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Default draws a cube
// ----------------------------------------------------------------------------
{
    Color line(1.0, 0.0, 0.0, 0.5);
    Color fill(0.0, 0.7, 1.0, 0.5);
    XL::Save<Color> saveLine(where->lineColor, line);
    XL::Save<Color> saveFill(where->fillColor, fill);
    Box3 cubeBox(-25, -25, -25, 50, 50, 50);
    Cube cube(cubeBox);
    cube.Draw(where);
}


void Transform::DrawSelection(Layout *where)
// ----------------------------------------------------------------------------
//   Transforms don't have a selection, they just draw themselves
// ----------------------------------------------------------------------------
{
    Draw(where);
}


void Transform::Identify(Layout *where)
// ----------------------------------------------------------------------------
//   Transforms don't have their own identity, they just draw themselves
// ----------------------------------------------------------------------------
{
    Draw(where);
}


void ResetTransform::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Reset the transformation matrix and other parameters
// ----------------------------------------------------------------------------
{
    Widget *widget = where->Display();
    widget->resetModelviewMatrix();
    GL.HasPixelBlur(false);
}


void Rotation::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    Rotation in a drawing
// ----------------------------------------------------------------------------
{
    // BUG? fmod required to avoid incorrect rotations with large values
    // (>1290000000)
    amount = fmod(amount, 360.0);
    double amod90 = fmod(amount, 90.0);
    if (amod90 < -0.01 || amod90 > 0.01)
        GL.HasPixelBlur(true);
    if (amount != 0.0)
    {
        if (where->offset != Vector3())
        {
            GL.Translate(where->offset.x, where->offset.y, where->offset.z);
            GL.Rotate(amount, xaxis, yaxis, zaxis);
            GL.Translate(-where->offset.x, -where->offset.y, -where->offset.z);
        }
        else
        {
            GL.Rotate(amount, xaxis, yaxis, zaxis);
        }
    }
}


void Translation::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    Rotation in a drawing
// ----------------------------------------------------------------------------
{
    (void) where;
    GL.Translate(xaxis, yaxis, zaxis);
    if (zaxis != 0.0)
        GL.HasPixelBlur(true);
}


void Scale::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    Scale in a drawing
// ----------------------------------------------------------------------------
{
    if (where->offset != Vector3())
    {
        GL.Translate(where->offset.x, where->offset.y, where->offset.z);
        GL.Scale(xaxis, yaxis, zaxis);
        GL.Translate(-where->offset.x, -where->offset.y, -where->offset.z);
    }
    else
    {
        GL.Scale(xaxis, yaxis, zaxis);
    }
    if (xaxis != 1.0 || yaxis != 1.0)
        GL.HasPixelBlur(true);
}


void ClipPlane::Draw(Layout *)
// ----------------------------------------------------------------------------
//   Setup a given clip plane
// ----------------------------------------------------------------------------
{
    static GLint maxClipPlanes = 0;
    if (maxClipPlanes == 0)
        glGetIntegerv(GL_MAX_CLIP_PLANES, &maxClipPlanes);

    if (plane >= 0 && plane < maxClipPlanes && plane <= GL_CLIP_PLANE5)
    {
        GLdouble result[] = { a, b, c, d };
        GL.Enable(GL_CLIP_PLANE0 + plane);
        GL.ClipPlane(GL_CLIP_PLANE0 + plane, result);
    }
}


void MoveTo::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    Position in a drawing
// ----------------------------------------------------------------------------
{
    where->offset = Point3(xaxis, yaxis, zaxis);
}


void MoveToRel::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    Position in a drawing
// ----------------------------------------------------------------------------
{
    where->offset += Vector3(xaxis, yaxis, zaxis);
}


TAO_END
