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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
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

public:
                        LayoutLine(coord left, coord right, TextFlow *flow);
                        LayoutLine(const LayoutLine &o);
                        ~LayoutLine();

    virtual void        Draw(Layout *where);
    virtual void        DrawSelection(Layout *);
    virtual void        Identify(Layout *l);
    virtual void        RefreshLayouts(Layout::Layouts &layouts);

    virtual Box3        Bounds(Layout *layout);
    virtual Box3        Space(Layout *layout);
    virtual LayoutLine *Break(BreakOrder &order, uint &sz);

    void                Compute(Layout *where);
    LayoutLine *        Remaining();
    virtual text        getType() { return "LayoutLine";}

public:
    LineJustifier       line;
    coord               left, right, perSolid;
    TextFlow          * flow;
    std::list<Drawing*>::iterator  flowRewindPoint;
};

typedef Justifier<LayoutLine *>     PageJustifier;

struct PageLayout : Layout
// ----------------------------------------------------------------------------
//   A 2D layout specialized for placing text and 2D shapes on pages
// ----------------------------------------------------------------------------
{
    typedef std::list<LayoutLine *>   Items;

public:
                        PageLayout(Widget *widget, TextFlow* flow);
                        PageLayout(const PageLayout &o);
                        ~PageLayout();

    virtual void        Draw(Layout *where);
    virtual void        DrawSelection(Layout *);
    virtual void        Identify(Layout *l);
    virtual void        RefreshLayouts(Layouts &layouts);

    virtual void        Add(Drawing *child);
    virtual void        AddLine(LayoutLine *child);
    virtual void        Clear();
    virtual Box3        Bounds(Layout *layout);
    virtual Box3        Space(Layout *layout);
    virtual PageLayout *NewChild()      { return new PageLayout(*this); }
    virtual PageLayout *Remaining();

    virtual void        Compute(Layout *where);
    virtual text        getType() { return "PageLayout";}

public:
    // Space requested for the layout
    Box3                space;
    TextFlow *          flow;
    Items               lines;
    Items::iterator  current;
    PageJustifier       page;
    Drawings::iterator  lastFlowPoint;

};


struct RevertLayoutState : LayoutState, Attribute
{
    RevertLayoutState(LayoutState &o):LayoutState(o){}
    virtual void  Draw(Layout *where)
    {
        offset = where->Offset();
        where->InheritState(this);
    }

    virtual text getType() { return "RevertLayoutState";}
};




struct TextFlow : Layout
// ----------------------------------------------------------------------------
//    Draw what remains in a page layout
// ----------------------------------------------------------------------------
{
    TextFlow(Layout *l, text flowName);
    ~TextFlow();

public:
    virtual void        Draw(Layout *where);
    virtual void        DrawSelection(Layout *);
    virtual void        Identify(Layout *l);
    virtual text        getType() { return "Textflow";}

    Drawings::iterator * getCurrentIterator() { return &currentIterator;}
    Drawing            * getCurrentElement();
    Drawings::iterator   end()   { return items.end();}
    Drawings           * getItems(){ return &items;}
    void                 resetIterator();
    bool                 atEnd();
    void insertAfterCurrent(Drawing *d);
    void rewindFlow(Drawings::iterator rewindPoint)
    {
        IFTRACE(justify)
                std::cerr << "TextFlow::rewindFlow "<<this<<std::endl;
        currentIterator = rewindPoint;
    }

public:
    text                flowName;
private:
    Drawings::iterator currentIterator;
};


struct BlockLayout : Layout
// ----------------------------------------------------------------------------
//   A 2D layout specialized for isolate text modifications
// ----------------------------------------------------------------------------
{
    BlockLayout(TextFlow *flow):Layout(*flow), flow(flow),
    revert(new RevertLayoutState((*flow)))
    {
        IFTRACE(justify)
                std::cerr << "<->BlockLayout::BlockLayout ["<<this
                <<"] from flow\n ";
    }
    BlockLayout( BlockLayout &o):Layout(o), flow(o.flow),
    revert(new RevertLayoutState(*(o.flow)))
    {
        IFTRACE(justify)
                std::cerr << "<->BlockLayout::BlockLayout ["<<this
                <<"] from BlockLayout " << &o <<std::endl;
    }
    ~BlockLayout()
    {
    }
    virtual void         Add(Drawing *child) { flow->Add(child);}
    virtual text         getType() { return "BlockLayout";}
    RevertLayoutState *  getRevertLayout() {return revert;}

    TextFlow          *flow;
    RevertLayoutState *revert;

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
    virtual text        getType() { return "AnchorLayout";}
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
