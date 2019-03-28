// *****************************************************************************
// space_layout.cpp                                                Tao3D project
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
// (C) 2010,2013, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2011,2013-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
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

#include "space_layout.h"
#include "attributes.h"

TAO_BEGIN

SpaceLayout::SpaceLayout(Widget *widget)
// ----------------------------------------------------------------------------
//   Constructor sets defaults
// ----------------------------------------------------------------------------
    : Layout(widget)
{
    record(justify, "Space layout %p in widget %p", this, widget);
}


SpaceLayout::~SpaceLayout()
// ----------------------------------------------------------------------------
//   Destructor
// ----------------------------------------------------------------------------
{
    record(justify, "Delete space layout %p", this);
}


Box3 SpaceLayout::Space(Layout *layout)
// ----------------------------------------------------------------------------
//   Return the space for the layout
// ----------------------------------------------------------------------------
{
    Box3 result = Bounds(layout);
    result |= space;
    return result;
}

TAO_END
