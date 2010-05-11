// ****************************************************************************
//  context.cpp                     (C) 1992-2003 Christophe de Dinechin (ddd)
//                                                                 XL2 project
// ****************************************************************************
//
//   File Description:
//
//     The execution environment for XL
//
//     This defines both the compile-time environment (Context), where we
//     keep symbolic information, e.g. how to rewrite trees, and the
//     runtime environment (Runtime), which we use while executing trees
//
//     See comments at beginning of context.h for details
//
//
//
// ****************************************************************************
// This document is confidential.
// Do not redistribute without written permission
// ****************************************************************************
// * File       : $RCSFile$
// * Revision   : $Revision$
// * Date       : $Date$
// ****************************************************************************

#include <iostream>
#include <cstdlib>
#include "context.h"
#include "tree.h"
#include "errors.h"
#include "options.h"
#include "renderer.h"
#include "basics.h"
#include "compiler.h"
#include "runtime.h"
#include <sstream>

XL_BEGIN

// ============================================================================
//
//   Symbols: symbols management
//
// ============================================================================

Symbols *Symbols::symbols = NULL;

Tree_p Symbols::Named(text name, bool deep)
// ----------------------------------------------------------------------------
//   Find the name in the current context
// ----------------------------------------------------------------------------
{
    for (Symbols *s = this; s; s = deep ? s->parent : NULL)
    {
        if (s->names.count(name) > 0)
            return s->names[name];
        symbols_set::iterator it;
        for (it = s->imported.begin(); it != s->imported.end(); it++)
            if ((*it)->names.count(name) > 0)
                return (*it)->names[name];
    }
    return NULL;
}


void Symbols::EnterName(text name, Tree_p value)
// ----------------------------------------------------------------------------
//   Enter a value in the namespace
// ----------------------------------------------------------------------------
{
    names[name] = value;
}


Name_p Symbols::Allocate(Name_p n)
// ----------------------------------------------------------------------------
//   Enter a value in the namespace
// ----------------------------------------------------------------------------
{
    if (Tree_p existing = names[n->value])
    {
        if (Name_p name = existing->AsName())
            if (name->value == n->value)
                return name;
        existing = Error("Redefining '$1' as data, was '$2'", n, existing);
        return existing->AsName();
    }
    names[n->value] = n;
    return n;
}


Rewrite *Symbols::EnterRewrite(Rewrite *rw)
// ----------------------------------------------------------------------------
//   Enter the given rewrite in the rewrites table
// ----------------------------------------------------------------------------
{
    // Record if we ever rewrite 0 or "ABC" in that scope
    if (rw->from->IsConstant())
        has_rewrites_for_constants = true;

    // Create symbol table for this rewrite
    Symbols *locals = new Symbols(this);
    rw->from->SetSymbols(locals);

    // Enter parameters in the symbol table
    ParameterMatch parms(locals);
    Tree_p check = rw->from->Do(parms);
    if (!check)
        Error("Parameter error for '$1'", rw->from);
    rw->parameters = parms.order;

    // If we are defining a name, store the definition in the symbols
    if (Name_p name = parms.defined->AsName())
        Allocate(name);

    if (rewrites)
    {
        /* Returns parent */ rewrites->Add(rw);
        return rw;
    }
    rewrites = rw;
    return rw;
}


Rewrite *Symbols::EnterRewrite(Tree_p from, Tree_p to)
// ----------------------------------------------------------------------------
//   Create a rewrite for the current context and enter it
// ----------------------------------------------------------------------------
{
    Rewrite *rewrite = new Rewrite(this, from, to);
    return EnterRewrite(rewrite);
}


void Symbols::Clear()
// ----------------------------------------------------------------------------
//   Clear all symbol tables
// ----------------------------------------------------------------------------
{
    symbol_table empty;
    names = empty;
    if (rewrites)
    {
        delete rewrites;
        rewrites = NULL;
    }
}



// ============================================================================
//
//    Evaluation of trees
//
// ============================================================================

Tree_p Symbols::Compile(Tree_p source, CompiledUnit &unit,
                        bool nullIfBad, bool keepAlternatives)
// ----------------------------------------------------------------------------
//    Return an optimized version of the source tree, ready to run
// ----------------------------------------------------------------------------
{
    // Make sure that errors are shown in the proper context
    LocalSave<Symbols *> saveSyms(symbols, this);

    // Record rewrites and data declarations in the current context
    DeclarationAction declare(this);
    Tree_p result = source->Do(declare);

    // Compile code for that tree
    CompileAction compile(this, unit, nullIfBad, keepAlternatives);
    result = source->Do(compile);

    // If we didn't compile successfully, report
    if (!result)
    {
        if (nullIfBad)
            return result;
        return Error("Couldn't compile '$1'", source);
    }

    // If we compiled successfully, get the code and store it
    return result;
}


Tree_p Symbols::CompileAll(Tree_p source,
                          bool nullIfBad,
                          bool keepAlternatives)
// ----------------------------------------------------------------------------
//   Compile a top-level tree
// ----------------------------------------------------------------------------
//    This associates an eval_fn to the tree, i.e. code that takes a tree
//    as input and returns a tree as output.
//    keepAlternatives is set by CompileCall to avoid eliding alternatives
//    based on the value of constants, so that if we compile
//    (key "X"), we also generate the code for (key "Y"), knowing that
//    CompileCall may change the constant at run-time. The objective is
//    to avoid re-generating LLVM code for each and every call
//    (it's more difficult to avoid leaking memory from LLVM)
{
    Compiler *compiler = Context::context->compiler;
    TreeList noParms;
    CompiledUnit unit (compiler, source, noParms);
    if (unit.IsForwardCall())
        return source;

    Tree_p result = Compile(source, unit, nullIfBad, keepAlternatives);
    if (!result)
        return result;

    eval_fn fn = unit.Finalize();
    source->code = fn;
    return source;
}


Tree_p Symbols::CompileCall(text callee, TreeList &arglist,
                           bool nullIfBad, bool cached)
