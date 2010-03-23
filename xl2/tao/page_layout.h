#ifndef PAGE_LAYOUT_H
#define PAGE_LAYOUT_H
// ****************************************************************************
//  page_layout.h                                                   Tao project
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

#include "layout.h"
#include "justification.h"
#include <cairo.h>


TAO_BEGIN

struct PageLayout : Layout
// ----------------------------------------------------------------------------
//   A 2D layout specialized for placing text and 2D shapes on pages
// ----------------------------------------------------------------------------
{
                        PageLayout();
                        PageLayout(const PageLayout &o);
                        ~PageLayout();

protected:
    // We use cairo to record most of our internal state (color, ...)
    cairo_surface_t      *surface;
    cairo_t              *context;

    // Justification information
    Justification       alongX, alongY;
};

TAO_END

#endif // PAGE_LAYOUT_H
