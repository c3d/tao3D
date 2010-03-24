// ****************************************************************************
//  shapes.cpp                                                     Tao project
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

#include "shapes.h"
#include "layout.h"
#include "attributes.h"
#include "path3d.h"
#include <GL/glew.h>
#include <QPainterPath>


TAO_BEGIN

// ============================================================================
//
//    Common utilities for drawing shapes
//
// ============================================================================

static double red = -1, green = -1, blue = -1, alpha = -1;


static inline bool setTexture(Layout *where)
// ----------------------------------------------------------------------------
//   Get the texture from the layout
// ----------------------------------------------------------------------------
{
    if (where->fillTexture)
    {
        glBindTexture(GL_TEXTURE_2D, where->fillTexture->glName);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_MULTISAMPLE);
    }
    else
    {
        glDisable(GL_TEXTURE_2D);
    }

    return where->fillTexture ? true : false;
}


static inline bool setFillColor(Layout *where)
// ----------------------------------------------------------------------------
//    Set the fill color and texture according to the layout attributes
// ----------------------------------------------------------------------------
{
    // Check if we have a non-transparent fill color
    if (where)
    {
        if (FillColor *color = where->fillColor)
        {
            if (color->alpha > 0.0)
            {
                // Check cached GL color
                if (color->red  == red  && color->green == green &&
                    color->blue == blue && color->alpha != alpha)
                    return true;
                
                glColor4f(color->red, color->green, color->blue, color->alpha);
                red = color->red;
                green = color->green;
                blue = color->blue;
                alpha = color->alpha;
                return true;
            }
        }
    }
    return false;
}


static inline bool setLineColor(Layout *where)
// ----------------------------------------------------------------------------
//    Set the outline color according to the layout attributes
// ----------------------------------------------------------------------------
{
    // Check if we have a non-transparent outline color
    if (where)
    {
        if (LineColor *color = where->lineColor)
        {
            if (color->alpha > 0.0)
            {
                // Check cached GL color
                if (color->red  == red  && color->green == green &&
                    color->blue == blue && color->alpha != alpha)
                    return true;
                
                glColor4f(color->red, color->green, color->blue, color->alpha);
                red = color->red;
                green = color->green;
                blue = color->blue;
                alpha = color->alpha;
                return true;
            }
        }
    }
    return false;
}


static void drawPath(Layout *where, GraphicPath &path)
// ----------------------------------------------------------------------------
//   Stroke and fill a path in the given layout
// ----------------------------------------------------------------------------
{
    setTexture(where);
    if (setFillColor(where))
    {
        path.mode = GL_POLYGON;
        path.Draw(where);
    }
    if (setLineColor(where))
    {
        path.mode = GL_LINE_STRIP;
        path.Draw(where);
    }
}



// ============================================================================
//
//   Rectangle
//
// ============================================================================

void Rectangle::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw a rectangle
// ----------------------------------------------------------------------------
{
    // Define a graphic path for the rectangle
    GraphicPath path;
    path.moveTo(Point3(bounds.lower.x, bounds.lower.y, 0));
    path.lineTo(Point3(bounds.upper.x, bounds.lower.y, 0));
    path.lineTo(Point3(bounds.upper.x, bounds.upper.y, 0));
    path.lineTo(Point3(bounds.lower.x, bounds.upper.y, 0));
    path.close();

    // And draw that path
    drawPath(where, path);
}



// ============================================================================
//
//   Rounded Rectangle
//
// ============================================================================

void RoundedRectangle::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw a rounded rectangle
// ----------------------------------------------------------------------------
{
    QPainterPath pp;
    if (radiusX > bounds.Width() / 2)
        radiusX = bounds.Width() / 2;
    if (radiusY > bounds.Height() / 2)
        radiusY = bounds.Height() / 2;
    pp.addRoundedRect(bounds.lower.x, bounds.lower.y,
                      bounds.Width(), bounds.Height(),
                      radiusX, radiusY);
    GraphicPath path;
    path.addQtPath(pp);

    drawPath(where, path);
}



// ============================================================================
//
//   Circle
//
// ============================================================================

void Circle::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw a circle
// ----------------------------------------------------------------------------
{
    (void) where;
}


Box3 Circle::Bounds()
// ----------------------------------------------------------------------------
//    Return the bounding box for a circle
// ----------------------------------------------------------------------------
{
    return Box3(center.x - radius, center.y - radius, 0,
                2*radius, 2*radius, 0);
}

TAO_END
