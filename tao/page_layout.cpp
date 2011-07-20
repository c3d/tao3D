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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "page_layout.h"
#include "attributes.h"
#include "text_drawing.h"
#include "justification.hpp"
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
                                                  uint &size,
                                                  bool &hadBreak,
                                                  bool &hadSep,
                                                  bool &done)
// ----------------------------------------------------------------------------
//   For drawings, we break at word boundaries
// ----------------------------------------------------------------------------
{
    order = Drawing::WordBreak;
    line_t result = item->Break(order, size);
    done = order > Drawing::SentenceBreak;
    hadSep = order > Drawing::WordBreak;
    hadBreak = order != Drawing::NoBreak;
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
    XL::Save<Point3> zeroOffset(l->offset, Point3(0,0,0));
    Box3 space = item->Space(l);
    return -space.Left();
}


template<> inline page_t Justifier<page_t>::Break(page_t line,
                                                  uint &size,
                                                  bool &hadBreak,
                                                  bool &hadSep,
                                                  bool &done)
// ----------------------------------------------------------------------------
//   For lines, we break at line boundaries
// ----------------------------------------------------------------------------
{
    order = Drawing::LineBreak;
    page_t result = line->Break(order, size);
    done = order > Drawing::ParaBreak;
    hadSep = order > Drawing::LineBreak;
    hadBreak = order != Drawing::NoBreak;
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
    XL::Save<Point3> zeroOffset(l->offset, Point3(0,0,0));
    Box3 space = item->Space(l);
    return space.Top();
}



// ============================================================================
//
//    LayoutLine: A single line in a page layout
//
// ============================================================================

LayoutLine::LayoutLine(coord left, coord right, TextFlow *flow)
// ----------------------------------------------------------------------------
//   Create a new line of drawing elements
// ----------------------------------------------------------------------------
    : line(flow->getItems(), flow->getCurrentIterator()), left(left),
    right(right), perSolid(0.0), flow(flow),
    flowRewindPoint(*(flow->getCurrentIterator()))
{
    IFTRACE(justify)
            std::cerr << "##### "<<left <<" ##### new LayoutLine L-R " << this
            <<"##### "<< right <<" #####\n";
}


LayoutLine::LayoutLine(const LayoutLine &o)
// ----------------------------------------------------------------------------
//   Copy a line
// ----------------------------------------------------------------------------
    : Drawing(o), line(o.line), left(o.left), right(o.right), flow(o.flow),
    flowRewindPoint(*(flow->getCurrentIterator()))
{
    IFTRACE(justify)
            std::cerr << "##### "<<left <<" ##### new LayoutLine " << this
            <<"##### "<< right <<" ##### Copy of "<<&o<<"\n";
}


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
    IFTRACE(justify)
            std::cerr << "->LayoutLine:"<<this<<":Draw(Layout * "<<where<<")\n";
    // Compute layout
    Compute(where);

    // Display all items
    LineJustifier::Places &places = line.places;
    LineJustifier::PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        LineJustifier::Place &place = *p;
        Drawing *child = place.item;
        IFTRACE(justify)
                std::cerr << "LayoutLine::Draw child is " << child->getType() << std::endl;

        Layout * ll = dynamic_cast<Layout*>(child);

        if (ll)
            ll->Inherit(where);
        else
            ll = where;
        XL::Save<coord> saveX(ll->offset.x, ll->offset.x+place.position);
        child->Draw(ll);
    }
    IFTRACE(justify)
            std::cerr << "<-LayoutLine:"<<this<<":Draw\n";
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
        XL::Save<coord> saveY(where->offset.x, where->offset.x+place.position);
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
        XL::Save<coord> saveY(where->offset.x, where->offset.x+place.position);
        child->Identify(where);
    }
}


void LayoutLine::RefreshLayouts(Layout::Layouts &out)
// ----------------------------------------------------------------------------
//   Copy all items that are layouts in the children
// ----------------------------------------------------------------------------
{
    LineJustifier::Places &ps = line.places;
    for (LineJustifier::PlacesIterator p = ps.begin(); p != ps.end(); p++)
        if (Layout *layout = dynamic_cast<Layout*>((*p).item))
            out.push_back(layout);
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
        XL::Save<coord> saveY(where->offset.x, where->offset.x+place.position);
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
        XL::Save<coord> saveX(where->offset.x,
                              where->offset.x + place.position);
        Box3 childSpace = child->Space(where);
        result |= childSpace;
    }
    return result;
}