// ----------------------------------------------------------------------------
//   Compile a top-level call, reusing calls if possible
// ----------------------------------------------------------------------------
{
    uint arity = arglist.size();
    text key = "";
    if (cached)
    {
        // Build key for this call
        const char keychars[] = "IRTN.[]|";
        std::ostringstream keyBuilder;
        keyBuilder << callee << ":";
        for (uint i = 0; i < arity; i++)
            keyBuilder << keychars[arglist[i]->Kind()];
        key = keyBuilder.str();

        // Check if we already have a call compiled
        if (Tree_p previous = calls[key])
        {
            if (arity)
            {
                // Replace arguments in place if necessary
                Prefix_p pfx = previous->AsPrefix();
                Tree_p *args = &pfx->right;
                while (*args && arity--)
                {
                    Tree_p value = arglist[arity];
                    Tree_p existing = *args;
                    if (arity)
                    {
                        Infix_p infix = existing->AsInfix();
                        args = &infix->left;
                        existing = infix->right;
                    }
                    if (Real_p rs = value->AsReal())
                    {
                        if (Real_p rt = existing->AsReal())
                            rt->value = rs->value;
                        else
                            Error("Real '$1' cannot replace non-real '$2'",
                                  value, existing);
                    }
                    else if (Integer_p is = value->AsInteger())
                    {
                        if (Integer_p it = existing->AsInteger())
                            it->value = is->value;
                        else
                            Error("Integer '$1' cannot replace "
                                  "non-integer '$2'", value, existing);
                    }
                    else if (Text_p ts = value->AsText())
                    {
                        if (Text_p tt = existing->AsText())
                            tt->value = ts->value;
                        else
                            Error("Text '$1' cannot replace non-text '$2'",
                                  value, existing);
                    }
                    else
                    {
                        Error("Call has unsupported type for '$1'", value);
                    }
                }
            }

            // Call the previously compiled code
            return previous;
        }
    }

    Tree_p call = new Name(callee);
    if (arity)
    {
        Tree_p args = arglist[0];
        for (uint a = 1; a < arity; a++)
            args = new Infix(",", args, arglist[a]);
        call = new Prefix(call, args);
    }
    call = CompileAll(call, nullIfBad, true);
    if (cached)
        calls[key] = call;
    return call;
}


Infix_p Symbols::CompileTypeTest(Tree_p type)
// ----------------------------------------------------------------------------
//   Compile a top-level infix, reusing code if possible
// ----------------------------------------------------------------------------
{
    // Check if we already have a call compiled for that type
    if (Tree_p previous = type_tests[type])
        if (Infix_p infix = previous->AsInfix())
            if (infix->code)
                return infix;

    // Create an infix node with two parameters for left and right
    Name_p valueParm = new Name("xl_value_to_typecheck");
    Infix_p call = new Infix(":", valueParm, type);
    TreeList parameters;
    parameters.push_back(valueParm);
    type_tests[type] = call;

    // Create the compilation unit for the infix with two parms
    Compiler *compiler = Context::context->compiler;
    CompiledUnit unit(compiler, call, parameters);
    if (unit.IsForwardCall())
        return call;

    // Create local symbols
    Symbols *locals = new Symbols (Symbols::symbols);

    // Record rewrites and data declarations in the current context
    DeclarationAction declare(locals);
    Tree_p callDecls = call->Do(declare);
    if (!callDecls)
        Error("Internal: Declaration error for call '$1'", callDecls);

    // Compile the body of the rewrite, keep all alternatives open
    CompileAction compile(locals, unit, false, false);
    Tree_p result = callDecls->Do(compile);
    if (!result)
        Error("Unable to compile '$1'", callDecls);

    // Even if technically, this is not an 'eval_fn' (it has more args),
    // we still record it to avoid recompiling multiple times
    eval_fn fn = compile.unit.Finalize();
    call->code = fn;
    return call;
}


Tree_p Symbols::Run(Tree_p code)
// ----------------------------------------------------------------------------
//   Execute a tree by applying the rewrites in the current context
// ----------------------------------------------------------------------------
{
    static uint index = 0;

    // Trace what we are doing
    Tree_p result = code;
    IFTRACE(eval)
        std::cerr << "EVAL" << ++index << ": " << code << '\n';
    uint opt = Options::options->optimize_level;

    // Optimized mode (compiled)
    if (opt)
    {
        // Check trees that we won't rewrite
        bool more =
            has_rewrites_for_constants || 
            !code ||
            !code->IsConstant();

        // Repeat until we get a stable result
        while (more)
        {
            if (!result->code)
            {
                Symbols *symbols = result->Symbols();
                if (!symbols)
                {
                    std::cerr << "WARNING: Tree '" << code
                              << "' has no symbols\n";
                    symbols = this;
                }
                result = symbols->CompileAll(result);
            }
            if (!result->code)
                return Ooops("Unable to compile '$1'", result);
            result = result->code(code);
            more = (result != code && result &&
                    (has_rewrites_for_constants || !result->IsConstant())) ;
            if (more)
                IFTRACE(eval)
                    std::cerr << "LOOP" << index << ": " << result << '\n';
            code = result;
        }
        IFTRACE(eval)
            std::cerr << "RSLT" << index-- << ": " << result << '\n';
        return result;
    } // if (opt)
    

    // If there is compiled code (generated or built-in), use it to evaluate
    if (code->code)
    {
        result = code->code(code);
        IFTRACE(eval)
            std::cerr << "CODE" << index-- << " at "
                      << (void *) code->code << ": "
                      << result << '\n';
        return result;
    }

    // Compute the hash key for the form we have to match
    RewriteKey   formKeyHash;
    code->Do(formKeyHash);

    ulong formKey = formKeyHash.Key();
    bool  found   = false;


    // Copmute the set of symbol tables we need to visit
    symbols_set  visited;
    symbols_list lookups;
    for (Symbols *s = this; s; s = s->Parent())
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

    // Lookup all the symbol tables for the appropriate rewrite
    symbols_list::iterator li;
    Name_p name = code->AsName();
    for (li = lookups.begin(); !found && li != lookups.end(); li++)
    {
        Symbols *s = *li;

        if (name)
        {
            Tree_p named = s->Named(name->value, false);
            if (named)
            {
                if (named->code)
                    result = named->code(named);
                else
                    result = named;
                found = true;
                break;
            }
        }

        Rewrite *candidate = s->Rewrites();
        while (candidate && !found)
        {
            // Compute the hash key for the 'from' of the current rewrite
            RewriteKey testKeyHash;
            candidate->from->Do(testKeyHash);
            ulong testKey = testKeyHash.Key();

            // If we have an exact match for the keys, we may have a winner
            if (testKey == formKey)
            {
                // Check if we can match the arguments
                Symbols args(symbols);
                InterpretedArgumentMatch matchArgs(code, symbols,
                                                   &args, candidate->symbols);
                Tree_p argsTest = candidate->from->Do(matchArgs);
                if (argsTest)
                {
                    // Record that we found something
                    found = true;

                    // If there is a rewrite form
                    if (!candidate->to)
                    {
                        result = argsTest;
                    }
                    else
                    {
                        // We should have same number of args and parms
                        Symbols &parms = *candidate->from->Symbols();
                        ulong parmCount = parms.names.size();
                        if (args.names.size() != parmCount)
                        {
                            symbol_iter a, p;
                            std::cerr << "Args/parms mismatch:\n";
                            std::cerr << "Parms:\n";
                            for (p = parms.names.begin();
                                 p != parms.names.end();
                                 p++)
                            {
                                text name = (*p).first;
                                Tree_p parm = parms.Named(name);
                                std::cerr << "   " << name
                                          << " = " << parm << "\n";
                            }
                            std::cerr << "Args:\n";
                            for (a = args.names.begin();
                                 a != args.names.end();
                                 a++)
                            {
                                text name = (*a).first;
                                Tree_p arg = args.Named(name);
                                std::cerr << "   " << name
                                          << " = " << arg << "\n";
                            }
                        }

                        if (eval_fn toCall = candidate->to->code)
                        {
                            // Map the arguments we found in parameter order
                            TreeList argsList;
                            TreeList::iterator p;
                            TreeList &order = candidate->parameters;
                            for (p = order.begin(); p != order.end(); p++)
                            {
                                Name_p name = (*p)->AsName();
                                Tree_p argValue = args.Named(name->value);
                                argsList.push_back(argValue);
                            }
                            result = xl_invoke(toCall, code,
                                               argsList.size(), &argsList[0]);
                        }
                        else
                        {
                            // Simply evaluate the target with the args set
                            // We will lookup symbols in local symbol table
                            LocalSave<Symbols *> save(Symbols::symbols, &args);
                            result = args.Run(candidate->to);
                        }


                    } // if (data form)
                } // Match args
            } // Match test key

            // Otherwise, go deeper in the hash table
            if (!found && candidate->hash.count(formKey) > 0)
                candidate = candidate->hash[formKey];
            else
                candidate = NULL;
        } // while(candidate)
    } // for(namespaces)


    IFTRACE(eval)
        std::cerr << "VALUE" << index-- << ": " << result << '\n';

    // If we didn't find anything, report it
    if (!found && !code->IsConstant())
        return Error("No rewrite candidate for '$1'", code);

    return result;
}



