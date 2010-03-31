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

void Drawing::Draw(Layout *)
// ----------------------------------------------------------------------------
//   Draw a shape for rendering purpose
// ----------------------------------------------------------------------------
{}


void Drawing::DrawSelection(Layout *layout)
// ----------------------------------------------------------------------------
//   Draw the marker indicating that a shape is selected
// ----------------------------------------------------------------------------
{
    Widget *widget = layout->Display();
    if (widget->selected())
    {
        Color line(1.0, 0.0, 0.0, 0.5);
        Color fill(0.0, 0.7, 1.0, 0.1);
        XL::LocalSave<Color> saveLine(layout->lineColor, line);
        XL::LocalSave<Color> saveFill(layout->fillColor, fill);
        widget->drawSelection(Bounds() + layout->Offset(), "selection");
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


Box3 Drawing::Bounds()
// ----------------------------------------------------------------------------
//   Return the bounding box for the shape
// ----------------------------------------------------------------------------
{
    // By default, it is empty
    return Box3();
}


Box3 Drawing::Space()
// ----------------------------------------------------------------------------
//   Return the space required around a shape
// ----------------------------------------------------------------------------
{
    // The default for the space is to be identical to the bounds
    return Bounds();
}


bool Drawing::IsWordBreak()
// ----------------------------------------------------------------------------
//   Tell if a shape is a word break
// ----------------------------------------------------------------------------
{
    return false;
}


bool Drawing::IsLineBreak()
// ----------------------------------------------------------------------------
//    Tell if a shape is a line break
// ----------------------------------------------------------------------------
{
    return false;
}


bool Drawing::IsAttribute()
// ----------------------------------------------------------------------------
//    Tell if a shape is an attribute (something that need not be drawn)
// ----------------------------------------------------------------------------
{
    return false;
}

TAO_END
