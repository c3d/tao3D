#ifndef FORMULAS_H
#define FORMULAS_H
// ****************************************************************************
//  formulas.h                                                      Tao project
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

namespace TaoFormulas
{

// Top-level entry point: enter all basic operations in the context
void EnterFormulas(XL::Context *context, XL::Symbols *globals);

// Top-level entry point: reset all global pointers related to formulas
void DeleteFormulas();

}

#endif // FORMULAS_H
