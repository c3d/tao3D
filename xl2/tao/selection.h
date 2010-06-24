#ifndef SELECTION_H
#define SELECTION_H
// ****************************************************************************
//  selection.h                                                     Tao project
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

struct Identify : Activity
// ----------------------------------------------------------------------------
//   Identify the object under the mouse
// ----------------------------------------------------------------------------
{
    Identify(text t, Widget *w);

//    virtual Activity *  Display(void);
//    virtual Activity *  Idle(void);
//    virtual Activity *  Click(uint button, uint count, int x, int y);
    virtual Activity *  MouseMove(int x, int y, bool active);

    uint whatIsHere(int x, int y);

    Box                 rectangle;
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

};

TAO_END

#endif // SELECTION_H
