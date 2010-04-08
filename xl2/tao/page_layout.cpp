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
#include "gl_keepers.h"

TAO_BEGIN

// ============================================================================
//
//    LayoutLine: A single line in a page layout
//
// ============================================================================

LayoutLine::LayoutLine()
// ----------------------------------------------------------------------------
//   Create a new line of drawing elements
// ----------------------------------------------------------------------------
    : line()
{}


LayoutLine::LayoutLine(const LayoutLine &o)
// ----------------------------------------------------------------------------
//   Copy a line
// ----------------------------------------------------------------------------
    : Drawing(o), line(o.line)
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
    // Save attributes that may be modified by Compute(), as well as offset
    XL::LocalSave<LayoutState> save(*where, *where);

    Compute(where);

    // Display all items
    LineJustifier::Places &places = line.places;
    LineJustifier::PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        LineJustifier::Place &place = *p;
        Drawing *child = place.item;
        where->offset.x = place.position;
        child->Draw(where);
    }
}


void LayoutLine::DrawSelection(Layout *where)
// ----------------------------------------------------------------------------
//   Recompute layout if necessary and draw selection for all children
// ----------------------------------------------------------------------------
//   REVISIT: There is a lot of copy-paste between Draw, DrawSelection, Identify
//   Consider using a pointer-to-member (ugly) or some clever trick?
{
    // Save attributes that may be modified by Compute(), as well as offset
    XL::LocalSave<LayoutState> save(*where, *where);

    Compute(where);

    // Display all items
    LineJustifier::Places &places = line.places;
    LineJustifier::PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        LineJustifier::Place &place = *p;
        Drawing *child = place.item;
        where->offset.x = place.position;
        child->DrawSelection(where);
    }
}


void LayoutLine::Identify(Layout *where)
// ----------------------------------------------------------------------------
//   Identify page elements for OpenGL
// ----------------------------------------------------------------------------
{
    // Save attributes that may be modified by Compute(), as well as offset
    XL::LocalSave<LayoutState> save(*where, *where);

    Compute(where);

    // Display all items
    LineJustifier::Places &places = line.places;
    LineJustifier::PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        LineJustifier::Place &place = *p;
        Drawing *child = place.item;
        where->offset.x = place.position;
        child->Identify(where);
    }
}


Box3 LayoutLine::Bounds()
// ----------------------------------------------------------------------------
//   Return the bounds for the box
// ----------------------------------------------------------------------------
{
    // Loop over all elements, offseting by their position
    Box3 result;
    LineJustifier::Places &places = line.places;
    LineJustifier::PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        LineJustifier::Place &place = *p;
        Drawing *child = place.item;
        Box3 childBounds = child->Bounds();
        childBounds += Vector3(place.position, 0, 0); // Horizontal offset
        result |= childBounds;
    }

    return result;
}


Box3 LayoutLine::Space()
// ----------------------------------------------------------------------------
//   Return the space for the box
// ----------------------------------------------------------------------------
{
    // Loop over all elements, offseting by their position
    Box3 result;
    LineJustifier::Places &places = line.places;
    LineJustifier::PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        LineJustifier::Place &place = *p;
        Drawing *child = place.item;
        Box3 childSpace = child->Space();
        childSpace += Vector3(place.position, 0, 0); // Horizontal offset
        result |= childSpace;
    }

    return result;
}


