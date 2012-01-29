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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "drawing.h"
#include "color.h"
#include "tao_gl.h"
#include <QFont>

TAO_BEGIN

struct Attribute : Drawing
// ----------------------------------------------------------------------------
//   Base class for all attributes
// ----------------------------------------------------------------------------
{
    Attribute(): Drawing() {}
    virtual void        DrawSelection(Layout *l)        { Draw(l); }
    virtual void        Identify(Layout *l)             { Draw(l); }
    virtual Box3        Bounds(Layout *l);
    virtual Box3        Space(Layout *l);
    virtual bool        IsAttribute()                   { return true; }
    virtual text        Type() { return "Attribute";}
};


struct DrawingBreak : Attribute
// ----------------------------------------------------------------------------
//   Break at the given level
// ----------------------------------------------------------------------------
{
    DrawingBreak(BreakOrder order): order(order) {}
    virtual Drawing *   Break(BreakOrder &order, uint &sz);
    virtual void        DrawSelection(Layout *l);
    virtual text        Type() { return "DrawingBreak";}
    BreakOrder order;
};


struct ColorAttribute : Attribute
// ----------------------------------------------------------------------------
//    Record a color
// ----------------------------------------------------------------------------
{
    ColorAttribute(float r, float g, float b, float a):
        Attribute(), color(r, g, b, a) {}
    Color color;
    virtual void        Evaluate(Layout *l)     { Draw(l); }
    virtual text        Type() { return "ColorAttribute";}
};


struct LineColor : ColorAttribute
// ----------------------------------------------------------------------------
//   Record the color for shape outline
// ----------------------------------------------------------------------------
{
    LineColor(float r, float g, float b, float a):
        ColorAttribute(r,g,b,a) {}
    virtual void        Draw(Layout *where);
    virtual text        Type() { return "LineColor";}
};


struct FillColor : ColorAttribute
// ----------------------------------------------------------------------------
//   Record the color for shape fill
// ----------------------------------------------------------------------------
{
    FillColor(float r, float g, float b, float a):
        ColorAttribute(r,g,b,a) {}
    virtual void        Draw(Layout *where);
    virtual text        Type() { return "FillColor";}
};


struct FillTexture : Attribute
// ----------------------------------------------------------------------------
//    Record a texture change
// ----------------------------------------------------------------------------
{
    FillTexture(uint glName, GLenum glType = GL_TEXTURE_2D, bool mipmap = false)
        : Attribute(), glName(glName), glType(glType), mipmap(mipmap) {}
    virtual void Draw(Layout *where);
    uint   glName;
    GLenum glType;
    bool   mipmap;
    virtual text        Type() { return "FillTexture";}
};


struct TextureUnit : Attribute
// ----------------------------------------------------------------------------
//    Record a texture wrapping setting
// ----------------------------------------------------------------------------
{
    TextureUnit(uint glUnit)
        : Attribute(), glUnit(glUnit) {}
    virtual void Draw(Layout *where);
    uint  glUnit;
    virtual text        Type() { return "TextureUnit";}
};


struct TextureWrap : Attribute
// ----------------------------------------------------------------------------
//    Record a texture wrapping setting
// ----------------------------------------------------------------------------
{
    TextureWrap(bool s, bool t)
        : Attribute(), s(s), t(t) {}
    virtual void Draw(Layout *where);
    bool s, t;
    virtual text        Type() { return "TextureWrap";}
};


struct TextureMode : Attribute
// ----------------------------------------------------------------------------
//    Record a texture blending setting
// ----------------------------------------------------------------------------
{
    TextureMode(GLenum mode)
        : Attribute(), mode(mode) {}
    virtual void Draw(Layout *where);
    GLenum mode;
    virtual text        Type() { return "TextureMode";}
};


struct TextureTransform : Attribute
// ----------------------------------------------------------------------------
//    Record a texture transform
// ----------------------------------------------------------------------------
{
    TextureTransform(bool enable)
        : Attribute(), enable(enable) {}
    virtual void Draw(Layout *where);
    bool  enable;
    virtual text        Type() { return "TextureTransform";}
};


struct Visibility : Attribute
// ----------------------------------------------------------------------------
//    Record the visibility when drawing elements
// ----------------------------------------------------------------------------
{
    Visibility(float v) : Attribute(), visibility(v) {}
    virtual void Draw(Layout *where);
    scale visibility;
    virtual text        Type() { return "Visibility";}
};


struct LineWidth : Attribute
// ----------------------------------------------------------------------------
//    Record the line width when drawing elements
// ----------------------------------------------------------------------------
{
    LineWidth(float w) : Attribute(), width(w) {}
    virtual void Draw(Layout *where);
    float width;
    virtual text        Type() { return "LineWidth";}
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
    virtual text        Type() { return "LineStipple";}
};


struct FontChange : Attribute
// ----------------------------------------------------------------------------
//   Change a font attribute
// ----------------------------------------------------------------------------
{
    FontChange(QFont font): Attribute(), font(font) {}
    virtual void Draw(Layout *where);
    QFont font;
    virtual void        Evaluate(Layout *l)     { Draw(l); }
    virtual text        Type() { return "FontChange";}
};


struct JustificationChange : Attribute
// ----------------------------------------------------------------------------
//   Change a justification attribute
// ----------------------------------------------------------------------------
{
    enum Axis { AlongX, AlongY, AlongZ };
    JustificationChange(float a, Axis x): Attribute(), amount(a), axis(x) {}
    virtual void        Draw(Layout *where);
    float amount;
    Axis  axis;
    virtual void        Evaluate(Layout *l)     { Draw(l); }
    virtual text        Type() { return "JustificationChange";}
};


