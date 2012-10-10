// ****************************************************************************
//  opengl_save.cpp                                                Tao project 
// ****************************************************************************
// 
//   File Description:
// 
//     Code to implement saving and restoring of OpenGL state
// 
// 
// 
// 
// 
// 
// 
// 
// ****************************************************************************
//  (C) 2011 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2011 Taodyne SAS
// ****************************************************************************

#include "opengl_save.h"

TAO_BEGIN

// Declare all the stacks we use
#define GS(type, name)          OpenGLChanges<type> OpenGLSave::name##_changes;
#include "opengl_state.tbl"

TAO_END
