// ****************************************************************************
//  page_layout.cpp                                                 Tao project
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
#include "attributes.h"
#include "text_drawing.h"
#include "gl_keepers.h"
#include "window.h"
#include <QFontMetrics>
#include <QFont>

TAO_BEGIN

// ============================================================================
//
//   Specializations for the Justifier computations
//
// ============================================================================

template<> inline line_t Justifier<line_t>::Break(line_t item,
                                                  bool *hadBreak,
                                                  bool *hadSep,
                                                  bool *done)
// ----------------------------------------------------------------------------
//   For drawings, we break at word boundaries
// ----------------------------------------------------------------------------
{
    Drawing::BreakOrder order = Drawing::WordBreak;
    line_t result = item->Break(order);
    *done = order > Drawing::SentenceBreak;
    *hadSep = order > Drawing::WordBreak;
    *hadBreak = order != Drawing::NoBreak;
    return result;
}


template<> inline scale Justifier<line_t>::Size(line_t item, Layout *layout)
// ----------------------------------------------------------------------------
//   For drawings, we compute the horizontal size
// ----------------------------------------------------------------------------
{
    Box3 space = item->Space(layout);
    scale result = space.Width();
    if (result < 0)
        result = 0;
    return result;
}


template<> inline scale Justifier<line_t>::SpaceSize(line_t item, Layout * l)
// ----------------------------------------------------------------------------
//   Return the size of the spaces at the end of an item
// ----------------------------------------------------------------------------
{
    return item->TrailingSpaceSize(l);
}


template<> inline coord Justifier<line_t>::ItemOffset(line_t item, Layout *l)
// ----------------------------------------------------------------------------
//   Return the horizontal offset for placing items
// ----------------------------------------------------------------------------
//   Since the bounds are supposed drawn at coordinates (0,0,0),
//   the offset is the opposite of the left of the bounds
{
    XL::LocalSave<Point3> zeroOffset(l->offset, Point3(0,0,0));
    Box3 space = item->Space(l);
    return -space.Left();
}


template<> inline page_t Justifier<page_t>::Break(page_t line,
                                                  bool *hadBreak,
                                                  bool *hadSep,
                                                  bool *done)
// ----------------------------------------------------------------------------
//   For lines, we break at line boundaries
// ----------------------------------------------------------------------------
{
    Drawing::BreakOrder order = Drawing::LineBreak;
    page_t result = line->Break(order);
    *done = order > Drawing::ParaBreak;
    *hadSep = order > Drawing::LineBreak;
    *hadBreak = order != Drawing::NoBreak;
    return result;
}


template<> inline scale Justifier<page_t>::Size(page_t line, Layout *l)
// ----------------------------------------------------------------------------
//   For lines, we compute the vertical size
// ----------------------------------------------------------------------------
{
    Box3 space = line->Space(l);
    scale result = space.Height();
    if (result < 0)
        result = 0;
    return result;
}


template<> inline scale Justifier<page_t>::SpaceSize(page_t, Layout *)
// ----------------------------------------------------------------------------
//   Return the size of a space for the layout
// ----------------------------------------------------------------------------
{
    // REVISIT: Paragraph interspace
    return 0;
}


template<> inline coord Justifier<page_t>::ItemOffset(page_t item, Layout *l)
// ----------------------------------------------------------------------------
//   Return the vertical offset for lines
// ----------------------------------------------------------------------------
//   Since the bounds are supposed to be computed at coordinates (0,0,0),
//   the offset for the top is Top()
{
    XL::LocalSave<Point3> zeroOffset(l->offset, Point3(0,0,0));
    Box3 space = item->Space(l);
    return space.Top();
}



// ============================================================================
//
//    LayoutLine: A single line in a page layout
//
// ============================================================================

LayoutLine::LayoutLine(coord left, coord right)
// ----------------------------------------------------------------------------
//   Create a new line of drawing elements
// ----------------------------------------------------------------------------
    : line(), left(left), right(right)
{}


