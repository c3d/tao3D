#ifndef TAO_GL_H
#define TAO_GL_H
// *****************************************************************************
// tao_gl.h                                                        Tao3D project
// *****************************************************************************
//
// File description:
//
//    Wrapper for OpenGL that adapts it depending on the platform
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
// (C) 2010-2011,2014-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2012, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
// (C) 2011, Soulisse Baptiste <soulisse@polytech.unice.fr>
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

#ifdef CONFIG_LINUX
# ifndef GL_GLEXT_PROTOTYPES
#  define GL_GLEXT_PROTOTYPES 1
# endif
#endif

#include <qglobal.h>

#if QT_VERSION >= 0x050000

# include <qopengl.h>

#  ifndef CONFIG_MINGW
#   define GLEW_OK 0
static inline int glewInit() { return GLEW_OK; }
#  endif

#else

# ifdef CONFIG_MINGW
#  include <tao/GL/glew.h>
# else
#  define GLEW_OK 0
#  define GLEW_VERSION 0
#  define glewGetString(V) "GLEW: not used"
#  define glewGetErrorString(err) "GLEW: not used"
static inline int glewInit() { return GLEW_OK; }
# endif
# if defined(CONFIG_MACOSX)
#  include <OpenGL/glu.h>
#  define glFramebufferTexture glFramebufferTextureEXT
# elif defined (CONFIG_LINUX)
#  include <GL/glu.h>
# else
   // glu.h included by glew.h
# endif

#endif

#endif // TAO_GL_H
