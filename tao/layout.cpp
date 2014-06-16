// ****************************************************************************
//  layout.cpp                                                      Tao project
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

#include "layout.h"
#include "gl_keepers.h"
#include "attributes.h"
#include "tao_tree.h"
#include "tao_utf8.h"
#include "preferences_pages.h"
#include <sstream>
#include "demangle.h"

TAO_BEGIN

int   Layout::polygonOffset   = 0;
scale Layout::factorBase      = 0;
scale Layout::factorIncrement = -0.005; // Experimental value
scale Layout::unitBase        = 0;
scale Layout::unitIncrement   = -1;
bool  Layout::inIdentify      = false;


LayoutState::LayoutState()
// ----------------------------------------------------------------------------
//   Default state for all layouts
// ----------------------------------------------------------------------------
    : offset(),
      font(qApp->font()),
      alongX(), alongY(), alongZ(),
      left(0), right(0), top(0), bottom(0),
      visibility(1),
      extrudeDepth(0),
      extrudeRadius(0),
      extrudeCount(1),
      lineWidth(1.0),
      lineColor(0,0,0,0),       // Transparent black
      fillColor(0,0,0,1),       // Black
      lightId(GL_LIGHT0),
      perPixelLighting(TaoApp->useShaderLighting),
      programId(0),
      groupDrag(false),
      transparency(false),
      blendOrShade(false)
{}


LayoutState::LayoutState(const LayoutState &o)
// ----------------------------------------------------------------------------
//   Copy state (may be used between layouts)
// ----------------------------------------------------------------------------
    : offset(o.offset),
      font(o.font),
      alongX(o.alongX), alongY(o.alongY), alongZ(o.alongZ),
      left(o.left), right(o.right), top(o.top), bottom(o.bottom),
      visibility(o.visibility),
      extrudeDepth(o.extrudeDepth),
      extrudeRadius(o.extrudeRadius),
      extrudeCount(o.extrudeCount),
      lineWidth(o.lineWidth),
      lineColor(o.lineColor),
      fillColor(o.fillColor),
      lightId(o.lightId),
      perPixelLighting(o.perPixelLighting),
      programId(o.programId),
      model(o.model),
      groupDrag(false),
      transparency(false),
      blendOrShade(false)
{}


void LayoutState::ClearAttributes()
// ----------------------------------------------------------------------------
//   Reset default state for a layout
// ----------------------------------------------------------------------------
{
    LayoutState zero;
    *this = zero;
}


text LayoutState::ToText(int type)
// ----------------------------------------------------------------------------
//   Helper function to display an event type in human-readable form
// ----------------------------------------------------------------------------
{
#define CASE(x) case QEvent::x: return #x
    switch (type)
    {
        CASE(Timer);
        CASE(MouseButtonPress);
        CASE(MouseButtonRelease);
        CASE(MouseButtonDblClick);
        CASE(MouseMove);
        CASE(KeyPress);
        CASE(KeyRelease);
        CASE(Resize);
        default: break;
    }
#undef CASE
    std::ostringstream ostr;
    ostr << type;
    return ostr.str();
}


text LayoutState::ToText(qevent_ids & ids)
// ----------------------------------------------------------------------------
//   Helper function to dump a set of QEvent identifiers
// ----------------------------------------------------------------------------
{
    std::string ret = "[]";
    std::ostringstream ostr;
    qevent_ids::iterator i;
    for (i = ids.begin(); i != ids.end(); i++)
        ostr << " " << ToText(*i);
    if (ostr.str().size())
        ret = "[" + ostr.str() + " ]";

    return ret;
}


Layout::Layout(Widget *widget)
// ----------------------------------------------------------------------------
//    Create an empty layout
// ----------------------------------------------------------------------------
    : Drawing(),
      LayoutState(widget->layout ? *widget->layout : LayoutState()),
      id(0), charId(0), parent(NULL),
      items(), display(widget), idx(-1),
      refreshEvents(), nextRefresh(DBL_MAX), lastRefresh(0)
{
    IFTRACE(justify)
        std::cerr << "<-> Layout::Layout ["<< this << "] parent widget is "
                  << widget << " layout is "
                  << widget->layout <<"\n";
}