LayoutLine::LayoutLine(const LayoutLine &o)
// ----------------------------------------------------------------------------
//   Copy a line
// ----------------------------------------------------------------------------
    : Drawing(o), line(o.line), left(o.left), right(o.right)
{}


LayoutLine::~LayoutLine()
// ----------------------------------------------------------------------------
//    Delete the elements we placed (and therefore own)
// ----------------------------------------------------------------------------
{
    line.Clear();
}


void LayoutLine::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Compute line layout and draw the placed elements
// ----------------------------------------------------------------------------
{
    // Compute layout
    Compute(where);

    // Display all items
    LineJustifier::Places &places = line.places;
    LineJustifier::PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        LineJustifier::Place &place = *p;
        Drawing *child = place.item;
        XL::LocalSave<coord> saveY(where->offset.x,
                                   where->offset.x + place.position);
        child->Draw(where);
    }
}


void LayoutLine::DrawSelection(Layout *where)
// ----------------------------------------------------------------------------
//   Recompute layout if necessary and draw selection for all children
// ----------------------------------------------------------------------------
{
    // Compute layout
    Compute(where);

    // Display all items
    LineJustifier::Places &places = line.places;
    LineJustifier::PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        LineJustifier::Place &place = *p;
        Drawing *child = place.item;
        XL::LocalSave<coord> saveY(where->offset.x,
                                   where->offset.x + place.position);
        child->DrawSelection(where);
    }
}


void LayoutLine::Identify(Layout *where)
// ----------------------------------------------------------------------------
//   Identify page elements for OpenGL
// ----------------------------------------------------------------------------
{
    Compute(where);

    // Display all items
    LineJustifier::Places &places = line.places;
    LineJustifier::PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        LineJustifier::Place &place = *p;
        Drawing *child = place.item;
        XL::LocalSave<coord> saveY(where->offset.x,
                                   where->offset.x + place.position);
        child->Identify(where);
    }
}


Box3 LayoutLine::Bounds(Layout *where)
// ----------------------------------------------------------------------------
//   Return the bounds for the box
// ----------------------------------------------------------------------------
{
    // Loop over all elements, offseting by their position
    Box3 result;
    LineJustifier::Places &places = line.places;
    LineJustifier::PlacesIterator p;
    if (places.size() == 0)
        result |= Point3();
    for (p = places.begin(); p != places.end(); p++)
    {
        LineJustifier::Place &place = *p;
        Drawing *child = place.item;
        XL::LocalSave<coord> saveY(where->offset.x,
                                   where->offset.x + place.position);
        Box3 childBounds = child->Bounds(where);
        result |= childBounds;
    }

    return result;
}


Box3 LayoutLine::Space(Layout *where)
// ----------------------------------------------------------------------------
//   Return the space for the box
// ----------------------------------------------------------------------------
{
    // Loop over all elements, offseting by their position
    Box3 result;
    LineJustifier::Places &places = line.places;
    LineJustifier::PlacesIterator p;
    if (places.size() == 0)
        result |= Point3();
    for (p = places.begin(); p != places.end(); p++)
    {
        LineJustifier::Place &place = *p;
        Drawing *child = place.item;
        XL::LocalSave<coord> saveY(where->offset.x,
                                   where->offset.x + place.position);
        Box3 childSpace = child->Space(where);
        result |= childSpace;
    }

    return result;
}


