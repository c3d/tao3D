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
#include <cairo.h>
#include <cairo-gl.h>

TAO_BEGIN

static OutlineColor     blackOutline    (0,0,0,1); // Black
static FillColor        transparentFill (0,0,0,0); // Transparent
static TextColor        blackText       (0,0,0,1); // Black


Layout::Layout()
// ----------------------------------------------------------------------------
//    Create an empty layout
// ----------------------------------------------------------------------------
    : Drawing(),
      outlineColor(NULL), fillColor(NULL), textColor(NULL), fillTexture(NULL),
      items(), offset(), surface(NULL), context(NULL)
{
    GLStateKeeper save;

    surface = cairo_gl_surface_create_for_current_gl_context();
    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
        XL::Ooops("Unable to create Cairo surface");

    context = cairo_create(surface);
    if (cairo_status(context) != CAIRO_STATUS_SUCCESS)
        XL::Ooops("Unable to create Cairo context");

    // Select the default colors
    blackOutline.Draw(this);
    transparentFill.Draw(this);
    blackText.Draw(this);
}


Layout::Layout(const Layout &o)
// ----------------------------------------------------------------------------
//   Copy constructor
// ----------------------------------------------------------------------------
    : Drawing(o),
      outlineColor(o.outlineColor), // REVISIT: Safe to copy?
      fillColor(o.fillColor),
      textColor(o.textColor),
      fillTexture(o.fillTexture),
      items(o.items), surface(NULL), context(NULL)
{
    GLStateKeeper save;

    surface = cairo_surface_reference(o.surface);
    context = cairo_create(surface);
    if (cairo_status(context) != CAIRO_STATUS_SUCCESS)
        XL::Ooops("Unable to create Cairo context");
}


Layout::~Layout()
// ----------------------------------------------------------------------------
//   Destroy a layout
// ----------------------------------------------------------------------------
{
    layout_items::iterator i;
    for (i = items.begin(); i != items.end(); i++)
        delete *i;

    if (context)
        cairo_destroy(context);
    if (surface)
        cairo_surface_destroy(surface);
}


void Layout::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the elements in the layout
// ----------------------------------------------------------------------------
{
    // Inherit offset from our parent layout if there is one
    if (where)
        offset = where->Offset();

    layout_items::iterator i;
    for (i = items.begin(); i != items.end(); i++)
        (*i)->Draw(this);
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


TAO_END
