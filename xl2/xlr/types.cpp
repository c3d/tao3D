// ****************************************************************************
//  types.cpp                                                       XLR project
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

#include "types.h"
#include "tree.h"
#include "runtime.h"

XL_BEGIN

// ============================================================================
//
//    Infer types by scanning source code for type information
//
// ============================================================================

Tree_p InferTypes::Do (Tree_p what)
// ----------------------------------------------------------------------------
//   Infer the type of some arbitrary tree
// ----------------------------------------------------------------------------
{
    // Otherwise, we don't know how to deal with it
    return Ooops("Cannot infer the type of '$1'", what);
}


Tree_p InferTypes::DoInteger(Integer_p what)
// ----------------------------------------------------------------------------
//   Return the integer type
// ----------------------------------------------------------------------------
{
    what->Set<TypeInfo> (integer_type);
    return integer_type;
}


Tree_p InferTypes::DoReal(Real_p what)
// ----------------------------------------------------------------------------
//   Return the real type
// ----------------------------------------------------------------------------
{
    what->Set<TypeInfo> (real_type);
    return real_type;
}


Tree_p InferTypes::DoText(Text_p what)
// ----------------------------------------------------------------------------
//   Return text or character type
// ----------------------------------------------------------------------------
{
    Tree_p type = what->opening == "'" ? character_type : text_type;
    what->Set<TypeInfo> (type);
    return type;
}


Tree_p InferTypes::DoName(Name_p what)
// ----------------------------------------------------------------------------
//   Return the type of the value of the name
// ----------------------------------------------------------------------------
{
    if (Tree_p value = symbols->Named(what->value))
    {
        if (Tree_p type = value->Get<TypeInfo>())
            return type;
        return Ooops("Unknown type for '$1'", what);
    }
    return Ooops("Unknown name '$1'", what);
}


Tree_p InferTypes::DoPrefix(Prefix_p what)
// ----------------------------------------------------------------------------
//   Infer all the possible types for a prefix expression
// ----------------------------------------------------------------------------
{
    return what;
}


Tree_p InferTypes::DoPostfix(Postfix_p what)
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
{
    return what;
}


Tree_p InferTypes::DoInfix(Infix_p what)
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
{
    return what;
}


Tree_p InferTypes::DoBlock(Block_p what)
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
{
    return what;
}



// ============================================================================
//
//    Check if a value matches a type
//
// ============================================================================

Tree_p MatchType::Do (Tree_p what)
// ----------------------------------------------------------------------------
//   If we get there, this is not a type match
// ----------------------------------------------------------------------------
{
    return MatchStructuredType(what);
}


Tree_p MatchType::DoInteger(Integer_p what)
// ----------------------------------------------------------------------------
//   An integer values matches the same value, the integer or real types
// ----------------------------------------------------------------------------
{
    // If the type is itself an integer, the value must be the same
    if (Integer_p it = what->AsInteger())
        return it->value == what->value ? what : NULL;

    // An integer value directly matches with integer type
    Normalize();
    if (type == integer_type)
        return what;

    // If the type we match against is real, add a conversion
    if (type == real_type)
        return new Prefix(new Name("real"), what);

    // Otherwise, check union types or fail
    return MatchStructuredType(what);
}


Tree_p MatchType::DoReal(Real_p what)
// ----------------------------------------------------------------------------
//   A real value matches the same value or real types
// ----------------------------------------------------------------------------
{
    // If the type is itself a real, the value must be the same
    if (Real_p rt = what->AsReal())
        return rt->value == what->value ? what : NULL;

    // A real value matches with the real type
    Normalize();
    if (type == real_type)
        return what;

    // Otherwise, check union types or fail
    return MatchStructuredType(what);
}


Tree_p MatchType::DoText(Text_p what)
// ----------------------------------------------------------------------------
//   Text values match text or character type
// ----------------------------------------------------------------------------
{
    // If the type is itself a text litteral, we must match value exactly
    if (Text_p tt = what->AsText())
        return (tt->value == what->value     &&
                tt->opening == what->opening &&
                tt->closing == what->closing
                ? what
                : NULL);

    // A text value matches either the text or character type
    Tree_p textType = (what->opening == "'" && what->closing == "'"
                      ? character_type
                      : text_type);
    Normalize();
    if (type == textType)
        return what;

    // Otherwise, check union types or fail
    return MatchStructuredType(what);
}


Tree_p MatchType::DoName(Name_p what)
// ----------------------------------------------------------------------------
//   A name matches if the value matches or if we expect a symbol
// ----------------------------------------------------------------------------
{
    Normalize();
    if (type == symbolicname_type)
        return what;
    if (Tree_p value = symbols->Named(what->value))
        return value->Do(this);
    return NULL;
}


