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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "layout.h"
#include "gl_keepers.h"
#include "attributes.h"
#include "tao_tree.h"
#include <sstream>

TAO_BEGIN

int   Layout::polygonOffset   = 0;
scale Layout::factorBase      = 0;
scale Layout::factorIncrement = -0.01; // Experimental value
scale Layout::unitBase        = 0;
scale Layout::unitIncrement   = -1;


LayoutState::LayoutState()
// ----------------------------------------------------------------------------
//   Default state for all layouts
// ----------------------------------------------------------------------------
    : offset(),
      font(qApp->font()),
      alongX(), alongY(), alongZ(),
      left(0), right(0), top(0), bottom(0),
      lineWidth(1.0),
      lineColor(0,0,0,0),       // Transparent black
      fillColor(0,0,0,1),       // Black
      fillTexture(0), lightId(GL_LIGHT0), programId(0),
      wrapS(false), wrapT(false), printing(false),
      planarRotation(0), planarScale(1),
      rotationId(0), translationId(0), scaleId(0),
      refreshEvents(), nextRefresh(DBL_MAX)
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
        scaleId(o.scaleId),
        refreshEvents(), nextRefresh(DBL_MAX)
{}



void LayoutState::Clear()
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
      isSelection(false), groupDrag(false),
      items(), display(widget)
{}


Layout::Layout(const Layout &o)
// ----------------------------------------------------------------------------
//   Copy constructor
// ----------------------------------------------------------------------------
    : Drawing(o), LayoutState(o), id(0), charId(0),
      hasPixelBlur(o.hasPixelBlur), hasMatrix(false), has3D(o.has3D),
      hasAttributes(false), hasTextureMatrix(false),
      isSelection(o.isSelection), groupDrag(false),
      items(), display(o.display)
{}


Layout::~Layout()
// ----------------------------------------------------------------------------
//   Destroy a layout
// ----------------------------------------------------------------------------
{
    Clear();
}


Layout *Layout::AddChild(uint childId, Tree_p self, Context_p ctx)
// ----------------------------------------------------------------------------
//   Add a new layout as a child of this one
// ----------------------------------------------------------------------------
{
    Layout *result = NewChild();
    Add(result);
    result->id = childId;
    result->self = self;
    result->ctx = ctx;
    if (ctx)
        result->ctxHash = LayoutCache::contextHash(ctx);
    return result;
}


void Layout::Clear()
// ----------------------------------------------------------------------------
//   Reset the layout to the initial setup
// ----------------------------------------------------------------------------
{
    layout_items::iterator i;
    for (i = items.begin(); i != items.end(); i++)
        delete *i;
    items.clear();

    // Initial state has no rotation or attribute changes
    hasPixelBlur = false;
    hasMatrix = false;
    has3D = false;
    hasAttributes = false;

    LayoutState::Clear();
}


// The bit values we save for a layout
static const GLbitfield GL_LAYOUT_BITS = GL_LINE_BIT | GL_TEXTURE_BIT;


void Layout::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the elements in the layout
// ----------------------------------------------------------------------------
{
    // Inherit offset from our parent layout if there is one
    XL::Save<Point3> save(offset, offset);
    GLAllStateKeeper glSave(hasAttributes?GL_LAYOUT_BITS:0,
                            hasMatrix, false, hasTextureMatrix);
    Inherit(where);

    // Display all items
    PushLayout(this);
    layout_items::iterator i;
    for (i = items.begin(); i != items.end(); i++)
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
    GLAllStateKeeper glSave(hasAttributes?GL_LAYOUT_BITS:0,
                            hasMatrix, false, hasTextureMatrix);
    Inherit(where);

    PushLayout(this);
    layout_items::iterator i;
    for (i = items.begin(); i != items.end(); i++)
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
    GLAllStateKeeper glSave(hasAttributes?GL_LAYOUT_BITS:0,
                            hasMatrix, false, hasTextureMatrix);
    Inherit(where);


    PushLayout(this);
    layout_items::iterator i;
    for (i = items.begin(); i != items.end(); i++)
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
    layout_items::iterator i;
    Inherit(layout);
    if (items.size() == 0)
        result |= Point3();
    else
        for (i = items.begin(); i != items.end(); i++)
            result |= (*i)->Bounds(this);
    return result;
}


Box3 Layout::Space(Layout *layout)
// ----------------------------------------------------------------------------
//   Compute the required space as the union of all item bounds
// ----------------------------------------------------------------------------
{
    Box3 result;
    layout_items::iterator i;
    Inherit(layout);
    if (items.size() == 0)
        result |= Point3();
    else
        for (i = items.begin(); i != items.end(); i++)
            result |= (*i)->Space(this);
    return result;
}


