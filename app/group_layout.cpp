// *****************************************************************************
// group_layout.cpp                                                Tao3D project
// *****************************************************************************
//
// File description:
//
//     GroupLayout implementation.
//
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010-2011, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2012,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
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

#include "group_layout.h"
#include "gl_keepers.h"

TAO_BEGIN


GroupLayout::GroupLayout(Widget *widget, Tree_p self)
// ----------------------------------------------------------------------------
//   Create a group layout
// ----------------------------------------------------------------------------
    : Layout(widget), self(self)
{}


GroupLayout::GroupLayout(const GroupLayout &o)
// ----------------------------------------------------------------------------
//   Create a copy of a group layout
// ----------------------------------------------------------------------------
    : Layout(o), self(o.self)
{}


void GroupLayout::DrawSelection(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the selection for the group or part of it
// ----------------------------------------------------------------------------
{
    if (where->groupDrag)
        groupDrag = true;

    Widget *widget = Display();
    uint selected = widget->selected(id);
    bool open = (selected & Widget::SELECTION_MASK) == Widget::CONTAINER_OPENED;
    bool sel = (selected || groupDrag) && !open;

    if (open)
    {
        // Individual drag for items below
        groupDrag = false;

        // Draw children selection
        Layout::DrawSelection(where);

        Box3 bounds = Bounds(where);
        XL::Save<Point3> zeroOffset(where->offset, Point3(0,0,0));
        widget->drawSelection(where, bounds, "open_group", where->id);
    }
    else if (sel)
    {
        // Select itself
        widget->select(self);

        // We are in "group drag" mode (we will drag children as well)
        groupDrag = true;

        // Draw selection here
        Drawing::DrawSelection(this);

        // Draw children selection to allow drag
        Layout::DrawSelection(this);
    }

    if (open || sel)
        widget->updateProgramSource();  // REVISIT
}


void GroupLayout::Add (Drawing *d)
// ----------------------------------------------------------------------------
//   Add a drawing to the layout and set drawing's group depth properly
// ----------------------------------------------------------------------------
{
    Layout::Add(d);
}


uint GroupLayout::Selected()
// ----------------------------------------------------------------------------
//   Selection state of a GroupLayout, only children are considered
// ----------------------------------------------------------------------------
{
    return ChildrenSelected();
}

TAO_END
