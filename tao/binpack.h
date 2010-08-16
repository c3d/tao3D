#ifndef BINPACK_H
#define BINPACK_H
// ****************************************************************************
//  binpack.h                                                       Tao project
// ****************************************************************************
//
//   File Description:
//
//    Binary packing of rectangles in a larger rectangle
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

#include "tao.h"
#include "base.h"


TAO_BEGIN

struct BinPacker
// ----------------------------------------------------------------------------
//    An algorithm to allocate rectangles within a rectangle
// ----------------------------------------------------------------------------
{
    BinPacker(uint width, uint height);
    ~BinPacker();

public:
    struct Rect { uint x1, y1, x2, y2; };

public:
    bool        Allocate(uint width, uint height, Rect &rect);
    void        Resize(uint w, uint h);
    uint        Width()                         { return width; }
    uint        Height()                        { return height; }

protected:
    struct Node
    {
        Node(uint split, bool horiz, uint area = 0,
             Node *first = NULL, Node *second = NULL);
        ~Node();
        int     split;  // < 0 for vertical split, >= 0 for horizontal
        uint    area;
        Node *  first;
        Node *  second;
    };

    Node *      Fit(Node **parent, uint w, uint h, Rect &r);
    
protected:
    uint        width;
    uint        height;
    Node *      top;
};

TAO_END

#endif // BINPACK_H
