#ifndef BINPACK_H
#define BINPACK_H
// *****************************************************************************
// binpack.h                                                       Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2013-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

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
    void        Clear();

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
