// ****************************************************************************
//  attributes.cpp                                                  Tao project
// ****************************************************************************
//
//   File Description:
//
//     Attributes that may apply to a shape, like color
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

#include "attributes.h"
#include "layout.h"
#include <GL/glew.h>
#include <iostream>


TAO_BEGIN

Drawing *DrawingBreak::Break(BreakOrder &order)
// ----------------------------------------------------------------------------
//   Perform a character, word, line, column or page break
// ----------------------------------------------------------------------------
{
    order = this->order;
    return NULL;
}


void LineColor::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Remember the color in the layout
// ----------------------------------------------------------------------------
{
    where->lineColor = color;
}


void FillColor::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Remember the color in the layout
// ----------------------------------------------------------------------------
{
    where->fillColor = color;
}


void FillTexture::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Replay a texture change
// ----------------------------------------------------------------------------
{
    where->fillTexture = glName;
}


void LineWidth::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Replay a line width change
// ----------------------------------------------------------------------------
{
    where->lineWidth = width;
    glLineWidth(width);
}


void LineStipple::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Replay a color change
// ----------------------------------------------------------------------------
{
    (void) where;
    if (scale)
    {
        glLineStipple(scale, pattern);
        glEnable(GL_LINE_STIPPLE);
    }
    else
    {
        glDisable(GL_LINE_STIPPLE);
    }
}


void FontChange::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Replay a font change
// ----------------------------------------------------------------------------
{
    where->font = font;
}


static inline Justification &layoutJustification(Layout *where,
                                                 JustificationChange::Axis a)
// ----------------------------------------------------------------------------
//   Return the justification to be modified by a given attribute
// ----------------------------------------------------------------------------
{
    switch (a)
    {
    default:
        std::cerr << "layoutJustification: Invalid axis " << a << "\n";

    case JustificationChange::AlongX:   return where->alongX;
    case JustificationChange::AlongY:   return where->alongY;
    case JustificationChange::AlongZ:   return where->alongZ;
    }
}


void JustificationChange::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Replay a color change
// ----------------------------------------------------------------------------
{
    layoutJustification(where, axis).amount = amount;
}


void CenteringChange::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Replay a color change
// ----------------------------------------------------------------------------
{
    layoutJustification(where, axis).centering = amount;
}


void SpreadChange::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Replay a color change
// ----------------------------------------------------------------------------
{
    layoutJustification(where, axis).spread = amount;
}


void SpacingChange::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Replay a color change
// ----------------------------------------------------------------------------
{
    layoutJustification(where, axis).spacing = amount;
}

TAO_END
