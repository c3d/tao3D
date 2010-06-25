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
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "attributes.h"
#include "layout.h"
#include "widget.h"
#include "tao_utf8.h"
#include <GL/glew.h>
#include <iostream>


TAO_BEGIN

Box3 Attribute::Bounds(Layout *where)
// ----------------------------------------------------------------------------
//   Make sure we apply attributes when computing bounds
// ----------------------------------------------------------------------------
{
    Draw(where);
    return Box3(where->offset, Vector3(0,0,0));
}


Box3 Attribute::Space(Layout *where)
// ----------------------------------------------------------------------------
//    Make sure we apply attributes when computing space
// ----------------------------------------------------------------------------
{
    Draw(where);
    return Box3(where->offset, Vector3(0,0,0));
}


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


void TextureWrap::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Replay a texture change
// ----------------------------------------------------------------------------
{
    where->wrapS = s;
    where->wrapT = t;
}


void TextureTransform::Draw(Layout *)
// ----------------------------------------------------------------------------
//   Enter or exit texture transform mode
// ----------------------------------------------------------------------------
{
    if (enable)
        glMatrixMode(GL_TEXTURE);
    else
        glMatrixMode(GL_MODELVIEW);
}


void LineWidth::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Replay a line width change
// ----------------------------------------------------------------------------
{
    where->lineWidth = width;
    if (width > 0.0)
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
//   Replay a justification change
// ----------------------------------------------------------------------------
{
    layoutJustification(where, axis).amount = amount;
}


void CenteringChange::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Replay a centering change
// ----------------------------------------------------------------------------
{
    layoutJustification(where, axis).centering = amount;
}


void SpreadChange::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Replay a spread change
// ----------------------------------------------------------------------------
{
    layoutJustification(where, axis).spread = amount;
}


void SpacingChange::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Replay a spacing change
// ----------------------------------------------------------------------------
{
    layoutJustification(where, axis).spacing = amount;
}


void MinimumSpacingChange::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Replay a change in minimum spacing
// ----------------------------------------------------------------------------
{
    layoutJustification(where, axis).after = amount;
    layoutJustification(where, axis).before = before;
}


void HorizontalMarginChange::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Replay an horizontal margin change
// ----------------------------------------------------------------------------
{
    where->left = left;
    where->right = right;
}


void VerticalMarginChange::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Replay a vertical margin change
// ----------------------------------------------------------------------------
{
    where->top = top;
    where->bottom = bottom;
}


void DepthTest::Draw(Layout *)
// ----------------------------------------------------------------------------
//   Enable or disable the depth test
// ----------------------------------------------------------------------------
{
    if (enable)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}


void RecordMouseCoordinates::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Record the widget mouse coordinates in a tree info
// ----------------------------------------------------------------------------
{
    Widget *widget = where->Display();
    widget->recordProjection();
    Point3 pos = widget->unprojectLastMouse();
    MouseCoordinatesInfo *info = self->GetInfo<MouseCoordinatesInfo>();
    if (!info)
    {
        info = new MouseCoordinatesInfo;
        self->SetInfo<MouseCoordinatesInfo>(info);
    }
    info->coordinates = pos;
}

TAO_END
