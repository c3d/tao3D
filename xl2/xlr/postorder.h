#ifndef POSTORDER_H
#define POSTORDER_H
// ****************************************************************************
//  postorder.h                                                    XLR project
// ****************************************************************************
//
//   File Description:
//
//     Implementation of the post-order traversal algorithm on a tree.
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


#include "tree.h"

XL_BEGIN

struct PostOrderTraversal : Action
// ----------------------------------------------------------------------------
//   Execute action on a tree (whole or part), following the post-order algo
// ----------------------------------------------------------------------------
{
    PostOrderTraversal (Action &action, bool fullScan = true):
        action(action), fullScan(fullScan) {}
    Tree_p DoBlock(Block_p what)
    {
        Tree_p  ret = NULL;
        if (what->child)
            ret = Do(what->child);
        if (!fullScan && ret)
            return ret;
        return what->Do(action);
    }
    Tree_p DoInfix(Infix_p what)
    {
        Tree_p  ret;
        ret = Do(what->left);
        if (!fullScan && ret)
            return ret;
        ret = Do(what->right);
        if (!fullScan && ret)
            return ret;
        return what->Do(action);
    }
    Tree_p DoPrefix(Prefix_p what)
    {
        Tree_p  ret;
        ret = Do(what->left);
        if (!fullScan && ret)
            return ret;
        ret = Do(what->right);
        if (!fullScan && ret)
            return ret;
        return what->Do(action);
    }
    Tree_p DoPostfix(Postfix_p what)
    {
        Tree_p  ret;
        ret = Do(what->left);
        if (!fullScan && ret)
            return ret;
        ret = Do(what->right);
        if (!fullScan && ret)
            return ret;
        return what->Do(action);
    }
    Tree_p Do(Tree_p what)
    {
        switch(what->Kind())
        {
        case INTEGER:
        case REAL:
        case TEXT:
        case NAME:          return what->Do(action);
        case BLOCK:         return DoBlock(what->AsBlock());
        case PREFIX:        return DoPrefix(what->AsPrefix());
        case POSTFIX:       return DoPostfix(what->AsPostfix());
        case INFIX:         return DoInfix(what->AsInfix());
        default:            assert(!"Unexpected tree kind");
        }
        return NULL;
    }

    Action & action;
    bool fullScan;
};

XL_END

#endif // POSTORDER_H
