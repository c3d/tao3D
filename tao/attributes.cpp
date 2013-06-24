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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "attributes.h"
#include "layout.h"
#include "widget.h"
#include "tao_utf8.h"
#include <iostream>
#include "text_drawing.h"
#include "application.h"
#include "preferences_pages.h"

TAO_BEGIN

Box3 Attribute::Bounds(Layout *where)
// ----------------------------------------------------------------------------
//   Make sure we apply attributes when computing bounds
// ----------------------------------------------------------------------------
{
    Draw(where);
    // Bug#130 Attribute has no bounds. (and not 0,0,0)
    return Box3();
}


Box3 Attribute::Space(Layout *where)
// ----------------------------------------------------------------------------
//    Make sure we apply attributes when computing space
// ----------------------------------------------------------------------------
{
    Draw(where);
    // Bug#130 Attribute takes no space. (and not 0,0,0)
    return Box3();
}


bool DrawingBreak::Paginate(PageLayout *page)
// ----------------------------------------------------------------------------
//   Perform pagination: append this as a character break
// ----------------------------------------------------------------------------
{
    return page->PaginateItem(this, order);
}


void DrawingBreak::DrawSelection(Layout *l)
// ----------------------------------------------------------------------------
//   Insert the break into the selection description (for copy)
// ----------------------------------------------------------------------------
{
    Widget     *widget       = l->Display();
    TextSelect *sel          = widget->textSelection();
    if (sel && sel->inSelection)
    {
        if (order == LineBreak)
            sel->cursor.insertText("\n");
        else if (order >= ParaBreak)
            sel->cursor.insertBlock();
    }
    Draw(l);
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


void CachedDrawing::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw a cached drawing from a display list
// ----------------------------------------------------------------------------
{
    (void) where;
    GL.CallList(displayList);
}


void FillTexture::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Remember the texture in the layout
// ----------------------------------------------------------------------------
{
    (void) where;
    GL.Enable(glType);
    CachedTexture *cached = NULL;
    QSharedPointer<TextureCache> cache = TextureCache::instance();
    cached = cache->bind(glName);
    if(!cached)
        GL.BindTexture(glType, glName);
}


void TextureUnit::Draw(Layout *where)
// ------------------------------------------------------------- ---------------
//   Remember the texture unit in the layout
// ----------------------------------------------------------------------------
{
    (void) where;
    GL.ActiveTexture(GL_TEXTURE0 + glUnit);
}


void TextureWrap::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Replay a texture change
// ----------------------------------------------------------------------------
{
    (void) where;
    GLenum wrapS = s ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    GLenum wrapT = t ? GL_REPEAT : GL_CLAMP_TO_EDGE;

    GL.TexParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    GL.TexParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);

    GL.TexUnitParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    GL.TexUnitParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
}


void TextureMode::Draw(Layout *where)
// ------------------------------------------------------------- ---------------
//   Replay a texture mode
// ----------------------------------------------------------------------------
{
    (void) where;
    GL.TexEnv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode);
}


void TextureMinFilter::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Replay a texture minifiying filter
// ----------------------------------------------------------------------------
{
    (void) where;
    GL.TexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    GL.TexUnitParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
}


void TextureMagFilter::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Replay a texture magnigication filter
// ----------------------------------------------------------------------------
{
    (void) where;
    GL.TexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    GL.TexUnitParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
}


void TextureMipMap::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Enable or disable mipmapping for textures
// ----------------------------------------------------------------------------
{
    (void) where;
    GL.HasMipMapping(enable);
}


void TextureTransform::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Enter or exit texture transform mode
// ----------------------------------------------------------------------------
{
    (void) where;
    if (enable)
        GL.MatrixMode(GL_TEXTURE);
    else
        GL.MatrixMode(GL_MODELVIEW);
}


void Visibility::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Replay a line width change
// ----------------------------------------------------------------------------
{
    if (visibility >= 0)
        where->visibility *= visibility;
    else
        where->visibility = -visibility;
}


void ExtrudeDepth::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Set the extrude depth in the layout
// ----------------------------------------------------------------------------
{
    where->extrudeDepth = depth;
}


void ExtrudeRadius::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Set the extrude radius in the layout
// ----------------------------------------------------------------------------
{
    where->extrudeRadius = radius;
}


void ExtrudeCount::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Set the extrude count in the layout
// ----------------------------------------------------------------------------
{
    where->extrudeCount = count;
}


