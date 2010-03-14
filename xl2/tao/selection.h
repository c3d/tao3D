#ifndef SELECTION_H
#define SELECTION_H
// ****************************************************************************
//  selection.h                                                     XLR project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "activity.h"
#include "coords3d.h"

TAO_BEGIN

struct Selection : Activity
// ----------------------------------------------------------------------------
//   A selection (represented by a selection rectangle)
// ----------------------------------------------------------------------------
{
    Selection(Widget *w);

    virtual void        Display(void);
    virtual bool        Idle(void);
    virtual bool        Click(uint button, bool down, int x, int y);
    virtual bool        MouseMove(int x, int y, bool active);

    Box                 rectangle;
};

TAO_END

#endif // SELECTION_H
