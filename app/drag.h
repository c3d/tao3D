#ifndef DRAG_H
#define DRAG_H
// *****************************************************************************
// drag.h                                                          Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

#include "activity.h"
#include "coords3d.h"
#include "tao_gl.h"
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
