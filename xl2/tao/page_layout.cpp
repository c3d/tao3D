// ****************************************************************************
//  page_layout.cpp                                                 Tao project
// ****************************************************************************
// 
//   File Description:
// 
// 
// 
// 
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

#include "page_layout.h"
#include "attributes.h"
#include "gl_keepers.h"
#include <cairo.h>
#include <cairo-gl.h>

TAO_BEGIN

PageLayout::PageLayout()
// ----------------------------------------------------------------------------
//   Create a new layout
// ----------------------------------------------------------------------------
    : Layout(),
      space(), horizontal(), vertical(),
      surface(NULL), context(NULL)
{
    GLStateKeeper save;

    surface = cairo_gl_surface_create_for_current_gl_context();
    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
        XL::Ooops("Unable to create Cairo surface");

    context = cairo_create(surface);
    if (cairo_status(context) != CAIRO_STATUS_SUCCESS)
        XL::Ooops("Unable to create Cairo context");
}


PageLayout::PageLayout(const PageLayout &o)
// ----------------------------------------------------------------------------
//   Copy a layout from another layout
// ----------------------------------------------------------------------------
    : Layout(o),
      space(o.space), horizontal(o.horizontal), vertical(o.vertical),
      surface(NULL), context(NULL)
{
    GLStateKeeper save;

    surface = cairo_surface_reference(o.surface);
    context = cairo_create(surface);
    if (cairo_status(context) != CAIRO_STATUS_SUCCESS)
        XL::Ooops("Unable to create Cairo context");
}


PageLayout::~PageLayout()
// ----------------------------------------------------------------------------
//    Destroy the cairo context
// ----------------------------------------------------------------------------
{
    if (context)
        cairo_destroy(context);
    if (surface)
        cairo_surface_destroy(surface);
}


Box3 PageLayout::Space()
// ----------------------------------------------------------------------------
//   Return the space for the box
// ----------------------------------------------------------------------------
{
    Box3 result = Bounds();
    result |= space;
    return result;
}


void PageLayout::Compute()
// ----------------------------------------------------------------------------
//   Layout the objects in the space box
// ----------------------------------------------------------------------------
{
}

TAO_END