void Layout::Add(Drawing *d)
// ----------------------------------------------------------------------------
//   Add a drawing to the items, return true if item fits in layout
// ----------------------------------------------------------------------------
{
    items.push_back(d);
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
    Layout *l;
    layout_items::iterator i;
    for (i = items.begin(); i != items.end(); i++)
        if ((l = dynamic_cast<Layout*>(*i)))
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
    sstr << (void*)this;
    if (self)
    {
        Tree *sself = XL::xl_source(self);
        if (Prefix *p = self->AsPrefix())
        {
            if (p->left)
            {
                if (Name *n = p->left->AsName())
                {
                    sstr << "[" << n->value;
                    XL::TreePosition pos = sself->Position();
                    if (pos != XL::Tree::NOWHERE)
                        sstr << "@" << pos ;
                    sstr << "]";
                }
            }
        }
    }
    return sstr.str();
}


bool Layout::Refresh(QEvent *e, Layout *parent)
// ----------------------------------------------------------------------------
//   Re-compute layout on event, return true if self or child changed
// ----------------------------------------------------------------------------
{
    bool need_refresh = false;
    Widget * widget = Widget::Tao();

    if (!e)
        return false;

    if (refreshEvents.count(e->type()))
    {
        if (e->type() != QEvent::Timer)
            need_refresh = true;
        else
            need_refresh = (nextRefresh <= widget->CurrentTime());
    }

    if (need_refresh)
    {
        // Refresh this layout by recomputing it and replacing it in the parent

        IFTRACE(layoutevents)
            std::cerr << "Layout " << PrettyId() << " needs updating\n";

        if (parent)
        {
            if (XL::MAIN->options.enable_layout_cache)
            {
                // Drop children: delete drawings but set aside layouts, in case
                // they can be reused by the evaluation step below
                layout_items::iterator it;
                for (it = items.begin(); it != items.end(); it++)
                {
                    if (Layout * layout = dynamic_cast<Layout*>(*it))
                        widget->layoutCache.insert(layout);
                    else
                        delete (*it);
                }
                items.clear();
            }

            // Create a new (empty) layout
            Layout * layout = new Layout(this->display);

            do
            {
                // Set new layout as the current layout in the current Widget
                XL::Save<Layout *> saveLayout(widget->layout, layout);

                // Re-evaluate the source code for 'this' layout: will create a
                // child layout in the new layout
                if (self && ctx)
                    ctx->Evaluate(self);
            } while (0);

            // Get the (updated) child layout
            if (layout->items.begin() == layout->items.end())
                return false;

            layout_items::iterator b = layout->items.begin();
            Layout *child = dynamic_cast<Layout*>(*b);
            if (!child)
                return false;

            // Preserve id for selection
            layout->id = id;

            // In our parent, replace 'this' by the child layout
            // REVISIT data structure for direct access (map)
            bool found = false;
            layout_items::iterator i;
            for (i = parent->items.begin(); i != parent->items.end(); i++)
            {
                if ((*i) == this)
                {
                    (*i) = child;
                    found = true;
                    break;
                }
            }
            Q_ASSERT(found);

            // Delete temporary layout (but not child!)
            layout->items.clear();
            delete layout;

            // We're useless now
            delete this;

            return child->RefreshChildren(e);
        }
        else
        {
            std::cerr << "Error: Refresh called on dirty root layout\n";
        }
    }
    else
    {
        IFTRACE(layoutevents)
            std::cerr << "Layout " << PrettyId() << " does not need updating\n";
    }

    // Forward event to all child layouts
    bool changed = RefreshChildren(e);

    // When done with topmost layout we can clear cache
    if (XL::MAIN->options.enable_layout_cache && !parent)
        widget->layoutCache.clear();

    return changed;
}


bool Layout::RefreshChildren(QEvent *e)
// ----------------------------------------------------------------------------
//   Refresh all child layouts
// ----------------------------------------------------------------------------
{
    bool result;
    Layout *layout;
    layout_items::iterator i;
    layout_items items_copy = items;
    for (i = items_copy.begin(); i != items_copy.end(); i++)
        if ((layout = dynamic_cast<Layout*>(*i)))
            result |= layout->Refresh(e, this);
    return result;
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
    has3D           = where->has3D;
    hasPixelBlur    = where->hasPixelBlur;
    groupDrag       = where->groupDrag;
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