Layout::Layout(const Layout &o)
// ----------------------------------------------------------------------------
//   Copy constructor
// ----------------------------------------------------------------------------
    : Drawing(o), LayoutState(o), id(0), charId(0), parent(NULL),
      items(), display(o.display), idx(-1),
      refreshEvents(), nextRefresh(DBL_MAX), lastRefresh(o.lastRefresh)
{
    IFTRACE(justify)
        std::cerr << "<-> Layout::Layout ["<< this << "] parent layout is "
                  << &o <<"\n";
}


Layout::~Layout()
// ----------------------------------------------------------------------------
//   Destroy a layout
// ----------------------------------------------------------------------------
{
    IFTRACE(justify)
        std::cerr << "-> Layout::~Layout ["<< this << "] \n";
    Clear();
    IFTRACE(justify)
        std::cerr << "<- Layout::~Layout ["<< this << "] \n";
}


Layout *Layout::AddChild(uint childId,
                         Tree_p body, Context_p ctx,
                         Layout *child)
// ----------------------------------------------------------------------------
//   Add a new layout as a child of this one
// ----------------------------------------------------------------------------
{
    IFTRACE(layoutevents)
        std::cerr << "Adding child id " << childId << " to " << id
                  << " with body: " << body << "\n";

    Layout *result = child ? child : NewChild();
    Add(result);
    result->idx = items.size();
    result->id = childId;
    result->body = body;
    result->ctx = ctx;
    result->lastRefresh = lastRefresh;
    return result;
}


void Layout::Clear()
// ----------------------------------------------------------------------------
//   Reset the layout (mostly) to the initial setup
// ----------------------------------------------------------------------------
{
    IFTRACE(justify)
        std::cerr << "-> Layout::Clear ["<< this << "] \n";

    // Clear caches before
    ClearCaches();

    // When clearing, we should have no cached information left anywhere
    assert(caches.size() == 0);
    
    // Remove items now that they are no longer referenced elsewhere
    for (Drawings::iterator i = items.begin(); i != items.end(); i++)
    {
        IFTRACE(justify)
            std::cerr << "-- Layout::Clear ["<< this << "] delete drawing "
                      << (void*) *i << " of type "
                      << demangle(typeid(**i).name()) << std::endl;
        delete *i;
    }
    items.clear();

    // Initial state has no rotation or attribute changes
    ClearAttributes();

    refreshEvents.clear();
    nextRefresh = DBL_MAX;
    // lastRefresh is NOT reset on purpose
    IFTRACE(justify)
        std::cerr << "<- Layout::Clear ["<< this << "] \n";
}


void Layout::ClearCaches()
// ----------------------------------------------------------------------------
//   Clear all cached information related to this layout
// ----------------------------------------------------------------------------
{
    // Clear all pages we may have sent data to
    IFTRACE(justify)
        std::cerr << "Layout::ClearCaches ["<< this << "]\n";

    // Remove cached references
    Drawings ch = caches;
    caches.clear();
    for (Drawings::iterator d = ch.begin(); d != ch.end(); d++)
    {
        IFTRACE(justify)
            std::cerr << "[" << this << "]  cache "
                      << demangle(typeid(**d).name())
                      << "@" << (void*) *d << "\n";
        (*d)->ClearCaches();
    }

    // Remove cached information from children
    for (Drawings::iterator d = items.begin(); d != items.end(); d++)
    {
        IFTRACE(justify)
            std::cerr << "[" << this << "]  child "
                      << demangle(typeid(**d).name())
                      << "@" << (void*) *d << "\n";
        (*d)->ClearCaches();
    }
}


void Layout::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the elements in the layout
// ----------------------------------------------------------------------------
{
    if (true)
    {
        // Inherit offset from our parent layout if there is one
        XL::Save<Point3> save(offset, offset);
        GLAllStateKeeper glSave;
        Inherit(where);
        
        // Display all items
        PushLayout();
        for (Drawings::iterator i = items.begin(); i != items.end(); i++)
        {
            Drawing *child = *i;
            child->Draw(this);
        }
        PopLayout();
    }

    // Two passes for transparency, see #2199
    if (!where && !transparency)
    {
        ClearPolygonOffset();
        ClearAttributes();
        GL.DepthMask(GL_FALSE);
        transparency = true;
        Draw(NULL);
        GL.DepthMask(GL_TRUE);
        transparency = false;
    }
}


