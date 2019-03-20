#ifndef TAO_GL_H
#define TAO_GL_H
// ****************************************************************************
//  tao_gl.h                                                        Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This software is licensed under the GNU General Public License v3
// See file COPYING for details.
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

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