LayoutLine *LayoutLine::Break(BreakOrder &order, uint &size)
// ----------------------------------------------------------------------------
//   Cut a line layout at a line, paragraph or column boundary
// ----------------------------------------------------------------------------
// LayoutLine has been build as a line so nothing to break down.
// Gives back the order and size value computed when created by the Adjust.
{
    order = line.order;
    size = line.numItems;
    return NULL;
}


void LayoutLine::Compute(Layout *layout)
// ----------------------------------------------------------------------------
//   Compute the placement of items on the line, preserving layout state
// ----------------------------------------------------------------------------
{
    IFTRACE(justify)
            std::cerr <<"-> LayoutLine::Compute[" << this << "]\n";
    // If we already computed the placement, re-use that
    if (line.places.size())
    {
        layout->alongX.perSolid = perSolid;
        return;
    }

    // If line start with a Layout, use it instead of the provided one.
    Layout * ll = dynamic_cast<Layout*>(flow->getCurrentElement());
    if (ll)
        ll->Inherit(layout);
    else
        ll = layout;

    if (RevertLayoutState* revert =
        dynamic_cast<RevertLayoutState*>(flow->getCurrentElement()))
        revert->Draw(ll);

    // Position one line of items
    if (left > right) std::swap(left, right);
    line.Adjust(left + ll->left, right - ll->right, ll->alongX, ll);
    perSolid = ll->alongX.perSolid;
    IFTRACE(justify)
    {
        line.Dump("Line justification");
        std::cerr <<"<- LayoutLine::Compute[" << this << "]\n";
    }
}


LayoutLine *LayoutLine::Remaining()
// ----------------------------------------------------------------------------
//   Build a layout line with items that were not processed
// ----------------------------------------------------------------------------
{
    // Check if we are done
    if (flow->atEnd() || line.places.size() == 0)
        return NULL;

    // Transfer remaining items over to new line
    LayoutLine *result = new LayoutLine(*this);

    return result;
}


// ============================================================================
//
//   PageLayout: Layout of a whole page
//
// ============================================================================

PageLayout::PageLayout(Widget *widget, TextFlow *flow)
// ----------------------------------------------------------------------------
//   Create a new layout
// ----------------------------------------------------------------------------
    : Layout(widget),
    space(),
    flow(flow),lines(), current(lines.begin()),
    page(&lines,&current), lastFlowPoint(*(flow->getCurrentIterator()))
{
    Inherit(widget->layout);
    Inherit(flow);
}


PageLayout::PageLayout(const PageLayout &o)
// ----------------------------------------------------------------------------
//   Copy a layout from another layout
// ----------------------------------------------------------------------------
    : Layout(o), space(), flow(o.flow),lines(), current(lines.begin()),
    page(&lines, &current), lastFlowPoint(*(flow->getCurrentIterator()))
{
    Inherit(flow);
    space |= Point3(0,0,0);
}


PageLayout::~PageLayout()
// ----------------------------------------------------------------------------
//    Destroy the page layout
// ----------------------------------------------------------------------------
{
    LayoutLine *ll = NULL;

    for  (Items::iterator  l = lines.begin(); l != lines.end(); l++)
    {
        ll = (*l);
        delete ll;
    }
    lines.clear();
//    std::cerr << "PageLayout::~PageLayout()\n";
}



void PageLayout::AddLine(LayoutLine *d)
// ----------------------------------------------------------------------------
//   Make sure we force a new computation of the justification
// ----------------------------------------------------------------------------
{
    if ( !d) return;

    return lines.push_back(d);
}

void PageLayout::Add(Drawing *d)
// ----------------------------------------------------------------------------
//   Make sure we force a new computation of the justification
// ----------------------------------------------------------------------------
{
    if ( !d) return;
    if (page.places.size())
    {
        // Adding elements after we computed a layout is messy at best
        std::cerr << "WARNING: PageLayout gets new elements after layout\n";
        assert(!"PageLayout gets a new element after layout");
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
    flow->offset = where->Offset();

    // Inherit state from our parent layout if there is one and compute layout
    Compute(flow);

    // Display all items
    PushLayout(this);
    PageJustifier::Places &places = page.places;
    PageJustifier::PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        PageJustifier::Place &place = *p;
        LayoutLine *child = place.item;
        XL::Save<coord> saveY(flow->offset.y, flow->offset.y + place.position);
        child->Draw(flow);
    }
    PopLayout(this);

    if(where)
       where->previousUnits = textureUnits;
}


