// ****************************************************************************
//  transforms.cpp                                                  Tao project
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

#include "transforms.h"
#include "layout.h"
#include "shapes3d.h"
#include "context.h"
#include "widget.h"
#include <GL/glew.h>


TAO_BEGIN

void Transform::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Default draws a cube
// ----------------------------------------------------------------------------
{
    Color line(1.0, 0.0, 0.0, 0.5);
    Color fill(0.0, 0.7, 1.0, 0.5);
    XL::LocalSave<Color> saveLine(where->lineColor, line);
    XL::LocalSave<Color> saveFill(where->fillColor, fill);
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
    glLoadIdentity();
    where->hasPixelBlur = false;
    where->hasMatrix = false;
    where->has3D = false;
}


void Rotation::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    Rotation in a drawing
// ----------------------------------------------------------------------------
{
    glRotatef(amount, xaxis, yaxis, zaxis);
    double amod90 = fmod(amount, 90.0);
    if (amod90 < -0.01 || amod90 > 0.01)
        where->hasPixelBlur = true;
    if (amount != 0.0)
    {
        if (xaxis != 0.0 || yaxis != 0.0)
            where->has3D = true;
        else if (zaxis > 0)
            where->planarRotation += amount;
        else if (zaxis < 0)
            where->planarRotation -= amount;
    }
    where->offset = Point3();
}


void Translation::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    Rotation in a drawing
// ----------------------------------------------------------------------------
{
    glTranslatef(xaxis, yaxis, zaxis);
    if (zaxis != 0.0)
        where->hasPixelBlur = true;
    where->offset = Point3();
}


void Scale::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    Scale in a drawing
// ----------------------------------------------------------------------------
{
    glScalef(xaxis, yaxis, zaxis);
    if (xaxis != 1.0 || yaxis != 1.0)
        where->hasPixelBlur = true;
    if (xaxis == yaxis && xaxis == zaxis)
        where->planarScale *= xaxis;
    else
        where->has3D = true;
    where->offset = Point3();
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