LayoutLine *LayoutLine::LineBreak()
// ----------------------------------------------------------------------------
//   Cut a line layout at a line or paragraph boundary
// ----------------------------------------------------------------------------
{
    Items::iterator i;
    Items &items = line.items;
    for (i = items.begin(); i != items.end(); i++)
    {
        Drawing *item = *i;
        Drawing *next = item->LineBreak();
        if (next)
        {
            // Keep the current item in this layout
            i++;

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
//   Compute the placement of items on the line
// ----------------------------------------------------------------------------
{
    // If we already computed the placement, re-use that
    if (line.places.size())
        return;

    // Position one line of items
    Box3 space = layout->Space();
    line.Adjust(space.Left(), space.Right(), layout->alongX, layout);
}


LayoutLine *LayoutLine::Remaining()
// ----------------------------------------------------------------------------
//   Build a layout line with items that were not processed
// ----------------------------------------------------------------------------
{
    // Check if we are done
    Items &items = line.items;
    if (items.size() == 0)
        return NULL;

    // Transfer remaining items over to new line
    LayoutLine *result = new LayoutLine(*this);
    result->Add(items.begin(), items.end());
    items.clear();

    return result;
}


void LayoutLine::ApplyAttributes(Layout *layout)
// ----------------------------------------------------------------------------
//   Apply the attributes we find in the line
// ----------------------------------------------------------------------------
{
    LineJustifier::Places &places = line.places;
    LineJustifier::PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        LineJustifier::Place &place = *p;
        Drawing *child = place.item;
        if (child->IsAttribute())
            child->Draw(layout);
    }
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
{
}


PageLayout::PageLayout(const PageLayout &o)
// ----------------------------------------------------------------------------
//   Copy a layout from another layout
// ----------------------------------------------------------------------------
    : Layout(o), space(o.space)
{
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
    // Save attributes that may be modified by Compute(), as well as offset
    XL::LocalSave<LayoutState> save(*where, *where);

    Compute();

    // Inherit state from our parent layout if there is one
    Inherit(where);

    // Display all items
    PageJustifier::Places &places = page.places;
    PageJustifier::PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        PageJustifier::Place &place = *p;
        LayoutLine *child = place.item;
        offset.y = place.position;
        child->Draw(this);
    }
}


void PageLayout::DrawSelection(Layout *where)
// ----------------------------------------------------------------------------
//   Recompute layout if necessary and draw selection for all children
// ----------------------------------------------------------------------------
//   REVISIT: There is a lot of copy-paste between Draw, DrawSelection, Identify
//   Consider using a pointer-to-member (ugly) or some clever trick?
{
    // Save attributes that may be modified by Compute(), as well as offset
    XL::LocalSave<LayoutState> save(*where, *where);

    Compute();

    // Inherit state from our parent layout if there is one
    Inherit(where);

    // Display all items
    PageJustifier::Places &places = page.places;
    PageJustifier::PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        PageJustifier::Place &place = *p;
        LayoutLine *child = place.item;
        offset.y = place.position;
        child->DrawSelection(this);
    }
}


void PageLayout::Identify(Layout *where)
// ----------------------------------------------------------------------------
//   Identify page elements for OpenGL
// ----------------------------------------------------------------------------
{
    // Save attributes that may be modified by Compute(), as well as offset
    XL::LocalSave<LayoutState> save(*where, *where);

    Compute();

    // Inherit state from our parent layout if there is one
    Inherit(where);

    // Display all items
    PageJustifier::Places &places = page.places;
    PageJustifier::PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        PageJustifier::Place &place = *p;
        LayoutLine *child = place.item;
        offset.y = place.position;
        child->Identify(this);
    }
}


Box3 PageLayout::Bounds()
// ----------------------------------------------------------------------------
//   Return the bounds for the page layout
// ----------------------------------------------------------------------------
{
    // Loop over all elements, offseting by their position
    Box3 result;
    PageJustifier::Places &places = page.places;
    PageJustifier::PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        PageJustifier::Place &place = *p;
        Drawing *child = place.item;
        Box3 childBounds = child->Bounds();
        childBounds += Vector3(0, place.position, 0); // Vertical offset
        result |= childBounds;
    }

    return result;
}


Box3 PageLayout::Space()
// ----------------------------------------------------------------------------
//   Return the space for the page layout
// ----------------------------------------------------------------------------
{
    // Loop over all elements, offseting by their position
    Box3 result;
    PageJustifier::Places &places = page.places;
    PageJustifier::PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        PageJustifier::Place &place = *p;
        Drawing *child = place.item;
        Box3 childSpace = child->Space();
        childSpace += Vector3(0, place.position, 0); // Vertical offset
        result |= childSpace;
    }

    return result;
}


void PageLayout::Compute()
// ----------------------------------------------------------------------------
//   Layout all elements on the page in as many lines as necessary
// ----------------------------------------------------------------------------
{
    // If we already computed the layout, just reuse that
    if (page.places.size())
        return;

    // Transfer all items into a single line
    LayoutLine *line = new LayoutLine();
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
    page.Adjust(space.Top(), space.Bottom(), alongY, this);
}

TAO_END
