// ****************************************************************************
//  drag.cpp                                                       Tao project
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

#include "drag.h"
//#include "coords3d.h"
#include <QtDebug>

TAO_BEGIN

Drag::Drag(Widget *w)
// ----------------------------------------------------------------------------
//   Initialize the drag object
// ----------------------------------------------------------------------------
    : Activity("Drag Activity", w)
{}

bool Drag::Click(uint button, bool down, int x, int y)
// ----------------------------------------------------------------------------
//   Initial and final click when dragging an object
// ----------------------------------------------------------------------------
{
    if (button & Qt::LeftButton)
    {
        if (down)
        {
            this->x = x;
            this->y = y;
        }
        else
        {
            delete this;
        }
    }

    return true;
}

bool Drag::MouseMove(int x, int y, bool active)
// ----------------------------------------------------------------------------
//   Save mouse position as it moves
// ----------------------------------------------------------------------------
{
    this->x = x;
    this->y = y;
    return true;
}

TAO_END
