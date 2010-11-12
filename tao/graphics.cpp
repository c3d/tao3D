// ****************************************************************************
//  graphics.cpp                                                    Tao project
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
//  (C) 2010 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "graphics.h"
#include "opcodes.h"
#include "options.h"
#include "widget.h"
#include "types.h"
#include "drawing.h"
#include "module_manager.h"
#include <iostream>


// ============================================================================
//
//    Top-level operation
//
// ============================================================================

using namespace XL;

#include "opcodes_declare.h"
#include "graphics.tbl"


void EnterGraphics()
// ----------------------------------------------------------------------------
//   Enter all the basic operations defined in graphics.tbl
// ----------------------------------------------------------------------------
{
    XL::Context *context = MAIN->context;
#include "opcodes_define.h"
#include "graphics.tbl"
}


void DeleteGraphics()
// ----------------------------------------------------------------------------
//   Delete all the global operations defined in graphics.tbl
// ----------------------------------------------------------------------------
{
#include "opcodes_delete.h"
#include "graphics.tbl"
}
