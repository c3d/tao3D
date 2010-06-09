
// ****************************************************************************
//  tree_cloning						   Tao project
// ****************************************************************************
//
//   File Description:
//
//     manage copy of tree with selection awarness.
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
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************
#include "widget.h"
#include "tree_cloning.h"
TAO_BEGIN


Tree * TaoTreeClone::Reselect(Tree *from, Tree *to)
// ----------------------------------------------------------------------------
//   Check if we change entries in the selection
// ----------------------------------------------------------------------------
{
    // Check if we are possibly changing the selection
    std::set<Tree_p> &sel = widget->selectionTrees;
    if (sel.count(from))
        sel.insert(to);

    // Check if we are possibly changing the next selection
    std::set<Tree_p> &nxSel = widget->selectNextTime;
    if (nxSel.count(from))
        nxSel.insert(to);

    // Check if we are possibly changing the page tree reference
    if (widget->pageTree == from)
        widget->pageTree = to;

    return to;
}


TAO_END
