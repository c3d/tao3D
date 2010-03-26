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
#include <GL/glew.h>


TAO_BEGIN

void Rotation::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    Rotation in a drawing
// ----------------------------------------------------------------------------
{
    (void) where;
    glRotatef(amount, xaxis, yaxis, zaxis);
    where->offset = Point3();
}


void Translation::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    Rotation in a drawing
// ----------------------------------------------------------------------------
{
    (void) where;
    glTranslatef(xaxis, yaxis, zaxis);
    where->offset = Point3();
}


void Scale::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    Scale in a drawing
// ----------------------------------------------------------------------------
{
    (void) where;
    glScalef(xaxis, yaxis, zaxis);
    where->offset = Point3();
}

TAO_END
