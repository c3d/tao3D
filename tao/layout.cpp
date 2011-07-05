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
#include <sstream>

TAO_BEGIN

int   Layout::polygonOffset   = 0;
scale Layout::factorBase      = 0;
scale Layout::factorIncrement = -0.01; // Experimental value
scale Layout::unitBase        = 0;
scale Layout::unitIncrement   = -1;
uint  Layout::globalProgramId = 0;


LayoutState::LayoutState()
// ----------------------------------------------------------------------------
//   Default state for all layouts
// ----------------------------------------------------------------------------
    : offset(),
      font(qApp->font()),
      alongX(), alongY(), alongZ(),
      left(0), right(0), top(0), bottom(0),
      visibility(1),
      lineWidth(1.0),
      lineColor(0,0,0,0),       // Transparent black
      fillColor(0,0,0,1),       // Black
      fillTexture(0), lightId(GL_LIGHT0), programId(0),
      wrapS(false), wrapT(false), printing(false),
      planarRotation(0), planarScale(1),
      rotationId(0), translationId(0), scaleId(0)
{}


LayoutState::LayoutState(const LayoutState &o)
// ----------------------------------------------------------------------------
//   Copy state (may be used between layouts)
// ----------------------------------------------------------------------------
      : offset(o.offset),
        font(o.font),
        alongX(o.alongX), alongY(o.alongY), alongZ(o.alongZ),
        left(o.left), right(o.right),
        top(o.top), bottom(o.bottom),
        visibility(o.visibility),
        lineWidth(o.lineWidth),
        lineColor(o.lineColor),
        fillColor(o.fillColor),
        fillTexture(o.fillTexture),
        lightId(o.lightId),
        programId(o.programId),
        wrapS(o.wrapS),
        wrapT(o.wrapT),
        printing(o.printing),
        planarRotation(o.planarRotation),
        planarScale(o.planarScale),
        rotationId(o.rotationId),
        translationId(o.translationId),
        scaleId(o.scaleId)
{}


void LayoutState::ClearAttributes()
// ----------------------------------------------------------------------------
//   Reset default state for a layout
// ----------------------------------------------------------------------------
{
    LayoutState zero;
    *this = zero;
}


text LayoutState::ToText(QEvent::Type type)
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
    : Drawing(), LayoutState(), id(0), charId(0),
      hasPixelBlur(false), hasMatrix(false), has3D(false),
      hasAttributes(false), hasTextureMatrix(false),
      hasLighting(false), hasMaterial(false),
      isSelection(false), groupDrag(false),
      items(), display(widget), idx(-1),
      refreshEvents(), nextRefresh(DBL_MAX)
{}


Layout::Layout(const Layout &o)
// ----------------------------------------------------------------------------
//   Copy constructor
// ----------------------------------------------------------------------------
    : Drawing(o), LayoutState(o), id(0), charId(0),
      hasPixelBlur(o.hasPixelBlur), hasMatrix(false), has3D(o.has3D),
      hasAttributes(false), hasTextureMatrix(false),
      hasLighting(false), hasMaterial(false),
      isSelection(o.isSelection), groupDrag(false),
      items(), display(o.display), idx(-1),
      refreshEvents(), nextRefresh(DBL_MAX)
{}


Layout::~Layout()
// ----------------------------------------------------------------------------
//   Destroy a layout
// ----------------------------------------------------------------------------
{
    Clear();
}


Layout *Layout::AddChild(uint childId,
                         Tree_p body, Context_p ctx,
                         Layout *child)
// ----------------------------------------------------------------------------
//   Add a new layout as a child of this one
// ----------------------------------------------------------------------------
{
    Layout *result = child ? child : NewChild();
    Add(result);
    result->idx = items.size();
    result->id = childId;
    result->body = body;
    result->ctx = ctx;
    if (ctx && XL::MAIN->options.enable_layout_cache)
        result->ctxHash = LayoutCache::contextHash(ctx);
    return result;
}


void Layout::Clear()
// ----------------------------------------------------------------------------
//   Reset the layout to the initial setup
// ----------------------------------------------------------------------------
{
    for (Drawings::iterator i = items.begin(); i != items.end(); i++)
        delete *i;
    items.clear();

    // Initial state has no rotation or attribute changes
    hasPixelBlur = false;
    hasMatrix = false;
    has3D = false;
    hasAttributes = false;

    ClearAttributes();

    refreshEvents.clear();
    nextRefresh = DBL_MAX;
}


