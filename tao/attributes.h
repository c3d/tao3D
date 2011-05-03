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
#include <QFont>
#include <QtOpenGL>

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
};


struct DrawingBreak : Attribute
// ----------------------------------------------------------------------------
//   Break at the given level
// ----------------------------------------------------------------------------
{
    DrawingBreak(BreakOrder order): order(order) {}
    virtual Drawing *   Break(BreakOrder &order, uint &sz);
    virtual void        DrawSelection(Layout *l);
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
};


struct LineColor : ColorAttribute
// ----------------------------------------------------------------------------
//   Record the color for shape outline
// ----------------------------------------------------------------------------
{
    LineColor(float r, float g, float b, float a):
        ColorAttribute(r,g,b,a) {}
    virtual void Draw(Layout *where);
};


struct FillColor : ColorAttribute
// ----------------------------------------------------------------------------
//   Record the color for shape fill
// ----------------------------------------------------------------------------
{
    FillColor(float r, float g, float b, float a):
        ColorAttribute(r,g,b,a) {}
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


struct TextureWrap : Attribute
// ----------------------------------------------------------------------------
//    Record a texture wrapping setting
// ----------------------------------------------------------------------------
{
    TextureWrap(bool s, bool t): Attribute(), s(s), t(t) {}
    virtual void Draw(Layout *where);
    bool        s, t;
};


struct TextureTransform : Attribute
// ----------------------------------------------------------------------------
//    Record a texture transform
// ----------------------------------------------------------------------------
{
    TextureTransform(bool enable): Attribute(), enable(enable) {}
    virtual void Draw(Layout *where);
    bool        enable;
};


struct Visibility : Attribute
// ----------------------------------------------------------------------------
//    Record the visibility when drawing elements
// ----------------------------------------------------------------------------
{
    Visibility(float v) : Attribute(), visibility(v) {}
    virtual void Draw(Layout *where);
    scale visibility;
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


struct FontChange : Attribute
// ----------------------------------------------------------------------------
//   Change a font attribute
// ----------------------------------------------------------------------------
{
    FontChange(QFont font): Attribute(), font(font) {}
    virtual void Draw(Layout *where);
    QFont font;
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


struct PartialJustificationChange : JustificationChange
// ----------------------------------------------------------------------------
//   Change justification for partial lines
// ----------------------------------------------------------------------------
{
    PartialJustificationChange(float a, Axis x): JustificationChange(a, x) {}
    virtual void Draw(Layout *where);
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


struct MinimumSpacingChange : JustificationChange
// ----------------------------------------------------------------------------
//   Change a spacing attribute - Y axis is for line spacing
// ----------------------------------------------------------------------------
{
    MinimumSpacingChange(float b, float a, Axis x)
        : JustificationChange(a, x), before(b) {}
    virtual void Draw(Layout *where);
    coord before;
};


struct HorizontalMarginChange : Attribute
// ----------------------------------------------------------------------------
//   Change the horizontal text margin
// ----------------------------------------------------------------------------
{
    HorizontalMarginChange(coord l, coord r): left(l), right(r) {}
    virtual void Draw(Layout *where);
    coord left, right;
};


struct VerticalMarginChange : Attribute
// ----------------------------------------------------------------------------
//   Change the vertical text margin
// ----------------------------------------------------------------------------
{
    VerticalMarginChange(coord t, coord b): top(t), bottom(b) {}
    virtual void Draw(Layout *where);
    coord top, bottom;
};


struct DepthTest : Attribute
// ----------------------------------------------------------------------------
//   Change the depth test
// ----------------------------------------------------------------------------
{
    DepthTest(bool enable): enable(enable) {}
    virtual void Draw(Layout *where);
    bool enable;
};


struct MouseCoordinatesInfo : XL::Info
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
};

TAO_END

#endif // ATTRIBUTES_H
