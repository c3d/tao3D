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


Layout::Layout(Widget *widget)
// ----------------------------------------------------------------------------
//    Create an empty layout
// ----------------------------------------------------------------------------
    : Drawing(),
      offset(),
      font(qApp->font()),
      alongX(), alongY(), alongZ(),
      lineColor(0,0,0,1),       // Black
      fillColor(0,0,0,0),       // Transparent black
      fillTexture(0),
      lastRotation(0), lastTranslation(0), lastScale(0),
      items(),
      display(widget)
{}


Layout::Layout(const Layout &o)
// ----------------------------------------------------------------------------
//   Copy constructor
// ----------------------------------------------------------------------------
    : Drawing(o),
      offset(),                 // Zero, because we take parent offset in Draw
      font(o.font),
      alongX(o.alongX), alongY(o.alongY), alongZ(o.alongZ),
      lineColor(o.lineColor),
      fillColor(o.fillColor),
      fillTexture(o.fillTexture),
      lastRotation(o.lastRotation),
      lastTranslation(o.lastTranslation),
      lastScale(o.lastScale),
      items(),
      display(o.display)
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

    offset.Set(0,0,0);
    font = qApp->font();

    alongX = alongY = alongZ = Justification();
    lineColor.Set(0,0,0,0); // Transparent black
    fillColor.Set(0,0,0,1); // Black
    fillTexture = 0;
}


void Layout::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the elements in the layout
// ----------------------------------------------------------------------------
{
    // Inherit offset from our parent layout if there is one
    XL::LocalSave<Point3> save(offset, offset);
    GLStateKeeper         glSave;
    if (where)
    {
        // Add offset of parent to the one we have
        offset += where->Offset();

        // Inherit color and other parameters as initial values
        font        = where->font;
        alongX      = where->alongX;
        alongY      = where->alongY;
        alongZ      = where->alongZ;
        lineColor   = where->lineColor;
        fillColor   = where->fillColor;
        fillTexture = where->fillTexture;
    }

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
    if (where)
    {
        // Add offset of parent to the one we have
        offset += where->Offset();

        // Inherit color and other parameters as initial values
        // These parameters may impact the rendering of the selection,
        // e.g. transparent colors may cause children not to be drawn
        font        = where->font;
        alongX      = where->alongX;
        alongY      = where->alongY;
        alongZ      = where->alongZ;
        lineColor   = where->lineColor;
        fillColor   = where->fillColor;
        fillTexture = where->fillTexture;
    }
        
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
    if (where)
    {
        // Add offset of parent to the one we have
        offset += where->Offset();

        // Inherit color and other parameters as initial values
        // These parameters may impact the rendering of the selection,
        // e.g. transparent colors may cause children not to be drawn
        font        = where->font;
        alongX      = where->alongX;
        alongY      = where->alongY;
        alongZ      = where->alongZ;
        lineColor   = where->lineColor;
        fillColor   = where->fillColor;
        fillTexture = where->fillTexture;
    }
        
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


Layout &Layout::Add(Drawing *d)
// ----------------------------------------------------------------------------
//   Add a drawing to the items - Override with layout computations
// ----------------------------------------------------------------------------
{
    items.push_back(d);
    return *this;
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

TAO_END
