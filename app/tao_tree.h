#ifndef TAO_TREE_H
#define TAO_TREE_H
// *****************************************************************************
// tao_tree.h                                                      Tao3D project
// *****************************************************************************
//
// File description:
//
//     Make the various XL tree types visible in Tao namespace
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
// (C) 2010,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
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

#include "tree.h"
#include "opcodes.h"
#include "tao.h"
#include "main.h"


TAO_BEGIN

typedef XL::Tree           Tree;
typedef XL::Tree_p         Tree_p;
typedef XL::Integer        Integer;
typedef XL::Integer_p      Integer_p;
typedef XL::Real           Real;
typedef XL::Real_p         Real_p;
typedef XL::Text           Text;
typedef XL::Text_p         Text_p;
typedef XL::Name           Name;
typedef XL::Name_p         Name_p;
typedef XL::Infix          Infix;
typedef XL::Infix_p        Infix_p;
typedef XL::Prefix         Prefix;
typedef XL::Prefix_p       Prefix_p;
typedef XL::Postfix        Postfix;
typedef XL::Postfix_p      Postfix_p;
typedef XL::Block          Block;
typedef XL::Block_p        Block_p;
typedef XL::Scope          Scope;
typedef XL::Scope_p        Scope_p;

typedef XL::Real_r         real_r;
typedef XL::Integer_r      integer_r;
typedef XL::Text_r         text_r;
typedef XL::Scope_r        scope_r;

typedef XL::Real_p         real_p;
typedef XL::Integer_p      integer_p;
typedef XL::Text_p         text_p;
typedef XL::Scope_p        scope_p;

typedef XL::TreeList       TreeList;

typedef XL::Scope          Scope;
typedef XL::Context        Context;
typedef XL::Context_p      Context_p;

typedef XL::SourceFile     SourceFile;

TAO_END

#endif // TAO_TREE_H
