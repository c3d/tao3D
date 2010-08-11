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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "coords3d.h"
#include <vector>
#include <iostream>

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
//   - 'center' indicates how the elements should be centered on page.
//     0.0 means on the left or top, 1.0 on the right or bottm, 0.5 on center
//   - 'spread' indicates how much of the justification is between elements.
//     0.0 means all justification on word boundaries, 1.0 all between letters.
//     Vertically, it's lines and paragraphs.
//   - 'spacing' indicates extra amount of space to add around elements
{
    Justification(float amount = 1.0,
                  float center = 0.0,
                  float spread = 0.0,
                  float spacing = 1.0,
                  float before = 0.0,
                  float after = 0.0)
        : amount(amount), centering(center), spread(spread),
          spacing(spacing), before(before), after(after) {}
    float       amount;
    float       centering;
    float       spread;
    float       spacing;
    float       before;
    float       after;
};


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
    typedef std::vector<Item>           Items;
    typedef typename Items::iterator    ItemsIterator;

public:
    Justifier(): items(), places() {}
    Justifier(const Justifier &): items(), places() {}
    ~Justifier() { Clear(); }

    // Position items in the layout
    bool        Adjust(coord start, coord end, Justification &j, Layout *l);

    // Build and clear the layout
    void        Add(Item item);
    void        Add(ItemsIterator first, ItemsIterator last);
    void        Clear();

    // Properties of the items in the layout
    Item        Break(Item item, bool *hadBreak, bool *hadSep, bool *done);
    scale       Size(Item item, Layout *);
    scale       SpaceSize(Item item, Layout *);
    coord       ItemOffset(Item item, Layout *);

    void        Dump(text msg, Layout *);

    // Structure recording an item after we placed it
    struct Place
    {
        Place(Item item, scale size = 0, coord pos = 0, bool solid=true)
            : size(size), position(pos), item(item), solid(solid) {}
        scale   size;
        coord   position;
        Item    item;
        bool    solid;
    };
    typedef std::vector<Place>          Places;
    typedef typename Places::iterator   PlacesIterator;

public:
    Items         items;        // Items remaining to be placed (e.g. broken)
    Places        places;       // Items placed on the layout
};



// ============================================================================
// 
//    Template members implementation
// 
// ============================================================================

template<class Item>
void Justifier<Item>::Add(Item item)
// ----------------------------------------------------------------------------
//   Add a single item to the justifier
// ----------------------------------------------------------------------------
{
    items.push_back(item);
}


template<class Item>
void Justifier<Item>::Add(ItemsIterator first, ItemsIterator last)
// ----------------------------------------------------------------------------
//   Add a range of items
// ----------------------------------------------------------------------------
{
    items.insert(items.end(), first, last);
}


template<class Item>
void Justifier<Item>::Clear()
// ----------------------------------------------------------------------------
//   Delete the elements we have moved in places
// ----------------------------------------------------------------------------
{
    // Delete items remaining to be placed
    ItemsIterator i;
    for (i = items.begin(); i != items.end(); i++)
        delete (*i);
    items.clear();

    // Delete placed items
    PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
        delete (*p).item;
    places.clear();
}


template<class Item>
bool Justifier<Item>::Adjust(coord start, coord end,
                             Justification &justify, Layout *layout)
// ----------------------------------------------------------------------------
//    Place elements until we reach the target size
// ----------------------------------------------------------------------------
{
    coord pos          = start;
    scale lastSpace    = 0;
    scale lastOversize = 0;
    bool  hasRoom      = true;
    bool  hadBreak     = false;
    bool  hadSeparator = false;
    bool  done         = false; // e.g. line break in a line
    uint  numBreaks    = 0;
    uint  numSolids    = 0;
    int   sign         = start < end ? 1 : -1;
    bool  firstElement = true;

    // Place items until there's none left or we are beyond the max position
    ItemsIterator i;
    for (i = items.begin(); hasRoom && !done && i != items.end(); i++)
    {
        // Get item and break it down into individual unbreakable units
        Item  item = *i;
        while (hasRoom && !done && item)
        {
            // Cut item at the first break point
            Item next = Break(item, &hadBreak, &hadSeparator, &done);

            // Test the size of what remains
            scale size = Size(item, layout);
            coord offset = ItemOffset(item, layout);
            scale spacing = justify.spacing;
            size *= spacing;
            if (hadSeparator)
            {
                coord additional = justify.before;
                if (!firstElement && additional < justify.after)
                    additional = justify.after;
                size += additional;
            }
            if (sign * pos + size > sign * end && sign * pos > sign * start)
            {
                // It doesn't fit, we need to stop here.
                // Erase what we already placed
                items.erase(items.begin(), i+1);

                // Insert the broken down items
                if (next)
                    items.insert(items.begin(), next);
                items.insert(items.begin(), item);

                hasRoom = false;
            }
            else
            {
                // It fits, place it
                hadBreak |= next != NULL;
                places.push_back(Place(item, size, pos+sign*offset, !hadBreak));
                pos += sign * size;
                lastSpace = SpaceSize(item, layout) * spacing;
                lastOversize = size * (spacing-1);
                item = next;
                if (hadBreak)
                    numBreaks++;
                else
                    numSolids++;

                if (size > 0)
                    firstElement = false;

                if (done)
                {
                    items.erase(items.begin(), i+1);
                    if (next)
                        items.insert(items.begin(), next);
                }
            }
        }
    }

    // Extra space that we can use for justification
    coord extra = (end + sign*(lastSpace + lastOversize) - pos);

    // Amount of justification
    coord just = extra * justify.amount;

    // If we have placed all the items, don't justify
    if (hasRoom)
    {
        just = 0;
        if (!done)
            items.clear();
    }
    // If there is no place where we can justify, center instead
    if (numSolids <= 1 && numBreaks <= 1)
        just = 0;
    if (lastSpace == 0)
        numBreaks++;

    // Offset we will use for centering
    coord offset = (extra - just) * justify.centering;

    // Allocate that extra space between breaks and non breaks
    coord forSolids = justify.spread * just;
    coord forBreaks = just - forSolids;

    // And allocate to individual items
    coord atSolid = forSolids / (numSolids > 1 ? numSolids - 1 : 1);
    coord atBreak = forBreaks / (numBreaks > 1 ? numBreaks - 1 : 1);

    // Now perform the adjustment on individual positions
    PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        Place &place = *p;
        place.position += offset;
        if (place.solid)
            offset += atSolid;
        else
            offset += atBreak;
    }

    // Return true if we placed all the items
    return hasRoom;
}


template <class Item>
void Justifier<Item>::Dump(text msg, Layout *l)
// ----------------------------------------------------------------------------
//   Dump the contents of a justifier for debugging purpose
// ----------------------------------------------------------------------------
{
    std::cout << msg << "\n";

    // Dump the items remaining
    ItemsIterator i;
    for (i = items.begin(); i != items.end(); i++)
    {
        Item item = *i;
        std::cout << " I" << i - items.begin() << ": "
                  << item << " (" << Size(item, l) << ")\n";
    }

    // Dump the places remaining
    PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        Place &place = *p;
        std::cout << " P" << p - places.begin() << ": "
                  << place.item << " (" 
                  << place.size << " @ " << place.position
                  << (place.solid ? " solid" : " break") << ")\n";
    }
}

TAO_END

#endif // JUSTIFICATION_H