Tree_p MatchType::DoPrefix(Prefix_p what)
// ----------------------------------------------------------------------------
//   Check if we have a similar form, or if this matches rewrites
// ----------------------------------------------------------------------------
{
    // Check if we match a prefix with the same shape
    if (Prefix_p prefixType = type->AsPrefix())
    {
        LocalSave<Tree_p> saveType = type;

        type = prefixType->left;
        Tree_p leftValue = NameMatch(what->left);
        if (leftValue)
        {
            type = prefixType->right;
            Tree_p rightValue = what->right->Do(this);
            if (rightValue)
                return what;
        }
    }

    // Check if the given type is 'prefix'
    Normalize();
    if (type == prefix_type)
        return what;

    // Otherwise check rewrites
    return Rewrites(what);
}


Tree_p MatchType::DoPostfix(Postfix_p what)
// ----------------------------------------------------------------------------
//   Check if we have a similar form, or if this matches rewrites
// ----------------------------------------------------------------------------
{
    // Check if we match a postfix with the same shape
    if (Postfix_p postfixType = type->AsPostfix())
    {
        LocalSave<Tree_p> saveType = type;

        type = postfixType->right;
        Tree_p rightValue = NameMatch(what->right);
        if (rightValue)
        {
            type = postfixType->left;
            Tree_p leftValue = what->left->Do(this);
            if (leftValue)
                return what;
        }
    }

    // Check if the given type is 'postfix'
    Normalize();
    if (type == postfix_type)
        return what;

    // Otherwise check rewrites
    return Rewrites(what);
}


Tree_p MatchType::DoInfix(Infix_p what)
// ----------------------------------------------------------------------------
//   Check if we have a similar form, or if it matches rewrites
// ----------------------------------------------------------------------------
{
    // Check if we match an infix with the same shape
    if (Infix_p infixType = type->AsInfix())
    {
        if (infixType->name == what->name)
        {
            LocalSave<Tree_p> saveType = type;

            type = infixType->left;
            Tree_p leftValue = what->left->Do(this);
            if (leftValue)
            {
                type = infixType->right;
                Tree_p rightValue = what->right->Do(this);
                if (rightValue)
                    return what;
            }
        }
    }

    // Check if the given type is 'infix'
    Normalize();
    if (type == infix_type)
        return what;

    // Otherwise check rewrites
    return Rewrites(what);
}


Tree_p MatchType::DoBlock(Block_p what)
// ----------------------------------------------------------------------------
//   Check if the evaluated value matches, otherwise check rewrites
// ----------------------------------------------------------------------------
{
    // Check if we match a block with the same shape
    if (Block_p blockType = type->AsBlock())
    {
        if (blockType->opening == what->opening &&
            blockType->closing == what->closing)
        {
            LocalSave<Tree_p> saveType = type;

            type = blockType->child;
            Tree_p childValue = what->child->Do(this);
            if (childValue)
                return what;
        }
    }

    // Check if the given type is 'block'
    Normalize();
    if (type == block_type)
        return what;

    // Check if the value is (X), in which case we test X
    if (what->opening == "(" && what->closing == ")")
        if (Tree_p childValue = what->child->Do(this))
            return childValue;

    // Otherwise check rewrites
    return Rewrites(what);
}


Tree_p MatchType::MatchStructuredType(Tree_p what, Tree_p kind)
// ----------------------------------------------------------------------------
//   Check structured types like T1|T2 or (T)
// ----------------------------------------------------------------------------
{
    // If this is some union type, i.e. T1 | T2 matches T1 values or T2 values
    if (Infix_p infixType = type->AsInfix())
    {
        if (infixType->name == "|")
        {
            LocalSave<Tree_p> saveType = type;

            type = infixType->left;
            Tree_p leftValue = what->Do(this);
            if (leftValue)
                return leftValue;

            type = infixType->right;
            Tree_p rightValue = what->Do(this);
            if (rightValue)
                return rightValue;
        }
    }

    // If this is a block, type (T) matches the same as T
    else if (Block_p blockType = type->AsBlock())
    {
        if (blockType->opening == "(" && blockType->closing == ")")
        {
            LocalSave<Tree_p> saveType = type;
            type = blockType->child;
            Tree_p childValue = what->Do(this);
            if (childValue)
                return childValue;
        }
    }

    // Check if the type is 'tree' or a given structure type
    else if (Name_p namedType = type->AsName())
    {
        if (namedType->value == "tree" || namedType == kind)
            return what;
    }

    return NULL;
}


