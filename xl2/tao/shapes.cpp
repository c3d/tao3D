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


TAO_BEGIN

// ============================================================================
//
//    Common utilities for drawing shapes
//
// ============================================================================

static Layout *fillLayout = NULL;
static Layout *outlineLayout = NULL;
static Layout *textLayout = NULL;


static bool setTexture(Layout *where)
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


static bool setFillColor(Layout *where)
// ----------------------------------------------------------------------------
//    Set the fill color and texture according to the layout attributes
// ----------------------------------------------------------------------------
{
    // Cache in case we use only fill
    if (fillLayout == where)
        return true;

    // Check if we have a non-transparent fill color
    if (where)
    {
        if (FillColor *color = where->fillColor)
        {
            if (color->alpha > 0.0)
            {
                // Load the color
                glColor4f(color->red, color->green, color->blue, color->alpha);

                fillLayout = where;
                outlineLayout = NULL;
                textLayout = NULL;
                return true;
            }
        }
    }
    return false;
}


static bool setOutlineColor(Layout *where)
// ----------------------------------------------------------------------------
//    Set the outline color according to the layout attributes
// ----------------------------------------------------------------------------
{
    // Cache in case we use only outline
    if (outlineLayout == where)
        return true;

    // Check if we have a non-transparent outline color
    if (where)
    {
        if (OutlineColor *color = where->outlineColor)
        {
            if (color->alpha > 0.0)
            {
                // No texture on the outline?
                glDisable(GL_TEXTURE_2D);

                // Load the color
                glColor4f(color->red, color->green, color->blue, color->alpha);

                fillLayout = NULL;
                outlineLayout = where;
                textLayout = NULL;
                return true;
            }
        }
    }
    return false;
}


static bool setTextColor(Layout *where)
// ----------------------------------------------------------------------------
//    Set the text color according to the layout attributes
// ----------------------------------------------------------------------------
{
    // Cache in case we use only text
    if (textLayout == where)
        return true;

    // Check if we have a non-transparent text color
    if (where)
    {
        if (TextColor *color = where->textColor)
        {
            if (color->alpha > 0.0)
            {
                // Load the color
                glColor4f(color->red, color->green, color->blue, color->alpha);

                fillLayout = NULL;
                outlineLayout = NULL;
                textLayout = where;
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
    if (setOutlineColor(where))
    {
        path.mode = GL_LINES;
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
    (void) where;
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
