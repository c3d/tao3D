// ****************************************************************************
//  page_layout.cpp                                                 XLR project
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
#include "attribute.h"

TAO_BEGIN

PageLayout::PageLayout()
// ----------------------------------------------------------------------------
//   Create a new layout
// ----------------------------------------------------------------------------
    : cairo_surface(NULL), cairo_context(NULL)
{
    GLStateKeeper save;

    cairo_surface = cairo_gl_surface_create_for_current_gl_context();
    if (cairo_surface_status(cairo_surface) != CAIRO_STATUS_SUCCESS)
        XL::Ooops("Unable to create Cairo surface");

    cairo_context = cairo_create(cairo_surface);
    if (cairo_status(cairo_context) != CAIRO_STATUS_SUCCESS)
        XL::Ooops("Unable to create Cairo context");
}


PageLayout::PageLayout(const PageLayout &o)
// ----------------------------------------------------------------------------
//   Copy a layout from another layout
// ----------------------------------------------------------------------------
    : cairo_surface(NULL), cairo_context(NULL)
{
    GLStateKeeper save;

    cairo_surface = cairo_surface_reference(o.cairo_surface);
    cairo_context = cairo_create(cairo_surface);
    if (cairo_status(cairo_context) != CAIRO_STATUS_SUCCESS)
        XL::Ooops("Unable to create Cairo context");
}


PageLayout::~PageLayout()
// ----------------------------------------------------------------------------
//    Destroy the cairo context
// ----------------------------------------------------------------------------

    if (context)
        cairo_destroy(cairo_context);
    if (surface)
        cairo_surface_destroy(cairo_surface);
}


void PageLayout::SetAttribute(Attribute *attribute)
// ----------------------------------------------------------------------------
//    Set the attribute in Cairo
// ----------------------------------------------------------------------------
{
    attribute->SetCairo(cairo_context);
}


TAO_END
