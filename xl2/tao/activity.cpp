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


void Activity::Display(void)
// ----------------------------------------------------------------------------
//   The default is to display nothing
// ----------------------------------------------------------------------------
{}


bool Activity::Idle(void)
// ----------------------------------------------------------------------------
//   The default idle callback for an activity is to do nothing
// ----------------------------------------------------------------------------
{
    return false;
}


bool Activity::Key(uint key, int x, int y)
// ----------------------------------------------------------------------------
//   The default is to not process keys, leave it to the next activity
// ----------------------------------------------------------------------------
{
    return false;
}


bool Activity::Click(uint button, bool down, int x, int y)
// ----------------------------------------------------------------------------
//   The default is to not deal with clicks, leave it to the next activity
// ----------------------------------------------------------------------------
{
    return false;
}


bool Activity::MouseMove(int x, int y, bool active)
// ----------------------------------------------------------------------------
//   The default is to not deal with mouse movements, leave it to the next
// ----------------------------------------------------------------------------
{
    return false;
}

TAO_END


