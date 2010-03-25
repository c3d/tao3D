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
#include <QtOpenGL>
#include <QPainterPath>


TAO_BEGIN

// ============================================================================
//
//    Common utilities for drawing shapes
//
// ============================================================================

double Shape::red = -1;
double Shape::green = -1;
double Shape::blue = -1;
double Shape::alpha = -1;


bool Shape::setTexture(Layout *where)
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


bool Shape::setFillColor(Layout *where)
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


bool Shape::setLineColor(Layout *where)
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



// ============================================================================
//
//   Shape drawing routines
//
// ============================================================================

void Rectangle::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw a rectangle
// ----------------------------------------------------------------------------
{
    GraphicPath path;
    path.moveTo(Point3(bounds.lower.x, bounds.lower.y, 0));
    path.lineTo(Point3(bounds.upper.x, bounds.lower.y, 0));
    path.lineTo(Point3(bounds.upper.x, bounds.upper.y, 0));
    path.lineTo(Point3(bounds.lower.x, bounds.upper.y, 0));
    path.close();
    path.Draw(where);
}


void RoundedRectangle::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw a rounded rectangle
// ----------------------------------------------------------------------------
{
    QPainterPath path;
    if (radiusX > bounds.Width() / 2)
        radiusX = bounds.Width() / 2;
    if (radiusY > bounds.Height() / 2)
        radiusY = bounds.Height() / 2;
    path.addRoundedRect(bounds.lower.x, bounds.lower.y,
                        bounds.Width(), bounds.Height(),
                        radiusX, radiusY);
    GraphicPath::Draw(where, path);
}


void Ellipse::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw an ellipse inside the bounds
// ----------------------------------------------------------------------------
{
    QPainterPath path;
    path.addEllipse(bounds.lower.x, bounds.lower.y,
                    bounds.Width(), bounds.Height());
    GraphicPath::Draw(where, path);
}


void EllipseArc::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    Draw an arc of ellipse
// ----------------------------------------------------------------------------
{
    QPainterPath path;
    Point center = bounds.Center();
    path.moveTo(center.x, center.y);
    path.arcTo(bounds.lower.x, bounds.lower.y,
               bounds.Width(), bounds.Height(),
               start, sweep);
    path.closeSubpath();
    GraphicPath::Draw(where, path);
}

TAO_END