// ============================================================================
//
//    Error handling
//
// ============================================================================

Tree_p  Symbols::Error(text message, Tree_p arg1, Tree_p arg2, Tree_p arg3)
// ----------------------------------------------------------------------------
//   Execute the innermost error handler
// ----------------------------------------------------------------------------
{
    XLCall call("error");
    call, message;
    if (arg1)
        call, arg1;
    if (arg2)
        call, arg2;
    if (arg3)
        call, arg3;

    Tree_p result = call(this, true, false);
    if (!result)
    {
        // Fallback to displaying error on std::err
        XL::Error err(message, arg1, arg2, arg3);
        err.Display();
        return XL::xl_false;
    }
    return result;
}


bool Symbols::Mark(GCAction &gc)
// ----------------------------------------------------------------------------
//    Mark all elements in a symbol table that we must keep around
// ----------------------------------------------------------------------------
{
    // Don't do this twice, it's expensive
    typedef std::pair<symbols_set::iterator, bool> inserted;
    inserted result = gc.alive_symbols.insert(this);
    if (result.second)
    {
        // Mark all the trees we reference
        for (symbol_iter y = names.begin(); y != names.end(); y++)
            if (Tree_p named = (*y).second)
                named->Do(gc);
        for (symbol_iter call = calls.begin(); call != calls.end(); call++)
            if (Tree_p named = (*call).second)
                named->Do(gc);
        for (value_iter tt = type_tests.begin(); tt != type_tests.end(); tt++)
            if (Tree_p typecheck = (*tt).second)
                typecheck->Do(gc);
        if (rewrites)
            rewrites->Do(gc);

        // Mark all imported symbol tables
        symbols_set::iterator is;
        for (is = imported.begin(); is != imported.end(); is++)
        {
            Symbols *syms = *is;
            syms->Mark(gc);
        }

        // Mark parent if it exists
        if (parent)
            parent->Mark(gc);
    }
    return result.second;
}



// ============================================================================
//
//   Garbage collection
//
// ============================================================================
//   This is just a rather simple mark and sweep garbage collector.

ulong Context::gc_increment = 5;
ulong Context::gc_growth_percent = 100;
Context *Context::context = NULL;


Context::~Context()
// ----------------------------------------------------------------------------
//   Delete all globals allocated by that context
// ----------------------------------------------------------------------------
{
    if (context == this)
        context = NULL;
}


Tree_p *Context::AddGlobal(Tree_p value)
// ----------------------------------------------------------------------------
//   Create a global, immutable address for LLVM
// ----------------------------------------------------------------------------
{
    Tree_p *ptr = new Tree_p ;
    *ptr = value;
    return ptr;
}



void Context::CollectGarbage ()
// ----------------------------------------------------------------------------
//   Mark all active trees
// ----------------------------------------------------------------------------
{
return;
    if (active.size() > gc_threshold)
    {
        GCAction gc;
        ulong deletedCount = 0, activeCount = 0;

        IFTRACE(memory)
            std::cerr << "Garbage collecting...";

        // Mark roots
        for (root_set::iterator a = roots.begin(); a != roots.end(); a++)
            if ((*a)->tree)
                (*a)->tree->Do(gc);

        // Mark root symbol tables
        Symbols::Mark(gc);
        if (Symbols::symbols && Symbols::symbols != this)
            Symbols::symbols->Mark(gc);

        // Mark renderer formats
        formats_table::iterator f;
        formats_table &formats = Renderer::renderer->formats;
        for (f = formats.begin(); f != formats.end(); f++)
            (*f).second->Do(gc);

        // Mark all resources in the LLVM generated code that want to free
        // (this may mark some trees as not eligible for deletion)
        active_set::iterator a;
        if (compiler)
        {
            for (a = active.begin(); a != active.end(); a++)
                if (!gc.alive.count(*a))
                    compiler->FreeResources(*a, gc);
            compiler->FreeResources(gc);
        }

        // Then delete all trees in active set that are no longer referenced
        for (a = active.begin(); a != active.end(); a++)
        {
            activeCount++;
            if (!gc.alive.count(*a))
            {
                deletedCount++;
                delete *a;
            }
        }

        // Same with the symbol tables
        symbols_set::iterator as;
        for (as = active_symbols.begin(); as != active_symbols.end(); as++)
            if (!gc.alive_symbols.count(*as))
                delete *as;

        // Record new state
        active = gc.alive;
        active_symbols = gc.alive_symbols;

        // The new threshold is computed as the sum of the currently active
        // trees (scaled by growth_percent) and the trees we just deleted
        // (scaled by gc_increment)
        gc_threshold = active.size() * gc_growth_percent / 100 +
            deletedCount * gc_increment / 100;

        // Update statistics
        IFTRACE(memory)
            std::cerr << "done: Purged " << deletedCount
                      << " trees out of " << activeCount
                      << " threshold " << gc_threshold << "\n";
    }
}



// ============================================================================
//
//    Interpreted Argument matching - Test input arguments against parameters
//
// ============================================================================

Tree_p InterpretedArgumentMatch::Do(Tree_p)
// ----------------------------------------------------------------------------
//   Default is to return failure
// ----------------------------------------------------------------------------
{
    return NULL;
}


Tree_p InterpretedArgumentMatch::DoInteger(Integer_p what)
// ----------------------------------------------------------------------------
//   An integer argument matches the exact value
// ----------------------------------------------------------------------------
{
    // Evaluate the test value
    Tree_p value = xl_evaluate(test);

    // If this is an integer, compare the values
    Integer_p it = value->AsInteger();
    if (!it)
        return NULL;
    if (it->value == what->value)
        return what;

    return NULL;
}


