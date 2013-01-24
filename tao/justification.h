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
    Justifier(): places(), data(NULL), interspace(-1) {}
    ~Justifier() { Clear(); }

    // Build the layout
    void        BeginLayout(coord start, coord end, Justification &j);
    bool        AddItem(Item item, uint count = 1,
                        bool pack = true, bool forceBreak = false,
                        scale size = 0, coord offset = 0, scale lastSpace = 0);
    void        EndLayout(float *perSolid, float *perBreak);

    // Adding items to the layout
    bool        Empty()                 { return places.size() == 0; }
    Item        Current()               { return places.back().item; }
    bool        HasRoom()               { return data->hasRoom; }
    bool        HadRoom()               { return interspace < 0.0; /* UGLY */ }

    // Clear the layout
    void        Clear();
    void        PurgeItems() {}         // Specialize to delete 'places' items
    void        PopItem();

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
        { }
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
        int             sign;
        bool            forceBreak;
        bool            hasRoom;
    };

public:
    Places                      places;      // Items placed on the layout
    LayoutData *                data;
    scale                       interspace;
};

TAO_END

#endif // JUSTIFICATION_H




#ifdef JUSTIFICATION_H_IMPLEMENTATION

#include "demangle.h"

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
      numItems(0),
      numBreaks(0),
      sign(start <= end ? 1 : -1),
      forceBreak(false),
      hasRoom(true)
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
void Justifier<Item>::PopItem()
// ----------------------------------------------------------------------------
//    Pop items from the current list
// ----------------------------------------------------------------------------
{
    places.pop_back();
    if (!places.size())
        interspace = data ? data->justify.before : -1;
}


template<class Item>
void Justifier<Item>::BeginLayout(coord start, coord end, Justification &j)
// ----------------------------------------------------------------------------
//   Create data for the given 
// ----------------------------------------------------------------------------
{
    IFTRACE(justify)
        std::cerr << "Justifier[" << this << "]::BeginLayout "
                  << start << " - " << end << "\n";
    delete data;
    data = new LayoutData(start, end, j);
};


template<class Item>
bool Justifier<Item>::AddItem(Item item, uint count,
                              bool solid, bool forceBreak,
                              scale size, coord offset, scale lspace)
// ----------------------------------------------------------------------------
//   Place item and returns true if it fits, otherwise return false
// ----------------------------------------------------------------------------
{
    // Quick exit if we are already full
    if (!data->hasRoom)
        return false;

    IFTRACE(justify)
        std::cerr << "Justifier[" << this << "]::AddItem "
                  << item << ":" << demangle(typeid(*item).name())
                  << " * " << count 
                  << (solid ? " solid " : " break ")
                  << size << " + " << offset << " "
                  << (forceBreak ? "force-break\n" : "\n");

    // Shortcuts for elements of data
    Justification &justify      = data->justify;
    coord         &pos          = data->pos;
    coord         &start        = data->start;
    coord         &end          = data->end;
    coord         &lastSpace    = data->lastSpace;
    coord         &lastOversize = data->lastOversize;
    uint          &numItems     = data->numItems;
    uint          &numBreaks    = data->numBreaks;
    int           &sign         = data->sign;
    bool          &hasRoom      = data->hasRoom;

    // Record interspace for the next line
    scale ispace = 0;
    if (!solid)
    {
        ispace = interspace;
        if (ispace < justify.after)
            ispace = justify.after;
        interspace = justify.before;
    }

    // Test the size of what remains
    scale spacing = justify.spacing;
    coord originalSize = size;
    size *= spacing;
    if (!solid && size < originalSize + ispace)
        size = originalSize + ispace;
    if (sign * pos + originalSize > sign * end &&
        sign * pos > sign * start)
    {
        // No more place on the current line
        hasRoom = false;
        return false;
    }

    IFTRACE(justify)
        std::cerr << "Justifier[" << this << "]: Place at "
                  << pos << (sign == 1 ? " + " : "-") << offset
                  << " = " << pos + sign*offset << std::endl;

    coord offPos = pos + sign * offset;
    places.push_back(Place(item, count, solid, size, offPos));
    pos += sign * size;
    lastOversize = size - originalSize;

    // If this was a hard break, we no longer have room
    if (forceBreak)
    {
        hasRoom = false;
        data->forceBreak = forceBreak;
        if (size <= 0)
            return false;
        else if (!solid)
            numBreaks--;
    }

    // Count solids, breaks and individual items (e.g. glyphs)
    if (size > 0)
    {
        if (!solid)
            numBreaks++;
        numItems += count;

        // Record size of last space
        lastSpace = lspace;
    }

    // We were successful inserting that item
    return true;
}


template<class Item>
void Justifier<Item>::EndLayout(float *perSolid, float *perBreak)
// ----------------------------------------------------------------------------
//   Final positioning of items after we processed all of them
// ----------------------------------------------------------------------------
{
    IFTRACE(justify)
        std::cerr << "Justifier[" << this << "]::EndLayout\n";

    // Shortcuts for elements of data
    Justification &justify      = data->justify;
    coord         &pos          = data->pos;
    coord         &end          = data->end;
    coord         &lastSpace    = data->lastSpace;
    coord         &lastOversize = data->lastOversize;
    uint          &numItems     = data->numItems;
    uint          &numBreaks    = data->numBreaks;
    int           &sign         = data->sign;
    bool          &forceBreak   = data->forceBreak;
    bool          &hasRoom      = data->hasRoom;


    // Extra space that we can use for justification
    scale atEnd = sign * (lastSpace + lastOversize);
    scale extra = end - pos + atEnd;

    // Amount of justification
    scale just = extra * justify.amount;

    // If we placed all the items or we had a hard break, partial justify
    if (hasRoom || forceBreak)
        just = extra * justify.partial;

    // Count last space as a break only if had a non-zero size
    if (lastSpace == 0)
        numBreaks++;

    // Offset we will use for centering
    coord offset = (extra - just) * justify.centering;

    // Allocate extra space between characters
    scale spread = justify.spread;
    coord forSolids = just * spread;
    if ((hasRoom || forceBreak) && numBreaks <= 1)
        forSolids = just;
    coord atSolid   = forSolids / (numItems > 2 ? numItems - 2 : 1);

    // Allocate extra space between breaks
    coord forBreaks = just - forSolids;
    coord atBreak = forBreaks / (numBreaks > 1 ? numBreaks - 1 : 1);

    // Store that for use in the text_drawing routines
    *perSolid = atSolid;
    *perBreak = atBreak;

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
#endif // JUSTIFICATION_H_IMPLEMENTATION