Tree_p  MatchType::Rewrites(Tree_p what)
// ----------------------------------------------------------------------------
//   Check the various rewrites and see if there is one where types match
// ----------------------------------------------------------------------------
{
    // Compute the hash key for the form we have to match
    ulong formKey, testKey;
    RewriteKey formKeyHash;
    what->Do(formKeyHash);
    formKey = formKeyHash.Key();
    symbols_set visited;
    symbols_list lookups;

    // Build all the symbol tables that we are going to look into
    for (Symbols *s = symbols; s; s = s->Parent())
    {
        if (!visited.count(s))
        {
            lookups.push_back(s);
            visited.insert(s);
            symbols_set::iterator si;
            for (si = s->imported.begin(); si != s->imported.end(); si++)
            {
                if (!visited.count(*si))
                {
                    visited.insert(*si);
                    lookups.push_back(*si);
                }
            }
        }
    }

    // Iterate over all symbol tables listed above
    symbols_list::iterator li;
    for (li = lookups.begin(); li != lookups.end(); li++)
    {
        Symbols *s = *li;

        Rewrite *candidate = s->Rewrites();
        while (candidate)
        {
            // Compute the hash key for the 'from' of the current rewrite
            RewriteKey testKeyHash;
            candidate->from->Do(testKeyHash);
            testKey = testKeyHash.Key();

            // If we have an exact match for the keys, we may have a winner
            if (testKey == formKey)
            {
                // Create the invokation point
                Symbols args(symbols);
                ArgumentTypeMatch matchArgs(what, symbols,
                                            &args, candidate->symbols);
                Tree_p argsTest = candidate->from->Do(matchArgs);

                // If we found something, type matched
                if (argsTest)
                    return what;
            } // Match test key

            // Otherwise, follow the hash chain for the next candidate
            if (candidate->hash.count(formKey) > 0)
                candidate = candidate->hash[formKey];
            else
                candidate = NULL;
        } // while(candidate)
    } // for(namespaces)

    return NULL;
}


