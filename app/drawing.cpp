// *****************************************************************************
// drawing.cpp                                                     Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

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


Drawing::~Drawing()
// ----------------------------------------------------------------------------
//   Destroy a Drawing
// ----------------------------------------------------------------------------
{
    count--;
}


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
    if (widget->selected(layout))
    {
        Box3 bounds = Bounds(layout);
        XL::Save<Point3> zeroOffset(layout->offset, Point3(0,0,0));
        widget->drawSelection(bounds, "selection", layout);
    }
}


void Drawing::Evaluate(Layout *)
// ----------------------------------------------------------------------------
//   Evaluate a drawing (typically attributes) when added to a layout
// ----------------------------------------------------------------------------
{
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


bool Drawing::Paginate(PageLayout *page)
// ----------------------------------------------------------------------------
//   Perform pagination: default is to insert the current drawing
// ----------------------------------------------------------------------------
{
    return page->PaginateItem(this);
}


void Drawing::ClearCaches()
// ----------------------------------------------------------------------------
//   Clear any information not owned by this item and that can be reconstructed
// ----------------------------------------------------------------------------
{
}


void Drawing::Clear()
// ----------------------------------------------------------------------------
//   Clear any information owned by this drawing
// ----------------------------------------------------------------------------
{
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
