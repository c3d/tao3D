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


uint GroupLayout::DrawSelection(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the selection for the group or part of it
// ----------------------------------------------------------------------------
{
    (void) where;
    uint sel = Layout::DrawSelection(this);
    uint dclicks = Widget::doubleClicks(sel);
    Widget *widget = Display();

    bool show = sel && (dclicks == groupDepth);
    bool hide = sel && !show;
    if (show)
    {
        // Draw group bounding box
        widget->select(self);
        widget->select(id, 1);
        Drawing::DrawSelection(this);
    }
    else if (hide)
    {
        widget->deselect(self);
        widget->select(id, 0);
    }
    if (show || hide)
        widget->updateProgramSource();  // REVISIT

    return sel;
}


void GroupLayout::Add (Drawing *d)
// ----------------------------------------------------------------------------
//   Add a drawing to the layout and set drawing's group depth properly
// ----------------------------------------------------------------------------
{
    Layout::Add(d);
    d->groupDepth = this->groupDepth + 1;
}


TAO_END