Tree_p MatchType::Normalize()
// ----------------------------------------------------------------------------
//    Find the normalized type for built-ins, e.g. integer->integer_type
// ----------------------------------------------------------------------------
{
    if (Name_p named = type->AsName())
    {
        text name = named->value;

        // Look it up in the symbol table (takes precedence over builtins)
        if (Tree_p value = symbols->Named(name))
        {
            type = value;
        }
        else
        {
            // Check built-in types
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
#define TYPE(symbol)                            \
            if (name == #symbol)                \
                type = symbol##_type;
#include "basics.tbl"
        }
    }

    return type;
}


Tree_p MatchType::NameMatch(Tree_p what)
// ----------------------------------------------------------------------------
//    Check if either we have a matching name, or a regular match
// ----------------------------------------------------------------------------
{
    if (Name_p name = what->AsName())
    {
        if (Name_p typedName = type->AsName())
        {
            if (typedName->value == name->value)
                return what;
        }
    }

    // Otherwise, check normal rules for types
    return what->Do(this);
}



// ============================================================================
//
//    Argument matching - Test input arguments against parameters
//
// ============================================================================

Tree_p ArgumentTypeMatch::Do(Tree_p what)
// ----------------------------------------------------------------------------
//   Default is to return failure
// ----------------------------------------------------------------------------
{
    return NULL;
}


Tree_p ArgumentTypeMatch::DoInteger(Integer_p what)
// ----------------------------------------------------------------------------
//   An integer argument matches the exact value
// ----------------------------------------------------------------------------
{
    // If the tested tree is a constant, it must be an integer with same value
    if (test->IsConstant())
    {
        Integer_p it = test->AsInteger();
        if (!it)
            return NULL;
        if (it->value == what->value)
            return what;
    }

    return NULL;
}


Tree_p ArgumentTypeMatch::DoReal(Real_p what)
// ----------------------------------------------------------------------------
//   A real matches the exact value
// ----------------------------------------------------------------------------
{
    // If the tested tree is a constant, it must be an integer with same value
    if (test->IsConstant())
    {
        Real_p rt = test->AsReal();
        if (!rt)
            return NULL;
        if (rt->value == what->value)
            return what;
    }

    return NULL;
}


Tree_p ArgumentTypeMatch::DoText(Text_p what)
// ----------------------------------------------------------------------------
//   A text matches the exact value
// ----------------------------------------------------------------------------
{
    // If the tested tree is a constant, it must be an integer with same value
    if (test->IsConstant())
    {
        Text_p tt = test->AsText();
        if (!tt)
            return NULL;
        if (tt->value == what->value)
            return what;
    }

    return NULL;
}


Tree_p ArgumentTypeMatch::DoName(Name_p what)
// ----------------------------------------------------------------------------
//    Bind arguments to parameters being defined in the shape
// ----------------------------------------------------------------------------
{
    if (!defined)
    {
        // The first name we see must match exactly, e.g. 'sin' in 'sin X'
        defined = what;
        if (Name_p nt = test->AsName())
            if (nt->value == what->value)
                return what;
        return NULL;
    }
    else
    {
        // Check if the name already exists, e.g. 'false' or 'A+A'
        // If it does, we generate a run-time check to verify equality
        if (Tree_p existing = rewrite->Named(what->value))
        {
            // Check if the test is an identity
            if (Name_p nt = test->AsName())
            {
                if (nt->code == xl_identity)
                {
                    if (nt->value == what->value)
                        return what;
                    if (Name_p existingName = existing->AsName())
                        if (existingName->value == nt->value)
                            return what;
                    return NULL;
                }
            }
        }

        return what;
    }
}


Tree_p ArgumentTypeMatch::DoBlock(Block_p what)
// ----------------------------------------------------------------------------
//   Check if we match a block
// ----------------------------------------------------------------------------
{
    // Test if we exactly match the block, i.e. the reference is a block
    if (Block_p bt = test->AsBlock())
    {
        if (bt->opening == what->opening &&
            bt->closing == what->closing)
        {
            test = bt->child;
            Tree_p br = what->child->Do(this);
            test = bt;
            if (br)
                return br;
        }
    }

    // Otherwise, if the block is an indent or parenthese, optimize away
    if ((what->opening == "(" && what->closing == ")") ||
        (what->opening == Block::indent && what->closing == Block::unindent))
    {
        return what->child->Do(this);
    }

    return NULL;
}


Tree_p ArgumentTypeMatch::DoInfix(Infix_p what)
// ----------------------------------------------------------------------------
//   Check if we match an infix operator
// ----------------------------------------------------------------------------
{
    // Check if we match an infix tree like 'x,y' with a name like 'A'
    if (what->name != ":")
    {
        if (Name_p name = test->AsName())
        {
            name = name;
        }
    }

    if (Infix_p it = test->AsInfix())
    {
        // Check if we match the tree, e.g. A+B vs 2+3
        if (it->name == what->name)
        {
            if (!defined)
                defined = what;
            test = it->left;
            Tree_p lr = what->left->Do(this);
            test = it;
            if (!lr)
                return NULL;
            test = it->right;
            Tree_p rr = what->right->Do(this);
            test = it;
            if (!rr)
                return NULL;
            return what;
        }
    }

    // Check if we match a type, e.g. 2 vs. 'K : integer'
    if (what->name == ":")
    {
        // Check the variable name, e.g. K in example above
        Name_p varName = what->left->AsName();
        if (!varName)
            return Ooops("Expected a name, got '$1' ", what->left);

        // Check if the name already exists
        if (Tree_p existing = rewrite->Named(varName->value))
            return Ooops("Name '$1' already exists as '$2'",
                         what->left, existing);

        return what;
    }

    // Otherwise, this is a mismatch
    return NULL;
}


Tree_p ArgumentTypeMatch::DoPrefix(Prefix_p what)
// ----------------------------------------------------------------------------
//   For prefix expressions, simply test left then right
// ----------------------------------------------------------------------------
{
    if (Prefix_p pt = test->AsPrefix())
    {
        // Check if we match the tree, e.g. f(A) vs. f(2)
        // Note that we must test left first to define 'f' in above case
        Infix_p defined_infix = defined->AsInfix();
        if (defined_infix)
            defined = NULL;

        test = pt->left;
        Tree_p lr = what->left->Do(this);
        test = pt;
        if (!lr)
            return NULL;
        test = pt->right;
        Tree_p rr = what->right->Do(this);
        test = pt;
        if (!rr)
            return NULL;
        if (!defined && defined_infix)
            defined = defined_infix;
        return what;
    }
    return NULL;
}


Tree_p ArgumentTypeMatch::DoPostfix(Postfix_p what)
// ----------------------------------------------------------------------------
//    For postfix expressions, simply test right, then left
// ----------------------------------------------------------------------------
{
    if (Postfix_p pt = test->AsPostfix())
    {
        // Check if we match the tree, e.g. A! vs 2!
        // Note that ordering is reverse compared to prefix, so that
        // the 'defined' names is set correctly
        test = pt->right;
        Tree_p rr = what->right->Do(this);
        test = pt;
        if (!rr)
            return NULL;
        test = pt->left;
        Tree_p lr = what->left->Do(this);
        test = pt;
        if (!lr)
            return NULL;
        return what;
    }
    return NULL;
}

XL_END
