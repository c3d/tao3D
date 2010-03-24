#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H
// ****************************************************************************
//  attributes.h                                                    Tao project
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

#include "drawing.h"

TAO_BEGIN

struct Attribute : Drawing
// ----------------------------------------------------------------------------
//   Base class for all attributes
// ----------------------------------------------------------------------------
{
    Attribute(): Drawing() {}
    virtual bool IsAttribute()          { return true; }
};


struct Color : Attribute
// ----------------------------------------------------------------------------
//    Record a color
// ----------------------------------------------------------------------------
{
    Color(float r, float g, float b, float a):
        Attribute(), red(r), green(g), blue(b), alpha(a) {}
    float red, green, blue, alpha;
};


struct OutlineColor : Color
// ----------------------------------------------------------------------------
//   Record the color for shape outline
// ----------------------------------------------------------------------------
{
    OutlineColor(float r, float g, float b, float a):
        Color(r,g,b,a) {}
    virtual void Draw(Layout *where);
};


struct FillColor : Color
// ----------------------------------------------------------------------------
//   Record the color for shape fill
// ----------------------------------------------------------------------------
{
    FillColor(float r, float g, float b, float a):
        Color(r,g,b,a) {}
    virtual void Draw(Layout *where);
};


struct TextColor : Color
// ----------------------------------------------------------------------------
//   Record the color for text
// ----------------------------------------------------------------------------
{
    TextColor(float r, float g, float b, float a):
        Color(r,g,b,a) {}
    virtual void Draw(Layout *where);
};


struct FillTexture : Attribute
// ----------------------------------------------------------------------------
//    Record a texture change
// ----------------------------------------------------------------------------
{
    FillTexture(uint glName): Attribute(), glName(glName) {}
    virtual void Draw(Layout *where);
    uint        glName;
};


struct LineWidth : Attribute
// ----------------------------------------------------------------------------
//    Record the line width when drawing elements
// ----------------------------------------------------------------------------
{
    LineWidth(float w) : Attribute(), width(w) {}
    virtual void Draw(Layout *where);
    float width;
};


struct LineStipple : Attribute
// ----------------------------------------------------------------------------
//    Record the line width when drawing elements
// ----------------------------------------------------------------------------
{
    LineStipple(uint16 pattern, uint scale = 1)
        : Attribute(), scale(scale), pattern(pattern) {}
    virtual void Draw(Layout *where);
    uint16 scale;
    uint16 pattern;
};


struct JustificationChange : Attribute
// ----------------------------------------------------------------------------
//   Change a justification attribute
// ----------------------------------------------------------------------------
{
    enum Axis { AlongX, AlongY, AlongZ };
    JustificationChange(float a, Axis x): Attribute(), amount(a), axis(x) {}
    virtual void Draw(Layout *where);
    float amount;
    Axis  axis;
};


struct CenteringChange : JustificationChange
// ----------------------------------------------------------------------------
//   Change a centering attribute
// ----------------------------------------------------------------------------
{
    CenteringChange(float a, Axis x): JustificationChange(a, x) {}
    virtual void Draw(Layout *where);
};


struct SpreadChange : JustificationChange
// ----------------------------------------------------------------------------
//   Change a spread attribute
// ----------------------------------------------------------------------------
{
    SpreadChange(float a, Axis x): JustificationChange(a, x) {}
    virtual void Draw(Layout *where);
};


struct SpacingChange : JustificationChange
// ----------------------------------------------------------------------------
//   Change a spacing attribute - Y axis is for line spacing
// ----------------------------------------------------------------------------
{
    SpacingChange(float a, Axis x): JustificationChange(a, x) {}
    virtual void Draw(Layout *where);
};

TAO_END

#endif // ATTRIBUTES_H