void LineWidth::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Replay a line width change
// ----------------------------------------------------------------------------
{
    where->lineWidth = width;
    if (width > 0.0)
        GL.LineWidth(width * where->PrinterScaling());
}


void LineStipple::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Replay a color change
// ----------------------------------------------------------------------------
{
    (void) where;
    if (scale)
    {
        GL.LineStipple(scale, pattern);
        GL.Enable(GL_LINE_STIPPLE);
    }
    else
    {
        GL.Disable(GL_LINE_STIPPLE);
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


void PartialJustificationChange::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Replay a partial justification change
// ----------------------------------------------------------------------------
{
    layoutJustification(where, axis).partial = amount;
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
        GL.Enable(GL_DEPTH_TEST);
    else
        GL.Disable(GL_DEPTH_TEST);
}


void DepthMask::Draw(Layout *)
// ----------------------------------------------------------------------------
//   Enable or disable the depth mask
// ----------------------------------------------------------------------------
{
    GL.DepthMask(enable ? GL_TRUE : GL_FALSE);
}


void DepthFunc::Draw(Layout *)
// ----------------------------------------------------------------------------
//   Specifies the depth comparison function
// ----------------------------------------------------------------------------
{
    GL.DepthFunc(func);
}


void BlendFunc::Draw(Layout *)
// ----------------------------------------------------------------------------
//   Change the blend function
// ----------------------------------------------------------------------------
{
    GL.BlendFunc(sfactor, dfactor);
}


void BlendFuncSeparate::Draw(Layout *)
// ----------------------------------------------------------------------------
//   Change the blend function separately for alpha and color
// ----------------------------------------------------------------------------
{
    GL.BlendFuncSeparate(sfactor, dfactor, sfalpha, dfalpha);
}


void BlendEquation::Draw(Layout *)
// ----------------------------------------------------------------------------
//   Change the blend equation
// ----------------------------------------------------------------------------
{
    GL.BlendEquation(equation);
}


void RecordMouseCoordinates::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Record the widget mouse coordinates in a tree info
// ----------------------------------------------------------------------------
{
    Widget *widget = where->Display();
    if (widget->mouseTracking())
    {
        CoordinatesInfo *info = self->GetInfo<CoordinatesInfo>();
        if (!info)
        {
            info = new CoordinatesInfo;
            self->SetInfo<CoordinatesInfo>(info);
        }

        widget->recordProjection(info->projection, info->model, info->viewport);
        Point3 pos = widget->unprojectLastMouse(info->projection,
                                                info->model, info->viewport);
        info->coordinates = pos;
    }
}


void ConvertScreenCoordinates::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Convert screen coordinates to world coordinates (get Z depth)
// ----------------------------------------------------------------------------
{
    Widget *widget = where->Display();
    CoordinatesInfo *info = self->GetInfo<CoordinatesInfo>();
    if (!info)
    {
        info = new CoordinatesInfo;
        self->SetInfo<CoordinatesInfo>(info);
    }

    widget->recordProjection(info->projection, info->model, info->viewport);
    info->coordinates = widget->objectToWorld(x, y, info->projection, info->model, info->viewport);
}

TAO_END



// ****************************************************************************
// 
//    Code generation from attributes.tbl
// 
// ****************************************************************************

#include "graphics.h"
#include "opcodes.h"
#include "options.h"
#include "widget.h"
#include "types.h"
#include "drawing.h"
#include "layout.h"
#include "module_manager.h"
#include <iostream>


// ============================================================================
//
//    Top-level operation
//
// ============================================================================

#include "widget.h"

using namespace XL;

#include "opcodes_declare.h"
#include "attributes.tbl"

namespace Tao
{

#define ATTRIBUTE(Name, Accessor)               \
void Name##Attribute::Draw(Layout *where)       \
{                                               \
    where->Accessor(value);                     \
}
#include "attributes.tbl"


void EnterAttributes()
// ----------------------------------------------------------------------------
//   Enter all the basic operations defined in attributes.tbl
// ----------------------------------------------------------------------------
{
    XL::Context *context = MAIN->context;
#include "opcodes_define.h"
#include "attributes.tbl"
}


void DeleteAttributes()
// ----------------------------------------------------------------------------
//   Delete all the global operations defined in attributes.tbl
// ----------------------------------------------------------------------------
{
#include "opcodes_delete.h"
#include "attributes.tbl"
}

}