Tree_p InterpretedArgumentMatch::DoReal(Real_p what)
// ----------------------------------------------------------------------------
//   A real matches the exact value
// ----------------------------------------------------------------------------
{
    // Evaluate the test value
    Tree_p value = xl_evaluate(test);

    // If this is an integer, compare the values
    Real_p rt = value->AsReal();
    if (!rt)
        return NULL;
    if (rt->value == what->value)
        return what;

    return NULL;
}


Tree_p InterpretedArgumentMatch::DoText(Text_p what)
// ----------------------------------------------------------------------------
//   A text matches the exact value
// ----------------------------------------------------------------------------
{
    // Evaluate the test value
    Tree_p value = xl_evaluate(test);

    // If this is an integer, compare the values
    Text_p tt = value->AsText();
    if (!tt)
        return NULL;
    if (tt->value == what->value)
        return what;

    return NULL;
}


Tree_p InterpretedArgumentMatch::DoName(Name_p what)
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
        // If it does, verify equality with the one that already exists
        if (Tree_p existing = rewrite->Named(what->value))
        {
            TreeMatch match(test);
            if (existing->Do(match))
                return existing;
            return NULL;
        }

        // First occurence of the name: enter it in the symbol table
        locals->EnterName(what->value, test);
        return what;
    }
}


Tree_p InterpretedArgumentMatch::DoBlock(Block_p what)
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
        (what->opening == "{" && what->closing == "}") ||
        (what->opening == Block::indent && what->closing == Block::unindent))
    {
        return what->child->Do(this);
    }

    return NULL;
}


Tree_p InterpretedArgumentMatch::DoInfix(Infix_p what)
// ----------------------------------------------------------------------------
//   Check if we match an infix operator
// ----------------------------------------------------------------------------
{
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

        // Evaluate type expression, e.g. 'integer' in example above
        Tree_p typeExpr = xl_evaluate(what->right);
        if (!typeExpr)
            return NULL;

        // REVISIT: Very slow and leaking memory
        // Check if the type matches the value
        Infix_p typeTest = new Infix(":", test, typeExpr,
                                    what->right->Position());
        typeTest->SetSymbols (symbols);
        Tree_p afterCast = xl_evaluate(typeTest);
        if (!afterCast)
            return NULL;

        // Enter the compiled expression in the symbol table
        locals->EnterName(varName->value, afterCast);

        return what;
    }

    // Otherwise, this is a mismatch
    return NULL;
}


Tree_p InterpretedArgumentMatch::DoPrefix(Prefix_p what)
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


Tree_p InterpretedArgumentMatch::DoPostfix(Postfix_p what)
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



// ============================================================================
//
//    Parameter match - Isolate parameters in an rewrite source
//
// ============================================================================

Tree_p ParameterMatch::Do(Tree_p what)
// ----------------------------------------------------------------------------
//   Nothing to do for leaves
// ----------------------------------------------------------------------------
{
    return what;
}


Tree_p ParameterMatch::DoInteger(Integer_p what)
// ----------------------------------------------------------------------------
//   Nothing to do for leaves
// ----------------------------------------------------------------------------
{
    return what;
}


Tree_p ParameterMatch::DoReal(Real_p what)
// ----------------------------------------------------------------------------
//   Nothing to do for leaves
// ----------------------------------------------------------------------------
{
    return what;
}


Tree_p ParameterMatch::DoText(Text_p what)
// ----------------------------------------------------------------------------
//   Nothing to do for leaves
// ----------------------------------------------------------------------------
{
    return what;
}


Tree_p ParameterMatch::DoName(Name_p what)
// ----------------------------------------------------------------------------
//    Identify the parameters being defined in the shape
// ----------------------------------------------------------------------------
{
    if (!defined)
    {
        // The first name we see must match exactly, e.g. 'sin' in 'sin X'
        defined = what;
        return what;
    }
    else
    {
        // Check if the name already exists, e.g. 'false' or 'A+A'
        if (Tree_p existing = symbols->Named(what->value))
            return existing;

        // If first occurence of the name, enter it in symbol table
        Tree_p result = symbols->Allocate(what);
        order.push_back(result);
        return result;
    }
}


Tree_p ParameterMatch::DoBlock(Block_p what)
// ----------------------------------------------------------------------------
//   Parameters in a block belong to the child
// ----------------------------------------------------------------------------
{
    return what->child->Do(this);
}


Tree_p ParameterMatch::DoInfix(Infix_p what)
// ----------------------------------------------------------------------------
//   Check if we match an infix operator
// ----------------------------------------------------------------------------
{
    // Check if we match a type, e.g. 2 vs. 'K : integer'
    if (what->name == ":")
    {
        // Check the variable name, e.g. K in example above
        Name_p varName = what->left->AsName();
        if (!varName)
            return Ooops("Expected a name, got '$1' ", what->left);

        // Check if the name already exists
        if (Tree_p existing = symbols->Named(varName->value))
            return Ooops("Typed name '$1' already exists as '$2'",
                         what->left, existing);

        // Enter the name in symbol table
        Tree_p result = symbols->Allocate(varName);
        order.push_back(result);
        return result;
    }

    // If this is the first one, this is what we define
    if (!defined)
        defined = what;

    // Otherwise, test left and right
    Tree_p lr = what->left->Do(this);
    if (!lr)
        return NULL;
    Tree_p rr = what->right->Do(this);
    if (!rr)
        return NULL;
    return what;
}


Tree_p ParameterMatch::DoPrefix(Prefix_p what)
// ----------------------------------------------------------------------------
//   For prefix expressions, simply test left then right
// ----------------------------------------------------------------------------
{
    Infix_p defined_infix = defined->AsInfix();
    if (defined_infix)
        defined = NULL;

    Tree_p lr = what->left->Do(this);
    if (!lr)
        return NULL;
    Tree_p rr = what->right->Do(this);
    if (!rr)
        return NULL;

    if (!defined && defined_infix)
        defined = defined_infix;

    return what;
}


Tree_p ParameterMatch::DoPostfix(Postfix_p what)
// ----------------------------------------------------------------------------
//    For postfix expressions, simply test right, then left
// ----------------------------------------------------------------------------
{
    // Note that ordering is reverse compared to prefix, so that
    // the 'defined' names is set correctly
    Tree_p rr = what->right->Do(this);
    if (!rr)
        return NULL;
    Tree_p lr = what->left->Do(this);
    if (!lr)
        return NULL;
    return what;
}



// ============================================================================
//
//    Argument matching - Test input arguments against parameters
//
// ============================================================================

Tree_p ArgumentMatch::Compile(Tree_p source)
// ----------------------------------------------------------------------------
//    Compile the source tree, and record we use the value in expr cache
// ----------------------------------------------------------------------------
{
    // Compile the code
    if (!unit.IsKnown(source))
    {
        source = symbols->Compile(source, unit, true);
        if (!source)
            return NULL; // No match
    }
    else
    {
        // Generate code to evaluate the argument
        LocalSave<bool> nib(compile->nullIfBad, true);
        source = source->Do(compile);
    }

    return source;
}