void Layout::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the elements in the layout
// ----------------------------------------------------------------------------
{
    // Inherit offset from our parent layout if there is one
    XL::Save<Point3> save(offset, offset);
    GLAllStateKeeper glSave(glSaveBits(),
                            hasMatrix, false, hasTextureMatrix);
    Inherit(where);

    // Display all items
    PushLayout(this);
    for (Drawings::iterator i = items.begin(); i != items.end(); i++)
    {
        Drawing *child = *i;
        child->Draw(this);
    }
    PopLayout(this);
}


void Layout::DrawSelection(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the selection for the elements in the layout
// ----------------------------------------------------------------------------
{
    // Inherit offset from our parent layout if there is one
    XL::Save<Point3> save(offset, offset);
    GLAllStateKeeper glSave(glSaveBits(),
                            hasMatrix, false, hasTextureMatrix);
    Inherit(where);

    PushLayout(this);
    for (Drawings::iterator i = items.begin(); i != items.end(); i++)
    {
        Drawing *child = *i;
        child->DrawSelection(this);
    }
    PopLayout(this);
}


void Layout::Identify(Layout *where)
// ----------------------------------------------------------------------------
//   Identify the elements of the layout for OpenGL selection
// ----------------------------------------------------------------------------
{
    // Inherit offset from our parent layout if there is one
    XL::Save<Point3> save(offset, offset);
    GLAllStateKeeper glSave(glSaveBits(),
                            hasMatrix, false, hasTextureMatrix);
    Inherit(where);


    PushLayout(this);
    for (Drawings::iterator i = items.begin(); i != items.end(); i++)
    {
        Drawing *child = *i;
        child->Identify(this);
    }
    PopLayout(this);
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


void Layout::Add(Drawing *d)
// ----------------------------------------------------------------------------
//   Add a drawing to the items, return true if item fits in layout
// ----------------------------------------------------------------------------
{
    items.push_back(d);
    if (d->IsAttribute())
        d->Draw(this);
}


void Layout::PolygonOffset()
// ----------------------------------------------------------------------------
//   Compute a polygon offset for the next shape being drawn
// ----------------------------------------------------------------------------
{
    int offset = polygonOffset++;
    glPolygonOffset (factorBase + offset * factorIncrement,
                     unitBase + offset * unitIncrement);
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
    selected &= Widget::SELECTION_MASK;
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
    bool changed = false;
    if (!e)
        return false;

    Widget * widget = Widget::Tao();
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
        IFTRACE(layoutevents)
            std::cerr << "Layout " << layoutId << " needs updating\n";

        refreshEvents.clear();
        nextRefresh = DBL_MAX;

        if (XL::MAIN->options.enable_layout_cache)
        {
            // Drop children: delete drawings but set aside layouts, in case
            // they can be reused by the evaluation step below
            for (Drawings::iterator it = items.begin(); it != items.end(); it++)
            {
                if (Layout * layout = dynamic_cast<Layout*>(*it))
                    widget->layoutCache.insert(layout);
                else
                    delete (*it);
            }
            items.clear();
        }

        // Check if we can evaluate locally
        if (ctx && body)
        {
            // Clear old contents of the layout, drop all children
            Clear();

            // Set new layout as the current layout in the current Widget
            XL::Save<Layout *> saveLayout(widget->layout, this);

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

        changed = true;
    }
    else
    {
        IFTRACE(layoutevents)
            std::cerr << "Layout " << layoutId << " does not need updating\n";
    }

    // Forward event to all child layouts
    changed |= RefreshChildren(e, now, dbg);

    // When done with topmost layout we can clear cache
    if (XL::MAIN->options.enable_layout_cache && !parent)
        widget->layoutCache.clear();

    return changed;
}


void Layout::RefreshLayouts(Layouts &out)
// ----------------------------------------------------------------------------
//   Copy all items that are layouts in the children
// ----------------------------------------------------------------------------
{
    for (Drawings::iterator i = items.begin(); i != items.end(); i++)
        if (Layout *layout = dynamic_cast<Layout *> (*i))
            out.push_back(layout);
}


bool Layout::RefreshChildren(QEvent *e, double now, QString dbg)
// ----------------------------------------------------------------------------
//   Refresh all child layouts
// ----------------------------------------------------------------------------
{
    bool result = false;
    Layouts lyo;
    RefreshLayouts(lyo);
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
    RefreshLayouts(lyo);
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
    RefreshLayouts(lyo);
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
    if (nextRefresh != DBL_MAX && nextRefresh <= when)
        return true;
    return false;
}


void Layout::RefreshOn(Layout *layout)
// ----------------------------------------------------------------------------
//   Refresh whenever other layout would refresh
// ----------------------------------------------------------------------------
{
    IFTRACE(layoutevents)
        std::cerr << "Copying layout events from " << (void*)layout << " to "
                  << (void*)this << "\n";
    refreshEvents.insert(layout->refreshEvents.begin(),
                         layout->refreshEvents.end());
    if (layout->nextRefresh < nextRefresh)
        nextRefresh = layout->nextRefresh;
}


void Layout::RefreshOn(QEvent::Type type, double when)
// ----------------------------------------------------------------------------
//   Ask for refresh on specified event (and time if event is QEvent::Timer)
// ----------------------------------------------------------------------------
{
    if (refreshEvents.count(type) == 0)
        refreshEvents.insert(type);

    if (type == QEvent::Timer)
        if (when < nextRefresh)
            nextRefresh = when;
}


void Layout::NoRefreshOn(QEvent::Type type)
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
    // Reset the index of characters
    charId = 0;

    if (!where)
        return;

    // Add offset of parent to the one we have
    offset = where->Offset();
    LayoutState::InheritState(where);
    has3D           = where->has3D;
    hasPixelBlur    = where->hasPixelBlur;
    groupDrag       = where->groupDrag;
    hasMaterial     = where->hasMaterial;
}

void LayoutState::InheritState(LayoutState *where)
// ----------------------------------------------------------------------------
//   Inherit state from some other layoutState except offset
// ----------------------------------------------------------------------------
{
    // Inherit color and other parameters as initial values
    // Note that these may really impact what gets rendered,
    // e.g. transparent colors may cause shapes to be drawn or not
    font            = where->font;
    alongX          = where->alongX;
    alongY          = where->alongY;
    alongZ          = where->alongZ;
    left            = where->left;
    right           = where->right;
    top             = where->top;
    bottom          = where->bottom;
    visibility      = where->visibility;
    lineWidth       = where->lineWidth;
    lineColor       = where->lineColor;
    fillColor       = where->fillColor;
    fillTexture     = where->fillTexture;
    lightId         = where->lightId;
    programId       = where->programId;
    wrapS           = where->wrapS;
    wrapT           = where->wrapT;
    printing        = where->printing;
    planarRotation  = where->planarRotation;
    planarScale     = where->planarScale;
}

void LayoutState::toDebugString(std::ostream &out)
{
    out << "LayoutState["<<this<<"]\n";
    out << "\tfont            = " << font.toString().toStdString() << std::endl;
//    out << "\talongX          = " << alongX << std::endl;
//    out << "\talongY          = " << alongY << std::endl;
//    out << "\talongZ          = " << alongZ << std::endl;
    out << "\tleft            = " << left << std::endl;
    out << "\tright           = " << right << std::endl;
    out << "\ttop             = " << top << std::endl;
    out << "\tbottom          = " << bottom << std::endl;
    out << "\tvisibility      = " << visibility << std::endl;
    out << "\tlineWidth       = " << lineWidth << std::endl;
    out << "\tlineColor       = " << lineColor << std::endl;
    out << "\tfillColor       = " << fillColor << std::endl;
    out << "\tfillTexture     = " << fillTexture << std::endl;
    out << "\tlightId         = " << lightId << std::endl;
    out << "\tprogramId       = " << programId << std::endl;
    out << "\twrapS           = " << wrapS << std::endl;
    out << "\twrapT           = " << wrapT << std::endl;
    out << "\tprinting        = " << printing << std::endl;
    out << "\tplanarRotation  = " << planarRotation << std::endl;
    out << "\tplanarScale     = " << planarScale << std::endl;

}

void Layout::PushLayout(Layout *where)
// ----------------------------------------------------------------------------
//   Save away information required to maintain selection hierarchy
// ----------------------------------------------------------------------------
{
    // Check if the group was opened. If so, update OpenGL name
    if (uint groupId = id)
    {
        Widget *widget = where->Display();
        widget->selectionContainerPush();

        uint open = widget->selected(id);
        if (open & Widget::CONTAINER_OPENED)
            groupId |= Widget::CONTAINER_OPENED;
        glPushName(groupId);
    }
}


void Layout::PopLayout(Layout *where)
// ----------------------------------------------------------------------------
//   Restore information required to maintain selection hierarchy
// ----------------------------------------------------------------------------
{
    if (id)
    {
        Widget *widget = where->Display();
        widget->selectionContainerPop();
        glPopName();
    }
}


uint Layout::CharacterId()
// ----------------------------------------------------------------------------
//    Allocate a character ID
// ----------------------------------------------------------------------------
//    We also increment the widget's selection ID so that we account
//    for the right number of selectable items
{
    display->selectionId();
    return ++charId;
}


double Layout::PrinterScaling()
// ----------------------------------------------------------------------------
//   Return the scaling for printing
// ----------------------------------------------------------------------------
{
    return display->printerScaling();
}

TAO_END
