// ****************************************************************************
//  binpack.cpp                                                     Tao project
// ****************************************************************************
//
//   File Description:
//
//    Binary packing of textures in a larger texture
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

#include "binpack.h"
#include <cassert>


TAO_BEGIN

// ============================================================================
//
//    Binary Packing structure
//
// ============================================================================

BinPacker::BinPacker(uint width, uint height)
// ----------------------------------------------------------------------------
//   Create a rectangle binary packing
// ----------------------------------------------------------------------------
    : width(width), height(height), top(NULL)
{}


BinPacker::~BinPacker()
// ----------------------------------------------------------------------------
//   Destructor for rectangle binary packing
// ----------------------------------------------------------------------------
{
    delete top;
}


bool BinPacker::Allocate(uint w, uint h, Rect &rect)
// ----------------------------------------------------------------------------
//   Do a binary partition of the rectangle to find a place big enough to fit
// ----------------------------------------------------------------------------
{
    rect.x1 = 0;
    rect.x2 = width;
    rect.y1 = 0;
    rect.y2 = height;
    Node *node = Fit(&top, w, h, rect);
    return node != NULL;
}


BinPacker::Node *BinPacker::Fit(Node **parent, uint w, uint h, Rect &r)
// ----------------------------------------------------------------------------
//    Find a best fit for the rectangle
// ----------------------------------------------------------------------------
//    In the following algorithm, first=NULL indicates first is occupied
{
    // If we got here with a request we can't possibly satisfy, give it up
    uint availW = r.x2 - r.x1;
    uint availH = r.y2 - r.y1;
    if (w > availW || h > availH)
        return NULL;

    // Check if there is a node to explore
    Node *node = *parent;
    if (node)
    {
        // If the remaining area in this node is insufficient, exit now
        if (node->area < w * h)
            return NULL;

        // Compute the position of the split point
        int split = node->split;
        bool horizontal = split >= 0;
        if (!horizontal)
            split = ~split;

        // Compare with what we have to fit
        uint request, avail1, avail2, save1, save2, *v1, *v2;
        if (horizontal)
        {
            request = w;
            avail1 = split - r.x1;
            avail2 = r.x2 - split;
            save1 = r.x1;
            save2 = r.x2;
            v1 = &r.x1;
            v2 = &r.x2;
        }
        else
        {
            request = h;
            avail1 = split - r.y1;
            avail2 = r.y2 - split;
            save1 = r.y1;
            save2 = r.y2;
            v1 = &r.y1;
            v2 = &r.y2;
        }

        // Check if we can't satisfy this request on either side
        if (request > avail1 && request > avail2)
            return NULL;

        // Check if there is potentially enough space in first rectangle
        if (node->first && request <= avail1)
        {
            *v2 = split;
            if (Node *found = Fit(&node->first, w, h, r))
            {
                node->area -= w*h;
                return found;
            }
            *v2 = save2;
        }

        // There was no first (i.e. occupied) or it doesn't fit
        if (request <= avail2)
        {
            *v1 = split;
            if (Node *found = Fit(&node->second, w, h, r))
            {
                node->area -= w*h;
                return found;
            }
            *v1 = save1;
        }

        // Couldn't fit on left nor right... give up
        return NULL;
    }

    // Here, *parent is a NULL node, and we are in big enough empty space
    assert (!node);
    assert (availW >= w && availH >= h);

    // Finish rectangle
    r.y2 = r.y1 + h;
    r.x2 = r.x1 + w;

    // Test if we have an exact width match
    if (w == availW)
    {
        // Split leftover space vertically
        node = new Node(r.y2, false, w * (availH - h));
    }
    else if (h == availH)
    {
        // Split leftover space horizontally
        node = new Node(r.x2, true, h * (availW - w));
    }
    else
    {
        // Try to preserve the largest possible space for future allocations
        uint remaining = availW*availH - w*h;
        
        if (availW - w > availH - h)
        {
            // More horizontal space available, split horizontally first
            node = new Node(r.x2, true, remaining,
                            new Node(r.y2, false, w * (availH-h), NULL, NULL));
        }
        else
        {
            // More vertical space available, split vertically first
            node = new Node(r.y2, false, remaining,
                            new Node(r.x2, true, h * (availW-w), NULL, NULL));
        }
    }

    *parent = node;
    return node;
}


void BinPacker::Resize(uint w, uint h)
// ----------------------------------------------------------------------------
//   Change the size of the allocation space (only to make it larger)
// ----------------------------------------------------------------------------
{
    if (w > width && h > height)
    {
        if (top)
        {
            uint area = top->area;
            uint areaH = (h - height) * width + area;
            uint areaW = (w - width) * h + areaH;
            top = new Node(width, true, areaW,
                           new Node(height, false, areaH, top));
        }
        width = w;
        height = h;                                    
    }
}



// ============================================================================
//
//   Binary packing node
//
// ============================================================================

BinPacker::Node::Node(uint split, bool horizontal, uint area,
                      Node *first, Node *second)
// ----------------------------------------------------------------------------
//   Create a node
// ----------------------------------------------------------------------------
    : split(int(horizontal ? split : ~split)), area(area),
      first(first), second(second)
{}


BinPacker::Node::~Node()
// ----------------------------------------------------------------------------
//   Delete a node and all children
// ----------------------------------------------------------------------------
{
    delete first;
    delete second;
}

TAO_END
