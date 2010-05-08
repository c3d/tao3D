#ifndef TYPES_H
#define TYPES_H
// ****************************************************************************
//  types.h                                                         XLR project
// ****************************************************************************
//
//   File Description:
//
//     The type system in XL
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
//
//  The type system in XL is somewhat similar to what is found in Haskell,
//  except that it's based on the shape of trees.
//
//  The fundamental types are:
//  - integer, type of 0, 1, 2, ...
//  - real, type of 0.3, 4.2, 6.3e21
//  - text, type of "ABC" and "Hello World"
//  - character, type of 'A' and ' '
//  - boolean, type of true and false
//
//  The type constructors are:
//  - T1 |  T2 : Values of either type T1 or T2
//  - T1 -> T2 : A function taking T1 and returning T2
//  - expr: A tree with the given shape, e.g  (T1, T2), T1+T2
//
//  The type analysis phase consists in scanning the input tree,
//  recording type information and returning typed trees.


#include "tree.h"
#include "context.h"
#include <map>

XL_BEGIN

struct TypeInfo : Info
// ----------------------------------------------------------------------------
//    Information recording the type of a given tree
// ----------------------------------------------------------------------------
{
    typedef Tree_p       data_t;
    TypeInfo(Tree_p type): type(type) {}
    operator            data_t()  { return type; }
    Tree_p               type;
};


struct InferTypes : Action
// ----------------------------------------------------------------------------
//    Infer the types in an expression
// ----------------------------------------------------------------------------
{
    InferTypes(Symbols *s): Action(), symbols(s) {}

    Tree_p Do (Tree_p what);
    Tree_p DoInteger(Integer_p what);
    Tree_p DoReal(Real_p what);
    Tree_p DoText(Text_p what);
    Tree_p DoName(Name_p what);
    Tree_p DoPrefix(Prefix_p what);
    Tree_p DoPostfix(Postfix_p what);
    Tree_p DoInfix(Infix_p what);
    Tree_p DoBlock(Block_p what);

    Symbols_p   symbols;
};


struct MatchType : Action
// ----------------------------------------------------------------------------
//   An action that checks if a value matches a type
// ----------------------------------------------------------------------------
{
    MatchType(Symbols *s, Tree_p t): symbols(s), type(t) {}

    Tree_p Do(Tree_p what);
    Tree_p DoInteger(Integer_p what);
    Tree_p DoReal(Real_p what);
    Tree_p DoText(Text_p what);
    Tree_p DoName(Name_p what);
    Tree_p DoPrefix(Prefix_p what);
    Tree_p DoPostfix(Postfix_p what);
    Tree_p DoInfix(Infix_p what);
    Tree_p DoBlock(Block_p what);

    Tree_p MatchStructuredType(Tree_p what, Tree_p kind = NULL);
    Tree_p Rewrites(Tree_p what);
    Tree_p Normalize();
    Tree_p NameMatch(Tree_p what);

    Symbols_p symbols;
    Tree_p    type;
};


struct ArgumentTypeMatch : Action
// ----------------------------------------------------------------------------
//   Check if a tree matches the form of the left of a rewrite
// ----------------------------------------------------------------------------
{
    ArgumentTypeMatch (Tree_p t,
                       Symbols *s, Symbols *l, Symbols *r) :
        symbols(s), locals(l), rewrite(r), test(t), defined(NULL) {}

    // Action callbacks
    virtual Tree_p Do(Tree_p what);
    virtual Tree_p DoInteger(Integer_p what);
    virtual Tree_p DoReal(Real_p what);
    virtual Tree_p DoText(Text_p what);
    virtual Tree_p DoName(Name_p what);
    virtual Tree_p DoPrefix(Prefix_p what);
    virtual Tree_p DoPostfix(Postfix_p what);
    virtual Tree_p DoInfix(Infix_p what);
    virtual Tree_p DoBlock(Block_p what);

public:
    Symbols_p      symbols;     // Context in which we evaluate values
    Symbols_p      locals;      // Symbols where we declare arguments
    Symbols_p      rewrite;     // Symbols in which the rewrite was declared
    Tree_p         test;        // Tree we test
    Tree_p         defined;     // Tree we define
};



// ============================================================================
// 
//   Declare the basic types
// 
// ============================================================================

#undef INFIX
#undef PREFIX
#undef POSTFIX
#undef BLOCK
#undef NAME
#undef TYPE
#undef PARM
#undef DS
#undef RS

#define INFIX(name, rtype, t1, symbol, t2, code)
#define PARM(symbol, type)
#define PREFIX(name, rtype, symbol, parms, code)
#define POSTFIX(name, rtype, parms, symbol, code)
#define BLOCK(name, rtype, open, type, close, code)
#define NAME(symbol)
#define TYPE(symbol)    extern Name_p symbol##_type;
#include "basics.tbl"

XL_END

#endif // TYPES_H
