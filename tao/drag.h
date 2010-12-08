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
#include <QtOpenGL>
#include <QMap>

TAO_BEGIN

struct Drag : Activity
// ----------------------------------------------------------------------------
//   A drag action (typically used to make a shape follow the mouse)
// ----------------------------------------------------------------------------
{
    Drag(Widget *w);
    ~Drag();

    virtual Activity *  Click(uint button, uint count, int x, int y);
    virtual Activity *  MouseMove(int x, int y, bool active);
    virtual Activity *  Display(void);

    Point3              Origin(coord z = 0);
    Point3              Previous(coord z = 0);
    Point3              Current(coord z = 0);
    Vector3             Delta();
    Vector3             DeltaFromOrigin();

public:
    int                 x0, y0;
    int                 x1, y1;
    int                 x2, y2;
    QMap<GLuint, coord> w0, h0;  // Width/height of shapes being resized
};

TAO_END

#endif // DRAG_H