LayoutLine *LayoutLine::Break(BreakOrder &order)
// ----------------------------------------------------------------------------
//   Cut a line layout at a line, paragraph or column boundary
// ----------------------------------------------------------------------------
{
    Items &items = line.items;
    Items::iterator i;
    LineJustifier::Places &places = line.places;
    LineJustifier::PlacesIterator p, pcopy;

    // First loop over items already placed
    if (order >= LineBreak)
    {
        for (p = places.begin(); p != places.end(); p++)
        {
            LineJustifier::Place &place = *p;
            Drawing *item = place.item;
            while (item)
            {
                BreakOrder itemOrder = LineBreak;
                Drawing *next = item->Break(itemOrder);
                if (next || order < itemOrder)
                {
                    order = itemOrder;

                    // Append what remains after line break to the new layout
                    LayoutLine *result = new LayoutLine(*this);
                    if (next)
                        result->Add(next);
                    for (pcopy = p+1; pcopy != places.end(); pcopy++)
                    {
                        LineJustifier::Place &source = *pcopy;
                        result->Add(source.item);
                    }

                    // Also transfers the leftover items
                    result->Add(items.begin(), items.end());
                    items.clear();

                    // Erase what we transferred to the next line
                    places.erase(p+1, places.end());

                    // Return the new line
                    return result;
                } // if (next)

                item = next;
            } // while(item)
        } // for (all places)
    } // if (high order)

    // If not found in placed item, iterates again over leftover items
    for (i = items.begin(); i != items.end(); i++)
    {
        Drawing *item = *i;
        BreakOrder itemOrder = LineBreak;
        Drawing *next = item->Break(itemOrder);
        if (next || itemOrder > Drawing::LineBreak)
        {
            // Keep the current item in this layout
            i++;
            order = itemOrder;

            // Append what remains after the line break to the new layout
            LayoutLine *result = new LayoutLine(*this);
            result->Add(next);
            result->Add(i, items.end());

            // Erase what we transferred to the next line (don't delete twice)
            items.erase(i, items.end());

            // Return the new line
            return result;
        }
    }

    // There was no line break
    return NULL;
}


void LayoutLine::Add(Drawing *item)
// ----------------------------------------------------------------------------
//   Add an element to the line
// ----------------------------------------------------------------------------
{
    if (line.places.size())
    {
        // Adding elements after we computed a layout is messy at best
        std::cerr << "WARNING: LayoutLine gets new element after layout\n";
        line.Clear();
    }

    line.Add(item);
}


void LayoutLine::Add(Items::iterator first, Items::iterator last)
// ----------------------------------------------------------------------------
//   Copy a range of items into the line
// ----------------------------------------------------------------------------
{
    if (line.places.size())
    {
        // Adding elements after we computed a layout is messy at best
        std::cerr << "WARNING: LayoutLine gets new elements after layout\n";
        line.Clear();
    }

    line.Add(first, last);
}


void LayoutLine::Compute(Layout *layout)
// ----------------------------------------------------------------------------
//   Compute the placement of items on the line, preserving layout state
// ----------------------------------------------------------------------------
{
    // If we already computed the placement, re-use that
    if (line.places.size())
        return;

    // Position one line of items
    if (left > right) std::swap(left, right);
    line.Adjust(left + layout->left, right - layout->right,
                layout->alongX, layout);
}