Tree_p ArgumentMatch::CompileValue(Tree_p source)
// ----------------------------------------------------------------------------
//   Compile the source and make sure we evaluate it
// ----------------------------------------------------------------------------
{
    Tree_p result = Compile(source);

    if (result)
    {
        if (Name_p name = result->AsName())
        {
            llvm::BasicBlock * bb = unit.BeginLazy(name);
            unit.NeedStorage(name);
            if (!name->Symbols())
                name->SetSymbols(symbols);
            unit.CallEvaluate(name);
            unit.EndLazy(name, bb);
        }
    }
    return result;
}


Tree_p ArgumentMatch::CompileClosure(Tree_p source)
// ----------------------------------------------------------------------------
//    Compile the source tree for lazy evaluation, i.e. wrap in code
// ----------------------------------------------------------------------------
{
    // Compile leaves normally
    if (source->IsLeaf())
        return Compile(source);

    // For more complex expression, return a constant tree
    unit.ConstantTree(source);

    // Record which elements of the expression are captured from context
    Context *context = Context::context;
    Compiler *compiler = context->compiler;
    EnvironmentScan env(symbols);
    Tree_p envOK = source->Do(env);
    if (!envOK)
        return Ooops("Internal: what environment in '$1'?", source);

    // Create the parameter list with all imported locals
    TreeList parms, args;
    capture_table::iterator c;
    for (c = env.captured.begin(); c != env.captured.end(); c++)
    {
        Tree_p name = (*c).first;
        Symbols *where = (*c).second;
        if (where == context || where == Symbols::symbols)
        {
            // This is a global, we'll find it running the target.
        }
        else if (unit.IsKnown(name))
        {
            // This is a local: simply pass it around
            parms.push_back(name);
            args.push_back(name);
        }
        else
        {
            // This is a local 'name' like a form definition
            // We don't need to pass these around.
        }
    }

    // Create the compilation unit and check if we are already compiling this
    // Can this happen for a closure?
    CompiledUnit subUnit(compiler, source, parms);
    if (!subUnit.IsForwardCall())
    {
        Tree_p result = symbols->Compile(source, subUnit, true);
        if (!result)
            unit.ConstantTree(source);

        eval_fn fn = subUnit.Finalize();
        source->code = fn;
    }

    // Create a call to xl_new_closure to save the required trees
    unit.CreateClosure(source, args);

    return source;
}


Tree_p ArgumentMatch::Do(Tree_p)
// ----------------------------------------------------------------------------
//   Default is to return failure
// ----------------------------------------------------------------------------
{
    return NULL;
}


Tree_p ArgumentMatch::DoInteger(Integer_p what)
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
        if (!compile->keepAlternatives)
        {
            if (it->value == what->value)
                return what;
            return NULL;
        }
    }

    // Compile the test tree
    Tree_p compiled = CompileValue(test);
    if (!compiled)
        return NULL;

    // Compare at run-time the actual tree value with the test value
    unit.IntegerTest(compiled, what->value);
    return compiled;
}


Tree_p ArgumentMatch::DoReal(Real_p what)
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
        if (!compile->keepAlternatives)
        {
            if (rt->value == what->value)
                return what;
            return NULL;
        }
    }

    // Compile the test tree
    Tree_p compiled = CompileValue(test);
    if (!compiled)
        return NULL;

    // Compare at run-time the actual tree value with the test value
    unit.RealTest(compiled, what->value);
    return compiled;
}


Tree_p ArgumentMatch::DoText(Text_p what)
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
        if (!compile->keepAlternatives)
        {
            if (tt->value == what->value)
                return what;
            return NULL;
        }
    }

    // Compile the test tree
    Tree_p compiled = CompileValue(test);
    if (!compiled)
        return NULL;

    // Compare at run-time the actual tree value with the test value
    unit.TextTest(compiled, what->value);
    return compiled;
}


Tree_p ArgumentMatch::DoName(Name_p what)
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
                    return NULL;
                }
            }

            // Insert a dynamic tree comparison test
            Tree_p testCode = Compile(test);
            if (!testCode)
                return NULL;
            Tree_p thisCode = Compile(existing);
            if (!thisCode)
                return NULL;
            unit.ShapeTest(testCode, thisCode);

            // Return compilation success
            return what;
        }

        // If first occurence of the name, enter it in symbol table
        Tree_p compiled = CompileClosure(test);
        if (!compiled)
            return NULL;

        locals->EnterName(what->value, compiled);
        return what;
    }
}


Tree_p ArgumentMatch::DoBlock(Block_p what)
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
        (what->opening == "{" && what->closing == "}") ||
        (what->opening == Block::indent && what->closing == Block::unindent))
    {
        return what->child->Do(this);
    }

    return NULL;
}