void PageLayout::DrawSelection(Layout *where)
// ----------------------------------------------------------------------------
//   Recompute layout if necessary and draw selection for all children
// ----------------------------------------------------------------------------
{
    // Remember the initial selection ID
    Widget     *widget   = where->Display();
    TextSelect *sel      = widget->textSelection();
    uint        selected = widget->selected(id);
    GLuint      lineStart, lineEnd;

    flow->offset = where->Offset();
    id = where->id;
    // Inherit state from our parent layout if there is one and compute layout
    Compute(flow);

    // Clear the selection box in case it was set by some previous text
    if (sel)
    {
        sel->selBox.Empty();
        sel->formulaBox.Empty();
        sel->formulaMode = 0;
    }

    // Check if the text layout was opened, if so draw the text box
    if (selected)
    {
        Box3 bounds = space;
        XL::Save<Point3> zeroOffset(flow->offset, Point3(0,0,0));
        if (selected & Widget::CONTAINER_OPENED)
            widget->drawSelection(flow, bounds, "open_textbox", where->id);
        else
            widget->drawSelection(flow, bounds, "selected_textbox", where->id);
    }

    // Display all items
    PushLayout(this);
    PageJustifier::Places &places = page.places;
    PageJustifier::PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        PageJustifier::Place &place = *p;
        LayoutLine *child = place.item;

        if (!sel)
        {
            // No text selection, just draw children directly
            XL::Save<coord> saveY(flow->offset.y, flow->offset.y + place.position);
            child->DrawSelection(flow);
        }
        else
        {
            // Text selection: Draw the selection box
            sel->processLineBreak();
            lineStart = widget->selectionCurrentId();
            XL::Save<coord> saveY(flow->offset.y, flow->offset.y + place.position);
            child->DrawSelection(flow);
            flow->offset.y = saveY.saved;
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
                XL::Save<Point3> zeroOffset(flow->offset, Point3());
                widget->drawSelection(flow, sel->selBox, mode, 0);
                sel->selBox.Empty();
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }

            if (sel->formulaBox.Width() > 0 && sel->formulaBox.Height() > 0)
            {
                glBlendFunc(GL_DST_COLOR, GL_ZERO);
                text mode = "formula_highlight";
                XL::Save<Point3> zeroOffset(flow->offset, Point3());
                widget->drawSelection(flow, sel->formulaBox, mode, 0);
                sel->formulaBox.Empty();
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
        }
    }
    PopLayout(this);

    // Save color and font as necessary for color selectors
    if (widget->selected(where))
        widget->saveSelectionColorAndFont(flow);
}


void PageLayout::Identify(Layout */*where*/)
// ----------------------------------------------------------------------------
//   Identify page elements for OpenGL
// ----------------------------------------------------------------------------
{
    // Inherit state from our parent layout if there is one and compute layout
    Compute(flow);

    // Display all items
    PushLayout(this);
    PageJustifier::Places &places = page.places;
    PageJustifier::PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        PageJustifier::Place &place = *p;
        LayoutLine *child = place.item;
        XL::Save<coord> saveY(flow->offset.y, flow->offset.y + place.position);
        child->Identify(flow);
    }
    PopLayout(this);

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
        XL::Save<coord> saveY(offset.y, offset.y + place.position);
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
    Box3 bounds;
    if (page.places.size())
        bounds =  Bounds(this);
    result |= bounds;
    return result;
}


void PageLayout::RefreshLayouts(Layouts &out)
// ----------------------------------------------------------------------------
//   Refresh all child layouts
// ----------------------------------------------------------------------------
{
    PageJustifier::PlacesIterator p;
    PageJustifier::Places places = page.places;
    for (p = places.begin(); p != places.end(); p++)
        (*p).item->RefreshLayouts(out);

    Layout::RefreshLayouts(out);
}