LayoutLine *LayoutLine::Remaining()
// ----------------------------------------------------------------------------
//   Build a layout line with items that were not processed
// ----------------------------------------------------------------------------
{
    // Check if we are done
    Items &items = line.items;
    if (items.size() == 0 || line.places.size() == 0)
        return NULL;

    // Transfer remaining items over to new line
    LayoutLine *result = new LayoutLine(*this);
    result->Add(items.begin(), items.end());
    items.clear();

    return result;
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
    : Layout(widget), space()
{}


PageLayout::PageLayout(const PageLayout &o)
// ----------------------------------------------------------------------------
//   Copy a layout from another layout
// ----------------------------------------------------------------------------
    : Layout(o), space(), page()
{
    space |= Point3(0,0,0);
}


PageLayout::~PageLayout()
// ----------------------------------------------------------------------------
//    Destroy the page layout
// ----------------------------------------------------------------------------
{}


void PageLayout::Add(Drawing *d)
// ----------------------------------------------------------------------------
//   Make sure we force a new computation of the justification
// ----------------------------------------------------------------------------
{
    if (page.places.size())
    {
        // Adding elements after we computed a layout is messy at best
        std::cerr << "WARNING: PageLayout gets new element after layout\n";
        page.Clear();
    }
    return Layout::Add(d);
}


void PageLayout::Add(Items::iterator first, Items::iterator last)
// ----------------------------------------------------------------------------
//   Copy a range of leftover lines into this layout
// ----------------------------------------------------------------------------
{
    if (page.places.size())
    {
        // Adding elements after we computed a layout is messy at best
        std::cerr << "WARNING: PageLayout gets new elements after layout\n";
        page.Clear();
    }

    // Loop over all the lines, and extract their items, adding them back here
    // We need to add them fresh here, because an overflow layout may not
    // have the same dimensions as the original layout, so we need Compute()
    Items::iterator l;
    typedef Justifier<Drawing *> LineJustifier;
    for (l = first; l != last; l++)
    {
        LayoutLine *lp = *l;
        LineJustifier &line = lp->line;

        // Add elements that were originally placed
        LineJustifier::Places &places = line.places;
        LineJustifier::PlacesIterator p;
        for (p = places.begin(); p != places.end(); p++)
        {
            LineJustifier::Place &place = *p;
            items.push_back(place.item);
        }
        places.clear();

        // Then again with leftover if any (normally not that many that late)
        items.insert(items.end(), line.items.begin(), line.items.end());
        line.items.clear();
    }
}


void PageLayout::Clear()
// ----------------------------------------------------------------------------
//   When we clear a page layout, we need to also clear placed items
// ----------------------------------------------------------------------------
{
    page.Clear();
    Layout::Clear();
}


void PageLayout::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw in an enclosing layout after computing the page layout here
// ----------------------------------------------------------------------------
//   This differs from Layout::Draw mostly in that we first compute the layout
//   and then only iterate on the items that were placed, not all items,
//   taking the layout offset from the placed position
{
    // Inherit state from our parent layout if there is one and compute layout
    Compute(where);

    // Display all items
    PushLayout(where);
    PageJustifier::Places &places = page.places;
    PageJustifier::PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        PageJustifier::Place &place = *p;
        LayoutLine *child = place.item;
        XL::LocalSave<coord> saveY(offset.y, offset.y + place.position);
        child->Draw(this);
    }
    PopLayout(where);
}


void PageLayout::DrawSelection(Layout *where)
// ----------------------------------------------------------------------------
//   Recompute layout if necessary and draw selection for all children
// ----------------------------------------------------------------------------
{
    // Remember the initial selection ID
    Widget     *widget  = where->Display();
    TextSelect *sel     = widget->textSelection();
    GLuint      startId = widget->selectionCurrentId();
    GLuint      lineStart, lineEnd;

    // Inherit state from our parent layout if there is one and compute layout
    Compute(where);

    // Clear the selection box in case it was set by some previous text
    if (sel)
    {
        sel->selBox.Empty();
        sel->formulaBox.Empty();
    }

    // Display all items
    PushLayout(where);
    PageJustifier::Places &places = page.places;
    PageJustifier::PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        PageJustifier::Place &place = *p;
        LayoutLine *child = place.item;

        if (!sel)
        {
            // No text selection, just draw children directly
            XL::LocalSave<coord> saveY(offset.y, offset.y + place.position);
            child->DrawSelection(this);
        }
        else
        {
            // Text selection: Draw the selection box
            sel->newLine();
            lineStart = widget->selectionCurrentId();
            XL::LocalSave<coord> saveY(offset.y, offset.y + place.position);
            child->DrawSelection(this);
            offset.y = saveY.saved;
            lineEnd = widget->selectionCurrentId();

            if (sel->selBox.Width() > 0 && sel->selBox.Height() > 0)
            {
                if (PageLayout *pl = dynamic_cast<PageLayout *> (where))
                {
                    if (sel->point != sel->mark)
                    {
                        coord y = sel->selBox.Bottom();
                        if (sel->start()<=lineStart && sel->end()>=lineStart)
                            sel->selBox |= Point3(pl->space.Left(), y, 0);
                        if (sel->end() >= lineEnd && sel->start() <= lineEnd)
                            sel->selBox |= Point3(pl->space.Right(), y, 0);
                    }
                }

                glBlendFunc(GL_DST_COLOR, GL_ZERO);
                text mode = sel->textMode ? "text_selection" : "text_highlight";
                XL::LocalSave<Point3> zeroOffset(where->offset, Point3());
                widget->drawSelection(where, sel->selBox, mode, 0);
                sel->selBox.Empty();
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }

            if (sel->formulaBox.Width() > 0 && sel->formulaBox.Height() > 0)
            {
                glBlendFunc(GL_DST_COLOR, GL_ZERO);
                text mode = "formula_highlight";
                XL::LocalSave<Point3> zeroOffset(where->offset, Point3());
                widget->drawSelection(where, sel->formulaBox, mode, 0);
                sel->formulaBox.Empty();
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
        }
    }
    PopLayout(where);

    // Assign an ID for the page layout itself and draw a rectangle in it
    GLuint endId = widget->selectionCurrentId();
    if (TextSelect *sel = widget->textSelection())
        if (sel->findingLayout)
            if (sel->start() <= endId+1 && sel->end() >= startId)
                widget->select(where->id, Widget::CHARACTER_SELECTED);
    if (widget->selected(where))
        widget->saveSelectionColorAndFont(where);
}


