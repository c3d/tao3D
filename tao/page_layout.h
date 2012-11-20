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
#include "attributes.h"


TAO_BEGIN

struct TextFlow;
struct TextSplit;
struct TextSelect;


struct LayoutLine : Drawing
// ----------------------------------------------------------------------------
//   A single line of text in a layout
// ----------------------------------------------------------------------------
//   Unlike a layout, a LayoutLine doesn't own the items it draws.
//   The layout does, and is ultimately responsible for deleting them.
{
    typedef Justifier<Drawing *>        LineJustifier;
    typedef Layout::Drawings            Drawings;

public:
                        LayoutLine(coord left, coord right, Justification &j);
                        LayoutLine(const LayoutLine &o);
                        ~LayoutLine();

    virtual void        Draw(Layout *where);
    virtual void        DrawSelection(Layout *);
    virtual void        Identify(Layout *l);

    virtual Box3        Bounds(Layout *layout);
    virtual Box3        Space(Layout *layout);
    virtual bool        Paginate(PageLayout *);

    void                PerformLayout();

public:
    Box                 bounds;
    LineJustifier       line;
    float               perSolid, perBreak;
};


struct PageLayout : Layout
// ----------------------------------------------------------------------------
//   A 2D layout specialized for placing text and 2D shapes on pages
// ----------------------------------------------------------------------------
{
    typedef Justifier<LayoutLine *>     PageJustifier;

public:
                        PageLayout(Widget *widget);
                        PageLayout(const PageLayout &o);
                        ~PageLayout();

    virtual void        Draw(Layout *where);
    virtual void        DrawSelection(Layout *);
    virtual void        Identify(Layout *l);
    virtual Box3        Bounds(Layout *layout);
    virtual Box3        Space(Layout *layout);
    virtual bool        Paginate(PageLayout *page);

    virtual void        Add(Drawing *child);
    virtual void        Clear();
    virtual PageLayout *NewChild()      { return new PageLayout(*this); }

    virtual void        Compute(Layout *where);

    bool                PaginateItem(Drawing *d,
                                     BreakOrder order = NoBreak,
                                     uint count = 1);
    bool                PaginateLastLine(bool hardBreak, bool hasInterspace);
    void                DrawPlaceholder();
    void                DrawSelectionBox(TextSelect *sel,
                                         Drawing *child,
                                         coord savedY);
    void                SetLastSplit(TextSplit *);
    TextSplit *         LastSplit();

public:
    // Space requested for the layout
    Box                 space, bounds;
    PageJustifier       page;
    TextFlow *          currentFlow;
    uint                selectId; // Selection Id of its englobing layout.
};


struct TextFlow : Layout
// ----------------------------------------------------------------------------
//    Record drawings that can later be displayed in a page 
// ----------------------------------------------------------------------------
{
    TextFlow(Layout *l, text flowName);
    ~TextFlow();

public:
    virtual void        Draw(Layout *where);
    virtual void        DrawSelection(Layout *);
    virtual void        Identify(Layout *l);
    virtual void        Clear();
    virtual bool        Paginate(PageLayout *page);

    void                Transfer(LayoutLine *line);
    TextSplit *         LastSplit()                     { return lastSplit; }
    void                SetLastSplit(TextSplit *split)  { lastSplit = split; }

public:
    text                flowName;
    std::set<uint>      textBoxIds;           // Set of layoutID for selection

private:
    uint                current;
    Drawings            reject;
    TextSplit *         lastSplit;
};


struct TextFlowReplay : Drawing
// ----------------------------------------------------------------------------
//   Redraw a given text layout
// ----------------------------------------------------------------------------
{
    TextFlowReplay(TextFlow *flow) : flow(flow) {}

public:
    virtual void        Draw(Layout *where);
    virtual void        DrawSelection(Layout *);
    virtual void        Identify(Layout *l);
    virtual bool        Paginate(PageLayout *page);

public:
    TextFlow *          flow;
};


struct TextSpan : Layout
// ----------------------------------------------------------------------------
//   A 2D layout specialized for isolate text modifications
// ----------------------------------------------------------------------------
{
    TextSpan(Layout *layout): Layout(*layout), state(*layout), restore(NULL)
    {
        IFTRACE(justify)
            std::cerr << "<->TextSpan::TextSpan ["<< this
                      << "] from layout " << layout << "\n";
    }
    TextSpan(const TextSpan &o): Layout(o), state(o.state), restore(NULL)
    {
        IFTRACE(justify)
            std::cerr << "<->TextSpan::TextSpan ["<< this
                      << "] from TextSpan " << &o << "\n";
    }
    ~TextSpan()         { delete restore;}

    virtual void        Draw(Layout *where);
    virtual void        DrawSelection(Layout *);
    virtual void        Identify(Layout *l);
    virtual bool        Paginate(PageLayout *page);

public:
    struct Save : Attribute, LayoutState
    {
        Save() {}
        virtual void        Draw(Layout *where);
    };
    struct Restore : Attribute
    {
        Restore(Save *save) : saved(save)       {}
        ~Restore()                              { delete saved; }
        virtual void        Draw(Layout *where);
        Save *saved;
    };

public:
    LayoutState         state;
    Restore *           restore;
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
    virtual bool        Paginate(PageLayout *page);

    virtual Box3        Bounds(Layout *layout);
    virtual Box3        Space(Layout *layout);
    virtual AnchorLayout *NewChild()      { return new AnchorLayout(*this); }
};


// Specializations for Justifier computations
template<> void         Justifier<LayoutLine *>::PurgeItems();
TAO_END

#endif // PAGE_LAYOUT_H
