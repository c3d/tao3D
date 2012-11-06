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
    Justifier(): places(), data(NULL), interspace(0) {}
    ~Justifier() { Clear(); }

    // Build the layout
    void        BeginLayout(coord start, coord end, Justification &j);
    bool        AddItem(Item item, uint count = 1, bool solid = true,
                        scale size = 0, coord offset = 0, scale lastSpace = 0,
                        bool hard = false);
    void        EndLayout();

    // Adding items to the layout
    bool        Empty()                 { return places.size() == 0; }
    Item        Current()               { return places.back().item; }
    void        PopItem()               { places.pop_back(); }
    bool        HasRoom()               { return data->hasRoom; }
    bool        HadRoom()               { return interspace < 0.0; /* UGLY */ }

    // Clear the layout
    void        Clear();
    void        PurgeItems() {}         // Specialize to delete 'places' items

    // Debug
    void        Dump(text msg);

public:
    // Structure recording an item after we placed it
    struct Place
    {
        Place(Item item, uint itemCount = 0, bool solid=true,
              scale size = 0, coord pos = 0)
            : item(item), size(size), position(pos),
              itemCount(itemCount),
              solid(solid)
        {}
        Item    item;
        scale   size;
        coord   position;
        uint    itemCount       : 31;
        bool    solid           : 1;
    };
    typedef std::vector<Place>          Places;
    typedef typename Places::iterator   PlacesIterator;

    // Structure used during only during layout
    struct LayoutData
    {
        LayoutData(coord start, coord end, Justification &j);
        Justification  &justify;
        coord           start, end, pos;
        coord           lastSpace;
        coord           lastOversize;
        uint            numItems;
        uint            numBreaks;
        uint            numSolids;
        int             sign;
        bool            hasRoom;
        bool            hardBreak;
    };

public:
    Places                      places;      // Items placed on the layout
    LayoutData *                data;
    scale                       interspace;
};

TAO_END

#endif // JUSTIFICATION_H
