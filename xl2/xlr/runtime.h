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

Tree *xl_identity(Tree *);
Tree *xl_evaluate(Tree *);
Tree *xl_repeat(Tree *code, longlong count);
Tree *xl_source(Tree *);
Tree *xl_set_source(Tree *val, Tree *src);
bool  xl_same_text(Tree *, const char *);
bool  xl_same_shape(Tree *t1, Tree *t2);
Tree* xl_infix_match_check(Tree *value, Infix *ref);
Tree *xl_type_check(Tree *value, Tree *type);

Tree *xl_new_integer(longlong value);
Tree *xl_new_real(double value);
Tree *xl_new_character(kstring value);
Tree *xl_new_text(kstring value);
Tree *xl_new_xtext(kstring value, kstring open, kstring close);
Tree *xl_new_block(Block *source, Tree *child);
Tree *xl_new_prefix(Prefix *source, Tree *left, Tree *right);
Tree *xl_new_postfix(Postfix *source, Tree *left, Tree *right);
Tree *xl_new_infix(Infix *source, Tree *left, Tree *right);

Tree *xl_new_closure(Tree *expr, uint ntrees, ...);
Tree *xl_type_error(Tree *tree);

Tree *xl_boolean_cast(Tree *source, Tree *value);
Tree *xl_integer_cast(Tree *source, Tree *value);
Tree *xl_real_cast(Tree *source, Tree *value);
Tree *xl_text_cast(Tree *source, Tree *value);
Tree *xl_character_cast(Tree *source, Tree *value);
Tree *xl_tree_cast(Tree *source, Tree *value);
Tree *xl_symbolicname_cast(Tree *source, Tree *value);
Tree *xl_infix_cast(Tree *source, Tree *value);
Tree *xl_prefix_cast(Tree *source, Tree *value);
Tree *xl_postfix_cast(Tree *source, Tree *value);
Tree *xl_block_cast(Tree *source, Tree *value);



// ============================================================================
//
//    Call management
//
// ============================================================================

Tree *xl_invoke(Tree *(*toCall)(Tree *),
                Tree *source, uint numarg, Tree **args);

struct XLCall
// ----------------------------------------------------------------------------
//    A structure that encapsulates a call to an XL tree
// ----------------------------------------------------------------------------
{
    XLCall(text name): name(name), args() {}

    // Adding arguments
    XLCall &operator, (Tree *tree) { args.push_back(tree); return *this; }
    XLCall &operator, (Tree &tree) { return *this, &tree; }
    XLCall &operator, (longlong v) { return *this, new Integer(v); }
    XLCall &operator()(double  v)  { return *this, new Real(v); }
    XLCall &operator()(text  v)    { return *this, new Text(v); }

    // Calling in a given symbol context
    Tree * operator() (Symbols *syms = NULL);

public:
    text        name;
    tree_list   args;
};


// ============================================================================
//
//    Loading trees from external files
//
// ============================================================================

Tree *xl_load(text name);
Tree *xl_load_csv(text name);
Tree *xl_load_tsv(text name);



// ============================================================================
// 
//    Inline functions
// 
// ============================================================================

inline Tree *xl_source(Tree *value)
// ----------------------------------------------------------------------------
//   Return the source that led to the evaluation of a given tree
// ----------------------------------------------------------------------------
{
    if (Tree *source = value->source)
        return source;
    return value;
}


inline Tree *xl_set_source(Tree *value, Tree *source)
// ----------------------------------------------------------------------------
//   Return the source that led to the evaluation of a given tree
// ----------------------------------------------------------------------------
{
    value->source = xl_source(source);
    return value;
}

XL_END

#endif // RUNTIME_H
