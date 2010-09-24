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
#include "shapes.h"
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
                        LayoutLine(coord left, coord right);
                        LayoutLine(const LayoutLine &o);
                        ~LayoutLine();

    virtual void        Draw(Layout *where);
    virtual void        DrawSelection(Layout *);
    virtual void        Identify(Layout *l);

    virtual Box3        Bounds(Layout *layout);
    virtual Box3        Space(Layout *layout);
    virtual LayoutLine *Break(BreakOrder &order, uint &sz);

    void                Add(Drawing *d);
    void                Add(Items::iterator first, Items::iterator last);
    void                Compute(Layout *where);
    LayoutLine *        Remaining();

public:
    LineJustifier       line;
    coord               left, right, perSolid;
};


struct PageLayout : Layout
// ----------------------------------------------------------------------------
//   A 2D layout specialized for placing text and 2D shapes on pages
// ----------------------------------------------------------------------------
{
    typedef Justifier<LayoutLine *>     PageJustifier;
    typedef std::vector<LayoutLine *>   Items;

public:
                        PageLayout(Widget *widget);
                        PageLayout(const PageLayout &o);
                        ~PageLayout();

    virtual void        Draw(Layout *where);
    virtual void        DrawSelection(Layout *);
    virtual void        Identify(Layout *l);

    virtual void        Add(Drawing *child);
    void                Add(Items::iterator first, Items::iterator last);
    virtual void        Clear();
    virtual Box3        Bounds(Layout *layout);
    virtual Box3        Space(Layout *layout);
    virtual PageLayout *NewChild()      { return new PageLayout(*this); }
    virtual PageLayout *Remaining();

    void                Inherit(Layout *other);
    void                Compute(Layout *where);

public:
    // Space requested for the layout
    Box3                space;
    PageJustifier       page;
};


struct PageLayoutOverflow : PlaceholderRectangle
// ----------------------------------------------------------------------------
//    Draw what remains in a page layout
// ----------------------------------------------------------------------------
{
    PageLayoutOverflow(const Box &bounds, Widget *widget, text flowName);
    ~PageLayoutOverflow();

public:
    bool                HasData(Layout *where);
    virtual void        Draw(Layout *where);
    virtual void        DrawSelection(Layout *);
    virtual void        Identify(Layout *l);

public:
    Widget *            widget;
    text                flowName;
    PageLayout *        child;
};


struct AnchorLayout : Layout
// ----------------------------------------------------------------------------
//   A special layout used to anchor items at a given position
// ----------------------------------------------------------------------------
{
                        AnchorLayout(Widget *widget);
                        AnchorLayout(const AnchorLayout &o);
                        ~AnchorLayout();

    virtual void        Draw(Layout *where);
    virtual void        DrawSelection(Layout *);
    virtual void        Identify(Layout *l);

    virtual Box3        Bounds(Layout *layout);
    virtual Box3        Space(Layout *layout);
    virtual AnchorLayout *NewChild()      { return new AnchorLayout(*this); }
};


// Specializations for Justifier computations
typedef Drawing *       line_t;
template<> line_t       Justifier<line_t>::Break(line_t,
                                                 uint &size,
                                                 bool &hadBreak,
                                                 bool &hadSeps,
                                                 bool &done);
template<> scale        Justifier<line_t>::Size(line_t, Layout *);
template<> scale        Justifier<line_t>::SpaceSize(line_t, Layout *);
template<> coord        Justifier<line_t>::ItemOffset(line_t, Layout *);

typedef LayoutLine *    page_t;
template<> page_t       Justifier<page_t>::Break(page_t,
                                                 uint &size,
                                                 bool &hadBreak,
                                                 bool &hadSeps,
                                                 bool &done);
template<> scale        Justifier<page_t>::Size(page_t, Layout *);
template<> scale        Justifier<page_t>::SpaceSize(page_t, Layout *);
template<> coord        Justifier<page_t>::ItemOffset(page_t, Layout *);

TAO_END

#endif // PAGE_LAYOUT_H