// *****************************************************************************
// formulas.cpp                                                    Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2009-2011,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

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
