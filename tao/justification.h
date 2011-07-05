#ifndef JUSTIFICATION_H
#define JUSTIFICATION_H
// ****************************************************************************
//  justification.h                                                Tao project
// ****************************************************************************
//
//   File Description:
//
//     Low-level handling of justification tasks
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

#include "coords3d.h"
#include "drawing.h"
#include <list>
#include <vector>
#include <iostream>
#include <Qt>
TAO_BEGIN

struct Layout;

struct Justification
// ----------------------------------------------------------------------------
//   Describes how elements are supposed to be justified
// ----------------------------------------------------------------------------
//   The same structure is used for horizontal and vertical justification.
//   There are three parameters:
//   - 'amount' indicates how much of the total gap we justify.
//     0.0 means packed (no justification), 1.0 means fully justified
//   - 'partial' indicates how much of the gap we justify for a partially
//     filled line. 0.0 means packed, 1.0 means fully justified.
//   - 'center' indicates how the elements should be centered on page.
//     0.0 means on the left or top, 1.0 on the right or bottm, 0.5 on center
//   - 'spread' indicates how much of the justification is between elements.
//     0.0 means all justification on word boundaries, 1.0 all between letters.
//     Vertically, it's lines and paragraphs.
//   - 'spacing' indicates extra amount of space to add around elements
{
    Justification(float amount = 1.0,
                  float partial = 0.0,
                  float center = 0.0,
                  float spread = 0.0,
                  float spacing = 1.0,
                  float before = 0.0,
                  float after = 0.0)
        : amount(amount), partial(partial), centering(center), spread(spread),
          spacing(spacing), before(before), after(after),
          perSolid(0.0), perBreak(0.0) {}
    float       amount;
    float       partial;
    float       centering;
    float       spread;
    float       spacing;
    float       before;
    float       after;
    float       perSolid;
    float       perBreak;

    Qt::Alignment toQtHAlign()
    {
        if (amount >= 0.5)
            return Qt::AlignJustify;
        if (centering <= 0.4)
            return Qt::AlignLeft;
        if (centering <= 0.6)
            return Qt::AlignHCenter;
        return Qt::AlignRight;
    }

    Qt::Alignment toQtVAlign()
    {
        if (centering <= 0.4)
            return Qt::AlignTop;
        if (centering <= 0.6)
            return Qt::AlignVCenter;
        return Qt::AlignBottom;
    }

};

struct TextFlow;

template <class Item>
struct Justifier
// ----------------------------------------------------------------------------
//    Object used to layout lines (vertically) or glyphs (horizontally)
// ----------------------------------------------------------------------------
//    An important invariant for memory management reasons is that items are
//    either in items or in places but not both. Furthermore, if a drawing
//    is broken up (e.g. by LineBreak), both elements are tracked in either
//    items or places.
{
    typedef std::list<Item>           Items;
    typedef typename Items::iterator  ItemsIterator;

public:
    Justifier(Items *flow, ItemsIterator *start):
            items(flow), item_start(start), places(), numItems(0), order(Drawing::NoBreak) {}
    Justifier(const Justifier &j):
            items(j.items), item_start(j.item_start), places(), numItems(0), order(Drawing::NoBreak) {}
    ~Justifier() { Clear(); }

    // Position items in the layout
    bool        Adjust(coord start, coord end, Justification &j, Layout *l);
    bool        CutUp(coord &start, coord end, Justification &j, Layout *l);
    bool        Placing(coord pos, coord end, Justification &j, Layout *l);

    // Build and clear the layout
    void        Clear();

    // Properties of the items in the layout
    Item        Break(Item item, uint &size,
                      bool &hadBreak, bool &hadSep, bool &done);
    scale       Size(Item item, Layout *);
    scale       SpaceSize(Item item, Layout *);
    coord       ItemOffset(Item item, Layout *);
    void        InsertNext(Item next);
    void        Dump(text msg);

    // Structure recording an item after we placed it
    struct Place
    {
        Place(Item item, uint itemCount = 0,
              scale size = 0, coord pos = 0, bool solid=true)
            : size(size), position(pos),
              item(item), itemCount(itemCount),
              solid(solid)
        {}
        scale   size;
        coord   position;
        Item    item;
        uint    itemCount;
        bool    solid;
    };
    typedef std::vector<Place>          Places;
    typedef typename Places::iterator   PlacesIterator;

public:
    Items         * items;  // List of items to be placed.
    ItemsIterator * item_start;// Iterator on items to be placed (e.g. broken)
    Places         places;   // Items placed on the layout
    uint           numItems;
    Drawing::BreakOrder order;
};

TAO_END

#endif // JUSTIFICATION_H
