#ifndef TAO_GLU_H
#define TAO_GLU_H
// ****************************************************************************
//  tao_glu.h                                                       Tao project
// ****************************************************************************
//
//   File Description:
//
//    Wrapper for GLU that adapts it depending on the platform
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

#ifdef CONFIG_MACOSX
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#endif // TAO_GLU_H
