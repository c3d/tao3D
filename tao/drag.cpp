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
#include "widget.h"
#include <QtDebug>

TAO_BEGIN

Drag::Drag(Widget *w)
// ----------------------------------------------------------------------------
//   Initialize the drag object
// ----------------------------------------------------------------------------
    : Activity("Drag", w)
{
    widget->setDragging(true);
}


Drag::~Drag()
// ----------------------------------------------------------------------------
//   End of drag
// ----------------------------------------------------------------------------
{
    widget->setDragging(false);
}


Activity *Drag::Click(uint button, uint count, int x, int y)
// ----------------------------------------------------------------------------
//   Initial and final click when dragging an object
// ----------------------------------------------------------------------------
{
    if (button & Qt::LeftButton)
    {
        if (count)
        {
            x0 = x1 = x2 = x;
            y0 = y1 = y2 = y;
        }
        else
        {
            Activity *next = this->next;
            widget->handleId = 0;
            delete this;
            return next;
        }
    }

    // Pass it down the chain
    return next;
}


Activity *Drag::MouseMove(int x, int y, bool active)
// ----------------------------------------------------------------------------
//   Save mouse position as it moves
// ----------------------------------------------------------------------------
{
    if (active && !widget->isReadOnly())
    {
        x2 = x;
        y2 = y;
    }
    widget->refresh();

    // Pass it down the chain
    return next;
}


Activity *Drag::Display(void)
// ----------------------------------------------------------------------------
//   Reset delta once per screen refresh
// ----------------------------------------------------------------------------
{
    x1 = x2;
    y1 = y2;
    return next;                // Display following activities
}


Point3 Drag::Origin(coord z)
// ----------------------------------------------------------------------------
//   Return the position where the user first clicked
// ----------------------------------------------------------------------------
{
    return widget->unproject (x0, y0, z);
}


Point3 Drag::Previous(coord z)
// ----------------------------------------------------------------------------
//   Return the position where the mouse was previously
// ----------------------------------------------------------------------------
{
    return widget->unproject (x1, y1, z);
}


Point3 Drag::Current(coord z)
// ----------------------------------------------------------------------------
//   Return the position where the mouse is now
// ----------------------------------------------------------------------------
{
    return widget->unproject (x2, y2, z);
}


Vector3 Drag::Delta()
// ----------------------------------------------------------------------------
//   Return the difference between the last mouse position and current one
// ----------------------------------------------------------------------------
{
    return Current() - Previous();
}


Vector3 Drag::DeltaFromOrigin()
// ----------------------------------------------------------------------------
//   Return the difference between the first mouse position and current one
// ----------------------------------------------------------------------------
{
    return Current() - Origin();
}

TAO_END
