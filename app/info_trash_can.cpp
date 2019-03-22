// *****************************************************************************
// info_trash_can.cpp                                              Tao3D project
// *****************************************************************************
//
// File description:
//
//    A container for XL::Info objects. Used for deferred deletion.
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

#include "info_trash_can.h"

TAO_BEGIN

std::vector<XL::Info *> InfoTrashCan::trash;

void InfoTrashCan::Empty()
// ------------------------------------------------------------------------
//   Empty the trash can
// ------------------------------------------------------------------------
{
    while (!trash.empty())
    {
        XL::Info *tmp = trash.back();
        trash.pop_back();
        delete tmp;
    }

}


void InfoTrashCan::DeferredDelete(XL::Info *obj)
// ------------------------------------------------------------------------
//   Mark object for deletion
// ------------------------------------------------------------------------
{
    trash.push_back(obj);
}

TAO_END
