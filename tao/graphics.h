#ifndef GRAPHICS_H
#define GRAPHICS_H
// ****************************************************************************
//  graphics.h                                                      Tao project
// ****************************************************************************
//
//   File Description:
//
//     Build the interface between the XL programming language and Tao
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
// * File       : $RCSFile$
// * Revision   : $Revision$
// * Date       : $Date$
// ****************************************************************************

#include "context.h"



// ============================================================================
//
//   Top level entry point
//
// ============================================================================

// Top-level entry point: enter all basic operations in the context
void EnterGraphics();

// Top-level entry point: reset all global pointers related to graphics
void DeleteGraphics();

#endif // GRAPHICS_H
