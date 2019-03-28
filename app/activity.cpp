// *****************************************************************************
// activity.cpp                                                    Tao3D project
// *****************************************************************************
//
// File description:
//
//     An interactive activity tracked by the Tao system
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
// (C) 2010,2012,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2012, Jérôme Forissier <jerome@taodyne.com>
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

#include "activity.h"
#include "widget.h"
#include <iostream>


TAO_BEGIN

Activity::Activity(text n, Widget *widget)
// ----------------------------------------------------------------------------
//   Create an activity and link it in the given state
// ----------------------------------------------------------------------------
    : next(widget->activities), name(n), widget(widget)
{
    widget->activities = this;
}


Activity::~Activity()
// ----------------------------------------------------------------------------
//   Unlink the activity from the state
// ----------------------------------------------------------------------------
{
    Activity *prev = NULL;
    for (Activity *a = widget->activities; a; a = a->next)
    {
        if (a == this)
        {
            if (prev)
                prev->next = next;
            else
                widget->activities = next;
            break;
        }
        prev = a;
    }
}


Activity *Activity::Display(void)
// ----------------------------------------------------------------------------
//   The default is to display the next activity
// ----------------------------------------------------------------------------
{
    return next;
}


Activity * Activity::Idle(void)
// ----------------------------------------------------------------------------
//   Call the next idle activity
// ----------------------------------------------------------------------------
{
    return next;
}


#pragma GCC diagnostic ignored "-Wunused-parameter"
Activity *Activity::Key(text key)
// ----------------------------------------------------------------------------
//   The default is to not process keys, leave it to the next activity
// ----------------------------------------------------------------------------
{
    return next;
}


Activity *Activity::Click(uint button, uint count, int x, int y)
// ----------------------------------------------------------------------------
//   The default is to not deal with clicks, leave it to the next activity
// ----------------------------------------------------------------------------
{
    return next;
}


Activity *Activity::MouseMove(int x, int y, bool active)
// ----------------------------------------------------------------------------
//   The default is to not deal with mouse movements, leave it to the next
// ----------------------------------------------------------------------------
{
    return next;
}

TAO_END
