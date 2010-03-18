#ifndef DRAG_H
#define DRAG_H
// ****************************************************************************
//  drag.h                                                         Tao project
// ****************************************************************************
//
//   File Description:
//
//     An activity to drag widgets
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
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "activity.h"
#include "coords3d.h"

TAO_BEGIN

struct Drag : Activity
// ----------------------------------------------------------------------------
//   A drag action (typically to make a shape follow the mouse)
// ----------------------------------------------------------------------------
{
    Drag(Widget *w);

    virtual bool        Click(uint button, bool down, int x, int y);
    virtual bool        MouseMove(int x, int y, bool active);

    int                 x;
    int                 y;
};

TAO_END

#endif // DRAG_H
