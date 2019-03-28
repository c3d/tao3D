// *****************************************************************************
// apply_changes.cpp                                               Tao3D project
// *****************************************************************************
//
// File description:
//
//     Check changes between the running tree and files on disk
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
// (C) 2010,2012,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2012, Jérôme Forissier <jerome@taodyne.com>
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

#include "tao/tao_gl.h"
#include "apply_changes.h"
#include "main.h"
#include "widget_surface.h"
#include "widget.h"
#include "tao_utf8.h"
#include <iostream>
#include <sstream>


RECORDER(filesync, 32, "File synchronization (on file reload)");

TAO_BEGIN

struct TreeHash
// ----------------------------------------------------------------------------
//    Hash function for a tree
// ----------------------------------------------------------------------------
{
    typedef uint64 value_type;
    value_type Do(Integer *what)
    {
        return what->value;
    }
    value_type Do(Real *what)
    {
        return *((value_type *) &what->value);
    }
    value_type Do(Text *what)
    {
        return Compute(what->value,   0xBABAC00L)
            ^  Compute(what->opening, 0xBADF00D)
            ^  Compute(what->closing, 0x600DBEEF);
    }
    value_type Do(Name *what)
    {
        return Compute(what->value, 0xFABD00DL);
    }
    value_type Do(Infix *what)
    {
        return Compute(what->name, 0xABB00L)
            ^  what->left->Do(this)
            ^  what->right->Do(this);
    }
    value_type Do(Prefix *what)
    {
        return 0x73F7541F7
            ^  what->left->Do(this)
            ^  what->right->Do(this);
    }
    value_type Do(Postfix *what)
    {
        return 0xB057F1C5
            ^  what->left->Do(this)
            ^  what->right->Do(this);
    }
    value_type Do(Block *what)
    {
        return 0xB70C4
            ^  what->child->Do(this);
    }
    static value_type Compute(kstring ptr, value_type seed)
    {
        value_type result = seed;
        kstring last = ptr + 64;
        while (*ptr && ptr < last)
            result = (result * 0x1081) ^ *ptr++;
        return result;
    }
    static value_type Compute(const text &value, value_type seed)
    {
        return Compute(value.c_str(), seed);
    }
};


void ScanImportedFiles(import_set &done, bool markChanged)
// ----------------------------------------------------------------------------
//   Compute the set of imported symbols
// ----------------------------------------------------------------------------
{
    using namespace XL;

    source_files &files = MAIN->files;
    source_files::iterator it;

    // Loop on source files
    for (it = files.begin(); it != files.end(); it++)
    {
        SourceFile &sf = (*it).second;
        Tree *source = sf.tree;
        if (source && !done.count(&sf))
        {
            done.insert(&sf);
            if (markChanged)
            {
                TreeHash hash;
                uint64 old_hash = sf.hash;
                uint64 new_hash = source->Do(hash);
                record(filesync, "Computed hash for %s: %llX (was %llX)",
                       sf.name.c_str(), new_hash, old_hash);
                sf.changed = old_hash != new_hash;
            }
        }
    }
}

TAO_END
