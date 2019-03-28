#ifndef SPACE_LAYOUT_H
#define SPACE_LAYOUT_H
// *****************************************************************************
// space_layout.h                                                  Tao3D project
// *****************************************************************************
//
// File description:
//
//     Layout objects in 3D space (z-ordering, ...)
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

#include "layout.h"
#include "justification.h"

TAO_BEGIN

struct SpaceLayout : Layout
// ----------------------------------------------------------------------------
//   Layout objects in 3D space
// ----------------------------------------------------------------------------
{
                        SpaceLayout(Widget *widget);
                        ~SpaceLayout();

    virtual Box3        Space(Layout *layout);
    virtual SpaceLayout*NewChild()      { return new SpaceLayout(*this); }

public:
    // Space requested for the layout
    Box3                space;
    Justification       alongX, alongY, alongZ;
};

TAO_END

#endif // SPACE_LAYOUT_H
