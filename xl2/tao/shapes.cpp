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
        glBindTexture(GL_TEXTURE_2D, where->fillTexture);
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
        Color &color = where->fillColor;
        if (color.alpha > 0.0)
        {
            // Check cached GL color
            if (color.red  == red  && color.green == green &&
                color.blue == blue && color.alpha != alpha)
                return true;

            glColor4f(color.red, color.green, color.blue, color.alpha);
            red = color.red;
            green = color.green;
            blue = color.blue;
            alpha = color.alpha;
            return true;
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
        Color &color = where->lineColor;
        if (color.alpha > 0.0)
        {
            // Check cached GL color
            if (color.red  == red  && color.green == green &&
                color.blue == blue && color.alpha != alpha)
                return true;
                
            glColor4f(color.red, color.green, color.blue, color.alpha);
            red = color.red;
            green = color.green;
            blue = color.blue;
            alpha = color.alpha;
            return true;
        }
    }
    return false;
}


void Shape::Draw(GraphicPath &path)
// ----------------------------------------------------------------------------
//    Draw the shape in a path
// ----------------------------------------------------------------------------
{
    (void) path;
}


void Shape::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    Draw the shape using a path
// ----------------------------------------------------------------------------
{
    GraphicPath path;
    Draw(path);
    path.Draw(where);
}



// ============================================================================
//
//   Shape drawing routines
//
// ============================================================================

void Rectangle::Draw(GraphicPath &path)
// ----------------------------------------------------------------------------
//   Draw a rectangle
// ----------------------------------------------------------------------------
{
    path.moveTo(Point3(bounds.lower.x, bounds.lower.y, 0));
    path.lineTo(Point3(bounds.upper.x, bounds.lower.y, 0));
    path.lineTo(Point3(bounds.upper.x, bounds.upper.y, 0));
    path.lineTo(Point3(bounds.lower.x, bounds.upper.y, 0));
    path.close();
}


void RoundedRectangle::Draw(GraphicPath &path)
// ----------------------------------------------------------------------------
//   Draw a rounded rectangle
// ----------------------------------------------------------------------------
{
    QPainterPath qt;
    if (radiusX > bounds.Width() / 2)
        radiusX = bounds.Width() / 2;
    if (radiusY > bounds.Height() / 2)
        radiusY = bounds.Height() / 2;
    qt.addRoundedRect(bounds.lower.x, bounds.lower.y,
                      bounds.Width(), bounds.Height(),
                      radiusX, radiusY);
    path.addQtPath(qt);
}


void Ellipse::Draw(GraphicPath &path)
// ----------------------------------------------------------------------------
//   Draw an ellipse inside the bounds
// ----------------------------------------------------------------------------
{
    QPainterPath qt;
    qt.addEllipse(bounds.lower.x, bounds.lower.y,
                  bounds.Width(), bounds.Height());
    path.addQtPath(qt);
}


void EllipseArc::Draw(GraphicPath &path)
// ----------------------------------------------------------------------------
//    Draw an arc of ellipse
// ----------------------------------------------------------------------------
{
    QPainterPath qt;
    Point center = bounds.Center();
    qt.moveTo(center.x, center.y);
    qt.arcTo(bounds.lower.x, bounds.lower.y,
             bounds.Width(), bounds.Height(),
             start, sweep);
    qt.closeSubpath();
    path.addQtPath(qt);
}


void StarPolygon::Draw(GraphicPath &path)
// ----------------------------------------------------------------------------
//   Draw a regular polygon or star
// ----------------------------------------------------------------------------
{
    scale  R_r    = cos(q * M_PI/p) / cos((q-1) * M_PI/p);
    scale  w1     = bounds.Width()/2;
    scale  w2     = w1 * R_r;
    scale  h1     = bounds.Height()/2;
    scale  h2     = h1 * R_r;
    Point3 center = bounds.Center();
    coord  cx     = center.x;
    coord  cy     = center.y;

    for (uint i = 0; i < p; i++)
    {
        double a1 = i * 2*M_PI/p + M_PI_2;
        double a2 = a1 + M_PI/p;
        double x1 = cx + w1 * cos(a1);
        double y1 = cy + h1 * sin(a1);
        double x2 = cx + w2 * cos(a2);
        double y2 = cy + h2 * sin(a2);

        if (i)
            path.lineTo(Point3(x1, y1, 0));
        else
            path.moveTo(Point3(x1, y1, 0));

        path.lineTo(Point3(x2, y2, 0));
    }
    path.close();
}

TAO_END
