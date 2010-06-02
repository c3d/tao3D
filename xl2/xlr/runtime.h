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
#include <set>


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
Tree *xl_repeat(Tree *self, Tree *code, longlong count);
Tree *xl_map(Tree *data, Tree *code, text row = "\n", text column = ",");
Tree *xl_source(Tree *);
Tree *xl_set_source(Tree *val, Tree *src);
bool  xl_same_text(Tree * , const char *);
bool  xl_same_shape(Tree *t1, Tree *t2);
Tree *xl_infix_match_check(Tree *value, kstring name);
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
Tree *xl_evaluate_children(Tree *tree);

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

Tree *xl_invoke(eval_fn toCall, Tree *source, TreeList &args);

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
    XLCall &operator, (double  v)  { return *this, new Real(v); }
    XLCall &operator, (text  v)    { return *this, new Text(v); }

    // Calling in a given symbol context
    Tree *  operator() (Symbols *syms = NULL,
                       bool nullIfBad = false, bool cached = true);
    Tree *  build(Symbols *syms = NULL);

public:
    text        name;
    TreeList    args;
};



// ============================================================================
// 
//    Actions used for mapping
// 
// ============================================================================

struct MapAction : Action
// ----------------------------------------------------------------------------
//   Map a given code onto each element
// ----------------------------------------------------------------------------
{
    MapAction(Tree *code, bool eval = true);
    MapAction(Tree *code, text separator, bool eval = true);
    MapAction(Tree *code, text row, text column, bool eval = true);

    virtual Tree *Do(Tree *what);

    virtual Tree *DoInfix(Infix *what);
    virtual Tree *DoPrefix(Prefix *what);
    virtual Tree *DoPostfix(Postfix *what);
    virtual Tree *DoBlock(Block *what);

    Tree *  Map(Tree *what);

public:
    typedef Tree *      (*map_fn) (Tree *self, Tree *arg);
    Tree_p              code;
    map_fn              function;
    std::set<text>      separators;
    bool                evaluate;
};


struct CurryFunctionInfo : Info
// ----------------------------------------------------------------------------
//   Hold a single-argument function for a given tree
// ----------------------------------------------------------------------------
//   REVISIT: According to Wikipedia, really a Moses Schönfinkel function
{
    CurryFunctionInfo(): function(NULL), symbols(NULL) {}
    Tree_p      code;
    eval_fn     function;
    Symbols_p   symbols;
};



// ============================================================================
// 
//   Stack depth management
// 
// ============================================================================

struct StackDepthCheck
// ----------------------------------------------------------------------------
//   Verify that we don't go too deep into the stack
// ----------------------------------------------------------------------------
{
    StackDepthCheck(Tree *what)
    {
        stack_depth++;
        if (stack_depth > max_stack_depth)
            StackOverflow(what);
    }
    ~StackDepthCheck()
    {
        stack_depth--;
        if (stack_depth == 0 && !in_error_handler)
            in_error = false;
    }
    operator bool()
    {
        return in_error && !in_error_handler;
    }
    void StackOverflow(Tree *what);

protected:
    static uint         stack_depth;
    static uint         max_stack_depth;
    static bool         in_error_handler;
    static bool         in_error;
};



// ============================================================================
//
//    Loading trees from external files
//
// ============================================================================

Tree *xl_load(text name);
Tree *xl_load_data(text name, text prefix,
                   text fieldSeps = ",;", text recordSeps = "\n");



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
    source = xl_source(source);
    if (source == value)
        source = NULL;
    value->source = source;
    return value;
}

XL_END

#endif // RUNTIME_H
