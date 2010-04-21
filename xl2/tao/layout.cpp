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
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "layout.h"
#include "gl_keepers.h"
#include "attributes.h"

TAO_BEGIN

int Layout::polygonOffset = 0;


LayoutState::LayoutState()
// ----------------------------------------------------------------------------
//   Default state for all layouts
// ----------------------------------------------------------------------------
    : offset(),
      font(qApp->font()),
      alongX(), alongY(), alongZ(),
      lineColor(0,0,0,1),       // Black
      fillColor(0,0,0,0),       // Transparent black
      fillTexture(0),
      rotationId(0), translationId(0), scaleId(0), id(0)
{}


LayoutState::LayoutState(const LayoutState &o)
// ----------------------------------------------------------------------------
//   Copy state (may be used between layouts)
// ----------------------------------------------------------------------------
      : offset(o.offset),
        font(o.font),
        alongX(o.alongX), alongY(o.alongY), alongZ(o.alongZ),
        lineColor(o.lineColor),
        fillColor(o.fillColor),
        fillTexture(o.fillTexture),
        rotationId(o.rotationId),
        translationId(o.translationId),
        scaleId(o.scaleId),
        id(o.id)
{}



void LayoutState::Clear()
// ----------------------------------------------------------------------------
//   Reset default state for a layout
// ----------------------------------------------------------------------------
{
    offset.Set(0,0,0);
    font = qApp->font();

    alongX = alongY = alongZ = Justification();
    lineColor.Set(0,0,0,0); // Transparent black
    fillColor.Set(0,0,0,1); // Black
    fillTexture = 0;
}


Layout::Layout(Widget *widget)
// ----------------------------------------------------------------------------
//    Create an empty layout
// ----------------------------------------------------------------------------
    : Drawing(), LayoutState(), items(), display(widget)
{}


Layout::Layout(const Layout &o)
// ----------------------------------------------------------------------------
//   Copy constructor
// ----------------------------------------------------------------------------
    : Drawing(o), LayoutState(o), items(), display(o.display)
{}


Layout::~Layout()
// ----------------------------------------------------------------------------
//   Destroy a layout
// ----------------------------------------------------------------------------
{
    Clear();
}


Layout *Layout::AddChild()
// ----------------------------------------------------------------------------
//   Add a new layout as a child of this one
// ----------------------------------------------------------------------------
{
    Layout *result = NewChild();
    Add(result);
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

    LayoutState::Clear();
}


void Layout::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the elements in the layout
// ----------------------------------------------------------------------------
{
    // Inherit offset from our parent layout if there is one
    XL::LocalSave<Point3> save(offset, offset);
    GLStateKeeper         glSave;
    Inherit(where);

    // Display all items
    layout_items::iterator i;
    for (i = items.begin(); i != items.end(); i++)
    {
        Drawing *child = *i;
        child->Draw(this);
    }
}


void Layout::DrawSelection(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the selection for the elements in the layout
// ----------------------------------------------------------------------------
{
    // Inherit offset from our parent layout if there is one
    XL::LocalSave<Point3> save(offset, offset);
    GLStateKeeper         glSave;
    Inherit(where);

    layout_items::iterator i;
    for (i = items.begin(); i != items.end(); i++)
    {
        Drawing *child = *i;
        child->DrawSelection(this);
    }
}


void Layout::Identify(Layout *where)
// ----------------------------------------------------------------------------
//   Identify the elements of the layout for OpenGL selection
// ----------------------------------------------------------------------------
{
    // Inherit offset from our parent layout if there is one
    XL::LocalSave<Point3> save(offset, offset);
    GLStateKeeper         glSave;
    Inherit(where);
        
    layout_items::iterator i;
    for (i = items.begin(); i != items.end(); i++)
    {
        Drawing *child = *i;
        child->Identify(this);
    }
}


Vector3 Layout::Offset()
// ----------------------------------------------------------------------------
//   Return the offset at which the layout expects us to draw
// ----------------------------------------------------------------------------
{
    return offset;
}


Box3 Layout::Bounds()
// ----------------------------------------------------------------------------
//   Compute the bounding box as the union of all item bounds
// ----------------------------------------------------------------------------
{
    Box3 result;
    layout_items::iterator i;
    for (i = items.begin(); i != items.end(); i++)
        result |= (*i)->Bounds();
    return result;
}


Box3 Layout::Space()
// ----------------------------------------------------------------------------
//   Compute the required space as the union of all item bounds
// ----------------------------------------------------------------------------
{
    Box3 result;
    layout_items::iterator i;
    for (i = items.begin(); i != items.end(); i++)
        result |= (*i)->Space();
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
    const double XY_SCALE = 5.0; // Good enough for approx 45 degrees Y angle
    const double UNITS = 2.0;
    glPolygonOffset (XY_SCALE * polygonOffset++, UNITS);
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
    glLoadName(id);
    if (!where)
        return;

    // Add offset of parent to the one we have
    offset += where->Offset();

    // Inherit color and other parameters as initial values
    // Note that these may really impact what gets rendered,
    // e.g. transparent colors may cause shapes to be drawn or not
    font        = where->font;
    alongX      = where->alongX;
    alongY      = where->alongY;
    alongZ      = where->alongZ;
    lineColor   = where->lineColor;
    fillColor   = where->fillColor;
    fillTexture = where->fillTexture;
}

TAO_END
