// ****************************************************************************
//  tao_options.cpp                                               Tao3D project
// ****************************************************************************
//
//   File Description:
//
//
//
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2019 Christophe de Dinechin <christophe@dinechin.org>
//   This software is licensed under the GNU General Public License v3
// ****************************************************************************
//   This file is part of Tao3D.
//
//   Tao3D is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   Foobar is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with Tao3D.  If not, see <https://www.gnu.org/licenses/>.
// ****************************************************************************

#include "tao_options.h"

namespace Tao { namespace Opt {
IntegerOption   syncInterval
("autosync", "Interval for automatic file sync (ms)",      500,   20, 100000);
IntegerOption   saveInterval
("autosave", "Interval for automatic file save (ms)",     1000,  500, 100000);
IntegerOption   commitInterval
("autocommit", "Interval for automatic file commit (ms)", 5000, 1000, 100000);
IntegerOption   pullInterval
("autopull", "Interval for automatic file pull (ms)",     5000, 1000, 100000);
BooleanOption   splash
("splash", "Show splash screen", true);
BooleanOption   transparent
("transparent", "Make window transparent", false);
BooleanOption   windowVisible
("window_visible", "Make window visible", true);
BooleanOption   slideShow
("slideshow", "Open document in slide-show mode (full-screen)", false);
AliasOption     fullScreen("F", slideShow);
BooleanOption   enableModules
("modules", "Enable external modules", true);
IntegerOption   printResolution
("print_dpi", "Select print resolution in dot-per-inch (DPI)", 300, 72, 3600);
TextOption      renderMode
("render", "Offline rendering - Use -render help for info", "");
TextOption      displayMode
("display_mode", "Select display mode - Use -display help for info", "");
BooleanOption   gcThread
("gcthread", "Run the garbage collector in its own thread", true);
#ifndef CFG_NOGIT
BooleanOption   gitRepository
("gitrepository", "Activate git repository for documents", false);
#endif // CFG_NOGIT
BooleanOption   showVersion
("version", "Display version info and exit", false);
BooleanOption   showGLInfo
("glinfo", "Display GL information and exit", false);
TextOption      fontPath
("fontpath", "Path for additional fonts", "");
BooleanOption   textureCache
("texture_cache", "Activate compressed texture cache", false);
}}
