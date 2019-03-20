// ****************************************************************************
//  apply_changes.cpp                                               Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This software is licensed under the GNU General Public License v3.
// See file COPYING for details.
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

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