void PageLayout::Identify(Layout *where)
// ----------------------------------------------------------------------------
//   Identify page elements for OpenGL
// ----------------------------------------------------------------------------
{
    // Inherit state from our parent layout if there is one and compute layout
    Compute(where);

    // Display all items
    PushLayout(where);
    PageJustifier::Places &places = page.places;
    PageJustifier::PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        PageJustifier::Place &place = *p;
        LayoutLine *child = place.item;
        XL::LocalSave<coord> saveY(offset.y, offset.y + place.position);
        child->Identify(this);
    }
    PopLayout(where);

    coord x = space.Left(),  y = space.Bottom();
    coord w = space.Width(), h = space.Height();
    coord z = (space.Front() + space.Back()) / 2;
    coord array[4][3] =
    {
        { x,     y,     z },
        { x + w, y,     z },
        { x + w, y + h, z },
        { x,     y + h, z }
    };

    glColor4f(0.2,0.6,1.0,0.1);
    glVertexPointer(3, GL_DOUBLE, 0, array);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_QUADS, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
}


Box3 PageLayout::Bounds(Layout *layout)
// ----------------------------------------------------------------------------
//   Return the bounds for the page layout
// ----------------------------------------------------------------------------
{
    // Inherit from surrounding context and compute layout
    Compute(layout);

    // Loop over all elements, offseting by their position
    Box3 result;
    PageJustifier::Places &places = page.places;
    PageJustifier::PlacesIterator p;
    if (places.size() == 0)
        result |= Point3();
    for (p = places.begin(); p != places.end(); p++)
    {
        PageJustifier::Place &place = *p;
        Drawing *child = place.item;
        XL::LocalSave<coord> saveY(offset.y, offset.y + place.position);
        Box3 childBounds = child->Bounds(this);
        result |= childBounds;
    }

    return result;
}


Box3 PageLayout::Space(Layout *layout)
// ----------------------------------------------------------------------------
//   Return the space for the page layout
// ----------------------------------------------------------------------------
{
    // Inherit context and compute layout
    Compute(layout);

    Box3 result = space;
    if (page.places.size())
        result |= Bounds(this);
    return result;
}


void PageLayout::Inherit(Layout *other)
// ----------------------------------------------------------------------------
//    Make sure we also inherit the surrounding layout's ID
// ----------------------------------------------------------------------------
{
    if (other)
        id = other->id;
    Layout::Inherit(other);
}


void PageLayout::Compute(Layout *where)
// ----------------------------------------------------------------------------
//   Layout all elements on the page, preserving layout state
// ----------------------------------------------------------------------------
{
    // Get attributes from surrounding context
    Inherit(where);

    // If we already computed the layout, just reuse that
    if (page.places.size())
        return;

    // Transfer all items into a single line
    LayoutLine *line = new LayoutLine(space.Left(), space.Right());
    line->Add(items.begin(), items.end());
    items.clear();

    // Loop while there are more lines to place
    while (line)
    {
        // Remember the line for vertical layout
        page.Add(line);

        // Compute horizontal layout for current line
        line->Compute(this);

        // Get next line if there is anything left to layout
        line = line->Remaining();
    }

    // Now that we have all lines, do the vertical layout
    coord top = space.Top() - this->top;
    coord bottom = space.Bottom() + this->bottom;
    if (top < bottom) std::swap(top, bottom);
    page.Adjust(top, bottom, alongY, this);

    // Restore state from surrounding context
    Inherit(where);
}


