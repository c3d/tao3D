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

TAO_BEGIN

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
    Justification(float a = 1.0, float c = 0.0, float s = 0.0, float spc = 1.0)
        : amount(a), centering(c), spread(s), spacing(spc) {}
    float       amount;
    float       centering;
    float       spread;
    float       spacing;
};


template <class Item>
struct Justifier
// ----------------------------------------------------------------------------
//    Object used to layout lines (vertically) or glyphs (horizontally)
// ----------------------------------------------------------------------------
{
    Justifier(): target(0), total(0), justify(), items(), placed() {}
    ~Justifier() {}

    // Add elements to the layout
    void        AddItem(coord size, Item *item);
    void        AddBreak(coord size);

public:
    // Structure recording an item (NULL = break) and its size / position
    struct JustifierItem
    {
        JustifierItem(Item *item, coord size):
            item(item), size(size), position(0) {}
        bool    IsBreak()       { return item == NULL; }

        Item *  item;
        coord   size;
        coord   position;
    };
    typedef std::vector<JustifierItem> JustifierItems;
    typedef typename JustifierItems::iterator JustifierItemsIterator;

public:
    // Compute the layout, return number of items
    coord       TotalSize() { return total; }
    JustifierItems &Compute();
    JustifierItems &Compute(coord target, const Justification &justify)
    {
        this->target = target;
        this->justify = justify;
        return Compute();
    }

public:
    coord          target;      // Target size for the layout
    coord          total;       // Size of remaining elements
    Justification  justify;     // Justification for the layout
    JustifierItems items;       // Items remaining to layout
    JustifierItems placed;      // Items placed by last Compute()
};



// ============================================================================
// 
//    Template members implementation
// 
// ============================================================================

template<class Item>
void Justifier<Item>::AddItem(coord size, Item *item)
// ----------------------------------------------------------------------------
//    Add an item to the layout
// ----------------------------------------------------------------------------
{
    items.push_back(JustifierItem(item, size));
    total += size;
}


template<class Item>
void Justifier<Item>::AddBreak(coord size)
// ----------------------------------------------------------------------------
//    Add a break to the layout (identified with NULL pointer)
// ----------------------------------------------------------------------------
{
    items.push_back(JustifierItem(NULL, size));
    total += size;
}


template<class Item>
typename Justifier<Item>::JustifierItems &Justifier<Item>::Compute()
// ----------------------------------------------------------------------------
//    Place elements until we reach the target size
// ----------------------------------------------------------------------------
{
    JustifierItemsIterator i;
    JustifierItemsIterator lastBreak = items.begin();
    coord               size = 0.0;
    coord               sizeAtBreak = -1.0;
    bool                isBreak = false;
    uint                numBreaks = 0;
    uint                numNonBreaks = 0;
    uint                numNonBreaksBeforeLastBreak = 0;

    // Find last break before we reach target size
    for (i = items.begin(); i != items.end(); i++)
    {
        // Record the last break we encountered
        isBreak = (*i).IsBreak();
        if (isBreak)
        {
            lastBreak = i;
            sizeAtBreak = size;
            numBreaks++;
            numNonBreaksBeforeLastBreak = numNonBreaks;
        }
        else
        {
            numNonBreaks++;
        }

        // Add size 
        double next = size + (*i).size;
        if (next >= target)
            break;
        size = next;
    }

    // Compute the actual size of the items to place
    if (sizeAtBreak < 0.0)
    {
        sizeAtBreak = size;
        lastBreak = i;
    }
    if (!numBreaks)
    {
        numBreaks = 1;
        numNonBreaksBeforeLastBreak = numNonBreaks;
    }
    numNonBreaks = numNonBreaksBeforeLastBreak;
    if (!numNonBreaks)
    {
        numNonBreaks = 1;
    }

    // Compute the extra space that we can use for justification
    coord extra = (target - sizeAtBreak) * justify.amount;

    // Compute the offset we will use for centering
    coord offset = (target - extra) * justify.centering;

    // Allocate that extra space between breaks and non breaks
    coord forNonBreaks = justify.spread * extra;
    coord forBreaks = extra - forNonBreaks;

    // And allocate to individual items
    coord atNonBreak = forNonBreaks / numNonBreaks;
    coord atBreak = forBreaks / numBreaks;

    // Copy and place the individual elements
    for (i = items.begin(); i != lastBreak; i++)
    {
        JustifierItem item = *i;
        item.position = offset;
        offset += size + (item.IsBreak() ? atBreak : atNonBreak);
        placed.push_back(item);
    }

    // Remove elements from items.
    items.erase(items.first(), lastBreak);

    // Return the placed elements
    return placed;
}

TAO_END

#endif // LAYOUT_H
