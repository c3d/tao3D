// ****************************************************************************
//  shapes3d.cpp                                                    XLR project
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

#include "shapes3d.h"

TAO_BEGIN

void Sphere::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the sphere
// ----------------------------------------------------------------------------
{
    (void) where;
}


Box3 Sphere::Bounds()
// ----------------------------------------------------------------------------
//   Return the bounding box for a sphere
// ----------------------------------------------------------------------------
{
    return Box3(center.x-radius, center.y-radius, center.z-radius,
                2*radius, 2*radius, 2*radius);
}

TAO_END
