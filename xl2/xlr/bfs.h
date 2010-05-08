#ifndef BFS_H
#define BFS_H
// ****************************************************************************
//  bfs.h                                                          XLR project
// ****************************************************************************
//
//   File Description:
//
//     Implementation of the breadth-first search algorithm on a tree.
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
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************
// * File       : $RCSFile$
// * Revision   : $Revision$
// * Date       : $Date$
// ****************************************************************************


#include <queue>
#include "tree.h"

XL_BEGIN

struct BreadthFirstSearch : Action
// ----------------------------------------------------------------------------
//   Execute ActionClass on a tree (whole or part), in breadth-first order
// ----------------------------------------------------------------------------
{
    BreadthFirstSearch (Action &action, bool fullScan = true):
        action(action), fullScan(fullScan) {}
    Tree_p DoBlock(Block_p what)
    {
        return Do(what);
    }
    Tree_p DoInfix(Infix_p what)
    {
        return Do(what);
    }
    Tree_p DoPrefix(Prefix_p what)
    {
        return Do(what);
    }
    Tree_p DoPostfix(Postfix_p what)
    {
        return Do(what);
    }
    Tree_p Do(Tree_p what)
    {
        nodes.push(what);
        do
        {
            Block_p  bl; Infix_p  in; Prefix_p  pr; Postfix_p  po;
            Tree_p  curr, res;

            curr = nodes.front();
            if (!curr)
            {
                nodes.pop();
                continue;
            }
            res = curr->Do(action);
            if (!fullScan && res)
                return res;
            nodes.pop();
            if ((bl = curr->AsBlock()))
            {
              nodes.push(bl->child);
            }
            if ((in = curr->AsInfix()))
            {
              nodes.push(in->left);
              nodes.push(in->right);
            }
            if ((pr = curr->AsPrefix()))
            {
              nodes.push(pr->left);
              nodes.push(pr->right);
            }
            if ((po = curr->AsPostfix()))
            {
              nodes.push(po->left);
              nodes.push(po->right);
            }
        }
        while (!nodes.empty());
        return NULL;
    }

    Action & action;
    bool fullScan;
    std::queue<Tree_p> nodes;
};

XL_END

#endif // BFS_H
