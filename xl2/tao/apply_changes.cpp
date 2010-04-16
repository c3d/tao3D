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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "apply_changes.h"
#include "main.h"
#include "widget_surface.h"
#include "hash.h"
#include "sha1_ostream.h"
#include <sys/stat.h>
#include <iostream>
#include <sstream>

TAO_BEGIN

bool ImportedFilesChanged(XL::Tree *prog,
                          import_set &done,
                          bool markChanged)
// ----------------------------------------------------------------------------
//   Compute the set of imported symbols
// ----------------------------------------------------------------------------
{
    using namespace XL;

    source_files &files = MAIN->files;
    source_files::iterator it;
    symbols_set imported;
    bool more = true;
    bool result = false;

    // Make sure we detect changes in builtins.xl
    imported.insert(Symbols::symbols);

    while (more)
    {
        more = false;
        for (it = files.begin(); it != files.end(); it++)
        {
            SourceFile &sf = (*it).second;
            if (!done.count(&sf))
            {
                if (sf.tree.tree == prog || imported.count(sf.symbols))
                {
                    done.insert(&sf);
                    if (markChanged)
                    {
                        text prev_hash = sf.hash;
                        TreeHashAction<> hash(XL::TreeHashAction<>::Force);
                        sf.tree.tree->Do(hash);
                        std::ostringstream os;
                        os << sf.tree.tree->Get< HashInfo<> > ();
                        sf.hash = os.str();

                        if (!prev_hash.empty() && sf.hash != prev_hash)
                            sf.changed = true;
                    }
                    struct stat st;
                    stat (sf.name.c_str(), &st);
                    if (st.st_mtime > sf.modified)
                        result = true;

                    // Check the imported trees
                    symbols_set &symset = sf.symbols->imported;
                    symbols_set::iterator si;
                    for (si = symset.begin(); si != symset.end(); si++)
                    {
                        if (!imported.count(*si))
                        {
                            more = true;
                            imported.insert(*si);
                        }
                    }
                }
            }
        }
    }
    return result;
}


XL::Tree *PruneInfo::Do(XL::Tree *what)
// ----------------------------------------------------------------------------
//   Prune info that we don't want to preserve when the tree changes
// ----------------------------------------------------------------------------
{
    what->Purge<WidgetSurface>();
    return what;
}

TAO_END
