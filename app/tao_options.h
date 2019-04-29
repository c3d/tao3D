#ifndef TAO_OPTIONS_H
#define TAO_OPTIONS_H
// ****************************************************************************
//  tao_options.h                                                 Tao3D project
// ****************************************************************************
//
//   File Description:
//
//     The options specific to the Tao3D application
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

#include "options.h"

namespace Tao {
namespace Opt {

using namespace XL;

extern IntegerOption    syncInterval;
extern IntegerOption    saveInterval;
extern IntegerOption    commitInterval;
extern IntegerOption    pullInterval;
extern BooleanOption    splash;
extern BooleanOption    transparent;
extern BooleanOption    windowVisible;
extern BooleanOption    slideShow;
extern BooleanOption    enableModules;
extern IntegerOption    printResolution;
extern TextOption       renderMode;
extern TextOption       preloadFiles;
extern TextOption       displayMode;
extern BooleanOption    gcThread;
extern BooleanOption    compressTextures;
extern BooleanOption    showVersion;
extern BooleanOption    showGLInfo;
extern TextOption       fontPath;
extern BooleanOption    textureCache;
extern BooleanOption    undoWidget;

extern TextOption       proofOfPlayFile;
extern IntegerOption    proofOfPlayInterval;
extern IntegerOption    proofOfPlayWidth;
extern IntegerOption    proofOfPlayHeight;

#ifndef CFG_NOGIT
extern BooleanOption   gitRepository;
#endif // CFG_NOGIT

}
}

#endif // TAO_OPTIONS_H
