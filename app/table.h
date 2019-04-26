#ifndef TABLE_H
#define TABLE_H
// *****************************************************************************
// table.h                                                         Tao3D project
// *****************************************************************************
//
// File description:
//
//    Table-style layout of elements
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
// (C) 2003-2004,2006,2010-2011,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
// (C) 2004, Sébastien Brochet <sebbrochet@sourceforge.net>
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

#include "drawing.h"
#include "layout.h"
#include "tao_tree.h"

TAO_BEGIN

struct Table : Layout
// ----------------------------------------------------------------------------
//    A table-like layout for graphic elements
// ----------------------------------------------------------------------------
{
    Table(Widget *display, Scope *, Real_p x, Real_p y, uint r, uint c);
    ~Table();

    virtual void        Draw(Layout *);
    virtual void        DrawSelection(Layout *);
    virtual void        Evaluate(Layout *);
    virtual void        Identify(Layout *);
    virtual Box3        Bounds(Layout *);
    virtual Box3        Space(Layout *);
    virtual void        Compute(Layout *);
    virtual void        Add (Drawing *d);

    void                NextCell();

public:
    Scope_p                scope;
    Real_p                 x, y;
    uint                   rows, columns;
    uint                   row, column;
    Box                    margins;
    std::vector<scale>     columnWidth;
    std::vector<scale>     rowHeight;
    std::vector<coord>     columnOffset;
    std::vector<coord>     rowOffset;
    Box3                   bounds;
    Box                    cellBox;
    Tree_p                 fill, border;
    TreeList               cellFill, cellBorder;
    Layouts                fills, borders;
};

TAO_END

#endif // TABLE_H
