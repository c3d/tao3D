#ifndef GROUP_LAYOUT_H
#define GROUP_LAYOUT_H
// *****************************************************************************
// group_layout.h                                                  Tao3D project
// *****************************************************************************
//
// File description:
//
//     A logical group of graphical objetcs (Drawings).
//     A GroupLayout typically contains Shapes or other GroupLayouts.
//     Objects in a group can be selected simultaneously by a single click,
//     and navigation through the group hierarchy is possible with double
//     clicks.
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
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

#include "layout.h"

TAO_BEGIN


struct GroupLayout : Layout
// ----------------------------------------------------------------------------
//   A logical group of Drawings, selectable as a whole or individually
// ----------------------------------------------------------------------------
{
                        GroupLayout(Widget *widget, Tree_p self);
                        GroupLayout(const GroupLayout &o);

    virtual void        DrawSelection(Layout *);
    virtual void        Add (Drawing *d);
    virtual uint        Selected();

protected:
    Tree_p              self;
};

TAO_END

#endif // GROUP_H
