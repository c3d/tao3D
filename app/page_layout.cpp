// *****************************************************************************
// page_layout.cpp                                                 Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2011-2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2010-2013, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2012,2014, Jérôme Forissier <jerome@taodyne.com>
// (C) 2011-2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
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

#include "page_layout.h"
#include "attributes.h"
#include "text_drawing.h"
#define JUSTIFICATION_H_IMPLEMENTATION
#include "justification.h"
#include "gl_keepers.h"
#include "window.h"
#include "path3d.h"
#include "demangle.h"
#include "tao_glu.h"
#include <QFontMetrics>
#include <QFont>


RECORDER_DEFINE(justify, 16, "Text justification operations");

TAO_BEGIN

// ============================================================================
//
//   Specializations for the Justifier computations
//
// ============================================================================

template<> void Justifier<LayoutLine *>::PurgeItems()
// ----------------------------------------------------------------------------
//   The layout lines are actually owned by the page layout justifier
// ----------------------------------------------------------------------------
{
    for (PlacesIterator p = places.begin(); p != places.end(); p++)
        delete (*p).item;
}



// ============================================================================
//
//    LayoutLine: A single line in a page layout
//
// ============================================================================

LayoutLine::LayoutLine(coord left, coord right, Justification &j)
// ----------------------------------------------------------------------------
//   Create a new line of drawing elements
// ----------------------------------------------------------------------------
    : bounds(Point(left, 0), Point(right, 0)), line(), perSolid(0), perBreak(0)
{
    record(justify, "Layout line %p: %f - %f", this, left, right);
    line.BeginLayout(left, right, j);
}


LayoutLine::~LayoutLine()
// ----------------------------------------------------------------------------
//    The real work happens in the LineJustifier's destructor
// ----------------------------------------------------------------------------
{
    record(justify, "Delete layout line %p", this);
    line.Clear();
}


void LayoutLine::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Compute line layout and draw the placed elements
// ----------------------------------------------------------------------------
{
    assert(!line.data && "LayoutLine::Draw called before layout");

    record(justify, "Draw layout line %p in layout %p", this, where);

    // Copy the perSolid we computed to the layout we draw in
    where->alongX.perSolid = perSolid;
    where->alongX.perBreak = perBreak;

    // Display all items
    LineJustifier::Places &places = line.places;
    LineJustifier::PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        LineJustifier::Place &place = *p;
        Drawing *child = place.item;
        XL::Save<coord> saveX(where->offset.x, where->offset.x+place.position);
        child->Draw(where);
    }
}


void LayoutLine::DrawSelection(Layout *where)
// ----------------------------------------------------------------------------
//   Recompute layout if necessary and draw selection for all children
// ----------------------------------------------------------------------------
{
    assert(!line.data && "LayoutLine::DrawSelection called before layout");

    // Copy the perSolid we computed to the layout we draw in
    where->alongX.perSolid = perSolid;
    where->alongX.perBreak = perBreak;

    // Display all items
    LineJustifier::Places &places = line.places;
    LineJustifier::PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        LineJustifier::Place &place = *p;
        Drawing *child = place.item;
        XL::Save<coord> saveY(where->offset.x, where->offset.x+place.position);
        child->DrawSelection(where);
    }
}


void LayoutLine::Identify(Layout *where)
// ----------------------------------------------------------------------------
//   Identify page elements for OpenGL
// ----------------------------------------------------------------------------
{
    assert(!line.data && "LayoutLine::Identify called before layout");

    // Copy the perSolid we computed to the layout we draw in
    where->alongX.perSolid = perSolid;
    where->alongX.perBreak = perBreak;

    // Display all items
    LineJustifier::Places &places = line.places;
    LineJustifier::PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        LineJustifier::Place &place = *p;
        Drawing *child = place.item;
        XL::Save<coord> saveY(where->offset.x, where->offset.x+place.position);
        child->Identify(where);
    }
}


Box3 LayoutLine::Bounds(Layout *)
// ----------------------------------------------------------------------------
//   Return the bounds for the box
// ----------------------------------------------------------------------------
{
    assert(!line.data && "LayoutLine::Bounds called before layout");
    return bounds; // Converts Box to Box3
}


