#ifndef ACTIVITY_H
#define ACTIVITY_H
// *****************************************************************************
// activity.h                                                      Tao3D project
// *****************************************************************************
//
// File description:
//
//    A temporary interactive activity in the user interface, e.g. selection
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
// (C) 2010,2012, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2012, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
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

#include "base.h"
#include "tao.h"

TAO_BEGIN

struct Widget;

struct Activity
// ----------------------------------------------------------------------------
//  Encapsulate a temporary UI activity, e.g. selection
// ----------------------------------------------------------------------------
{
    Activity(text name, Widget *s);
    virtual ~Activity();

    // Callbacks, return true if this activity 'handled' the event
    virtual Activity *  Display(void);
    virtual Activity *  Idle(void);
    virtual Activity *  Key(text key);
    virtual Activity *  Click(uint button, uint count, int x, int y);
    virtual Activity *  MouseMove(int x, int y, bool active);

public:
    Activity *          next;
    text                name;
    Widget *            widget;
};

TAO_END

#endif // ACTIVITY_H
