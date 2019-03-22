#ifndef PAGE_LAYOUT_H
#define PAGE_LAYOUT_H
// *****************************************************************************
// page_layout.h                                                   Tao3D project
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
// (C) 2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2010-2013, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2014, Jérôme Forissier <jerome@taodyne.com>
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
    virtual void        ClearCaches();
    virtual Layout *    NewChild();

    virtual void        Compute(Layout *where);

    bool                PaginateItem(Drawing *d,
                                     BreakOrder order = NoBreak,
                                     uint count = 1);
    bool                PaginateLastLine(BreakOrder order);
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
    virtual void        ClearCaches();

    void                Transfer(LayoutLine *line);
    TextSplit *         LastSplit()                     { return lastSplit; }
    void                SetLastSplit(TextSplit *split);

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
    TextFlowReplay(text flowName) : flowName(flowName) {}

public:
    virtual void        Draw(Layout *where);
    virtual void        DrawSelection(Layout *);
    virtual void        Identify(Layout *l);
    virtual bool        Paginate(PageLayout *page);

public:
    text                flowName;
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
    ~TextSpan()         {
        IFTRACE(justify)
            std::cerr << "<->TextSpan::~TextSpan ["<< this
                      << "] \n";
        delete restore;
    }

    virtual void        Draw(Layout *where);
    virtual void        DrawSelection(Layout *);
    virtual void        Identify(Layout *l);
    virtual bool        Paginate(PageLayout *page);

public:
    struct Save : Attribute, LayoutState
    {
        Save() {}
        virtual void        Draw(Layout *where);
        virtual Box3        Bounds(Layout *)    { return Box3(); }
        virtual Box3        Space(Layout *where);
        GraphicSave* save;
    };
    struct Restore : Attribute
    {
        Restore(Save *save) : saved(save)       {}
        ~Restore()                              { delete saved; }
        virtual void        Draw(Layout *where);
        virtual Box3        Bounds(Layout *)    { return Box3(); }
        virtual Box3        Space(Layout *where);
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
                        AnchorLayout(Widget *widget, bool abs = false);
                        AnchorLayout(const AnchorLayout &o);
                        ~AnchorLayout();

    virtual void        Draw(Layout *where);
    virtual void        DrawSelection(Layout *);
    virtual void        Identify(Layout *l);
    virtual bool        Paginate(PageLayout *page);

    virtual Box3        Bounds(Layout *layout);
    virtual Box3        Space(Layout *layout);
    virtual AnchorLayout *NewChild()      { return new AnchorLayout(*this); }
                        bool absolute;
};


// Specializations for Justifier computations
template<> void         Justifier<LayoutLine *>::PurgeItems();
TAO_END

#endif // PAGE_LAYOUT_H