Box3 LayoutLine::Space(Layout *)
// ----------------------------------------------------------------------------
//   Return the space for the box
// ----------------------------------------------------------------------------
{
    assert(!line.data && "LayoutLine::Space called before layout");
    return bounds; // Convert Box to Box3
}


bool LayoutLine::Paginate(PageLayout *)
// ----------------------------------------------------------------------------
//   We should never arrive here, since layout lines are only created by pages
// ----------------------------------------------------------------------------
{
    assert(!"LayoutLine::Paginate called");
    return false;
}


void LayoutLine::PerformLayout()
// ----------------------------------------------------------------------------
//   Layout the current line
// ----------------------------------------------------------------------------
{
    if (line.data)
        line.EndLayout(&perSolid, &perBreak);
}



// ============================================================================
//
//   PageLayout: Layout of a whole page
//
// ============================================================================

PageLayout::PageLayout(Widget *widget)
// ----------------------------------------------------------------------------
//   Create a new layout
// ----------------------------------------------------------------------------
    : Layout(widget),
      space(), bounds(), page(), currentFlow(NULL), selectId(0)
{
    record(justify, "Page layout %p from widget %p", this, widget);
}


PageLayout::PageLayout(const PageLayout &o)
// ----------------------------------------------------------------------------
//   Copy a layout from another layout
// ----------------------------------------------------------------------------
    : Layout(o), space(o.space), bounds(o.bounds),
      page(), currentFlow(NULL), selectId(0)
{
    record(justify, "Page layout %p from layout %p", this, &o);
    space |= Point(0,0);
}


PageLayout::~PageLayout()
// ----------------------------------------------------------------------------
//    Destroy the page layout
// ----------------------------------------------------------------------------
{
    record(justify, "Delete page layout %p", this);
    Clear();
}


void PageLayout::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw in an enclosing layout after computing the page layout here
// ----------------------------------------------------------------------------
//   This differs from Layout::Draw mostly in that we first compute the layout
//   and then only iterate on the items that were placed, not all items,
//   taking the layout offset from the placed position
{
    record(justify, "Draw page layout %p offset %f", this, where->Offset());

    // Inherit state from where, compute layout if necessary
    Compute(where);

    // Inherit state from our parent layout if there is one and compute layout
    if (page.places.size() == 0 && !page.HadRoom())
        return DrawPlaceholder();

    // Display all items
    GLAllStateKeeper glSave;
    PushLayout();
    PageJustifier::Places &all = page.places;
    for (PageJustifier::PlacesIterator p = all.begin(); p != all.end(); p++)
    {
        PageJustifier::Place &place = *p;
        LayoutLine *child = place.item;
        record(justify, "Page layout %p from %f to %f",
               this, where->offset.y, where->offset.y + place.position);
        XL::Save<coord> saveY(offset.y, offset.y+place.position);
        child->Draw(this);
    }
    PopLayout();
    record(justify, "Page layout %p done, offset %f", this, where->Offset());
}


void PageLayout::DrawSelection(Layout *where)
// ----------------------------------------------------------------------------
//   Recompute layout if necessary and draw selection for all children
// ----------------------------------------------------------------------------
{
    // Remember the initial selection ID
    Widget     *widget   = where->Display();
    TextSelect *sel      = widget->textSelection();

    id = where->id;
    // Inherit state from our parent layout if there is one and compute layout
    Compute(where);

    // Clear the selection box in case it was set by some previous text
    if (sel)
    {
        sel->selBox.Empty();
        sel->formulaBox.Empty();
        sel->formulaMode = 0;
    }

    // Check if the text layout was opened, if so draw the text box
    uint selected = widget->selected(id);
    if (selected)
    {
        Box3 bounds = space;
        XL::Save<Point3> zeroOffset(where->offset, Point3(0,0,0));
        if ((selected & Widget::SELECTION_MASK) == Widget::CONTAINER_OPENED)
            widget->drawSelection(where, bounds, "open_textbox", where->id);
        else
            widget->drawSelection(where, bounds, "selected_textbox", where->id);
    }

    // Display all items
    GLAllStateKeeper glSave;
    PushLayout();
    PageJustifier::Places &places = page.places;
    PageJustifier::PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        PageJustifier::Place &place = *p;
        LayoutLine *child = place.item;

        XL::Save<coord> saveY(offset.y, offset.y+place.position);
        if (sel)
            DrawSelectionBox(sel, child, saveY.saved);
        else
            // No text selection, just draw children directly
            child->DrawSelection(this);
    }
    PopLayout();

    // Save color and font as necessary for color selectors
    if (widget->selected(this))
        widget->saveSelectionColorAndFont(this);
}


