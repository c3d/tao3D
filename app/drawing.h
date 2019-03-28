#ifndef DRAWING_H
#define DRAWING_H
// *****************************************************************************
// drawing.h                                                       Tao3D project
// *****************************************************************************
//
// File description:
//
//      Elements that can be drawn on a 2D layout
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
// (C) 2011-2012, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2010-2011, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2012, Jérôme Forissier <jerome@taodyne.com>
// (C) 2011-2012, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
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

#include "coords3d.h"
#include "tao.h"
#include "tao_tree.h"
#include <vector>


TAO_BEGIN
struct TextureState;
struct Layout;
struct PageLayout;


struct Drawing
// ----------------------------------------------------------------------------
//   Something that can be drawn in a 2D or 3D layout
// ----------------------------------------------------------------------------
//   Draw() draws the shape in the given layout
//   Bounds() returns the untransformed bounding box for the shape
//   Space() returns the untransformed space desired around object
//   For instance, for text, Space() considers font line height, not Bounds()
{
    typedef std::vector<Drawing *>      Drawings;

                        Drawing();
                        Drawing(const Drawing &);
    virtual             ~Drawing();

    virtual void        Draw(Layout *);
    virtual void        DrawSelection(Layout *);
    virtual void        Evaluate(Layout *);
    virtual void        Identify(Layout *);
    virtual Box3        Bounds(Layout *);
    virtual Box3        Space(Layout *);
    virtual Tree *      Source();

    virtual bool        Paginate(PageLayout *page);
    virtual void        Clear();
    virtual void        ClearCaches();
    virtual scale       TrailingSpaceSize(Layout *);
    virtual bool        IsAttribute();
    virtual bool        IsRTL() { return false; }

    static uint count;
};


enum BreakOrder
// ----------------------------------------------------------------------------
//   An enumeration indicating where we can break lines
// ----------------------------------------------------------------------------
{
    NoBreak,
    CharBreak, WordBreak, SentenceBreak, LineBreak, ParaBreak,
    ColumnBreak, PageBreak,
    AnyBreak
};

TAO_END

#endif // DRAWING_H
