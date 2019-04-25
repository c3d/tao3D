#ifndef LAYOUT_H
#define LAYOUT_H
// *****************************************************************************
// layout.h                                                        Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2011-2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2010-2011,2013, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2013, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
// (C) 2011-2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

#include "drawing.h"
#include "color.h"
#include "justification.h"
#include "tao_gl.h"
#include "application.h"
#include "texture_cache.h"
#include "matrix.h"
#include "opengl_state.h"
#include <vector>
#include <set>
#include <QFont>
#include <QEvent>
#include <float.h>
#include <recorder/recorder.h>

RECORDER_DECLARE(layoutevents);


TAO_BEGIN
struct Widget;
struct Layout;

struct LayoutState
// ----------------------------------------------------------------------------
//   The state we want to preserve in a layout
// ----------------------------------------------------------------------------
{
                        LayoutState();
                        LayoutState(const LayoutState &o);

public:
    typedef std::set<int>                       qevent_ids;

public:
    void                ClearAttributes();
    static text         ToText(qevent_ids & ids);
    static text         ToText(int type);
    void                InheritState(LayoutState *other);
    void                toDebugString(std::ostream &out) const;

public:
    Vector3             offset;
    QFont               font;
    Justification       alongX, alongY, alongZ;
    coord               left, right, top, bottom; // Margins
    scale               visibility;
    scale               extrudeDepth;
    scale               extrudeRadius;
    int                 extrudeCount;
    scale               lineWidth;
    Color               lineColor;
    Color               fillColor;

    // Lighting states
    uint                lightId;
    bool                perPixelLighting;
    uint                programId;

    // Transformations
    Matrix4             model;
    bool                groupDrag       : 1;
    bool                transparency    : 1;
    bool                blendOrShade    : 1;
};


typedef std::vector<Layout *> Layouts;
typedef std::set<text>        LayoutNames;

struct Layout : Drawing, LayoutState
// ----------------------------------------------------------------------------
//   A layout is responsible for laying out Drawing objects in 2D or 3D space
// ----------------------------------------------------------------------------
{
public:
                        Layout(Widget *display);
                        Layout(const Layout &o);
                        ~Layout();

    // Drawing interface
    virtual void        Draw(Layout *where);
    virtual void        DrawSelection(Layout *);
    virtual void        Identify(Layout *);
    virtual Box3        Bounds(Layout *);
    virtual Box3        Space(Layout *);
    virtual bool        Paginate(PageLayout *page);

    // Layout interface
    virtual void        Add (Drawing *d);
    virtual Vector3     Offset();
    virtual Layout *    NewChild()       { return new Layout(*this); }
    virtual Layout *    AddChild(uint id = 0,
                                 Tree_p body = 0, Scope_p scope = 0,
                                 Layout *child = NULL);
    virtual void        Clear();
    virtual void        ClearCaches();
    virtual Widget *    Display()        { return display; }
    virtual void        PolygonOffset(bool willDraw);
    virtual void        ClearPolygonOffset();
    virtual uint        Selected();
    virtual uint        ChildrenSelected();

    // Event interface
    bool                Refresh(QEvent *e, double now,
                                Layout *parent=NULL, QString dbg = "");
    void                ChildLayouts(Layouts &layouts, bool recurse = false);
    bool                RefreshChildren(QEvent *e, double now, QString debug);
    bool                NeedRefresh(QEvent *e, double when);
    bool                RefreshOn(Layout *);
    bool                RefreshOnUp(Layout *);
    bool                RefreshOn(int type, double when = DBL_MAX);
    void                NoRefreshOn(int type);
    qevent_ids          RefreshEvents();
    qevent_ids          RefreshEventsUp();
    double              NextRefresh();
    void                AddName(text name);
    void                AddDep(text name);
    void                CheckRefreshDeps();

    LayoutState &       operator=(const LayoutState &o);
    virtual void        Inherit(Layout *other);
    void                PushLayout();
    void                PopLayout();
    uint                CharacterId();
    double              PrinterScaling();
    text                PrettyId();

    void                CachesInfoFrom(Drawing *d)      { caches.push_back(d); }

    // Used to optimize away texturing and programs if in Identify
    static bool         InIdentify()    { return inIdentify; }

public:
    // OpenGL identification for that shape and for characters within
    uint                id;
    uint                charId;

protected:
    Layout *            parent;
    // List of drawing elements
    Drawings            items;
    Widget *            display;
    // Debug: index in parent items (-1 = root layout)
    int                 idx;
    Drawings            caches;  // Drawings that may reference us
    friend struct TextFlow;
    // Refresh dependencies
    LayoutNames         names; // Name(s) of this layout
    LayoutNames         deps;  // Layout(s) to refresh with this one

public:
    qevent_ids          refreshEvents;
    double              nextRefresh;
    Tree_p              body;
    Scope_p             scope;

public:
    // Static attributes for polygon offset computation
    static int          polygonOffset;
    static scale        factorBase, factorIncrement;
    static scale        unitBase, unitIncrement;
    static bool         inIdentify;
};


struct StereoLayout : Layout
// ----------------------------------------------------------------------------
//   A layout that activates only for some viewpoint
// ----------------------------------------------------------------------------
{
    StereoLayout(const Layout &o, ulong viewpoints = ~0UL)
        : Layout(o), viewpoints(viewpoints) {}

    virtual void        Draw(Layout *where);
    virtual void        DrawSelection(Layout *where);
    bool                Valid(Layout *where);

    ulong       viewpoints;
};


TAO_END

RECORDER_DECLARE(layoutevents);

#endif // LAYOUT_H
