// ****************************************************************************
//  activity.cpp                                                    Tao project
// ****************************************************************************
// 
//   File Description:
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
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

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

Activity *Activity::Key(uint key, bool down)
// ----------------------------------------------------------------------------
//   The default is to not process keys, leave it to the next activity
// ----------------------------------------------------------------------------
{
    return next;
}


Activity *Activity::Click(uint button, bool down, int x, int y)
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


