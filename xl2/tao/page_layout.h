#ifndef PAGE_LAYOUT_H
#define PAGE_LAYOUT_H
// ****************************************************************************
//  page_layout.h                                                   Tao project
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

#include "layout.h"
#include "justification.h"

TAO_BEGIN

struct LayoutLine : Drawing
// ----------------------------------------------------------------------------
//   A single line of text in a layout
// ----------------------------------------------------------------------------
//   Unlike a layout, a LayoutLine doesn't own the items it draws.
//   The layout does, and is ultimately responsible for deleting them.
{
    typedef Justifier<Drawing *>        LineJustifier;
    typedef std::vector<Drawing *>      Items;

public:
                        LayoutLine();
                        LayoutLine(const LayoutLine &o);
                        ~LayoutLine();

    virtual void        Draw(Layout *where);
    virtual void        DrawSelection(Layout *);
    virtual void        Identify(Layout *l);

    virtual Box3        Bounds();
    virtual Box3        Space();
    virtual LayoutLine *LineBreak();

    void                Add(Drawing *d);
    void                Add(Items::iterator first, Items::iterator last);
    void                Compute(Layout *where);
    LayoutLine *        Remaining();
    void                ApplyAttributes(Layout *layout);

public:
    LineJustifier       line;
};


struct PageLayout : Layout
// ----------------------------------------------------------------------------
//   A 2D layout specialized for placing text and 2D shapes on pages
// ----------------------------------------------------------------------------
{
    typedef Justifier<LayoutLine *> PageJustifier;

public:
                        PageLayout(Widget *widget);
                        PageLayout(const PageLayout &o);
                        ~PageLayout();

    virtual void        Draw(Layout *where);
    virtual void        DrawSelection(Layout *);
    virtual void        Identify(Layout *l);

    virtual void        Add(Drawing *child);
    virtual void        Clear();
    virtual Box3        Bounds();
    virtual Box3        Space();
    virtual PageLayout *NewChild()      { return new PageLayout(*this); }

    void                Compute();

public:
    // Space requested for the layout
    Box3                space;
    PageJustifier       page;
};


template<> inline Drawing *Justifier<Drawing *>::Break(Drawing *item)
// ----------------------------------------------------------------------------
//   For drawings, we break at word boundaries
// ----------------------------------------------------------------------------
{
    return item->WordBreak();
}


template<> inline scale Justifier<Drawing *>::Size(Drawing *item)
// ----------------------------------------------------------------------------
//   For drawings, we compute the horizontal size
// ----------------------------------------------------------------------------
{
    return item->Space().Width();
}


template<> inline void Justifier<Drawing *>::
ApplyAttributes(Drawing *item, Layout *layout)
// ----------------------------------------------------------------------------
//   For drawings, we compute the horizontal size
// ----------------------------------------------------------------------------
{
    if (item->IsAttribute())
        item->Draw(layout);
}


template<> inline LayoutLine *Justifier<LayoutLine *>::Break(LayoutLine *layout)
// ----------------------------------------------------------------------------
//   For lines, we break at line boundaries
// ----------------------------------------------------------------------------
{
    return layout->LineBreak();
}


template<> inline scale Justifier<LayoutLine *>::Size(LayoutLine *item)
// ----------------------------------------------------------------------------
//   For lines, we compute the vertical size
// ----------------------------------------------------------------------------
{
    return item->Space().Height();
}


template<> inline void Justifier<LayoutLine *>::
ApplyAttributes(LayoutLine *item, Layout *layout)
// ----------------------------------------------------------------------------
//   For drawings, we compute the horizontal size
// ----------------------------------------------------------------------------
{
    item->ApplyAttributes(layout);
}


TAO_END

#endif // PAGE_LAYOUT_H