struct PartialJustificationChange : JustificationChange
// ----------------------------------------------------------------------------
//   Change justification for partial lines
// ----------------------------------------------------------------------------
{
    PartialJustificationChange(float a, Axis x): JustificationChange(a, x) {}
    virtual void Draw(Layout *where);
    virtual text Type() { return "PartialJustificationChange";}
};


struct CenteringChange : JustificationChange
// ----------------------------------------------------------------------------
//   Change a centering attribute
// ----------------------------------------------------------------------------
{
    CenteringChange(float a, Axis x): JustificationChange(a, x) {}
    virtual void Draw(Layout *where);
    virtual text Type() { return "CenteringChange";}
};


struct SpreadChange : JustificationChange
// ----------------------------------------------------------------------------
//   Change a spread attribute
// ----------------------------------------------------------------------------
{
    SpreadChange(float a, Axis x): JustificationChange(a, x) {}
    virtual void Draw(Layout *where);
    virtual text Type() { return "SpreadChange";}
};


struct SpacingChange : JustificationChange
// ----------------------------------------------------------------------------
//   Change a spacing attribute - Y axis is for line spacing
// ----------------------------------------------------------------------------
{
    SpacingChange(float a, Axis x): JustificationChange(a, x) {}
    virtual void Draw(Layout *where);
    virtual text Type() { return "SpacingChange";}
};


struct MinimumSpacingChange : JustificationChange
// ----------------------------------------------------------------------------
//   Change a spacing attribute - Y axis is for line spacing
// ----------------------------------------------------------------------------
{
    MinimumSpacingChange(float b, float a, Axis x)
        : JustificationChange(a, x), before(b) {}
    virtual void Draw(Layout *where);
    coord before;
    virtual text Type() { return "MinimumSpacingChange";}
};


struct HorizontalMarginChange : Attribute
// ----------------------------------------------------------------------------
//   Change the horizontal text margin
// ----------------------------------------------------------------------------
{
    HorizontalMarginChange(coord l, coord r): left(l), right(r) {}
    virtual void Draw(Layout *where);
    coord left, right;
    virtual void Evaluate(Layout *l)     { Draw(l); }
    virtual text Type() { return "HorizontalMarginChange";}
};


struct VerticalMarginChange : Attribute
// ----------------------------------------------------------------------------
//   Change the vertical text margin
// ----------------------------------------------------------------------------
{
    VerticalMarginChange(coord t, coord b): top(t), bottom(b) {}
    virtual void Draw(Layout *where);
    coord top, bottom;
    virtual void Evaluate(Layout *l)     { Draw(l); }
    virtual text Type() { return "VerticalMarginChange";}
};


struct DepthTest : Attribute
// ----------------------------------------------------------------------------
//   Change the depth test
// ----------------------------------------------------------------------------
{
    DepthTest(bool enable): enable(enable) {}
    virtual void Draw(Layout *where);
    virtual text Type() { return "DepthTest";}
    bool enable;
};


struct BlendFunc : Attribute
// ----------------------------------------------------------------------------
//   Change the blend function
// ----------------------------------------------------------------------------
{
    BlendFunc(GLenum sf, GLenum df): sfactor(sf), dfactor(df) {}
    virtual void Draw(Layout *where);
    virtual text Type() { return "BlendFunc"; }
    GLenum sfactor, dfactor;
};


struct BlendFuncSeparate : BlendFunc
// ----------------------------------------------------------------------------
//   Change the blend function separately for alpha and color
// ----------------------------------------------------------------------------
{
    BlendFuncSeparate(GLenum sf, GLenum df, GLenum sfa, GLenum dfa)
        : BlendFunc(sf, df), sfalpha(sfa), dfalpha(dfa) {}
    virtual void Draw(Layout *where);
    virtual text Type() { return "BlendFuncSeparate"; }
    GLenum sfalpha, dfalpha;
};


struct BlendEquation : Attribute
// ----------------------------------------------------------------------------
//   Change the blend equation
// ----------------------------------------------------------------------------
{
    BlendEquation(GLenum be) : equation(be) {}
    virtual void Draw(Layout *where);
    virtual text Type() { return "BlendEquation"; }
    GLenum equation;
};


struct CoordinatesInfo : XL::Info
// ----------------------------------------------------------------------------
//   Record unprojected mouse coordinates
// ----------------------------------------------------------------------------
{
    Point3      coordinates;
    GLdouble    projection[16], model[16];
    GLint       viewport[4];
};


struct RecordMouseCoordinates : Attribute
// ----------------------------------------------------------------------------
//    Record the mouse coordinates in current projection matrix for a tree
// ----------------------------------------------------------------------------
{
    RecordMouseCoordinates(Tree *self): self(self) {}
    virtual void Draw(Layout *where);
    Tree_p self;
    virtual text        Type() { return "RecordMouseCoordinates";}
};


struct ConvertScreenCoordinates : Attribute
// ----------------------------------------------------------------------------
//   Conversion to world coordinates
// ----------------------------------------------------------------------------
{
    ConvertScreenCoordinates(Tree *self, coord x, coord y)
        : self(self), x(x), y(y) {}
    virtual void Draw(Layout *where);
    virtual void DrawSelection(Layout *)        {}
    virtual void Identify(Layout *)        {}
    Tree_p self;
    coord x,y;
};



// ============================================================================
// 
//   Entering attributes in the symbols table
// 
// ============================================================================

extern void EnterAttributes();
extern void DeleteAttributes();

TAO_END

#endif // ATTRIBUTES_H
