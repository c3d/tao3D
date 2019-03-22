#ifndef SELECTION_H
#define SELECTION_H
// *****************************************************************************
// selection.h                                                     Tao3D project
// *****************************************************************************
//
// File description:
//
//     An activity for selection rectangles and clicks
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
// (C) 2010-2012, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2012, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
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

#include "activity.h"
#include "coords3d.h"
#include <vector>

TAO_BEGIN

struct Identify : Activity
// ----------------------------------------------------------------------------
//   Identify the object under the mouse
// ----------------------------------------------------------------------------
{
    Identify(text t, Widget *w);

public:
    typedef std::vector<uint> id_list;

public:
    uint        ObjectAtPoint(coord x, coord y);
    uint        ObjectInRectangle(const Box &rectangle,
                                  uint *handleId = 0, // Selection handle or 0
                                  uint *charId   = 0, // Selected char or 0
                                  uint *childId  = 0, // Selected child or 0
                                  uint *parentId = 0);// Parent or 0
    int         ObjectsInRectangle(const Box &rectangle, id_list &list);
};


struct MouseFocusTracker : Identify
// ----------------------------------------------------------------------------
//   Track which object gets the focus based on mouse movements
// ----------------------------------------------------------------------------
{
    MouseFocusTracker(Widget *w);
    virtual Activity *  MouseMove(int x, int y, bool active);
    virtual Activity *  Click(uint button, uint count, int x, int y);
    uint previous;
};


struct Selection : Identify
// ----------------------------------------------------------------------------
//   A selection (represented by a selection rectangle)
// ----------------------------------------------------------------------------
{
    Selection(Widget *w);

    virtual Activity *  Display(void);
    virtual Activity *  Idle(void);
    virtual Activity *  Click(uint button, uint count, int x, int y);
    virtual Activity *  MouseMove(int x, int y, bool active);

public:
    typedef std::map<uint, uint> selection_map; // Widget::selection_map
    static bool selectionsMatch(selection_map &s1, selection_map &s2);
    Box                 rectangle;
    selection_map       savedSelection;
};

TAO_END

#endif // SELECTION_H
