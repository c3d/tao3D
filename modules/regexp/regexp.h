// *****************************************************************************
// regexp.h                                                        Tao3D project
// *****************************************************************************
//
// File description:
//
//     Module offering regular expressions in Tao
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
// (C) 2012, Catherine Burvelle <catherine@taodyne.com>
// (C) 2012,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
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
#include "main.h"
#include "runtime.h"
#include "base.h"

using namespace XL;

enum RegExpMode
{
    REGEXP_FIRST,
    REGEXP_LAST,
    REGEXP_LIST,
    REGEXP_TEXT
};

Name_p regexpMatch(Context *, Tree *self, text input, text pattern);
Integer_p regexpSearch(Context *, Tree *self, text input, text pattern);
Tree_p regexpParse(Context *, Tree *self, text input, Tree *code_o, RegExpMode m);
Text_p regexpAt(Context *, Tree *self, uint index);
Integer_p regexpPos(Context *, Tree *self, uint index);
Integer_p regexpMatchedLength(Context *, Tree *self);
Text_p regexpEscape(Context *, Tree *self, text toEscape);
