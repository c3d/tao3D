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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
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
    Widget *widget = Display();
    uint selected = widget->selected(id);
    bool open = selected & Widget::CONTAINER_OPENED;
    bool sel = selected && !open;

    if (open)
    {
        // Draw children selection
        Layout::DrawSelection(where);
    }
    else if (sel)
    {
        // Draw selection here
        Drawing::DrawSelection(this);
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


void GroupLayout::Select(Layout *where)
// ----------------------------------------------------------------------------
//   Select the group (draw the bounding box, select all contained items)
// ----------------------------------------------------------------------------
{
    Widget *widget = Display();
    widget->select(self);
    widget->select(id, 1);
    SelectAll(false);
    Drawing::DrawSelection(this);
    SelectAll(true);
}


void GroupLayout::Deselect(Layout *)
// ----------------------------------------------------------------------------
//   Deselect the group (clear the bounding box, deselect all contained items)
// ----------------------------------------------------------------------------
{
    Widget *widget = Display();
    widget->deselect(self);
    widget->select(id, 0);
    SelectAll(false);
}


void GroupLayout::SelectAll(bool doSelect)
// ----------------------------------------------------------------------------
//   Recursively (de)select all selectable items in a GroupLayout
// ----------------------------------------------------------------------------
{
    Widget *widget = Display();
    layout_items::iterator i;
    for (i = items.begin(); i != items.end(); i++)
    {
        Drawing *child = *i;
        if (GroupLayout *gl = dynamic_cast<GroupLayout*>(child))
            gl->SelectAll(doSelect);
        else if (Layout *l  = dynamic_cast<Layout *>(child))
            if (doSelect != (bool)widget->selected(l->id))
                if (doSelect)
                    widget->select(l->id, 1);
                else
                    widget->select(l->id, -1);
    }
}

TAO_END
