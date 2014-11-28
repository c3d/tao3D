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

#include <qopengl.h>

#endif // TAO_GL_H
