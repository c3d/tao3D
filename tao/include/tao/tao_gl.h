#ifndef TAO_GL_H
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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
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
#  define glewInit()
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
