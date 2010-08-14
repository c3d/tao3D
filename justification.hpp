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

TAO_BEGIN

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
    coord pos           = start;
    scale lastSpace     = 0;
    scale lastOversize  = 0;
    scale totalSpace    = 0;
    bool  hasRoom       = true;
    bool  hadBreak      = false;
    bool  hadSeparator  = false;
    bool  done          = false; // e.g. line break in a line
    uint  numBreaks     = 0;
    uint  numSolids     = 0;
    uint  numItems      = 0;
    uint  itemCount     = 0;
    int   sign          = start < end ? 1 : -1;
    bool  firstElement  = true;

    // Place items until there's none left or we are beyond the max position
    ItemsIterator i;
    for (i = items.begin(); hasRoom && !done && i != items.end(); i++)
    {
        // Get item and break it down into individual unbreakable units
        Item  item = *i;
        while (hasRoom && !done && item)
        {
            // Cut item at the first break point
            itemCount = 0;
            Item next = Break(item, itemCount, hadBreak, hadSeparator, done);

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
                totalSpace += lastSpace + lastOversize;
                item = next;

                if (size > 0)
                {
                    if (hadBreak)
                        numBreaks++;
                    else
                        numSolids++;
                    numItems += itemCount;
                    firstElement = false;
                }
  
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
    scale atEnd = sign * (lastSpace + lastOversize);
    scale extra = (end + atEnd - pos);

    // Amount of justification
    scale just = extra * justify.amount;

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

    // Allocate extra space between characters
    coord forSolids = just * justify.spread;
    coord atSolid   = forSolids / (numItems>numBreaks ? numItems-numBreaks : 1);

    // Allocate extra space between breaks
    coord lastSolid = atSolid * itemCount;
    coord forBreaks = just - forSolids + lastSolid + justify.spread * lastSpace;
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
                offset += atBreak;
        }
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

    // Dump the placed items
    PlacesIterator p;
    for (p = places.begin(); p != places.end(); p++)
    {
        Place &place = *p;
        std::cout << " P" << p - places.begin() << ": "
                  << place.item << " ("
                  << place.size << " @ " << place.position
                  << (place.solid ? " solid" : " break") << ")\n";
    }

    // Dump the items remaining
    ItemsIterator i;
    for (i = items.begin(); i != items.end(); i++)
    {
        Item item = *i;
        std::cout << " I" << i - items.begin() << ": "
                  << item << " (" << Size(item, l) << ")\n";
    }

}

TAO_END

#endif // JUSTIFICATION_HPP