void PageLayout::Compute(Layout */*where*/)
// ----------------------------------------------------------------------------
//   Layout all elements on the page, preserving layout state
// ----------------------------------------------------------------------------
{
    IFTRACE(justify)
            std::cerr << "->PageLayout::Compute ["<< this <<"]\n";
    if (flow->atEnd() )
    {
        IFTRACE(justify)
                std::cerr << "<-PageLayout::Compute ["<< this <<"] Flow at end\n";
        return;
    }
    // Get attributes from surrounding context
    Inherit(flow);

    // If we already computed the layout,
    // just reuse that and place the flow accordingly
    if (page.places.size())
    {
        flow->rewindFlow(lastFlowPoint);
        IFTRACE(justify)
                std::cerr << "<-PageLayout::Compute ["<< this
                <<"] (already computed)\n";
        return;
    }

    coord top = space.Top() - this->top;
    coord bottom = space.Bottom() + this->bottom;
    if (top < bottom) std::swap(top, bottom);

    // Transfer all items into a single line
    LayoutLine *line = new LayoutLine(space.Left(), space.Right(), flow);
    {
        // Save the font to let the vertical computing work with the
        // original one. BUG#407
        XL::Save<QFont> saveFont(this->font);

        bool  firstElement  = true;
        scale page_height = 0;
        Justification justify = this->alongY;
        // Loop while there are more lines to place
        while (line && page_height < top - bottom)
        {
            // Compute horizontal layout for current line
            line->Compute(this);

            // Compute and check the size
            scale size = line->Space(this).Height();
            size *= justify.spacing;
            if (line->line.order > LineBreak)
            {
                coord additional = justify.before;
                if (!firstElement && additional < justify.after)
                    additional = justify.after;
                if ( firstElement ) firstElement = false;
                size += additional;
            }
            page_height +=  size;
            if (page_height > top - bottom)
            {
                flow->rewindFlow(line->flowRewindPoint);
                break;
            }
            // Remember the line for vertical layout
            AddLine(line);
            // Must stop here
            if (line->line.order > ParaBreak)
                break;
            // Get next line if there is anything left to layout
            line = line->Remaining();
        }
    }
    // Now that we have all lines, do the vertical layout
    current = lines.begin();
    page.Adjust(top, bottom, this->alongY, this);
    lastFlowPoint = *(flow->getCurrentIterator());
    IFTRACE(justify)
        page.Dump("Page justification");

    IFTRACE(justify)
            std::cerr << "<-PageLayout::Compute ["<< this <<"]\n";
}


PageLayout *PageLayout::Remaining()
// ----------------------------------------------------------------------------
//   Build a layout with items that were not processed
// ----------------------------------------------------------------------------
{
    // Check if we are done
    if (flow->atEnd() || page.places.size() == 0)
        return NULL;

    return new PageLayout(*this);
}



// ============================================================================
//
//    PageLayoutOverflow: Overflow for a page layout
//
// ============================================================================

TextFlow::TextFlow(Layout *layout, text flowName)
// ----------------------------------------------------------------------------
//   Create a page layout overflow rectangle
// ----------------------------------------------------------------------------
    : Layout(*layout), flowName(flowName), currentIterator(items.begin())
{
    IFTRACE(justify)
            std::cerr << "TextFlow::TextFlow[" << this
            << "] flowname " << flowName << std::endl;
}


TextFlow::~TextFlow()
// ----------------------------------------------------------------------------
//    If we got a child, then we need to delete it
// ----------------------------------------------------------------------------
{
}

void TextFlow::insertAfterCurrent(Drawing *d)
// ----------------------------------------------------------------------------
//    Inser the drawing as the next element of the current one
// ----------------------------------------------------------------------------
{
    if (items.size() == 0 || currentIterator == items.end())
        items.push_back(d);
    else
    {
        Drawings::iterator next = currentIterator;
        next++;
        items.insert(next, d);
    }
}

Drawing * TextFlow::getCurrentElement()
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
{
    if (! items.size()) return NULL;

    return *currentIterator;
}


bool TextFlow::atEnd()
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
{
    return (currentIterator == items.end()) || (items.empty());
}


void TextFlow::resetIterator()
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
{
    currentIterator = items.begin();
}


void TextFlow::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Check if the original layout has remaining elements
// ----------------------------------------------------------------------------
{
    Inherit(where);
    resetIterator();
    return ;
}


void TextFlow::DrawSelection(Layout *)
// ----------------------------------------------------------------------------
//   Draw the selection
// ----------------------------------------------------------------------------
{
    return;
}


void TextFlow::Identify(Layout *)
// ----------------------------------------------------------------------------
//   Identify elements of the layout
// ----------------------------------------------------------------------------
{
    return;
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
    GLMatrixKeeper saveMatrix;
    if (!where)
    {
        Layout::Draw(where);
        return;
    }

    Vector3 &o = where->offset;
    IFTRACE(justify)
            std::cerr << "AnchorLayout::Draw(Layout *" << where
            << ") translate to " << o <<std::endl;
    glTranslatef(o.x, o.y, o.z);
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
    glTranslatef(o.x, o.y, o.z);
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
    glTranslatef(o.x, o.y, o.z);
    XL::Save<Vector3> saveOffset(where->offset, Vector3());
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