void PageLayout::Identify(Layout *where)
// ----------------------------------------------------------------------------
//   Identify page elements for OpenGL
// ----------------------------------------------------------------------------
{
    // Inherit state from our parent layout if there is one and compute layout
    Compute(where);

    // Display all items
    GLAllStateKeeper glSave;
    PushLayout();
    PageJustifier::Places &places = page.places;
    PageJustifier::PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        PageJustifier::Place &place = *p;
        LayoutLine *child = place.item;
        XL::Save<coord> saveY(offset.y, offset.y+place.position);
        child->Identify(this);
    }
    PopLayout();

    coord x = space.Left(),  y = space.Bottom();
    coord w = space.Width(), h = space.Height();
    coord array[4][3] =
    {
        { x,     y,     0 },
        { x + w, y,     0 },
        { x + w, y + h, 0 },
        { x,     y + h, 0 }
    };

    GL.Color(0.2,0.6,1.0,0.1);
    // Load model view matrix
    GL.LoadMatrix();
    GL.VertexPointer(3, GL_DOUBLE, 0, array);
    GL.EnableClientState(GL_VERTEX_ARRAY);
    GL.DrawArrays(GL_QUADS, 0, 4);
    GL.DisableClientState(GL_VERTEX_ARRAY);
}


void PageLayout::DrawPlaceholder()
// ----------------------------------------------------------------------------
//   Draw a striked rectangle to indicate that the page layout is too small
// ----------------------------------------------------------------------------
{
    // Create path
    TesselatedPath path(GLU_TESS_WINDING_ODD);
    path.moveTo(Point3(space.Left(), space.Top(), 0));
    path.lineTo(Point3(space.Left(), space.Bottom(), 0));
    path.lineTo(Point3(space.Right(), space.Bottom(), 0));
    path.lineTo(Point3(space.Right(), space.Top(), 0));
    path.close();
    path.lineTo(Point3(space.Right(), space.Bottom(), 0));

    // Draw line width, colors and path
    LineWidth(3)                        .Draw(this);
    LineColor(0.8, 0.8, 0.8, 0.7)       .Draw(this);
    FillColor(0.2, 0.2, 0.2, 0.1)       .Draw(this);
    path                                .Draw(this);
}


