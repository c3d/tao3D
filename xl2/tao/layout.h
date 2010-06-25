#ifndef LAYOUT_H
#define LAYOUT_H
// ****************************************************************************
//  layout.h                                                        Tao project
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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "drawing.h"
#include "color.h"
#include "justification.h"
#include <vector>
#include <QFont>


TAO_BEGIN

struct Widget;

struct LayoutState
// ----------------------------------------------------------------------------
//   The state we want to preserve in a layout
// ----------------------------------------------------------------------------
{
                        LayoutState();
                        LayoutState(const LayoutState &o);

public:
    void                Clear();

public:
    Vector3             offset;
    QFont               font;
    Justification       alongX, alongY, alongZ;
    coord               left, right, top, bottom; // Margins
    scale               lineWidth;
    Color               lineColor;
    Color               fillColor;
    uint                fillTexture;
    bool                wrapS : 1;                // Texture wrapping
    bool                wrapT : 1;
    bool                printing : 1;
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
    virtual Layout *    AddChild(uint id = 0);
    virtual void        Clear();
    virtual Widget *    Display()        { return display; }
    virtual void        PolygonOffset();
    virtual uint        Selected();
    virtual uint        ChildrenSelected();

    LayoutState &       operator=(const LayoutState &o);
    void                Inherit(Layout *other);

public:
    // OpenGL identification for that shape
    uint                id;

    // For optimized drawing, we keep track of what changes
    bool                hasPixelBlur    : 1; // Pixels not aligning naturally
    bool                hasMatrix       : 1;
    bool                hasAttributes   : 1;
    bool                hasTextureMatrix: 1;
    bool                isSelection     : 1;

protected:
    // List of drawing elements
    typedef std::vector<Drawing *>      layout_items;
    layout_items        items;
    Widget *            display;

public:
    // Static attributes for polygon offset computation
    static int          polygonOffset;
    static scale        factorBase, factorIncrement;
    static scale        unitBase, unitIncrement;
};

TAO_END

#endif // LAYOUT_H
