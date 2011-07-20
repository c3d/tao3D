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
    virtual text        getType() { return "Attribute";}
};


struct DrawingBreak : Attribute
// ----------------------------------------------------------------------------
//   Break at the given level
// ----------------------------------------------------------------------------
{
    DrawingBreak(BreakOrder order): order(order) {}
    virtual Drawing *   Break(BreakOrder &order, uint &sz);
    virtual void        DrawSelection(Layout *l);
    virtual text        getType() { return "DrawingBreak";}
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
    virtual text        getType() { return "ColorAttribute";}
};


struct LineColor : ColorAttribute
// ----------------------------------------------------------------------------
//   Record the color for shape outline
// ----------------------------------------------------------------------------
{
    LineColor(float r, float g, float b, float a):
        ColorAttribute(r,g,b,a) {}
    virtual void Draw(Layout *where);
    virtual text        getType() { return "LineColor";}
};


struct FillColor : ColorAttribute
// ----------------------------------------------------------------------------
//   Record the color for shape fill
// ----------------------------------------------------------------------------
{
    FillColor(float r, float g, float b, float a):
        ColorAttribute(r,g,b,a) {}
    virtual void        Draw(Layout *where);
    virtual text        getType() { return "FillColor";}
};


struct FillTexture : Attribute
// ----------------------------------------------------------------------------
//    Record a texture change
// ----------------------------------------------------------------------------
{
    FillTexture(uint glName, uint glUnit, GLenum glType = GL_TEXTURE_2D): Attribute(), glName(glName), glUnit(glUnit), glType(glType) {}
    virtual void Draw(Layout *where);
    uint   glName;
    uint   glUnit;
    GLenum glType;
    virtual text        getType() { return "FillTexture";}
};


struct TextureWrap : Attribute
// ----------------------------------------------------------------------------
//    Record a texture wrapping setting
// ----------------------------------------------------------------------------
{
    TextureWrap(bool s, bool t, uint glUnit): Attribute(), s(s), t(t), glUnit(glUnit) {}
    virtual void Draw(Layout *where);
    bool s, t;
    uint  glUnit;
    virtual text        getType() { return "TextureWrap";}
};

struct TextureTransform : Attribute
// ----------------------------------------------------------------------------
//    Record a texture transform
// ----------------------------------------------------------------------------
{
    TextureTransform(bool enable, uint glUnit): Attribute(), enable(enable), glUnit(glUnit) {}
    virtual void Draw(Layout *where);
    bool  enable;
    uint  glUnit;
    virtual text        getType() { return "TextureTransform";}
};


struct Visibility : Attribute
// ----------------------------------------------------------------------------
//    Record the visibility when drawing elements
// ----------------------------------------------------------------------------
{
    Visibility(float v) : Attribute(), visibility(v) {}
    virtual void Draw(Layout *where);
    scale visibility;
    virtual text        getType() { return "Visibility";}
};


struct LineWidth : Attribute
// ----------------------------------------------------------------------------
//    Record the line width when drawing elements
// ----------------------------------------------------------------------------
{
    LineWidth(float w) : Attribute(), width(w) {}
    virtual void Draw(Layout *where);
    float width;
    virtual text        getType() { return "LineWidth";}
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
    virtual text        getType() { return "LineStipple";}
};


struct FontChange : Attribute
// ----------------------------------------------------------------------------
//   Change a font attribute
// ----------------------------------------------------------------------------
{
    FontChange(QFont font): Attribute(), font(font) {}
    virtual void Draw(Layout *where);
    QFont font;
    virtual text        getType() { return "FontChange";}
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
    virtual text        getType() { return "JustificationChange";}
};


struct PartialJustificationChange : JustificationChange
// ----------------------------------------------------------------------------
//   Change justification for partial lines
// ----------------------------------------------------------------------------
{
    PartialJustificationChange(float a, Axis x): JustificationChange(a, x) {}
    virtual void Draw(Layout *where);
    virtual text        getType() { return "PartialJustificationChange";}
};


struct CenteringChange : JustificationChange
// ----------------------------------------------------------------------------
//   Change a centering attribute
// ----------------------------------------------------------------------------
{
    CenteringChange(float a, Axis x): JustificationChange(a, x) {}
    virtual void Draw(Layout *where);
    virtual text        getType() { return "CenteringChange";}
};


struct SpreadChange : JustificationChange
// ----------------------------------------------------------------------------
//   Change a spread attribute
// ----------------------------------------------------------------------------
{
    SpreadChange(float a, Axis x): JustificationChange(a, x) {}
    virtual void Draw(Layout *where);
    virtual text        getType() { return "SpreadChange";}
};


struct SpacingChange : JustificationChange
// ----------------------------------------------------------------------------
//   Change a spacing attribute - Y axis is for line spacing
// ----------------------------------------------------------------------------
{
    SpacingChange(float a, Axis x): JustificationChange(a, x) {}
    virtual void Draw(Layout *where);
    virtual text        getType() { return "SpacingChange";}
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
    virtual text        getType() { return "MinimumSpacingChange";}
};


struct HorizontalMarginChange : Attribute
// ----------------------------------------------------------------------------
//   Change the horizontal text margin
// ----------------------------------------------------------------------------
{
    HorizontalMarginChange(coord l, coord r): left(l), right(r) {}
    virtual void Draw(Layout *where);
    coord left, right;
    virtual text        getType() { return "HorizontalMarginChange";}
};


struct VerticalMarginChange : Attribute
// ----------------------------------------------------------------------------
//   Change the vertical text margin
// ----------------------------------------------------------------------------
{
    VerticalMarginChange(coord t, coord b): top(t), bottom(b) {}
    virtual void Draw(Layout *where);
    coord top, bottom;
    virtual text        getType() { return "VerticalMarginChange";}
};


struct DepthTest : Attribute
// ----------------------------------------------------------------------------
//   Change the depth test
// ----------------------------------------------------------------------------
{
    DepthTest(bool enable): enable(enable) {}
    virtual void Draw(Layout *where);
    bool enable;
    virtual text        getType() { return "DepthTest";}
};


struct MouseCoordinatesInfo : XL::Info
// ----------------------------------------------------------------------------
//   Record unprojected mouse coordinates
// ----------------------------------------------------------------------------
{
    Point3      coordinates;
    GLdouble    projection[16], model[16];
    GLint       viewport[4];
    virtual text        getType() { return "MouseCoordinatesInfo";}
};


struct RecordMouseCoordinates : Attribute
// ----------------------------------------------------------------------------
//    Record the mouse coordinates in current projection matrix for a tree
// ----------------------------------------------------------------------------
{
    RecordMouseCoordinates(Tree *self): self(self) {}
    virtual void Draw(Layout *where);
    Tree_p self;
    virtual text        getType() { return "RecordMouseCoordinates";}
};

TAO_END

#endif // ATTRIBUTES_H