Tree_p ArgumentMatch::DoInfix(Infix_p what)
// ----------------------------------------------------------------------------
//   Check if we match an infix operator
// ----------------------------------------------------------------------------
{
    // Check if we match an infix tree like 'x,y' with a name like 'A'
    if (what->name != ":")
    {
        if (Name_p name = test->AsName())
        {
            // Evaluate 'A' to see if we will get something like x,y
            Tree_p compiled = CompileValue(name);
            if (!compiled)
                return NULL;

            // Build an infix tree corresponding to what we extract
            Name_p left = new Name("left");
            Name_p right = new Name("right");
            Infix_p extracted = new Infix(what->name, left, right);

            // Extract the infix parameters from actual value
            unit.InfixMatchTest(compiled, extracted);

            // Proceed with the infix we extracted to map the remaining args
            test = extracted;
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

        // Evaluate type expression, e.g. 'integer' in example above
        Tree_p typeExpr = Compile(what->right);
        if (!typeExpr)
            return NULL;

        // Compile what we are testing against
        Tree_p compiled = Compile(test);
        if (!compiled)
            return NULL;

        // Insert a run-time type test
        unit.TypeTest(compiled, typeExpr);

        // Enter the compiled expression in the symbol table
        locals->EnterName(varName->value, compiled);

        return what;
    }

    // Otherwise, this is a mismatch
    return NULL;
}


Tree_p ArgumentMatch::DoPrefix(Prefix_p what)
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


Tree_p ArgumentMatch::DoPostfix(Postfix_p what)
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



// ============================================================================
//
//    Environment scan - Identify which names are imported from context
//
// ============================================================================

Tree_p EnvironmentScan::Do(Tree_p what)
// ----------------------------------------------------------------------------
//   Nothing to do for leaves
// ----------------------------------------------------------------------------
{
    return what;
}


Tree_p EnvironmentScan::DoInteger(Integer_p what)
// ----------------------------------------------------------------------------
//   Nothing to do for leaves
// ----------------------------------------------------------------------------
{
    return what;
}


Tree_p EnvironmentScan::DoReal(Real_p what)
// ----------------------------------------------------------------------------
//   Nothing to do for leaves
// ----------------------------------------------------------------------------
{
    return what;
}


Tree_p EnvironmentScan::DoText(Text_p what)
// ----------------------------------------------------------------------------
//   Nothing to do for leaves
// ----------------------------------------------------------------------------
{
    return what;
}


Tree_p EnvironmentScan::DoName(Name_p what)
// ----------------------------------------------------------------------------
//    Check if name is found in context, if so record where we took it from
// ----------------------------------------------------------------------------
{
    for (Symbols *s = symbols; s; s = s->Parent())
    {
        if (Tree_p existing = s->Named(what->value, false))
        {
            // Found the symbol in the given symbol table
            if (!captured.count(existing))
                captured[existing] = s;
            break;
        }
    }
    return what;
}


Tree_p EnvironmentScan::DoBlock(Block_p what)
// ----------------------------------------------------------------------------
//   Parameters in a block are in its child
// ----------------------------------------------------------------------------
{
    return what->child->Do(this);
}


Tree_p EnvironmentScan::DoInfix(Infix_p what)
// ----------------------------------------------------------------------------
//   Check if we match an infix operator
// ----------------------------------------------------------------------------
{
    // Test left and right
    what->left->Do(this);
    what->right->Do(this);
    return what;
}


Tree_p EnvironmentScan::DoPrefix(Prefix_p what)
// ----------------------------------------------------------------------------
//   For prefix expressions, simply test left then right
// ----------------------------------------------------------------------------
{
    what->left->Do(this);
    what->right->Do(this);
    return what;
}


Tree_p EnvironmentScan::DoPostfix(Postfix_p what)
// ----------------------------------------------------------------------------
//    For postfix expressions, simply test right, then left
// ----------------------------------------------------------------------------
{
    // Order shouldn't really matter here (unlike ParameterMach)
    what->right->Do(this);
    what->left->Do(this);
    return what;
}



// ============================================================================
//
//   BuildChildren action: Build a non-leaf after evaluating children
//
// ============================================================================

BuildChildren::BuildChildren(CompileAction *comp)
// ----------------------------------------------------------------------------
//   Constructor saves the unit's nullIfBad and sets it
// ----------------------------------------------------------------------------
    : compile(comp), unit(comp->unit), saveNullIfBad(comp->nullIfBad)
{
    comp->nullIfBad = true;
}


BuildChildren::~BuildChildren()
// ----------------------------------------------------------------------------
//   Destructor restores the original nullIfBad settigns
// ----------------------------------------------------------------------------
{
    compile->nullIfBad = saveNullIfBad;
}


Tree_p BuildChildren::DoPrefix(Prefix_p what)
// ----------------------------------------------------------------------------
//   Evaluate children, then build a prefix
// ----------------------------------------------------------------------------
{
    unit.Left(what);
    what->left->Do(compile);
    unit.Right(what);
    what->right->Do(compile);
    unit.CallNewPrefix(what);
    return what;
}


Tree_p BuildChildren::DoPostfix(Postfix_p what)
// ----------------------------------------------------------------------------
//   Evaluate children, then build a postfix
// ----------------------------------------------------------------------------
{
    unit.Left(what);
    what->left->Do(compile);
    unit.Right(what);
    what->right->Do(compile);
    unit.CallNewPostfix(what);
    return what;
}


Tree_p BuildChildren::DoInfix(Infix_p what)
// ----------------------------------------------------------------------------
//   Evaluate children, then build an infix
// ----------------------------------------------------------------------------
{
    unit.Left(what);
    what->left->Do(compile);
    unit.Right(what);
    what->right->Do(compile);
    unit.CallNewInfix(what);
    return what;
}


Tree_p BuildChildren::DoBlock(Block_p what)
// ----------------------------------------------------------------------------
//   Evaluate children, then build a new block
// ----------------------------------------------------------------------------
{
    unit.Left(what);
    what->child->Do(compile);
    unit.CallNewBlock(what);
    return what;
}


// ============================================================================
//
//   Declaration action - Enter all tree rewrites in the current symbols
//
// ============================================================================

Tree_p DeclarationAction::Do(Tree_p what)
// ----------------------------------------------------------------------------
//   Default is to leave trees alone (for native trees)
// ----------------------------------------------------------------------------
{
    return what;
}


Tree_p DeclarationAction::DoInteger(Integer_p what)
// ----------------------------------------------------------------------------
//   Integers evaluate directly
// ----------------------------------------------------------------------------
{
    return what;
}


Tree_p DeclarationAction::DoReal(Real_p what)
// ----------------------------------------------------------------------------
//   Reals evaluate directly
// ----------------------------------------------------------------------------
{
    return what;
}


Tree_p DeclarationAction::DoText(Text_p what)
// ----------------------------------------------------------------------------
//   Text evaluates directly
// ----------------------------------------------------------------------------
{
    return what;
}


Tree_p DeclarationAction::DoName(Name_p what)
// ----------------------------------------------------------------------------
//   Build a unique reference in the context for the entity
// ----------------------------------------------------------------------------
{
    return what;
}


Tree_p DeclarationAction::DoBlock(Block_p what)
// ----------------------------------------------------------------------------
//   Declarations in a block belong to the child, not to us
// ----------------------------------------------------------------------------
{
    return what->child->Do(this);
}


Tree_p DeclarationAction::DoInfix(Infix_p what)
// ----------------------------------------------------------------------------
//   Compile built-in operators: \n ; -> and :
// ----------------------------------------------------------------------------
{
    // Check if this is an instruction list
    if (what->name == "\n" || what->name == ";")
    {
        // For instruction list, string declaration results together
        what->left->Do(this);
        what->right->Do(this);
        return what;
    }

    // Check if this is a rewrite declaration
    if (what->name == "->")
    {
        // Enter the rewrite
        EnterRewrite(what->left, what->right);
        return what;
    }

    return what;
}


Tree_p DeclarationAction::DoPrefix(Prefix_p what)
// ----------------------------------------------------------------------------
//    All prefix operations translate into a rewrite
// ----------------------------------------------------------------------------
{
    // Deal with 'data' declarations and 'load' statements
    if (Name_p name = what->left->AsName())
    {
        if (name->value == "data")
        {
            EnterRewrite(what->right, NULL);
            return what;
        }
        if (name->value == "load")
        {
            Text_p file = what->right->AsText();
            if (!file)
                return Ooops("Argument '$1' to 'load' is not a text",
                             what->right);
            return xl_load(file->value);
        }
    }

    return what;
}


Tree_p DeclarationAction::DoPostfix(Postfix_p what)
// ----------------------------------------------------------------------------
//    All postfix operations translate into a rewrite
// ----------------------------------------------------------------------------
{
    return what;
}


void DeclarationAction::EnterRewrite(Tree_p defined, Tree_p definition)
// ----------------------------------------------------------------------------
//   Add a definition in the current context
// ----------------------------------------------------------------------------
{
    if (Name_p name = defined->AsName())
    {
        symbols->EnterName(name->value, definition ? definition : name);
    }
    else
    {
        Rewrite *rewrite = new Rewrite(symbols, defined, definition);
        symbols->EnterRewrite(rewrite);
    }
}



// ============================================================================
//
//   Compilation action - Generation of "optimized" native trees
//
// ============================================================================

CompileAction::CompileAction(Symbols *s, CompiledUnit &u, bool nib, bool ka)
// ----------------------------------------------------------------------------
//   Constructor
// ----------------------------------------------------------------------------
    : symbols(s), unit(u), nullIfBad(nib), keepAlternatives(ka)
{}


Tree_p CompileAction::Do(Tree_p what)
// ----------------------------------------------------------------------------
//   Default is to leave trees alone (for native trees)
// ----------------------------------------------------------------------------
{
    return what;
}


Tree_p CompileAction::DoInteger(Integer_p what)
// ----------------------------------------------------------------------------
//   Integers evaluate directly
// ----------------------------------------------------------------------------
{
    unit.ConstantInteger(what);
    return what;
}


Tree_p CompileAction::DoReal(Real_p what)
// ----------------------------------------------------------------------------
//   Reals evaluate directly
// ----------------------------------------------------------------------------
{
    unit.ConstantReal(what);
    return what;
}


Tree_p CompileAction::DoText(Text_p what)
// ----------------------------------------------------------------------------
//   Text evaluates directly
// ----------------------------------------------------------------------------
{
    unit.ConstantText(what);
    return what;
}


Tree_p CompileAction::DoName(Name_p what)
// ----------------------------------------------------------------------------
//   Build a unique reference in the context for the entity
// ----------------------------------------------------------------------------
{
    // Normally, the name should have been declared in ParameterMatch
    if (Tree_p result = symbols->Named(what->value))
    {
        // Try to compile the definition of the name
        if (!result->AsName())
        {
            Rewrite rw(symbols, what, result);
            if (!what->Symbols())
                what->SetSymbols(symbols);
            result = rw.Compile();
        }

        // Check if there is code we need to call
        Compiler *compiler = Context::context->compiler;
        if (compiler->functions.count(result) &&
            compiler->functions[result] != unit.function)
        {
            // Case of "Name -> Foo": Invoke Name
            TreeList noArgs;
            unit.NeedStorage(what);
            unit.Invoke(what, result, noArgs);
            return what;
        }
        else if (unit.value.count(result))
        {
            // Case of "Foo(A,B) -> B" with B: evaluate B lazily
            unit.Copy(result, what, false);
            return what;
        }
        else
        {
            // Return the name itself by default
            unit.ConstantTree(result);
            unit.Copy(result, what);
            if (!result->Symbols())
                result->SetSymbols(symbols);
        }

        return result;
    }
    if (nullIfBad)
    {
        unit.ConstantTree(what);
        return what;
    }
    return Ooops("Name '$1' does not exist", what);
}


Tree_p CompileAction::DoBlock(Block_p what)
// ----------------------------------------------------------------------------
//   Optimize away indent or parenthese blocks, evaluate others
// ----------------------------------------------------------------------------
{
    if ((what->opening == Block::indent && what->closing == Block::unindent) ||
        (what->opening == "{" && what->closing == "}") ||
        (what->opening == "(" && what->closing == ")"))
    {
        if (unit.IsKnown(what))
            unit.Copy(what, what->child, false);
        Tree_p result = what->child->Do(this);
        if (!result)
            return NULL;
        if (unit.IsKnown(what->child))
        {
            if (!what->child->Symbols())
                what->child->SetSymbols(symbols);
        }
        unit.Copy(result, what);
        return what;
    }

    // In other cases, we need to evaluate rewrites
    return Rewrites(what);
}


Tree_p CompileAction::DoInfix(Infix_p what)
// ----------------------------------------------------------------------------
//   Compile built-in operators: \n ; -> and :
// ----------------------------------------------------------------------------
{
    // Check if this is an instruction list
    if (what->name == "\n" || what->name == ";")
    {
        // For instruction list, string compile results together
        if (!what->left->Do(this))
            return NULL;
        if (unit.IsKnown(what->left))
        {
            if (!what->left->Symbols())
                what->left->SetSymbols(symbols);
        }
        if (!what->right->Do(this))
            return NULL;
        if (unit.IsKnown(what->right))
        {
            if (!what->right->Symbols())
                what->right->SetSymbols(symbols);
            unit.Copy(what->right, what);
        }
        else if (unit.IsKnown(what->left))
        {
            unit.Copy(what->left, what);
        }
        return what;
    }

    // Check if this is a rewrite declaration
    if (what->name == "->")
    {
        // If so, skip, this has been done in DeclarationAction
        return what;
    }

    // In all other cases, look up the rewrites
    return Rewrites(what);
}


Tree_p CompileAction::DoPrefix(Prefix_p what)
// ----------------------------------------------------------------------------
//    Deal with data declarations, otherwise translate as a rewrite
// ----------------------------------------------------------------------------
{
    if (Name_p name = what->left->AsName())
    {
        if (name->value == "data")
            return what;
    }
    return Rewrites(what);
}


Tree_p CompileAction::DoPostfix(Postfix_p what)
// ----------------------------------------------------------------------------
//    All postfix operations translate into a rewrite
// ----------------------------------------------------------------------------
{
    return Rewrites(what);
}


Tree_p  CompileAction::Rewrites(Tree_p what)
// ----------------------------------------------------------------------------
//   Build code selecting among rewrites in current context
// ----------------------------------------------------------------------------
{
    // Compute the hash key for the form we have to match
    ulong formKey, testKey;
    RewriteKey formKeyHash;
    what->Do(formKeyHash);
    formKey = formKeyHash.Key();
    bool foundUnconditional = false;
    bool foundSomething = false;
    ExpressionReduction reduction (unit, what);
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
    for (li = lookups.begin(); !foundUnconditional && li != lookups.end(); li++)
    {
        Symbols *s = *li;

        Rewrite *candidate = s->Rewrites();
        while (candidate && !foundUnconditional)
        {
            // Compute the hash key for the 'from' of the current rewrite
            RewriteKey testKeyHash;
            candidate->from->Do(testKeyHash);
            testKey = testKeyHash.Key();

            // If we have an exact match for the keys, we may have a winner
            if (testKey == formKey)
            {
                // Create the invokation point
                reduction.NewForm();
                Symbols args(symbols);
                ArgumentMatch matchArgs(what,
                                        symbols, &args, candidate->symbols,
                                        this);
                Tree_p argsTest = candidate->from->Do(matchArgs);
                if (argsTest)
                {
                    // Record that we found something
                    foundSomething = true;

                    // If this is a data form, we are done
                    if (!candidate->to)
                    {
                        unit.ConstantTree(what);
                        foundUnconditional = !unit.failbb;
                        BuildChildren children(this);
                        what = what->Do(children);
                        unit.noeval.insert(what);
                        reduction.Succeeded();
                    }
                    else
                    {
                        // We should have same number of args and parms
                        Symbols &parms = *candidate->from->Symbols();
                        ulong parmCount = parms.names.size();
                        if (args.names.size() != parmCount)
                        {
                            symbol_iter a, p;
                            std::cerr << "Args/parms mismatch:\n";
                            std::cerr << "Parms:\n";
                            for (p = parms.names.begin();
                                 p != parms.names.end();
                                 p++)
                            {
                                text name = (*p).first;
                                Tree_p parm = parms.Named(name);
                                std::cerr << "   " << name
                                          << " = " << parm << "\n";
                            }
                            std::cerr << "Args:\n";
                            for (a = args.names.begin();
                                 a != args.names.end();
                                 a++)
                            {
                                text name = (*a).first;
                                Tree_p arg = args.Named(name);
                                std::cerr << "   " << name
                                          << " = " << arg << "\n";
                            }
                        }

                        // Map the arguments we found in parameter order
                        TreeList argsList;
                        TreeList::iterator p;
                        TreeList &order = candidate->parameters;
                        for (p = order.begin(); p != order.end(); p++)
                        {
                            Name_p name = (*p)->AsName();
                            Tree_p argValue = args.Named(name->value);
                            argsList.push_back(argValue);
                        }

                        // Compile the candidate
                        Tree_p code = candidate->Compile();

                        // Invoke the candidate
                        unit.Invoke(what, code, argsList);

                        // If there was no test code, don't keep testing further
                        foundUnconditional = !unit.failbb;

                        // This is the end of a successful invokation
                        reduction.Succeeded();
                    } // if (data form)
                } // Match args
                else
                {
                    // Indicate unsuccessful invokation
                    reduction.Failed();
                }
            } // Match test key

            // Otherwise, check if we have a key match in the hash table,
            // and if so follow it.
            if (!foundUnconditional && candidate->hash.count(formKey) > 0)
                candidate = candidate->hash[formKey];
            else
                candidate = NULL;
        } // while(candidate)
    } // for(namespaces)

    // If we didn't match anything, then emit an error at runtime
    if (!foundUnconditional && !nullIfBad)
        unit.CallTypeError(what);

    // If we didn't find anything, report it
    if (!foundSomething)
    {
        if (nullIfBad)
        {
            BuildChildren children(this);
            what = what->Do(children);
            return NULL;
        }
        return Ooops("No rewrite candidate for '$1'", what);
    }

    // Set the symbols for the result
    if (!what->Symbols())
        what->SetSymbols(symbols);

    return what;
}



// ============================================================================
//
//    Tree rewrites
//
// ============================================================================

Rewrite::~Rewrite()
// ----------------------------------------------------------------------------
//   Deletes all children rewrite if any
// ----------------------------------------------------------------------------
{
    rewrite_table::iterator it;
    for (it = hash.begin(); it != hash.end(); it++)
        delete ((*it).second);
}


Rewrite *Rewrite::Add (Rewrite *rewrite)
// ----------------------------------------------------------------------------
//   Add a new rewrite at the right place in an existing rewrite
// ----------------------------------------------------------------------------
{
    Rewrite *parent = this;
    ulong formKey;

    // Compute the hash key for the form we have to match
    RewriteKey formKeyHash;
    rewrite->from->Do(formKeyHash);
    formKey = formKeyHash.Key();

    while (parent)
    {
        // Check if we have a key match in the hash table,
        // and if so follow it.
        if (parent->hash.count(formKey) > 0)
        {
            parent = parent->hash[formKey];
        }
        else
        {
            parent->hash[formKey] = rewrite;
            return parent;
        }
    }

    return NULL;
}


Tree_p Rewrite::Do(Action &a)
// ----------------------------------------------------------------------------
//   Apply an action to the 'from' and 'to' fields and all referenced trees
// ----------------------------------------------------------------------------
{
    Tree_p result = from->Do(a);
    if (to)
        result = to->Do(a);
    for (rewrite_table::iterator i = hash.begin(); i != hash.end(); i++)
        result = (*i).second->Do(a);
    for (TreeList::iterator p=parameters.begin(); p!=parameters.end(); p++)
        result = (*p)->Do(a);
    return result;
}


Tree_p Rewrite::Compile(void)
// ----------------------------------------------------------------------------
//   Compile code for the 'to' form
// ----------------------------------------------------------------------------
//   This is similar to Context::Compile, except that it may generate a
//   function with more parameters, i.e. Tree_p f(Tree_p , Tree_p , ...),
//   where there is one input arg per variable in the 'from' tree
{
    assert (to || !"Rewrite::Compile called for data rewrite?");
    if (to->code)
        return to;

    Compiler *compiler = Context::context->compiler;

    // Create the compilation unit and check if we are already compiling this
    CompiledUnit unit(compiler, to, parameters);
    if (unit.IsForwardCall())
    {
        // Recursive compilation of that form
        return to;              // We know how to invoke it anyway
    }

    // Check that we had symbols defined for the 'from' tree
    if (!from->Symbols())
        return Ooops("Internal: No symbols for '$1'", from);

    // Create local symbols
    Symbols *locals = new Symbols (from->Symbols());

    // Record rewrites and data declarations in the current context
    DeclarationAction declare(locals);
    Tree_p toDecl = to->Do(declare);
    if (!toDecl)
        return Ooops("Internal: Declaration error for '$1'", to);

    // Compile the body of the rewrite
    CompileAction compile(locals, unit, false, false);
    Tree_p result = to->Do(compile);
    if (!result)
        return Ooops("Unable to compile '$1'", to);

    // Even if technically, this is not an 'eval_fn' (it has more args),
    // we still record it to avoid recompiling multiple times
    eval_fn fn = compile.unit.Finalize();
    to->code = fn;

    return to;
}

XL_END


extern "C" void debugrw(XL::Rewrite *r)
// ----------------------------------------------------------------------------
//   For the debugger, dump a rewrite
// ----------------------------------------------------------------------------
{
    if (r)
    {
        std::cerr << r->from << " -> " << r->to << "\n";
        XL::rewrite_table::iterator i;
        for (i = r->hash.begin(); i != r->hash.end(); i++)
            debugrw((*i).second);
    }
}


extern "C" void debugs(XL::Symbols *s)
// ----------------------------------------------------------------------------
//   For the debugger, dump a symbol table
// ----------------------------------------------------------------------------
{
    using namespace XL;
    std::cerr << "SYMBOLS AT " << s << "\n";

    std::cerr << "NAMES:\n";
    symbol_table::iterator i;
    for (i = s->names.begin(); i != s->names.end(); i++)
        std::cerr << (*i).first << ": " << (*i).second << "\n";

    std::cerr << "REWRITES:\n";
    debugrw(s->rewrites);
}


extern "C" void debugsc(XL::Symbols *s)
// ----------------------------------------------------------------------------
//   For the debugger, dump a symbol table
// ----------------------------------------------------------------------------
{
    using namespace XL;
    if (s)
    {
        debugsc(s->Parent());
        debugs(s);
    }
}
