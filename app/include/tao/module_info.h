#ifndef TAO_MODULE_INFO_H
#define TAO_MODULE_INFO_H
// *****************************************************************************
// module_info.h                                                   Tao3D project
// *****************************************************************************
//
// File description:
//
//    Public interface to Tao module information
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
// (C) 2010-2011,2013-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2013, Jérôme Forissier <jerome@taodyne.com>
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

#include <string>

namespace Tao {


struct ModuleInfo
// ------------------------------------------------------------------------
//   Information about a module
// ------------------------------------------------------------------------
{
    ModuleInfo() {}
    ModuleInfo(std::string id, std::string path)
        : id(id), path(path), ver(1.0), autoLoad(false) {}

    std::string id;
    std::string path;  // Qt format (e.g., "/Users/foo", "C:/Users/foo")
    std::string name;
    std::string desc;
    std::string icon;
    double      ver;
    std::string author;
    std::string website;
    std::string importName;
    bool        autoLoad;
    // Message supplied by the module as on_load_error, shown in case of
    // library load error
    std::string onLoadError;
    // Windows: value of %PATH% to be set to load the library (supplied by
    // the module as windows_load_path)
    std::string windowsLoadPath;
};

}

#endif // TAO_MODULE_INFO_H
