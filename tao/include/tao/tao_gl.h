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
// This file may be used in accordance with the terms and conditions contained
// in the Tao Presentations license agreement, or alternatively, in a signed
// license agreement between you and Taodyne SAS.
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#ifdef CONFIG_LINUX
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif
#endif

#ifdef CONFIG_MINGW
#  include <tao/GL/glew.h>
#else
#  define GLEW_OK 0
#  define GLEW_VERSION 0
#  define glewGetString(V) "GLEW: not used"
#  define glewGetErrorString(err) "GLEW: not used"
static inline int glewInit() { return GLEW_OK; }
#endif

#include <QGLFormat>
#include <QGLFramebufferObject>
#include <QGLShaderProgram>

#if defined(CONFIG_MACOSX)
#  include <OpenGL/glu.h>
#elif defined (CONFIG_LINUX)
#  include <GL/glu.h>
#else
   // glu.h included by glew.h
#endif

#endif // TAO_GL_H