void PageLayout::DrawSelectionBox(TextSelect *sel,Drawing *child,coord savedY)
// ----------------------------------------------------------------------------
//   Draw the selection box for a given text box
// ----------------------------------------------------------------------------
{
    Widget *widget = Display();

    // Text selection: Draw the selection box
    sel->processLineBreak();
    GLuint lineStart = widget->selectionCurrentId();
    child->DrawSelection(this);
    offset.y = savedY;
    GLuint lineEnd = widget->selectionCurrentId();

    if (sel->selBox.Width() > 0 && sel->selBox.Height() > 0)
    {
        if (sel->point != sel->mark)
        {
            coord y = sel->selBox.Bottom();
            if (sel->start()<=lineStart && sel->end()>=lineStart)
                sel->selBox |= Point3(space.Left(), y, 0);
            if (sel->end() >= lineEnd && sel->start() <= lineEnd)
                sel->selBox |= Point3(space.Right(), y, 0);
        }

        GL.BlendFunc(GL_DST_COLOR, GL_ZERO);
        text mode = sel->textMode ? "text_selection" : "text_highlight";
        XL::Save<Point3> zeroOffset(offset, Point3());
        widget->drawSelection(this, sel->selBox, mode, 0);
        sel->selBox.Empty();
        GL.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    if (sel->formulaBox.Width() > 0 && sel->formulaBox.Height() > 0)
    {
         GL.BlendFunc(GL_DST_COLOR, GL_ZERO);
        text mode = "formula_highlight";
        XL::Save<Point3> zeroOffset(offset, Point3());
        widget->drawSelection(this, sel->formulaBox, mode, 0);
        sel->formulaBox.Empty();
         GL.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
}


Box3 PageLayout::Bounds(Layout *layout)
// ----------------------------------------------------------------------------
//   Return the bounds for the page layout
// ----------------------------------------------------------------------------
{
    // Return the bounds after we computed the inner placement
    Compute(layout);
    return bounds;              // Converts Box to Box3
}


Box3 PageLayout::Space(Layout *layout)
// ----------------------------------------------------------------------------
//   Return the space for the page layout
// ----------------------------------------------------------------------------
{
    // Return the union of space and bounds, in case we take more than planned
    Compute(layout);
    bounds |= space;
    return bounds;              // Converts Box to Box3
}


bool PageLayout::Paginate(PageLayout *page)
// ----------------------------------------------------------------------------
//   Paginate a text box inside a text box
// ----------------------------------------------------------------------------
{
    // There is a possibility for this page to be in a text flow and refresh
    // independently from the page it is paginated in. So we need to ensure
    // that each page gets refreshed whenever the other is
    this->CachesInfoFrom(page);
    page->CachesInfoFrom(this);

    // We will treat the text box as a complete rectangular entity
    return page->PaginateItem(this);
}


void PageLayout::Add(Drawing *d)
// ----------------------------------------------------------------------------
//   Make sure we force a new computation of the justification
// ----------------------------------------------------------------------------
{
    assert(d);

    if (page.places.size())
    {
        // Adding elements after we computed a layout is bad practice
        std::cerr << "WARNING: PageLayout gets new elements after layout\n";
        page.Clear();
    }

    Layout::Add(d);
}


Layout *PageLayout::NewChild()
// ----------------------------------------------------------------------------
//   Creating a text span for locally inside a page layout
// ----------------------------------------------------------------------------
{
    return new TextSpan(this);
}


void PageLayout::Clear()
// ----------------------------------------------------------------------------
//   When we clear a page layout, we need to also clear placed items
// ----------------------------------------------------------------------------
{
    record(justify, "Clear page layout %p", this);
    Layout::Clear();
    assert(page.Empty() || !"PageLayout::Clear called with dirty cache");
}


void PageLayout::ClearCaches()
// ----------------------------------------------------------------------------
//   Drop references to objects we don't own (e.g. drawings in text flows)
// ----------------------------------------------------------------------------
{
    page.Clear();
    bounds = Box();
    Layout::ClearCaches();
}


void PageLayout::Compute(Layout *where)
// ----------------------------------------------------------------------------
//   Layout all elements on the page, preserving layout state
// ----------------------------------------------------------------------------
{
    record(justify, "Compute page layout %p in layout %p", this, where);

    // Get attributes from surrounding context
    Inherit(where);

    // If we already computed the layout, nothing to do
    if (page.places.size())
        return;

    // Check height of page, quick exit if page is empty
    coord top = space.Top() - this->top;
    coord bottom = space.Bottom() + this->bottom;
    if (top <= bottom)
        return;

    // Save GL state before paginate to fix #2971.
    GLAllStateKeeper save;

    // Begin pagination
    page.BeginLayout(top, bottom, alongY);

    // Paginate all the items that fit
    bool ok = true;
    for (Drawings::iterator i = items.begin(); ok && i != items.end(); i++)
    {
        Drawing *child = *i;
        ok = child->Paginate(this);
    }

    // We are done with the pagination
    if (ok && !page.Empty())
        PaginateLastLine(NoBreak);
    page.EndLayout(&alongY.perSolid, &alongY.perBreak);

    // Restore state as it was at the beginning of the layout
    Inherit(where);

}


bool PageLayout::PaginateItem(Drawing *drawing, BreakOrder order, uint count)
// ----------------------------------------------------------------------------
//    Add a drawable item to the current page
// ----------------------------------------------------------------------------
{
    // Quick exit if we are out of space
    if (!page.HasRoom())
        return false;

    // Find which layout line we are working with
    LayoutLine *line = NULL;
    if (!page.Empty())
    {
        line = page.Current();
        if (!line->line.HasRoom())
        {
            if (!PaginateLastLine(order))
                return false;
            line = NULL;
        }
    }
    if (!line)
    {
        line = new LayoutLine(space.Left()+left, space.Right()-right, alongX);
        bool lineFits = page.AddItem(line);
        if (!lineFits)
        {
            // No need to transfer: the just-created line is empty,
            // and since hardBreak is false it has not been added to places.
            delete line;
            return false;
        }
    }

    // Compute the size of the item
    XL::Save<Vector3> zeroOffset(this->offset, Vector3(0,0,0));
    Box3 drawSpace = drawing->Space(this);
    if (drawSpace.Width() < 0 || drawSpace.Height() < 0)
        drawSpace |= Point3(0,0,0);     // Eliminate empty boxes
    coord offset = -drawSpace.Left();
    scale size = fabs(drawSpace.Width());
    scale spc = drawing->TrailingSpaceSize(this);

    // Try to add item to layout line
    Justifier<Drawing *> &lj = line->line;
    bool pack = order < WordBreak;
    bool forceBreak = order >= LineBreak;
    bool fits = lj.AddItem(drawing, count, pack, forceBreak, size, offset, spc);

    // Adjust layout line
    if (fits || (forceBreak && count <= 1))
    {
        // If drawing fits, adjust the dimensions of the line accordingly
        Box &lineBounds = line->bounds;
        if (lineBounds.lower.y > drawSpace.lower.y)
            lineBounds.lower.y = drawSpace.lower.y;
        if (lineBounds.upper.y < drawSpace.upper.y)
            lineBounds.upper.y = drawSpace.upper.y;
    }
    if (!fits)
    {
        // If drawing doesn't fit, try to fit the last line vertically
        if (!PaginateLastLine(order))
            return false;

        // Create a new layout line
        line = new LayoutLine(space.Left()+left, space.Right()-right, alongX);
        bool lineFits = page.AddItem(line);
        if (!lineFits)
        {
            // No need to transfer: the just-created line is empty
            delete line;
            return false;
        }

        Justifier<Drawing *> &lj = line->line;
        bool fits =
            (forceBreak && size==0) ||
            lj.AddItem(drawing, count, pack, forceBreak, size, offset, spc);
        if (fits)
        {
            Box &lineBounds = line->bounds;
            if (lineBounds.lower.y > drawSpace.lower.y)
                lineBounds.lower.y = drawSpace.lower.y;
            if (lineBounds.upper.y < drawSpace.upper.y)
                lineBounds.upper.y = drawSpace.upper.y;
        }
        return fits;
    }

    // Happy campers - Everything fits
    return true;
}


bool PageLayout::PaginateLastLine(BreakOrder order)
// ----------------------------------------------------------------------------
//   Paginate the last line that was created
// ----------------------------------------------------------------------------
{
    assert (!page.Empty());

    LayoutLine *line = page.Current();
    page.PopItem();

    Box &lineBounds = line->bounds;
    scale lsize = fabs(lineBounds.Height());
    coord loffset = lineBounds.Top();
    bool pack = order <= LineBreak;
    bool forceBreak = order >= ColumnBreak;
    bool lfits = page.AddItem(line, 1, pack, forceBreak, lsize, loffset, 0);
    if (lfits)
    {
        line->PerformLayout();
    }
    else
    {
        if (currentFlow)
            currentFlow->Transfer(line);
        page.RemoveItem(line);
        delete line;
    }
    return lfits;
}


void PageLayout::SetLastSplit(TextSplit *split)
// ----------------------------------------------------------------------------
//   Record last text split if we have an active flow
// ----------------------------------------------------------------------------
{
    if (currentFlow)
        currentFlow->SetLastSplit(split);
}


TextSplit *PageLayout::LastSplit()
// ----------------------------------------------------------------------------
//   Return the last item placed in the last line, if any
// ----------------------------------------------------------------------------
{
    if (currentFlow)
        return currentFlow->LastSplit();
    return NULL;
}



// ============================================================================
//
//    TextFlow: Records item to draw e.g. in a PageLayout
//
// ============================================================================

TextFlow::TextFlow(Layout *layout, text flowName)
// ----------------------------------------------------------------------------
//   Create a text flow recording items in the given layout
// ----------------------------------------------------------------------------
    : Layout(*layout), flowName(flowName),
      textBoxIds(), current(0), reject(), lastSplit(NULL)
{
    record(justify, "Text flow %p from layout %p", this, layout);
}


TextFlow::~TextFlow()
// ----------------------------------------------------------------------------
//    Make sure that the widget drops references to this flow
// ----------------------------------------------------------------------------
{
    record(justify, "Delete text flow %p", this);
    Clear();
    display->eraseFlow(flowName);
}


void TextFlow::Draw(Layout *)
// ----------------------------------------------------------------------------
//   The text flow records things, doesn't display them
// ----------------------------------------------------------------------------
{
    // When we draw the text flow, reset drawing at the beginning
    charId = 0;
    current = 0;
    lastSplit = NULL;
}


void TextFlow::DrawSelection(Layout *)
// ----------------------------------------------------------------------------
//   The text flow records things, no display
// ----------------------------------------------------------------------------
{
    charId = 0;
    current = 0;
    lastSplit = NULL;
}


void TextFlow::Identify(Layout *)
// ----------------------------------------------------------------------------
//   The text flow records things, no display
// ----------------------------------------------------------------------------
{
    charId = 0;
    current = 0;
    lastSplit = NULL;
}


void TextFlow::Clear()
// ----------------------------------------------------------------------------
//   Reset the text flow to the initial setup
// ----------------------------------------------------------------------------
{
    record(justify, "Clear text flow %p", this);
    Layout::Clear();

    assert(reject.size() == 0 || !"TextFlow::Clear called with dirty 'reject'");
    assert(!lastSplit || !"TextFlow::Clear called with dirty 'lastSplit'");

    charId = 0;
    current = 0;
    textBoxIds.clear();
    reject.clear();
}


void TextFlow::ClearCaches()
// ----------------------------------------------------------------------------
//   Drop all cached references to other drawings from this text flow
// ----------------------------------------------------------------------------
{
    record(jutsify, "Clear text flow %p caches", this);

    // Clear caches from rejects
    Drawings r = reject;
    reject.clear();
    for (Drawings::iterator d = reject.begin(); d != reject.end(); d++)
        (*d)->ClearCaches();

    // Clear splits
    if (Drawing *ls = lastSplit)
    {
        lastSplit = NULL;
        ls->ClearCaches();
    }

    // We need to restart from the beginning
    current = 0;
    Layout::ClearCaches();
}


bool TextFlow::Paginate(PageLayout *page)
// ----------------------------------------------------------------------------
//   Paginate all elements in the text flow
// ----------------------------------------------------------------------------
//   This scenario happens if you put a text box definition inside a text flow
{
    record(justify, "Paginate text flow %p in page %p", this, page);
    XL::Save<TextFlow *> saveFlow(page->currentFlow, this);

    // Since text flows and pages can refresh independently, each one need
    // to notify the other if it is refreshed, in order to clear cached data
    // that may be deleted by the refresh.
    this->CachesInfoFrom(page);
    page->CachesInfoFrom(this);

    bool ok = true;

    // First try to playback items that were rejected before
    Drawings rej = reject;
    reject.clear();

    Drawings::iterator d;
    for (d = rej.begin(); ok && d != rej.end(); d++)
    {
        Drawing *child = *d;
        ok = child->Paginate(page);
    }
    reject.insert(reject.end(), d, rej.end());

    uint max = items.size();
    while (ok && current < max)
    {
        Drawing *child = items[current];
        ok = child->Paginate(page);
        if (ok)
            current++;
    }
    return ok;
}


void TextFlow::Transfer(LayoutLine *line)
// ----------------------------------------------------------------------------
//   Transfer to a text flow the items that don't fit in a text box
// ----------------------------------------------------------------------------
{
    typedef Justifier<Drawing *> DJ;
    DJ::Places &places = line->line.places;
    record(justify, "Transfer layout %p lines %p (%u places)",
           this, lines, places.size());
    for(DJ::PlacesIterator p = places.begin(); p != places.end(); p++)
        reject.push_back((*p).item);
    places.clear();
}


void TextFlow::SetLastSplit(TextSplit *split)
// ----------------------------------------------------------------------------
//   Set last paginated split
// ----------------------------------------------------------------------------
{
    lastSplit = split;
}



// ============================================================================
//
//    TextFlowReplay : Replay a given text flow in current layout
//
// ============================================================================

void TextFlowReplay::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the given text flow
// ----------------------------------------------------------------------------
{
    Widget *display = where->Display();
    if (TextFlow *flow = display->pageLayoutFlow(flowName))
    {
        if (PageLayout *page = dynamic_cast<PageLayout *> (where))
        {
            XL::Save<TextFlow *> saveFlow(page->currentFlow, flow);
            flow->Layout::Draw(where);
        }
        else
        {
            flow->Layout::Draw(where);
        }
    }
}


void TextFlowReplay::DrawSelection(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the selection for the given text flow
// ----------------------------------------------------------------------------
{
    Widget *display = where->Display();
    if (TextFlow *flow = display->pageLayoutFlow(flowName))
    {
        if (PageLayout *page = dynamic_cast<PageLayout *> (where))
        {
            XL::Save<TextFlow *> saveFlow(page->currentFlow, flow);
            flow->Layout::DrawSelection(where);
        }
        else
        {
            flow->Layout::DrawSelection(where);
        }
    }
}


void TextFlowReplay::Identify(Layout *where)
// ----------------------------------------------------------------------------
//   Identify the selection for a given text flow
// ----------------------------------------------------------------------------
{
     Widget *display = where->Display();
    if (TextFlow *flow = display->pageLayoutFlow(flowName))
    {
        if (PageLayout *page = dynamic_cast<PageLayout *> (where))
        {
            XL::Save<TextFlow *> saveFlow(page->currentFlow, flow);
            flow->Layout::Identify(where);
        }
        else
        {
            flow->Layout::Identify(where);
        }
    }
}


bool TextFlowReplay::Paginate(PageLayout *page)
// ----------------------------------------------------------------------------
//   Paginate the original text flow
// ----------------------------------------------------------------------------
{
    Widget *display = page->Display();
    if (TextFlow *flow = display->pageLayoutFlow(flowName))
    {
        XL::Save<TextFlow *> saveFlow(page->currentFlow, flow);
        return flow->Paginate(page);
    }
    return true;
}



// ============================================================================
//
//    Text Span: Save and restore text states
//
// ============================================================================

void TextSpan::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw items, then restore the state as it was initially
// ----------------------------------------------------------------------------
{
    if (where)
    {
        state.InheritState(where);
        Vector3 offset = where->offset;
        Layout::Draw(where);
        where->InheritState(&state);
        where->offset = offset;
    }
    else
    {
        Layout::Draw(where);
    }
}


void TextSpan::DrawSelection(Layout *where)
// ----------------------------------------------------------------------------
//   Draw selection, then restore the state as it was initially
// ----------------------------------------------------------------------------
{
    if (where)
    {
        state.InheritState(where);
        Vector3 offset = where->offset;
        Layout::DrawSelection(where);
        where->InheritState(&state);
        where->offset = offset;
    }
    else
    {
        Layout::DrawSelection(where);
    }
}


void TextSpan::Identify(Layout *where)
// ----------------------------------------------------------------------------
//   Identify selection, then restore the state as it was initially
// ----------------------------------------------------------------------------
{
    if (where)
    {
        state.InheritState(where);
        Vector3 offset = where->offset;
        Layout::Identify(where);
        where->InheritState(&state);
        where->offset = offset;
    }
    else
    {
        Layout::Identify(where);
    }
}


bool TextSpan::Paginate(PageLayout *page)
// ----------------------------------------------------------------------------
//   Paginate, then restore the state as it was initially
// ----------------------------------------------------------------------------
{
    Save *save;
    if (restore)
    {
        save = restore->saved;
    }
    else
    {
        save = new Save;
        restore = new Restore(save);
    }

    bool ok = (page->PaginateItem(save) &&
               Layout::Paginate(page) &&
               page->PaginateItem(restore));

    return ok;
}


void TextSpan::Save::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Save the attributes state
// ----------------------------------------------------------------------------
{
    InheritState(where);

    // Save GL state
    save = GL.Save();
}


Box3 TextSpan::Save::Space(Layout *where)
// ----------------------------------------------------------------------------
//   Save the attributes state except GL state and return no dimension
// ----------------------------------------------------------------------------
{
    InheritState(where);

    return Box3();
}


void TextSpan::Restore::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Restore the attributes state
// ----------------------------------------------------------------------------
{
    Vector3 offset = where->offset;
    where->InheritState(saved);
    where->offset = offset;

    // Restore GL state
    GL.Restore(saved->save);
}


Box3 TextSpan::Restore::Space(Layout *where)
// ----------------------------------------------------------------------------
//   Restore the attributes state except GL state and return no dimension
// ----------------------------------------------------------------------------
{
    Vector3 offset = where->offset;
    where->InheritState(saved);
    where->offset = offset;

    return Box3();
}



// ============================================================================
//
//    AnchorLayout: A layout used to anchor shapes in another layout
//
// ============================================================================

AnchorLayout::AnchorLayout(Widget *widget, bool abs)
// ----------------------------------------------------------------------------
//   Create an anchor layout
// ----------------------------------------------------------------------------
    : Layout(widget), absolute(abs)
{
    record(justify, "Anchor layout %p in widget %p", this, widget);
}


AnchorLayout::AnchorLayout(const AnchorLayout &o)
// ----------------------------------------------------------------------------
//   Create a copy of an anchor layout
// ----------------------------------------------------------------------------
    : Layout(o), absolute(false)
{
    record(justify, "Anchor layout %p from %p", this, &o);
}


AnchorLayout::~AnchorLayout()
// ----------------------------------------------------------------------------
//   Delete the layout
// ----------------------------------------------------------------------------
{
    record(justify, "Delete anchor layout %p", this);
}


void AnchorLayout::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw all the children
// ----------------------------------------------------------------------------
{
    record(justify, "Draw anchor layout %p in %p", this, where);
    GLMatrixKeeper saveMatrix;
    if (!where)
    {
        Layout::Draw(where);
        return;
    }

    Vector3 zero(0,0,0);
    Vector3 &o = absolute ? zero : where->offset;
    record(justify, "   offset (%f, %f, %f)", o.x, o.y, o.z);
    GL.Translate(o.x, o.y, o.z);
    XL::Save<Vector3> saveOffset(where->offset, Vector3());
    Layout::Draw(where);
}


void AnchorLayout::DrawSelection(Layout *where)
// ----------------------------------------------------------------------------
//   Draw selection for all the children
// ----------------------------------------------------------------------------
{
    GLMatrixKeeper saveMatrix;
    Vector3 &o = where->offset;
    GL.Translate(o.x, o.y, o.z);
    XL::Save<Vector3> saveOffset(where->offset, Vector3());
    return Layout::DrawSelection(where);
}


void AnchorLayout::Identify(Layout *where)
// ----------------------------------------------------------------------------
//   Identify all the children
// ----------------------------------------------------------------------------
{
    GLMatrixKeeper saveMatrix;
    Vector3 &o = where->offset;
    GL.Translate(o.x, o.y, o.z);
    XL::Save<Vector3> saveOffset(where->offset, Vector3());
    Layout::Identify(where);
}


bool AnchorLayout::Paginate(PageLayout *page)
// ----------------------------------------------------------------------------
//    Since the anchor layout is floating, do not paginate its contents
// ----------------------------------------------------------------------------
{
    record(justify, "Paginate anchor layout %p in page %p", this, page);

    // Like any layout, an anchor layout may refresh at its own pace.
    // If the anchor layout is inside a text flow, we need to ensure that
    // the page gets refreshed when the text flow or anchor layout is.
    page->CachesInfoFrom(this);
    this->CachesInfoFrom(page);

    return page->PaginateItem(this);
}


Box3 AnchorLayout::Bounds(Layout *where)
// ----------------------------------------------------------------------------
//   Unlike other layouts, anchors appear "empty" so that content can float
// ----------------------------------------------------------------------------
{
    return Box3(where->offset, Vector3(0,0,0));
}


Box3 AnchorLayout::Space(Layout *where)
// ----------------------------------------------------------------------------
//   Unlike other layouts, anchors appear "empty" so that content can float
// ----------------------------------------------------------------------------
{
    return Box3(where->offset, Vector3(0,0,0));
}

TAO_END
