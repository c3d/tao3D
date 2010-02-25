// ****************************************************************************
//  graphics.cpp                                                    XLR project
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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************
// * File       : $RCSFile$
// * Revision   : $Revision$
// * Date       : $Date$
// ****************************************************************************


#include "graphics.h"
#include "opcodes.h"
#include "options.h"
#include "widget.h"
#include <iostream>


// ============================================================================
//
//    Top-level operation
//
// ============================================================================

using namespace XL;

#include "opcodes_declare.h"
#include "graphics.tbl"


void EnterGraphics(XL::Context *c)
// ----------------------------------------------------------------------------
//   Enter all the basic operations defined in basics.tbl
// ----------------------------------------------------------------------------
{
    XL::Compiler *compiler = c->compiler;
#include "opcodes_define.h"
#include "graphics.tbl"
}
