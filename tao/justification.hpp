#ifndef JUSTIFICATION_HPP
// ****************************************************************************
//  justification.hpp                                               Tao project
// ****************************************************************************
//
//   File Description:
//
//    Implementation of template member functions found in justification.h
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
//  (C) 2010 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "justification.h"
#include "page_layout.h"
TAO_BEGIN

// ============================================================================
//
//    Template members implementation
//
// ============================================================================

template<class Item>
void Justifier<Item>::Clear()
// ----------------------------------------------------------------------------
//   Delete the elements we have moved in places
// ----------------------------------------------------------------------------
{
    // Delete placed items
    places.clear();
}


template<class Item>
bool Justifier<Item>::Adjust(coord start, coord end,
                             Justification &justify, Layout *layout)
// ----------------------------------------------------------------------------
//    Place elements until we reach the target size
// ----------------------------------------------------------------------------
{
    coord pos           = start;
    scale lastSpace     = 0;
    scale lastOversize  = 0;
    bool  hasRoom       = true;
    bool  hadBreak      = false;
    bool  hadSeparator  = false;
    bool  done          = false; // e.g. line break in a line
    uint  numBreaks     = 0;
    uint  numSolids     = 0;
    uint  itemCount     = 0;
    int   sign          = start <= end ? 1 : -1;
    bool  firstElement  = true;

    numItems = 0;

    // Place items until there's none left or we are beyond the max position
    for (/*item_start*/;
                       hasRoom && !done && (*item_start) != items->end();
                       (*item_start)++)
    {

        // Get item and break it down into individual unbreakable units
        Item  item = *(*item_start);
        // Cut item at the first break point
        itemCount = 0;
        Item next = Break(item, itemCount, hadBreak, hadSeparator, done);
        InsertNext(next);

        // Test the size of what remains
        scale size = Size(item, layout);
        scale spacing = justify.spacing;
        coord originalSize = size;
        size *= spacing;
        if (hadSeparator)
        {
            coord additional = justify.before;
            if (!firstElement && additional < justify.after)
                additional = justify.after;
            if ( firstElement ) firstElement = false;
            size += additional;
        }

        if (sign * pos + size > sign * end &&
            sign * pos > sign * start)
        {
            // No more place for the current on the line
            hasRoom = false;
            // Breaking here will prevent the (*item_start)++ to occur.
            // The current item in the flow will be the next to work on.
            break;
        }

        // It fits, place it
        hadBreak |= next != NULL;
        coord offset = ItemOffset(item, layout);
        places.push_back(Place(item, itemCount,
                               size, pos+sign*offset,
                               !hadBreak));
        pos += sign * size;
        lastOversize = size - originalSize;

        if (size > 0)
        {
            if (hadBreak)
                numBreaks++;
            else
                numSolids++;
            numItems += itemCount;
            firstElement = false;
        }
    }
    if (places.size() > 0)
        lastSpace = SpaceSize(places[places.size()-1].item, layout);

    // End of break start of placement.
    // Extra space that we can use for justification
    scale atEnd = sign * (lastSpace + lastOversize);
    scale extra = end - pos + atEnd;

    // Amount of justification
    scale just = extra * justify.amount;

    // If we have placed all the items, don't justify
    if (hasRoom)
        just = extra * justify.partial;

    // If there is no place where we can justify, center instead
    if (numSolids <= 1 && numBreaks <= 1)
        just = 0;
    if (lastSpace == 0)
        numBreaks++;

    // Offset we will use for centering
    coord offset = (extra - just) * justify.centering;

    // Allocate extra space between characters
    scale spread = justify.spread;
    coord forSolids = just * spread;
    coord atSolid   = forSolids / (numItems > 2 ? numItems - 2 : 1);

    // Allocate extra space between breaks
    coord forBreaks = just - forSolids;
    coord atBreak = forBreaks / (numBreaks > 1 ? numBreaks - 1 : 1);

    // Store that for use in the text_drawing routines
    justify.perSolid = atSolid;
    justify.perBreak = atBreak;

    // Now perform the adjustment on individual positions
    PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        Place &place = *p;
        place.position += offset;
        if (place.size > 0)
        {
            if (place.solid)
                offset += atSolid;
            else
                offset += atBreak + atSolid * place.itemCount;
        }
    }
    // Return true if we placed all the items
    return hasRoom;
}


template <class Item>
void  Justifier<Item>::InsertNext(Item next)
// ----------------------------------------------------------------------------
//   Insert in next position but without moving the current iterator
// ----------------------------------------------------------------------------
{
    if (!next) return;

    ItemsIterator it = (*item_start);

    if ( items->size() == 0 || it == items->end())
        items->push_back(next);
    else
    {
        it++;
        items->insert(it, next);
    }
}


template <class Item>
void Justifier<Item>::Dump(text msg)
// ----------------------------------------------------------------------------
//   Dump the contents of a justifier for debugging purpose
// ----------------------------------------------------------------------------
{
    std::cout << msg << "\n";

    // Dump the placed items
    PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        Place &place = *p;
        std::cerr << " P" << p - places.begin() << ": "
                  << place.item << " ("
                  << place.size << " @ " << place.position
                  << (place.solid ? " solid" : " break") << ")\n";
    }
}

TAO_END

#endif // JUSTIFICATION_HPP

