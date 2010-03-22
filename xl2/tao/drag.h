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
//   A drag action (typically used to make a shape follow the mouse)
// ----------------------------------------------------------------------------
{
    Drag(Widget *w);

    virtual Activity *  Click(uint button, bool down, int x, int y);
    virtual Activity *  MouseMove(int x, int y, bool active);
    virtual Activity *  Display(void);

    int                 x1;
    int                 y1;
    int                 x2;
    int                 y2;
};

TAO_END

#endif // DRAG_H
