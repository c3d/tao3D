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

#ifndef CONFIG_MACOSX 
#  include <tao/GL/glew.h>
#else
#  define glewInit()
#endif

#include <QGLFormat>
#include <QGLFramebufferObject>
#include <QGLShaderProgram>

#ifdef CONFIG_MACOSX 
#  include <OpenGL/glu.h>
#else
   // glu.h included by glew.h
#endif

#endif // TAO_GL_H
