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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
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
#include "texture_cache.h"
#include <iostream>


// ============================================================================
//
//    Top-level operation
//
// ============================================================================

using namespace XL;

#include "opcodes_declare.h"
#include "graphics.tbl"


Tree *xl_process_module_import(Symbols *symbols, Tree *source, phase_t phase)
// ----------------------------------------------------------------------------
//   Standard connector for 'import' statements
// ----------------------------------------------------------------------------
{
    if (Prefix *pfx = source->AsPrefix())
    {
        source->SetSymbols(symbols);
        return Tao::ModuleManager::import(MAIN->context, source,
                                          pfx->right, phase);
    }
    return NULL;
}


Tree *xl_process_module_config(Symbols *symbols, Tree *source, phase_t phase)
// ----------------------------------------------------------------------------
//   Standard connector for 'configuration' statements
// ----------------------------------------------------------------------------
{
    if (Prefix *pfx = source->AsPrefix())
    {
        source->SetSymbols(symbols);
        if (phase == DECLARATION_PHASE)
            return Tao::ModuleManager::import(MAIN->context, source,
                                              pfx->right, phase, false);
        return XL::xl_false;
    }
    return NULL;
}


namespace Tao
{

void EnterGraphics()
// ----------------------------------------------------------------------------
//   Enter all the basic operations defined in graphics.tbl
// ----------------------------------------------------------------------------
{
    XL::Context *context = MAIN->context;
#include "opcodes_define.h"
#include "graphics.tbl"
    xl_enter_declarator("import", xl_process_module_import);
    xl_enter_declarator("configuration", xl_process_module_config);
}


void DeleteGraphics()
// ----------------------------------------------------------------------------
//   Delete all the global operations defined in graphics.tbl
// ----------------------------------------------------------------------------
{
#include "opcodes_delete.h"
#include "graphics.tbl"
}

}
