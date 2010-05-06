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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jérôme Forissier <jerome@taodyne.com>
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
#include <iostream>


using namespace XL;

namespace TaoFormulas
{
#include "opcodes_declare.h"
#include "formulas.tbl"


void EnterFormulas(XL::Symbols *c)
// ----------------------------------------------------------------------------
//   Enter all the basic operations defined in basics.tbl
// ----------------------------------------------------------------------------
{
    XL::Compiler *compiler = XL::Context::context->compiler;
#define XL_SCOPE "xl_formulas_"
#include "opcodes_define.h"
#include "formulas.tbl"
}
}
