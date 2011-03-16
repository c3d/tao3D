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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "apply_changes.h"
#include "main.h"
#include "widget_surface.h"
#include "hash.h"
#include "sha1_ostream.h"
#include "widget.h"
#include <sys/stat.h>
#include <iostream>
#include <sstream>

TAO_BEGIN

bool ImportedFilesChanged(import_set &done,
                          bool markChanged)
// ----------------------------------------------------------------------------
//   Compute the set of imported symbols
// ----------------------------------------------------------------------------
{
    using namespace XL;

    source_files &files = MAIN->files;
    source_files::iterator it;
    bool result = false;

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
                    sf.changed = true;
            }
            struct stat st;
            st.st_mtime = sf.modified;
            stat (sf.name.c_str(), &st);
            if (st.st_mtime > sf.modified)
                result = true;
        }
    }
    return result;
}

TAO_END
