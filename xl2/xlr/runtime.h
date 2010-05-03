#ifndef RUNTIME_H
#define RUNTIME_H
// ****************************************************************************
//  runtime.h                       (C) 1992-2009 Christophe de Dinechin (ddd)
//                                                                 XL2 project
// ****************************************************************************
//
//   File Description:
//
//     Functions required for proper run-time execution of XL programs
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
// ****************************************************************************
// * File       : $RCSFile$
// * Revision   : $Revision$
// * Date       : $Date$
// ****************************************************************************

#include "base.h"
#include "tree.h"


XL_BEGIN
// ============================================================================
//
//    Forward declarations
//
// ============================================================================

struct Tree;
struct Block;
struct Infix;
struct Prefix;
struct Postfix;



// ============================================================================
//
//    Runtime functions with C interface
//
// ============================================================================

Tree_p xl_identity(Tree_p);
Tree_p xl_evaluate(Tree_p);
Tree_p xl_repeat(Tree_p self, Tree_p code, longlong count);
Tree_p xl_source(Tree_p);
Tree_p xl_set_source(Tree_p val, Tree_p src);
bool  xl_same_text(Tree_p , const char *);
bool  xl_same_shape(Tree_p t1, Tree_p t2);
Tree_p  xl_infix_match_check(Tree_p value, Infix_p ref);
Tree_p xl_type_check(Tree_p value, Tree_p type);

Tree_p xl_new_integer(longlong value);
Tree_p xl_new_real(double value);
Tree_p xl_new_character(kstring value);
Tree_p xl_new_text(kstring value);
Tree_p xl_new_xtext(kstring value, kstring open, kstring close);
Tree_p xl_new_block(Block_p source, Tree_p child);
Tree_p xl_new_prefix(Prefix_p source, Tree_p left, Tree_p right);
Tree_p xl_new_postfix(Postfix_p source, Tree_p left, Tree_p right);
Tree_p xl_new_infix(Infix_p source, Tree_p left, Tree_p right);

Tree_p xl_new_closure(Tree_p expr, uint ntrees, ...);
Tree_p xl_type_error(Tree_p tree);

Tree_p xl_boolean_cast(Tree_p source, Tree_p value);
Tree_p xl_integer_cast(Tree_p source, Tree_p value);
Tree_p xl_real_cast(Tree_p source, Tree_p value);
Tree_p xl_text_cast(Tree_p source, Tree_p value);
Tree_p xl_character_cast(Tree_p source, Tree_p value);
Tree_p xl_tree_cast(Tree_p source, Tree_p value);
Tree_p xl_symbolicname_cast(Tree_p source, Tree_p value);
Tree_p xl_infix_cast(Tree_p source, Tree_p value);
Tree_p xl_prefix_cast(Tree_p source, Tree_p value);
Tree_p xl_postfix_cast(Tree_p source, Tree_p value);
Tree_p xl_block_cast(Tree_p source, Tree_p value);



// ============================================================================
//
//    Call management
//
// ============================================================================

Tree_p xl_invoke(Tree_p (*toCall)(Tree_p),
                Tree_p source, uint numarg, Tree_p *args);

struct XLCall
// ----------------------------------------------------------------------------
//    A structure that encapsulates a call to an XL tree
// ----------------------------------------------------------------------------
{
    XLCall(text name): name(name), args() {}

    // Adding arguments
    XLCall &operator, (Tree_p tree) { args.push_back(tree); return *this; }
    XLCall &operator, (Tree &tree) { return *this, &tree; }
    XLCall &operator, (longlong v) { return *this, new Integer(v); }
    XLCall &operator, (double  v)  { return *this, new Real(v); }
    XLCall &operator, (text  v)    { return *this, new Text(v); }

    // Calling in a given symbol context
    Tree_p  operator() (Symbols *syms = NULL,
                       bool nullIfBad = false, bool cached = true);
    Tree_p  build(Symbols *syms = NULL);

public:
    text        name;
    TreeList    args;
};


// ============================================================================
//
//    Loading trees from external files
//
// ============================================================================

Tree_p xl_load(text name);
Tree_p xl_load_csv(text name);
Tree_p xl_load_tsv(text name);



// ============================================================================
// 
//    Inline functions
// 
// ============================================================================

inline Tree_p xl_source(Tree_p value)
// ----------------------------------------------------------------------------
//   Return the source that led to the evaluation of a given tree
// ----------------------------------------------------------------------------
{
    if (Tree_p source = value->source)
        return source;
    return value;
}


inline Tree_p xl_set_source(Tree_p value, Tree_p source)
// ----------------------------------------------------------------------------
//   Return the source that led to the evaluation of a given tree
// ----------------------------------------------------------------------------
{
    value->source = xl_source(source);
    return value;
}

XL_END

#endif // RUNTIME_H
