#ifndef ACTIVITY_H
#define ACTIVITY_H
// ****************************************************************************
//  activity.h                                                      XLR project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

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
    virtual void        Display(void);
    virtual bool        Idle(void);
    virtual bool        Key(uint key, bool down);
    virtual bool        Click(uint button, bool down, int x, int y);
    virtual bool        MouseMove(int x, int y, bool active);

public:
    Activity *          next;
    text                name;
    Widget *            widget;
};

TAO_END

#endif // ACTIVITY_H
