// ****************************************************************************
//  group_layout.cpp                                               Tao project
// ****************************************************************************
//
//   File Description:
//
//     GroupLayout implementation.
//
//
//
//
//
//
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

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
    bool open = selected & Widget::CONTAINER_OPENED;
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
