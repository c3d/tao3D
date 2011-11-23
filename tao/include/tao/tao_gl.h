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

#include <QtOpenGL>
#ifdef CONFIG_MACOSX 
#  include <OpenGL/glu.h>
#else
   // glu.h included by glew.h
#endif

inline bool glFramebufferIsValid()
// ----------------------------------------------------------------------------
//   Return true if current framebuffer is valid
// ----------------------------------------------------------------------------
//   This is unfortunately needed for bug #1658 on OSX Lion
{
    return glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE;
}

#endif // TAO_GL_H
