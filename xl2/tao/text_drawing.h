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
#include "shapes.h"
#include <QFont>


TAO_BEGIN

struct TextSpan : Shape
// ----------------------------------------------------------------------------
//    A contiguous run of glyphs
// ----------------------------------------------------------------------------
{
    TextSpan(text utf8): Shape(), utf8(utf8) {}
    virtual void Draw(Layout *where);
    virtual Box3 Bounds();
    virtual Box3 Space();
    text utf8;
};


struct TextFont : Attribute
// ----------------------------------------------------------------------------
//    Record a font change
// ----------------------------------------------------------------------------
{
    TextFont(const QFont &f): font(f) {}
    virtual void Draw(Layout *where);
    QFont font;
    static TextFont *last;
};

TAO_END

#endif // TEXT_DRAWING_H
