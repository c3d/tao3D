#ifndef LAYOUT_H
#define LAYOUT_H
// ****************************************************************************
//  layout.h                                                       Tao project
// ****************************************************************************
//
//   File Description:
//
//     Drawing object that is used to lay out objects in 2D or 3D space
//     Layouts are used to represent 2D pages, 3D spaces (Z ordering)
//     and text made of a sequence of glyphs
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
#include "justification.h"
#include "tao_gl.h"
#include <vector>
#include <set>
#include <QFont>
#include <QEvent>
#include <float.h>

#define MAX_TEX_UNITS 64

TAO_BEGIN

struct Widget;

struct TextureState
// ----------------------------------------------------------------------------
//   The state of texture we want to preserve
// ----------------------------------------------------------------------------
{
    TextureState(): wrapS(false), wrapT(false), id(0), mode(GL_MODULATE) {}
    bool        wrapS, wrapT;
    uint        id;
    GLenum      mode;
};

struct LayoutState
// ----------------------------------------------------------------------------
//   The state we want to preserve in a layout
// ----------------------------------------------------------------------------
{
                        LayoutState();
                        LayoutState(const LayoutState &o);

public:
    typedef std::set<QEvent::Type>              qevent_ids;
    typedef std::map<uint,TextureState>         tex_list;

public:
    void                ClearAttributes();
    static text         ToText(qevent_ids & ids);
    static text         ToText(QEvent::Type type);

public:
    Vector3             offset;
    QFont               font;
    Justification       alongX, alongY, alongZ;
    coord               left, right, top, bottom; // Margins
    scale               visibility;
    scale               lineWidth;
    Color               lineColor;
    Color               fillColor;
    uint                currentTexUnit;
    uint64              textureUnits; //Current used texture units
    uint64              previousUnits; //Previous used texture units
    tex_list            fillTextures;
    uint                lightId;
    uint                programId;
    bool                printing : 1;
    double              planarRotation;
    double              planarScale;
    uint                rotationId, translationId, scaleId;

};


struct Layout : Drawing, LayoutState
// ----------------------------------------------------------------------------
//   A layout is responsible for laying out Drawing objects in 2D or 3D space
// ----------------------------------------------------------------------------
{
                        Layout(Widget *display);
                        Layout(const Layout &o);
                        ~Layout();

    // Drawing interface
    virtual void        Draw(Layout *where);
    virtual void        DrawSelection(Layout *);
    virtual void        Identify(Layout *);
    virtual Box3        Bounds(Layout *);
    virtual Box3        Space(Layout *);

    // Layout interface
    virtual void        Add (Drawing *d);
    virtual Vector3     Offset();
    virtual Layout *    NewChild()       { return new Layout(*this); }
    virtual Layout *    AddChild(uint id=0, Tree_p body=0, Context_p ctx=0);
    virtual void        Clear();
    virtual Widget *    Display()        { return display; }
    virtual void        PolygonOffset();
    virtual uint        Selected();
    virtual uint        ChildrenSelected();

    // Event interface
    virtual bool        Refresh(QEvent *e, Layout *parent = NULL);
    bool                RefreshChildren(QEvent *e);

    LayoutState &       operator=(const LayoutState &o);
    void                Inherit(Layout *other);
    void                PushLayout(Layout *where);
    void                PopLayout(Layout *where);
    uint                CharacterId();
    double              PrinterScaling();
    text                PrettyId();

public:
    Widget *            display;
    // OpenGL identification for that shape and for characters within
    uint                id;
    uint                charId;
    // For optimized drawing, we keep track of what changes
    bool                hasPixelBlur    : 1; // Pixels not aligning naturally
    bool                hasMatrix       : 1;
    bool                has3D           : 1;
    bool                hasAttributes   : 1;
    uint64              hasTextureMatrix; // 64 texture units
    bool                hasLighting     : 1;
    bool                hasMaterial     : 1;
    bool                isSelection     : 1;
    bool                groupDrag       : 1;

    GLbitfield glSaveBits()
    {
        GLbitfield bits = 0;
        if (hasAttributes)
            bits |= (GL_LINE_BIT | GL_TEXTURE_BIT);
        if (hasLighting)
            bits |= GL_LIGHTING_BIT;
        return bits;
    }

protected:
    // List of drawing elements
    typedef std::vector<Drawing *>      layout_items;
    layout_items        items;

public:
    qevent_ids          refreshEvents;
    double              nextRefresh;
    Tree_p              body;
    Context_p           ctx;
    QByteArray          ctxHash;

public:
    // Static attributes for polygon offset computation
    static int          polygonOffset;
    static scale        factorBase, factorIncrement;
    static scale        unitBase, unitIncrement;
    static uint         globalProgramId;
};

TAO_END

#endif // LAYOUT_H
