// ****************************************************************************
//  formulas.cpp                                                    Tao project
// ****************************************************************************
//
//   File Description:
//
//     Enter the symbols that are allowed in a formula
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
// Ce logiciel est la propri√©t√© de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 JÈrÙme Forissier <jerome@taodyne.com>
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "opcodes.h"
#include "types.h"
#include "options.h"
#include "runtime.h"
#include "basics.h"
#include "widget.h"
#include "main.h"
#include <iostream>


using namespace XL;

namespace TaoFormulas
{
#define XL_SCOPE "xl_formulas_"
#include "opcodes_declare.h"
#include "formulas.tbl"


void EnterFormulas(XL::Context *context, XL::Symbols *globals)
// ----------------------------------------------------------------------------
//   Enter all the operations defined in formulas.tbl
// ----------------------------------------------------------------------------
{
    XL::Save<XL::Symbols_p> saveGlobals(MAIN->globals, globals);
#include "opcodes_define.h"
#include "formulas.tbl"
}


void DeleteFormulas()
// ----------------------------------------------------------------------------
//   Delete all the global operations defined in formulas.tbl
// ----------------------------------------------------------------------------
{
#include "opcodes_delete.h"
#include "formulas.tbl"
}
}
