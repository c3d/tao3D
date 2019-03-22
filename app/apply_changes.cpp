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

#include "tao/tao_gl.h"
#include "apply_changes.h"
#include "main.h"
#include "widget_surface.h"
#include "hash.h"
#include "sha1_ostream.h"
#include "widget.h"
#include "tao_utf8.h"
#include <iostream>
#include <sstream>

TAO_BEGIN

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
        if (sf.tree && !done.count(&sf))
        {
            done.insert(&sf);
            if (markChanged)
            {
                text prev_hash = sf.hash;
                TreeHashAction<> hash(XL::TreeHashAction<>::Force);
                sf.tree->Do(hash);
                std::ostringstream os;
                os << sf.tree->Get< HashInfo<> > ();
                sf.hash = os.str();

                if (!prev_hash.empty() && sf.hash != prev_hash)
                {
                    IFTRACE(filesync)
                        std::cerr << "Hash changed from " << prev_hash
                                  << " to " << sf.hash
                                  << " for file " << sf.name << "\n";
                    sf.changed = true;
                }
            }
        }
    }
}

TAO_END
