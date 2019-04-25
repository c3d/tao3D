// *****************************************************************************
// graphics.cpp                                                    Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2009-2012,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2012, Jérôme Forissier <jerome@taodyne.com>
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

#warning "Empty file - Consider removing"

#if 0
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

xl_enter_declarator("import", xl_process_module_import);
xl_enter_declarator("configuration", xl_process_module_config);
#endif
