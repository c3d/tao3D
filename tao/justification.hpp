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
Justifier<Item>::LayoutData::LayoutData(coord start, coord end,
                                        Justification &j)
// ----------------------------------------------------------------------------
//   Build the layout data with correct default values
// ----------------------------------------------------------------------------
    : justify(j),
      start(start),
      end(end),
      pos(start),
      lastSpace(0),
      lastOversize(0),
      numBreaks(0),
      numSolids(0),
      sign(start <= end ? 1 : -1),
      hasRoom(true),
      hardBreak(false)
{ }


template<class Item>
void Justifier<Item>::Clear()
// ----------------------------------------------------------------------------
//   Delete the elements we have moved in places
// ----------------------------------------------------------------------------
{
    PurgeItems();
    places.clear();
    delete data;
    data = NULL;
}


template<class Item>
void Justifier<Item>::BeginLayout(coord start, coord end, Justification &j)
// ----------------------------------------------------------------------------
//   Create data for the given 
// ----------------------------------------------------------------------------
{
    delete data;
    data = new LayoutData(start, end, j);
};


template<class Item>
bool Justifier<Item>::AddItem(Item item, uint count, bool solid,
                              scale size, coord offset, scale lspace,
                              bool hardBreak)
// ----------------------------------------------------------------------------
//   Place item and returns true if it fits, otherwise return false
// ----------------------------------------------------------------------------
{
    // Quick exit if we are already full
    if (!data->hasRoom)
        return false;

    // Shortcuts for elements of data
    Justification &justify      = data->justify;
    coord         &pos          = data->pos;
    coord         &start        = data->start;
    coord         &end          = data->end;
    coord         &lastSpace    = data->lastSpace;
    coord         &lastOversize = data->lastOversize;
    uint          &numItems     = data->numItems;
    uint          &numBreaks    = data->numBreaks;
    uint          &numSolids    = data->numSolids;
    int           &sign         = data->sign;
    bool          &hasRoom      = data->hasRoom;

    // Record interspace for the next line
    scale ispace = interspace;
    if (ispace < justify.before)
        ispace = justify.before;

    // Test the size of what remains
    scale spacing = justify.spacing;
    coord originalSize = size;
    size *= spacing;
    if (size < originalSize + ispace)
        size = originalSize + ispace;
    if (sign * pos + size > sign * end &&
        sign * pos > sign * start)
    {
        // No more place on the current line
        hasRoom = false;
        return false;
    }

    IFTRACE(justify)
        std::cerr << "Justifier::Adjust: Place at "
                  << pos << (sign == 1 ? " + " : "-") << offset
                  << " = " << pos + sign*offset << std::endl;

    coord offPos = pos + sign * offset;
    places.push_back(Place(item, count, solid, size, offPos));
    pos += sign * size;
    lastOversize = size - originalSize;

    // If this was a hard break, we no longer have room
    if (hardBreak)
    {
        hasRoom = false;
        data->hardBreak = true;
    }

    // Count solids, breaks and individual items (e.g. glyphs)
    if (size > 0)
    {
        if (solid)
            numSolids++;
        else
            numBreaks++;
        numItems += count;

        // Record size of last space and interspace
        lastSpace = lspace;
        interspace = justify.after;
    }

    // We were successful inserting that item
    return true;
}


template<class Item>
void Justifier<Item>::EndLayout()
// ----------------------------------------------------------------------------
//   Final positioning of items after we processed all of them
// ----------------------------------------------------------------------------
{
    // Shortcuts for elements of data
    Justification &justify      = data->justify;
    coord         &pos          = data->pos;
    coord         &end          = data->end;
    coord         &lastSpace    = data->lastSpace;
    coord         &lastOversize = data->lastOversize;
    uint          &numItems     = data->numItems;
    uint          &numBreaks    = data->numBreaks;
    uint          &numSolids    = data->numSolids;
    int           &sign         = data->sign;
    bool          &hasRoom      = data->hasRoom;
    bool          &hardBreak    = data->hardBreak;

    // Extra space that we can use for justification
    scale atEnd = sign * (lastSpace + lastOversize);
    scale extra = end - pos + atEnd;

    // Amount of justification
    scale just = extra * justify.amount;

    // If we placed all the items or we had a hard break, partial justify
    if (hasRoom || hardBreak)
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
        IFTRACE(justify)
                std::cerr << "Justifier<Item>::Adjust Place.position change by "
                          << offset << " to "
                          << place.position << std::endl;

        if (place.size > 0)
        {
            if (place.solid)
                offset += atSolid;
            else
                offset += atBreak + atSolid * place.itemCount;
        }
    }

    // UGLY: Preserve value of hasRoom for HadRoom
    interspace = data->hasRoom ? -1.0 : 0.0;

    // We no longer need the transient data
    delete data;
    data = NULL;
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

