// ****************************************************************************
//  drawing.cpp                                                     Tao project
// ****************************************************************************
// 
//   File Description:
// 
//      Elements that can be drawn on a 2D layout
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

#include "drawing.h"
#include "shapes3d.h"
#include "layout.h"
#include "widget.h"
#include "context.h"


TAO_BEGIN

uint Drawing::count = 0;

Drawing::Drawing()
// ----------------------------------------------------------------------------
//   Create a Drawing
// ----------------------------------------------------------------------------
{
    count++;
}


Drawing::Drawing(const Drawing&)
// ----------------------------------------------------------------------------
//   Copy Drawing
// ----------------------------------------------------------------------------
{
    count++;
}


void Drawing::Draw(Layout *)
// ----------------------------------------------------------------------------
//   Draw a shape for rendering purpose
// ----------------------------------------------------------------------------
{}


Drawing::~Drawing()
// ----------------------------------------------------------------------------
//   Destroy a Drawing
// ----------------------------------------------------------------------------
{
    count--;
}


void Drawing::DrawSelection(Layout *layout)
// ----------------------------------------------------------------------------
//   Draw the marker indicating that a shape is selected
// ----------------------------------------------------------------------------
{
    Widget *widget = layout->Display();
    if (widget->selected(layout))
    {
        Box3 bounds = Bounds(layout);
        XL::LocalSave<Point3> zeroOffset(layout->offset, Point3(0,0,0));
        widget->drawSelection(layout, bounds, "selection", layout->id);
    }
}


void Drawing::Identify(Layout *l)
// ----------------------------------------------------------------------------
//   Draw an area used to identify the shape for OpenGL selection
// ----------------------------------------------------------------------------
{
    // The default is to draw the shape
    Draw(l);
}


Box3 Drawing::Bounds(Layout *where)
// ----------------------------------------------------------------------------
//   Return the bounding box for the shape
// ----------------------------------------------------------------------------
{
    // By default, it is empty
    return Box3(where->offset, Vector3(0,0,0));
}


Box3 Drawing::Space(Layout *where)
// ----------------------------------------------------------------------------
//   Return the space required around a shape
// ----------------------------------------------------------------------------
{
    // The default for the space is to be identical to the bounds
    return Bounds(where);
}


Tree *Drawing::Source()
// ----------------------------------------------------------------------------
//   Return the source tree for this drawing
// ----------------------------------------------------------------------------
{
    return NULL;
}


Drawing *Drawing::Break(BreakOrder &order, uint &size)
// ----------------------------------------------------------------------------
//    Break a shape at the next line boundary, if any
// ----------------------------------------------------------------------------
{
    order = NoBreak;
    size = 1;
    return NULL;
}


scale Drawing::TrailingSpaceSize(Layout *)
// ----------------------------------------------------------------------------
//   By default, the trailing space is 0
// ----------------------------------------------------------------------------
{
    return 0;
}


bool Drawing::IsAttribute()
// ----------------------------------------------------------------------------
//    Tell if a shape is an attribute (something that need not be drawn)
// ----------------------------------------------------------------------------
{
    return false;
}

TAO_END