PageLayout *PageLayout::Remaining()
// ----------------------------------------------------------------------------
//   Build a layout with items that were not processed
// ----------------------------------------------------------------------------
{
    // Check if we are done
    Items &items = page.items;
    if (items.size() == 0 || page.places.size() == 0)
        return NULL;

    // Transfer remaining items over to new line
    PageLayout *result = new PageLayout(*this);
    result->Add(items.begin(), items.end());
    items.clear();

    return result;
}



// ============================================================================
//
//    PageLayoutOverflow: Overflow for a page layout
//
// ============================================================================

PageLayoutOverflow::PageLayoutOverflow(const Box &bounds,
                                       Widget *widget, text flowName)
// ----------------------------------------------------------------------------
//   Create a page layout overflow rectangle
// ----------------------------------------------------------------------------
    : PlaceholderRectangle(bounds),
      widget(widget), flowName(flowName), child(NULL)
{}


PageLayoutOverflow::~PageLayoutOverflow()
// ----------------------------------------------------------------------------
//    If we got a child, then we need to delete it
// ----------------------------------------------------------------------------
{
    delete child;
}


bool PageLayoutOverflow::HasData(Layout *where)
// ----------------------------------------------------------------------------
//    Check if the source has any data to give us
// ----------------------------------------------------------------------------
{
    if (!child)
    {
        PageLayout *&source = widget->pageLayoutFlow(flowName);
        if (source)
        {
            child = source->Remaining();
            source = child;
        }
    }
    if (child)
    {
        Vector3 offset = where->offset;
        where->Inherit(child);
        where->offset = offset;
        child->space = Box3(bounds.lower.x, bounds.lower.y, 0,
                            bounds.Width(), bounds.Height(), 0);
    }
    return child != NULL;
}


void PageLayoutOverflow::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Check if the original layout has remaining elements
// ----------------------------------------------------------------------------
{
    if (HasData(where))
        child->Draw(where);
    else
        PlaceholderRectangle::Draw(where);
}


void PageLayoutOverflow::DrawSelection(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the selection
// ----------------------------------------------------------------------------
{
    if (HasData(where))
        child->DrawSelection(where);
    else
        PlaceholderRectangle::Draw(where);
}


void PageLayoutOverflow::Identify(Layout *where)
// ----------------------------------------------------------------------------
//   Identify elements of the layout
// ----------------------------------------------------------------------------
{
    if (HasData(where))
        child->Identify(where);
    else
        PlaceholderRectangle::Draw(where);
}



// ============================================================================
//
//    AnchorLayout: A layout used to anchor shapes in another layout
//
// ============================================================================

AnchorLayout::AnchorLayout(Widget *widget)
// ----------------------------------------------------------------------------
//   Create an anchor layout
// ----------------------------------------------------------------------------
    : Layout(widget)
{}


AnchorLayout::AnchorLayout(const AnchorLayout &o)
// ----------------------------------------------------------------------------
//   Create a copy of an anchor layout
// ----------------------------------------------------------------------------
    : Layout(o)
{}


AnchorLayout::~AnchorLayout()
// ----------------------------------------------------------------------------
//   Delete the layout
// ----------------------------------------------------------------------------
{}


void AnchorLayout::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw all the children
// ----------------------------------------------------------------------------
{
    Layout::Draw(where);
}


void AnchorLayout::DrawSelection(Layout *where)
// ----------------------------------------------------------------------------
//   Draw selection for all the children
// ----------------------------------------------------------------------------
{
    return Layout::DrawSelection(where);
}


void AnchorLayout::Identify(Layout *where)
// ----------------------------------------------------------------------------
//   Identify all the children
// ----------------------------------------------------------------------------
{
    Layout::Identify(where);
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
