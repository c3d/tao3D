// ****************************************************************************
//  shapes_drawing.cpp                                              XLR project
// ****************************************************************************
//
//   File Description:
//
//     Drawing of elementary geometry shapes on a 2D layout
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

#include "shapes_drawing.h"
#include "widget.h"

TAO_BEGIN

// ============================================================================
//
//   Rectangle
//
// ============================================================================

void Rectangle::Draw(const Point &where)
// ----------------------------------------------------------------------------
//   Draw a rectangle
// ----------------------------------------------------------------------------
{
    coord px = where.x;
    coord py = where.y;
    coord xl = bounds.Left() + px;
    coord xu = bounds.Right() + px;
    coord yl = bounds.Bottom() + py;
    coord yu = bounds.Top() + py;

    glBegin(TAO(state.polygonMode));
    {
        glTexCoord2f(0.0,0.0);    glVertex3f (xl, yl, 0.0);
        glTexCoord2f(0.0,1.0);    glVertex3f (xl, yu, 0.0);
        glTexCoord2f(1.0,1.0);    glVertex3f (xu, yu, 0.0);
        glTexCoord2f(1.0,0.0);    glVertex3f (xu, yl, 0.0);
    }
    glEnd();
}



// ============================================================================
// 
//   Rounded Rectangle
// 
// ============================================================================

void RoundedRectangle::Draw(const Point &where)
// ----------------------------------------------------------------------------
//   Draw a rounded rectangle
// ----------------------------------------------------------------------------
{
    (void) where;
}



// ============================================================================
//
//   Circle
//
// ============================================================================

void Circle::Draw(const Point &where)
// ----------------------------------------------------------------------------
//   Draw a circle
// ----------------------------------------------------------------------------
{
    (void) where;
}


Box Circle::Bounds()
// ----------------------------------------------------------------------------
//    Return the bounding box for a circle
// ----------------------------------------------------------------------------
{
    return Box(center.x - radius, center.y - radius, 2*radius, 2*radius);
}

TAO_END