void Layout::DrawSelection(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the selection for the elements in the layout
// ----------------------------------------------------------------------------
{
    if (true)
    {
        // Inherit offset from our parent layout if there is one
        XL::Save<Point3> save(offset, offset);
        GLAllStateKeeper glSave;
        Inherit(where);
        
        PushLayout();
        for (Drawings::iterator i = items.begin(); i != items.end(); i++)
        {
            Drawing *child = *i;
            child->DrawSelection(this);
        }
        PopLayout();
    }

    // Two passes for transparency, see #2199
    if (!where && !transparency)
    {
        ClearPolygonOffset();
        ClearAttributes();
        GL.DepthMask(GL_FALSE);
        transparency = true;
        DrawSelection(NULL);
        GL.DepthMask(GL_TRUE);
        transparency = false;
    }
}


void Layout::Identify(Layout *where)
// ----------------------------------------------------------------------------
//   Identify the elements of the layout for OpenGL selection
// ----------------------------------------------------------------------------
{
    if (true)
    {
        // Remember that we are in Identify mode
        XL::Save<bool> saveIdenitfy(inIdentify, true);

        // Inherit offset from our parent layout if there is one
        XL::Save<Point3> save(offset, offset);
        GLAllStateKeeper glSave;
        Inherit(where);

        PushLayout();
        for (Drawings::iterator i = items.begin(); i != items.end(); i++)
        {
            Drawing *child = *i;
            child->Identify(this);
        }
        PopLayout();
    }

    // Two passes for transparency, see #2199
    if (!where && !transparency)
    {
        ClearPolygonOffset();
        ClearAttributes();
        GL.DepthMask(GL_FALSE);
        transparency = true;
        Identify(NULL);
        GL.DepthMask(GL_TRUE);
        transparency = false;
    }
}


Vector3 Layout::Offset()
// ----------------------------------------------------------------------------
//   Return the offset at which the layout expects us to draw
// ----------------------------------------------------------------------------
{
    return offset;
}


Box3 Layout::Bounds(Layout *layout)
// ----------------------------------------------------------------------------
//   Compute the bounding box as the union of all item bounds
// ----------------------------------------------------------------------------
{
    Box3 result;
    Inherit(layout);
    if (items.size() == 0)
        result |= Point3();
    else
        for (Drawings::iterator i = items.begin(); i != items.end(); i++)
            result |= (*i)->Bounds(this);
    return result;
}


Box3 Layout::Space(Layout *layout)
// ----------------------------------------------------------------------------
//   Compute the required space as the union of all item bounds
// ----------------------------------------------------------------------------
{
    Box3 result;
    Inherit(layout);
    if (items.size() == 0)
        result |= Point3();
    else
        for (Drawings::iterator i = items.begin(); i != items.end(); i++)
            result |= (*i)->Space(this);
    return result;
}


bool Layout::Paginate(PageLayout *page)
// ----------------------------------------------------------------------------
//   Paginate all the items in the given page
// ----------------------------------------------------------------------------
{
    bool ok = true;

    // Layouts are the basic unit for refresh. Since this layout may
    // be placed in a text flow and refresh independently from the page,
    // we need each to refresh when the other does. This causes
    // the page to re-compute its entire layout if we get refreshed.
    this->CachesInfoFrom(page);
    page->CachesInfoFrom(this);

    for (Drawings::iterator i = items.begin(); ok && i != items.end(); i++)
        ok = (*i)->Paginate(page);
    return ok;
}


void Layout::Add(Drawing *d)
// ----------------------------------------------------------------------------
//   Add a drawing to the items, return true if item fits in layout
// ----------------------------------------------------------------------------
{
    IFTRACE(justify)
        std::cerr << "<->Layout::Add[" << this << "] drawing is "
                  << d << ":"
                  << demangle(typeid(*d).name()) << std::endl;

    items.push_back(d);
    Layout * child = dynamic_cast<Layout *>(d);
    if (child)
        child->parent = this;
    d->Evaluate(this);
}


void Layout::PolygonOffset(bool willDraw)
// ----------------------------------------------------------------------------
//   Compute a polygon offset for the next shape being drawn
// ----------------------------------------------------------------------------
{
    int offset = polygonOffset++;
    if (willDraw)
        GL.PolygonOffset(factorBase + offset * factorIncrement,
                         unitBase + offset * unitIncrement);
}


void Layout::ClearPolygonOffset()
// ----------------------------------------------------------------------------
//   Clear the polygon offset, e.g. for 3D shapes
// ----------------------------------------------------------------------------
{
    polygonOffset = 0;
    GL.PolygonOffset(factorBase, unitBase);
}


uint Layout::ChildrenSelected()
// ----------------------------------------------------------------------------
//   The sum of chilren selections
// ----------------------------------------------------------------------------
{
    uint result = 0;
    for (Drawings::iterator i = items.begin(); i != items.end(); i++)
        if (Layout *l = dynamic_cast<Layout*>(*i))
            result += l->Selected();
    return result;
}


uint Layout::Selected()
// ----------------------------------------------------------------------------
//   Selection state of this layout plus the sum of chilren selections
// ----------------------------------------------------------------------------
{
    uint selected = Display()->selected(id);
    selected &= ~Widget::SELECTION_MASK;
    return selected + ChildrenSelected();
}


text Layout::PrettyId()
// ----------------------------------------------------------------------------
//   Helper function: return a human-readable id for the layout
// ----------------------------------------------------------------------------
{
    std::stringstream sstr;
    sstr << (void*)this << " id = " << id;
    if (body)
    {
        Tree *source = XL::xl_source(body);
        if (Prefix *p = source->AsPrefix())
        {
            if (p->left)
            {
                if (Name *n = p->left->AsName())
                {
                    sstr << "[" << n->value;
                    XL::TreePosition pos = source->Position();
                    if (pos != XL::Tree::NOWHERE)
                        sstr << "@" << pos ;
                    sstr << "]";
                }
            }
        }
    }
    return sstr.str();
}


bool Layout::Refresh(QEvent *e, double now, Layout *parent, QString dbg)
// ----------------------------------------------------------------------------
//   Re-compute layout on event, return true if self or child changed
// ----------------------------------------------------------------------------
{
    IFTRACE(justify)
        std::cerr << "->Layout::Refresh[" << this << "]  \n";
    bool changed = false;
    if (!e)
        return false;

    Widget * widget = Widget::Tao();
    if (widget->currentPrinter())
        return false;
    text layoutId;
    IFTRACE(layoutevents)
    {
        if (!dbg.isEmpty())
            dbg.append("/");
        dbg += QString("%1").arg(idx);
        layoutId = +dbg + " " + PrettyId();
    }

    if (NeedRefresh(e, now))
    {
        XL::Save<uint> saveId(widget->id, id);
        IFTRACE(layoutevents)
            std::cerr << "Layout " << layoutId
                      << " id " << id << " needs updating\n";

        refreshEvents.clear();
        nextRefresh = DBL_MAX;

        // Check if we can evaluate locally
        if (ctx && body)
        {
            IFTRACE(justify)
                std::cerr << "--Layout::Refresh[" << this << "] clears \n";

            // Clear old contents of the layout, drop all children
            Clear();

            // Inherit attributes
            Inherit(parent);

            // Set new layout as the current layout in the current Widget
            XL::Save<Layout *> saveLayout(widget->layout, this);
            GLAllStateKeeper save;

            IFTRACE(layoutevents)
                std::cerr << "Evaluating " << body << "\n";
            ctx->Evaluate(body);
        }
        else
        {
            if (this == (Layout*) widget->space)
                widget->refreshNow();
            else
                std::cerr << "Unexpected NULL ctx/body in non-root layout\n";
        }

        // Record date of last evaluation
        lastRefresh = now;

        changed = true;
    }
    else
    {
        IFTRACE(layoutevents)
        {
            std::cerr << "Layout " << layoutId << " does not need updating"
                      << " now=" << now
                      << " expires at ";
            if (nextRefresh == DBL_MAX)
                std::cerr << "(never)";
            else
                std::cerr << nextRefresh;
            std::cerr << "\n";
        }

        // Forward event to all child layouts
        changed |= RefreshChildren(e, now, dbg);
    }
    IFTRACE(justify)
        std::cerr << "<-Layout::Refresh[" << this << "]  \n";

    return changed;
}


void Layout::ChildLayouts(Layouts &out, bool recurse)
// ----------------------------------------------------------------------------
//   Copy all items that are layouts in the children
// ----------------------------------------------------------------------------
{
    for (Drawings::iterator i = items.begin(); i != items.end(); i++)
    {
        if (Layout *layout = dynamic_cast<Layout *> (*i))
        {
            out.push_back(layout);
            if (recurse)
                layout->ChildLayouts(out, true);
        }
    }
}


bool Layout::RefreshChildren(QEvent *e, double now, QString dbg)
// ----------------------------------------------------------------------------
//   Refresh all child layouts
// ----------------------------------------------------------------------------
{
    bool result = false;
    Layouts lyo;
    ChildLayouts(lyo);
    for (Layouts::iterator i = lyo.begin(); i != lyo.end(); i++)
        result |= (*i)->Refresh(e, now, this, dbg);
    return result;
}


LayoutState::qevent_ids Layout::RefreshEvents()
// ----------------------------------------------------------------------------
//   The set of all refresh events for this layout and its children
// ----------------------------------------------------------------------------
{
    LayoutState::qevent_ids events = refreshEvents;
    Layouts lyo;
    ChildLayouts(lyo);
    for (Layouts::iterator i = lyo.begin(); i != lyo.end(); i++)
    {
        LayoutState::qevent_ids evt = (*i)->RefreshEvents();
        events.insert(evt.begin(), evt.end());
    }
    return events;
}


double Layout::NextRefresh()
// ----------------------------------------------------------------------------
//   The date of next refresh for this layout or any of its children
// ----------------------------------------------------------------------------
{
    double next = nextRefresh;
    Layouts lyo;
    ChildLayouts(lyo);
    for (Layouts::iterator i = lyo.begin(); i != lyo.end(); i++)
        next = qMin(next, (*i)->NextRefresh());
    return next;
}


bool Layout::NeedRefresh(QEvent *e, double when)
// ----------------------------------------------------------------------------
//   Return true if event/time should trigger refresh of current layout
// ----------------------------------------------------------------------------
{
    QEvent::Type type = e->type();
    if (type != QEvent::Timer && refreshEvents.count(type))
        return true;
    // /!\ Don't check for (type == QEvent::Timer) below! If nextRefresh is
    // expired, then no matter which event is being processed, we need to
    // refresh. Otherwise the following test case would not run smooth when
    // moving the mouse:
    //   locally { translatex 100*sin time ; rectangle 0, 0, 150, 100 }
    //   locally { circle mouse_x, mouse_y, 50 }
    if (nextRefresh != DBL_MAX && nextRefresh <= when)
        return true;
    return false;
}


bool Layout::RefreshOn(Layout *layout)
// ----------------------------------------------------------------------------
//   Refresh whenever other layout would refresh
// ----------------------------------------------------------------------------
{
    bool changed = false;

    qevent_ids::size_type prev = refreshEvents.size();
    refreshEvents.insert(layout->refreshEvents.begin(),
                         layout->refreshEvents.end());
    if (refreshEvents.size() != prev)
        changed = true;

    if (layout->nextRefresh < nextRefresh)
    {
        nextRefresh = layout->nextRefresh;
        changed = true;
    }

    IFTRACE(layoutevents)
    {
        if (changed)
            std::cerr << "Refresh events copied from " << (void*)layout
                      << " to " << (void*)this << "\n";
    }

    return changed;
}


bool Layout::RefreshOnUp(Layout *layout)
// ----------------------------------------------------------------------------
//   Refresh whenever other layout would refresh (even due to a parent refresh)
// ----------------------------------------------------------------------------
{
    bool changed = false;
    for (Layout * lyo = layout; lyo; lyo = lyo->parent)
        if (RefreshOn(lyo))
            changed = true;
    return changed;
}


bool Layout::RefreshOn(int type, double when)
// ----------------------------------------------------------------------------
//   Ask for refresh on specified event (and time if event is QEvent::Timer)
// ----------------------------------------------------------------------------
{
    bool changed = false;

    if (refreshEvents.count(type) == 0)
    {
        refreshEvents.insert(type);
        changed = true;
    }

    if (type == QEvent::Timer &&
        when < nextRefresh)
    {
        nextRefresh = when;
        changed = true;
    }

    return changed;
}


void Layout::NoRefreshOn(int type)
// ----------------------------------------------------------------------------
//   Layout should NOT be updated on specified event
// ----------------------------------------------------------------------------
{
    if (refreshEvents.count(type) != 0)
    {
        refreshEvents.erase(type);
        if (type == QEvent::Timer)
            nextRefresh = DBL_MAX;
    }
}


void Layout::AddName(text name)
// ----------------------------------------------------------------------------
//   Give a name to this layout
// ----------------------------------------------------------------------------
{
    IFTRACE(layoutevents)
        std::cerr << "Name " << name << " is layout " << PrettyId() << "\n";
    names.insert(name);
}


void Layout::AddDep(text name)
// ----------------------------------------------------------------------------
//   Layout 'name' shall be refreshed when this one is refreshed
// ----------------------------------------------------------------------------
{
    IFTRACE(layoutevents)
        std::cerr << "Layout " << PrettyId() << " shall refresh "
                  << name << " \n";
    deps.insert(name);
}


void Layout::CheckRefreshDeps()
// ----------------------------------------------------------------------------
//   Copy refresh info of children to named dependent layouts (refresh_also)
// ----------------------------------------------------------------------------
{
    // All named layouts.
    // One name can be shared by multiple layout, and one layout can
    // have several names.
    std::map<text, Layouts> byName;
    // All layouts that declared a dependency
    Layouts withDeps;

    Layouts lyo;
    lyo.push_back(this);
    ChildLayouts(lyo, true);
    for (Layouts::iterator i = lyo.begin(); i != lyo.end(); i++)
    {
        if ((*i)->deps.size())
            withDeps.push_back((*i));
        LayoutNames &nam = (*i)->names;
        for (LayoutNames::iterator j = nam.begin(); j != nam.end(); j++)
            byName[(*j)].push_back(*i);
    }

    bool changed;
    int pass = 0;
    do
    {
        // Loop until nothing changes to handle indirect dependencies
        changed = false;
        for (Layouts::iterator i = withDeps.begin(); i != withDeps.end(); i++)
        {
            Layout *from = (*i);
            LayoutNames &dep = from->deps;
            for (LayoutNames::iterator j = dep.begin(); j != dep.end(); j++)
            {
                if (byName.count((*j)))
                {
                    Layouts &targets = byName[(*j)];
                    for (Layouts::iterator k = targets.begin();
                         k != targets.end(); k++)
                    {
                        Layout *to = (*k);
                        IFTRACE(layoutevents)
                            std::cerr << "Refresh dependency: from {"
                                      << from->PrettyId()
                                      << "} to {" << to->PrettyId()
                                      << " name " << (*j) << "}\n";
                        if (to->RefreshOnUp(from))
                            changed = true;
                    }
                }
            }
        }
        pass++;
    } while (changed);

    IFTRACE(layoutevents)
        std::cerr << "Refresh dependencies resolved in " << pass
                  << " pass" << ((pass > 1) ? "es" : "") << " of "
                  << withDeps.size() << "/" << lyo.size() << "\n";
}


LayoutState & Layout::operator=(const LayoutState &o)
// ----------------------------------------------------------------------------
//   Restore a previously saved state
// ----------------------------------------------------------------------------
{
    *((LayoutState *) this) = o;
    return *this;
}


void Layout::Inherit(Layout *where)
// ----------------------------------------------------------------------------
//   Inherit state from some other layout
// ----------------------------------------------------------------------------
{
    if (!where)
        return;

    // Add offset of parent to the one we have
    offset = where->Offset();
    LayoutState::InheritState(where);
}


void LayoutState::InheritState(LayoutState *where)
// ----------------------------------------------------------------------------
//   Inherit state from some other layoutState except offset
// ----------------------------------------------------------------------------
{
    // Inherit color and other parameters as initial values
    // Note that these may really impact what gets rendered,
    // e.g. transparent colors may cause shapes to be drawn or not
    font             = where->font;
    alongX           = where->alongX;
    alongY           = where->alongY;
    alongZ           = where->alongZ;
    left             = where->left;
    right            = where->right;
    top              = where->top;
    bottom           = where->bottom;
    visibility       = where->visibility;
    extrudeDepth     = where->extrudeDepth;
    extrudeRadius    = where->extrudeRadius;
    extrudeCount     = where->extrudeCount;
    lineWidth        = where->lineWidth;
    lineColor        = where->lineColor;
    fillColor        = where->fillColor;

    lightId          = where->lightId;
    perPixelLighting = where->perPixelLighting;

    programId        = where->programId;

    model            = where->model;

    groupDrag        = where->groupDrag;
    transparency     = where->transparency;
    blendOrShade     = where->blendOrShade;
}


void LayoutState::toDebugString(std::ostream &out) const
// ----------------------------------------------------------------------------
//   Show debug information
// ----------------------------------------------------------------------------
{
    out << "LayoutState[" << this << "]\n";
    out << "\tfont            = " << font.toString().toStdString() << std::endl;
    out << "\tleft            = " << left << std::endl;
    out << "\tright           = " << right << std::endl;
    out << "\ttop             = " << top << std::endl;
    out << "\tbottom          = " << bottom << std::endl;
    out << "\tvisibility      = " << visibility << std::endl;
    out << "\extrudeDepth     = " << extrudeDepth << std::endl;
    out << "\textrudeRadius   = " << extrudeRadius << std::endl;
    out << "\textrudeCount    = " << extrudeCount << std::endl;
    out << "\tlineWidth       = " << lineWidth << std::endl;
    out << "\tlineColor       = " << lineColor << std::endl;
    out << "\tfillColor       = " << fillColor << std::endl;
    out << "\tlightId         = " << lightId << std::endl;
    out << "\tprogramId       = " << programId << std::endl;
}


void Layout::PushLayout()
// ----------------------------------------------------------------------------
//   Save away information required to maintain selection hierarchy
// ----------------------------------------------------------------------------
{
    // Check if the group was opened. If so, update OpenGL name
    if (id & Widget::SELECTION_MASK)
    {
        uint groupId = id;
        Widget *widget = Display();
        widget->selectionContainerPush();
        
        uint open = widget->selected(id);
        if ((open & Widget::SELECTION_MASK) == Widget::CONTAINER_OPENED)
            groupId = (groupId & ~Widget::SELECTION_MASK)
                | Widget::CONTAINER_OPENED;
        GL.PushName(groupId);
    }
}


void Layout::PopLayout()
// ----------------------------------------------------------------------------
//   Restore information required to maintain selection hierarchy
// ----------------------------------------------------------------------------
{
    if (id & Widget::SELECTION_MASK)
    {
        Widget *widget = Display();
        widget->selectionContainerPop();
        GL.PopName();
    }
}


uint Layout::CharacterId()
// ----------------------------------------------------------------------------
//    Allocate a character ID
// ----------------------------------------------------------------------------
//    We also increment the widget's selection ID so that we account
//    for the right number of selectable items
{
    display->shapeId();
    return ++charId;
}


double Layout::PrinterScaling()
// ----------------------------------------------------------------------------
//   Return the scaling for printing
// ----------------------------------------------------------------------------
{
    return display->printerScaling();
}



// ============================================================================
//
//    Stereo layout
//
// ============================================================================

bool StereoLayout::Valid(Layout *where)
// ----------------------------------------------------------------------------
//    Return true if the view is to be shown
// ----------------------------------------------------------------------------
{
    Widget *widget = where->Display();
    ulong plane = widget->stereoPlane;
    ulong planes = widget->stereoPlanes;
    if (widget->eyeDistance < 0)
        plane = planes-1 - plane;
    return (1<<plane) & viewpoints;
}


void StereoLayout::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw only when the eye matches the given viewpoints
// ----------------------------------------------------------------------------
{
    if (Valid(where))
        Layout::Draw(where);
}


void StereoLayout::DrawSelection(Layout *where)
// ----------------------------------------------------------------------------
//   Draw selection only when the eye matches the given viewpoints
// ----------------------------------------------------------------------------
{
    if (Valid(where))
        Layout::DrawSelection(where);
}

TAO_END
