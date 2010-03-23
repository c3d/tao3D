#ifndef TEXT_DRAWING_H
#define TEXT_DRAWING_H
// ****************************************************************************
//  text_drawing.h                                                  Tao project
// ****************************************************************************
//
//   File Description:
//
//     Rendering of text
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

TAO_BEGIN

struct Glyphs : Shape
// ----------------------------------------------------------------------------
//    A contiguous run of glyphs
// ----------------------------------------------------------------------------
{
    Glyphs(text utf8): Shape(), utf8(utf8) {}
    virtual void Draw(Layout *where);
    virtual Box3 Bounds();
    virtual Box3 Space();
    text utf8;
};


struct FontChange : Attribute
// ----------------------------------------------------------------------------
//    Record a font change
// ----------------------------------------------------------------------------
{
    FontChange(text name): Attribute(), font(font) {}
    virtual void Draw(Layout *where);
    virtual Box3 Bounds();
    virtual Box3 Space();
    text font;
};


struct FontSizeChange : Attribute
// ----------------------------------------------------------------------------
//    Record a font size change
// ----------------------------------------------------------------------------
{
    FontSizeChange(float size): Attribute(), size(size) {}
    virtual void Draw(Layout *where);
    virtual Box3 Bounds();
    virtual Box3 Space();
    float size;
};

TAO_END

#endif // TEXT_DRAWING_H
