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

Tree *Symbols::Named(text name, bool deep)
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


void Symbols::EnterName(text name, Tree *value)
// ----------------------------------------------------------------------------
//   Enter a value in the namespace
// ----------------------------------------------------------------------------
{
    names[name] = value;
}


Name *Symbols::Allocate(Name *n)
// ----------------------------------------------------------------------------
//   Enter a value in the namespace
// ----------------------------------------------------------------------------
{
    if (Tree *existing = names[n->value])
    {
        if (Name *name = existing->AsName())
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
    // Create symbol table for this rewrite
    Symbols *locals = new Symbols(this);
    rw->from->SetSymbols(locals);

    // Enter parameters in the symbol table
    ParameterMatch parms(locals);
    Tree *check = rw->from->Do(parms);
    if (!check)
        Error("Parameter error for '$1'", rw->from);
    rw->parameters = parms.order;

    // If we are defining a name, store the definition in the symbols
    if (Name *name = parms.defined->AsName())
        Allocate(name);

    if (rewrites)
    {
        /* Returns parent */ rewrites->Add(rw);
        return rw;
    }
    rewrites = rw;
    return rw;
}


Rewrite *Symbols::EnterRewrite(Tree *from, Tree *to)
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

Tree *Symbols::Compile(Tree *source, CompiledUnit &unit,
                       bool nullIfBad, bool keepAlternatives)
// ----------------------------------------------------------------------------
//    Return an optimized version of the source tree, ready to run
// ----------------------------------------------------------------------------
{
    // Record rewrites and data declarations in the current context
    DeclarationAction declare(this);
    Tree *result = source->Do(declare);

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


Tree *Symbols::CompileAll(Tree *source, bool keepAlternatives)
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
    tree_list noParms;
    CompiledUnit unit (compiler, source, noParms);
    if (unit.IsForwardCall())
        return source;

    Tree *result = Compile(source, unit, false, keepAlternatives);
    if (!result)
        return result;

    eval_fn fn = unit.Finalize();
    source->code = fn;
    return source;
}


Tree *Symbols::CompileCall(text callee, tree_list &arglist)
// ----------------------------------------------------------------------------
//   Compile a top-level call, reusing calls if possible
// ----------------------------------------------------------------------------
{
    // Build key for this call
    uint arity = arglist.size();
    std::ostringstream keyBuilder;
    keyBuilder << callee << ":" << arity;
    text key = keyBuilder.str();

    // Check if we already have a call compiled
    if (Tree *previous = calls[key])
    {
        if (arity)
        {
            // Replace arguments in place if necessary
            Prefix *pfx = previous->AsPrefix();
            Tree **args = &pfx->right;
            while (*args && arity--)
            {
                Tree *value = arglist[arity];
                Tree *existing = *args;
                if (arity)
                {
                    Infix *infix = existing->AsInfix();
                    args = &infix->left;
                    existing = infix->right;
                }
                if (Real *rs = value->AsReal())
                {
                    if (Real *rt = existing->AsReal())
                        rt->value = rs->value;
                    else
                        Error("Real '$1' cannot replace non-real '$2'",
                              value, existing);
                }
                else if (Integer *is = value->AsInteger())
                {
                    if (Integer *it = existing->AsInteger())
                        it->value = is->value;
                    else
                        Error("Integer '$1' cannot replace non-integer '$2'",
                              value, existing);
                }
                else if (Text *ts = value->AsText())
                {
                    if (Text *tt = existing->AsText())
                        tt->value = ts->value;
                    else
                        Error("Text '$1' cannot replace non-text '$2'",
                              value, existing);
                }
            }
        }

        // Call the previously compiled code
        return previous;
    }

    Tree *call = new Name(callee);
    if (arity)
    {
        Tree *args = arglist[0];
        for (uint a = 1; a < arity; a++)
            args = new Infix(",", args, arglist[a]);
        call = new Prefix(call, args);
    }
    call = CompileAll(call, true);
    calls[key] = call;
    return call;
}


Tree *Symbols::Run(Tree *code)
// ----------------------------------------------------------------------------
//   Execute a compiled code tree - Very similar to xl_evaluate
// ----------------------------------------------------------------------------
{
    Tree *result = code;
    if (!result)
        return result;

     IFTRACE(eval)
        std::cerr << "RUN: " << code << '\n';

   if (!result->IsConstant())
    {
        if (!result->code)
        {
            Symbols *symbols = result->symbols;
            if (!symbols)
            {
                std::cerr << "WARNING: Tree '" << code << "' has no symbols\n";
                symbols = this;
            }
            result = symbols->CompileAll(result);
        }

        assert(result->code);
        result = result->code(code);
    }
    return result;
}



// ============================================================================
//
//   Garbage collection
//
// ============================================================================
//   This is just a rather simple mark and sweep garbage collector.

ulong Context::gc_increment = 200;
ulong Context::gc_growth_percent = 200;
Context *Context::context = NULL;


Context::~Context()
// ----------------------------------------------------------------------------
//   Delete all globals allocated by that context
// ----------------------------------------------------------------------------
{
    globals_table::iterator g;
    for (g = globals.begin(); g != globals.end(); g++)
        delete *g;
}


Tree **Context::AddGlobal(Tree *value)
// ----------------------------------------------------------------------------
//   Create a global, immutable address for LLVM
// ----------------------------------------------------------------------------
{
    Tree **ptr = new Tree*;
    *ptr = value;
    return ptr;
}


struct GCAction : Action
// ----------------------------------------------------------------------------
//   Mark trees for garbage collection and compute active set
// ----------------------------------------------------------------------------
{
    GCAction (): alive() {}
    ~GCAction () {}

    bool Mark(Tree *what)
    {
        typedef std::pair<active_set::iterator, bool> inserted;
        inserted ins = alive.insert(what);
        if (what->symbols) alive_symbols.insert(what->symbols);
        return ins.second;
    }
    Tree *Do(Tree *what)
    {
        Mark(what);
        return what;
    }
    Tree *DoBlock(Block *what)
    {
        if (Mark(what))
            Action::DoBlock(what);              // Do child
        return what;
    }
    Tree *DoInfix(Infix *what)
    {
        if (Mark(what))
            Action::DoInfix(what);              // Do children
        return what;
    }
    Tree *DoPrefix(Prefix *what)
    {
        if (Mark(what))
            Action::DoPrefix(what);             // Do children
        return what;
    }
    Tree *DoPostfix(Postfix *what)
    {
        if (Mark(what))
            Action::DoPostfix(what);            // Do children
        return what;
    }
    active_set  alive;
    symbols_set alive_symbols;
};


void Context::CollectGarbage ()
// ----------------------------------------------------------------------------
//   Mark all active trees
// ----------------------------------------------------------------------------
{
    if (active.size() > gc_threshold)
    {
        GCAction gc;
        ulong deletedCount = 0, activeCount = 0;

        IFTRACE(memory)
            std::cerr << "Garbage collecting...";

        // Mark roots, names, rewrites and stack
        for (root_set::iterator a = roots.begin(); a != roots.end(); a++)
            if ((*a)->tree)
                (*a)->tree->Do(gc);
        for (symbol_iter y = names.begin(); y != names.end(); y++)
            if (Tree *named = (*y).second)
                named->Do(gc);
        for (symbol_iter call = calls.begin(); call != calls.end(); call++)
            if (Tree *named = (*call).second)
                named->Do(gc);
        if (rewrites)
            rewrites->Do(gc);

        formats_table::iterator f;
        formats_table &formats = Renderer::renderer->formats;
        for (f = formats.begin(); f != formats.end(); f++)
            (*f).second->Do(gc);

        globals_table::iterator g;
        for (g = globals.begin(); g != globals.end(); g++)
            (**g)->Do(gc);

        // Then delete all trees in active set that are no longer referenced
        for (active_set::iterator a = active.begin();
             a != active.end();
             a++)
        {
            activeCount++;
            if (!gc.alive.count(*a))
            {
                deletedCount++;
                if (compiler)
                    compiler->FreeResources(*a);
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

        // Update statistics
        gc_threshold = active.size() * gc_growth_percent / 100 + gc_increment;
        IFTRACE(memory)
            std::cerr << "done: Purged " << deletedCount
                      << " trees out of " << activeCount
                      << " threshold " << gc_threshold << "\n";
    }
}



// ============================================================================
//
//    Hash key for tree rewrite
//
// ============================================================================
//    We use this hashing key to quickly determine if two trees "look the same"

struct RewriteKey : Action
// ----------------------------------------------------------------------------
//   Compute a hashing key for a rewrite
// ----------------------------------------------------------------------------
{
    RewriteKey (ulong base = 0): key(base) {}
    ulong Key()  { return key; }

    ulong Hash(ulong id, text t)
    {
        ulong result = 0xC0DED;
        text::iterator p;
        for (p = t.begin(); p != t.end(); p++)
            result = (result * 0x301) ^ *p;
        return id | (result << 3);
    }
    ulong Hash(ulong id, ulong value)
    {
        return id | (value << 3);
    }

    Tree *DoInteger(Integer *what)
    {
        key = (key << 3) ^ Hash(0, what->value);
        return what;
    }
    Tree *DoReal(Real *what)
    {
        key = (key << 3) ^ Hash(1, *((ulong *) &what->value));
        return what;
    }
    Tree *DoText(Text *what)
    {
        key = (key << 3) ^ Hash(2, what->value);
        return what;
    }
    Tree *DoName(Name *what)
    {
        key = (key << 3) ^ Hash(3, what->value);
        return what;
    }

    Tree *DoBlock(Block *what)
    {
        key = (key << 3) ^ Hash(4, what->opening + what->closing);
        return what;
    }
    Tree *DoInfix(Infix *what)
    {
        key = (key << 3) ^ Hash(5, what->name);
        return what;
    }
    Tree *DoPrefix(Prefix *what)
    {
        ulong old = key;
        key = 0; what->left->Do(this);
        key = (old << 3) ^ Hash(6, key);
        return what;
    }
    Tree *DoPostfix(Postfix *what)
    {
        ulong old = key;
        key = 0; what->right->Do(this);
        key = (old << 3) ^ Hash(7, key);
        return what;
    }
    Tree *Do(Tree *what)
    {
        key = (key << 3) ^ Hash(1, (ulong) what);
        return what;
    }

    ulong key;
};



// ============================================================================
//
//    Parameter match - Isolate parameters in an rewrite source
//
// ============================================================================

Tree *ParameterMatch::Do(Tree *what)
// ----------------------------------------------------------------------------
//   Nothing to do for leaves
// ----------------------------------------------------------------------------
{
    return what;
}


Tree *ParameterMatch::DoInteger(Integer *what)
// ----------------------------------------------------------------------------
//   Nothing to do for leaves
// ----------------------------------------------------------------------------
{
    return what;
}


Tree *ParameterMatch::DoReal(Real *what)
// ----------------------------------------------------------------------------
//   Nothing to do for leaves
// ----------------------------------------------------------------------------
{
    return what;
}


Tree *ParameterMatch::DoText(Text *what)
// ----------------------------------------------------------------------------
//   Nothing to do for leaves
// ----------------------------------------------------------------------------
{
    return what;
}


Tree *ParameterMatch::DoName(Name *what)
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
        if (Tree *existing = symbols->Named(what->value))
            return existing;

        // If first occurence of the name, enter it in symbol table
        Tree *result = symbols->Allocate(what);
        order.push_back(result);
        return result;
    }
}


Tree *ParameterMatch::DoBlock(Block *what)
// ----------------------------------------------------------------------------
//   Parameters in a block belong to the child
// ----------------------------------------------------------------------------
{
    return what->child->Do(this);
}


Tree *ParameterMatch::DoInfix(Infix *what)
// ----------------------------------------------------------------------------
//   Check if we match an infix operator
// ----------------------------------------------------------------------------
{
    // Check if we match a type, e.g. 2 vs. 'K : integer'
    if (what->name == ":")
    {
        // Check the variable name, e.g. K in example above
        Name *varName = what->left->AsName();
        if (!varName)
            return Error("Expected a name, got '$1' ", what->left);

        // Check if the name already exists
        if (Tree *existing = symbols->Named(varName->value))
            return Error("Typed name '$1' already exists as '$2'",
                         what->left, existing);

        // Enter the name in symbol table
        Tree *result = symbols->Allocate(varName);
        order.push_back(result);
        return result;
    }

    // If this is the first one, this is what we define
    if (!defined)
        defined = what;

    // Otherwise, test left and right
    Tree *lr = what->left->Do(this);
    if (!lr)
        return NULL;
    Tree *rr = what->right->Do(this);
    if (!rr)
        return NULL;
    return what;
}


Tree *ParameterMatch::DoPrefix(Prefix *what)
// ----------------------------------------------------------------------------
//   For prefix expressions, simply test left then right
// ----------------------------------------------------------------------------
{
    Infix *defined_infix = defined->AsInfix();
    if (defined_infix)
        defined = NULL;

    Tree *lr = what->left->Do(this);
    if (!lr)
        return NULL;
    Tree *rr = what->right->Do(this);
    if (!rr)
        return NULL;

    if (!defined && defined_infix)
        defined = defined_infix;

    return what;
}


Tree *ParameterMatch::DoPostfix(Postfix *what)
// ----------------------------------------------------------------------------
//    For postfix expressions, simply test right, then left
// ----------------------------------------------------------------------------
{
    // Note that ordering is reverse compared to prefix, so that
    // the 'defined' names is set correctly
    Tree *rr = what->right->Do(this);
    if (!rr)
        return NULL;
    Tree *lr = what->left->Do(this);
    if (!lr)
        return NULL;
    return what;
}



// ============================================================================
//
//    Argument matching - Test input arguments against parameters
//
// ============================================================================

Tree *ArgumentMatch::Compile(Tree *source)
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


Tree *ArgumentMatch::CompileValue(Tree *source)
// ----------------------------------------------------------------------------
//   Compile the source and make sure we evaluate it
// ----------------------------------------------------------------------------
{
    Tree *result = Compile(source);

    if (result)
    {
        if (Name *name = result->AsName())
        {
            llvm::BasicBlock *bb = unit.BeginLazy(name);
            unit.NeedStorage(name);
            if (!name->symbols)
                name->SetSymbols(symbols);
            unit.CallEvaluate(name);
            unit.EndLazy(name, bb);
        }
    }
    return result;
}


Tree *ArgumentMatch::CompileClosure(Tree *source)
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
    Tree *envOK = source->Do(env);
    if (!envOK)
        return Error("Internal: what environment in '$1'?", source);

    // Create the parameter list with all imported locals
    tree_list parms, args;
    capture_table::iterator c;
    for (c = env.captured.begin(); c != env.captured.end(); c++)
    {
        Tree *name = (*c).first;
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
        Tree *result = symbols->Compile(source, subUnit, true);
        if (!result)
            unit.ConstantTree(source);

        eval_fn fn = subUnit.Finalize();
        source->code = fn;
    }

    // Create a call to xl_new_closure to save the required trees
    unit.CreateClosure(source, args);

    return source;
}


Tree *ArgumentMatch::Do(Tree *what)
// ----------------------------------------------------------------------------
//   Default is to return failure
// ----------------------------------------------------------------------------
{
    return NULL;
}


Tree *ArgumentMatch::DoInteger(Integer *what)
// ----------------------------------------------------------------------------
//   An integer argument matches the exact value
// ----------------------------------------------------------------------------
{
    // If the tested tree is a constant, it must be an integer with same value
    if (test->IsConstant())
    {
        Integer *it = test->AsInteger();
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
    Tree *compiled = CompileValue(test);
    if (!compiled)
        return NULL;

    // Compare at run-time the actual tree value with the test value
    unit.IntegerTest(compiled, what->value);
    return compiled;
}


Tree *ArgumentMatch::DoReal(Real *what)
// ----------------------------------------------------------------------------
//   A real matches the exact value
// ----------------------------------------------------------------------------
{
    // If the tested tree is a constant, it must be an integer with same value
    if (test->IsConstant())
    {
        Real *rt = test->AsReal();
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
    Tree *compiled = CompileValue(test);
    if (!compiled)
        return NULL;

    // Compare at run-time the actual tree value with the test value
    unit.RealTest(compiled, what->value);
    return compiled;
}


Tree *ArgumentMatch::DoText(Text *what)
// ----------------------------------------------------------------------------
//   A text matches the exact value
// ----------------------------------------------------------------------------
{
    // If the tested tree is a constant, it must be an integer with same value
    if (test->IsConstant())
    {
        Text *tt = test->AsText();
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
    Tree *compiled = CompileValue(test);
    if (!compiled)
        return NULL;

    // Compare at run-time the actual tree value with the test value
    unit.TextTest(compiled, what->value);
    return compiled;
}


Tree *ArgumentMatch::DoName(Name *what)
// ----------------------------------------------------------------------------
//    Bind arguments to parameters being defined in the shape
// ----------------------------------------------------------------------------
{
    if (!defined)
    {
        // The first name we see must match exactly, e.g. 'sin' in 'sin X'
        defined = what;
        if (Name *nt = test->AsName())
            if (nt->value == what->value)
                return what;
        return NULL;
    }
    else
    {
        // Check if the name already exists, e.g. 'false' or 'A+A'
        // If it does, we generate a run-time check to verify equality
        if (Tree *existing = rewrite->Named(what->value))
        {
            // Insert a dynamic tree comparison test
            Tree *testCode = CompileValue(test);
            if (!testCode)
                return NULL;
            Tree *thisCode = Compile(existing);
            if (!thisCode)
                return NULL;
            unit.ShapeTest(testCode, thisCode);

            // Return compilation success
            return what;
        }

        // If first occurence of the name, enter it in symbol table
        Tree *compiled = CompileClosure(test);
        if (!compiled)
            return NULL;

        locals->EnterName(what->value, compiled);
        return what;
    }
}


Tree *ArgumentMatch::DoBlock(Block *what)
// ----------------------------------------------------------------------------
//   Check if we match a block
// ----------------------------------------------------------------------------
{
    // Test if we exactly match the block, i.e. the reference is a block
    if (Block *bt = test->AsBlock())
    {
        if (bt->opening == what->opening &&
            bt->closing == what->closing)
        {
            test = bt->child;
            Tree *br = what->child->Do(this);
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


Tree *ArgumentMatch::DoInfix(Infix *what)
// ----------------------------------------------------------------------------
//   Check if we match an infix operator
// ----------------------------------------------------------------------------
{
    if (Infix *it = test->AsInfix())
    {
        // Check if we match the tree, e.g. A+B vs 2+3
        if (it->name == what->name)
        {
            if (!defined)
                defined = what;
            test = it->left;
            Tree *lr = what->left->Do(this);
            test = it;
            if (!lr)
                return NULL;
            test = it->right;
            Tree *rr = what->right->Do(this);
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
        Name *varName = what->left->AsName();
        if (!varName)
            return Error("Expected a name, got '$1' ", what->left);

        // Check if the name already exists
        if (Tree *existing = rewrite->Named(varName->value))
            return Error("Name '$1' already exists as '$2'",
                         what->left, existing);

        // Evaluate type expression, e.g. 'integer' in example above
        Tree *typeExpr = Compile(what->right);
        if (!typeExpr)
            return NULL;

        // Compile what we are testing against
        Tree *compiled = CompileValue(test);
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


Tree *ArgumentMatch::DoPrefix(Prefix *what)
// ----------------------------------------------------------------------------
//   For prefix expressions, simply test left then right
// ----------------------------------------------------------------------------
{
    if (Prefix *pt = test->AsPrefix())
    {
        // Check if we match the tree, e.g. f(A) vs. f(2)
        // Note that we must test left first to define 'f' in above case
        Infix *defined_infix = defined->AsInfix();
        if (defined_infix)
            defined = NULL;

        test = pt->left;
        Tree *lr = what->left->Do(this);
        test = pt;
        if (!lr)
            return NULL;
        test = pt->right;
        Tree *rr = what->right->Do(this);
        test = pt;
        if (!rr)
            return NULL;
        if (!defined && defined_infix)
            defined = defined_infix;
        return what;
    }
    return NULL;
}


Tree *ArgumentMatch::DoPostfix(Postfix *what)
// ----------------------------------------------------------------------------
//    For postfix expressions, simply test right, then left
// ----------------------------------------------------------------------------
{
    if (Postfix *pt = test->AsPostfix())
    {
        // Check if we match the tree, e.g. A! vs 2!
        // Note that ordering is reverse compared to prefix, so that
        // the 'defined' names is set correctly
        test = pt->right;
        Tree *rr = what->right->Do(this);
        test = pt;
        if (!rr)
            return NULL;
        test = pt->left;
        Tree *lr = what->left->Do(this);
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

Tree *EnvironmentScan::Do(Tree *what)
// ----------------------------------------------------------------------------
//   Nothing to do for leaves
// ----------------------------------------------------------------------------
{
    return what;
}


Tree *EnvironmentScan::DoInteger(Integer *what)
// ----------------------------------------------------------------------------
//   Nothing to do for leaves
// ----------------------------------------------------------------------------
{
    return what;
}


Tree *EnvironmentScan::DoReal(Real *what)
// ----------------------------------------------------------------------------
//   Nothing to do for leaves
// ----------------------------------------------------------------------------
{
    return what;
}


Tree *EnvironmentScan::DoText(Text *what)
// ----------------------------------------------------------------------------
//   Nothing to do for leaves
// ----------------------------------------------------------------------------
{
    return what;
}


Tree *EnvironmentScan::DoName(Name *what)
// ----------------------------------------------------------------------------
//    Check if name is found in context, if so record where we took it from
// ----------------------------------------------------------------------------
{
    for (Symbols *s = symbols; s; s = s->Parent())
    {
        if (Tree *existing = s->Named(what->value, false))
        {
            // Found the symbol in the given symbol table
            if (!captured.count(existing))
                captured[existing] = s;
            break;
        }
    }
    return what;
}


Tree *EnvironmentScan::DoBlock(Block *what)
// ----------------------------------------------------------------------------
//   Parameters in a block are in its child
// ----------------------------------------------------------------------------
{
    return what->child->Do(this);
}


Tree *EnvironmentScan::DoInfix(Infix *what)
// ----------------------------------------------------------------------------
//   Check if we match an infix operator
// ----------------------------------------------------------------------------
{
    // Test left and right
    what->left->Do(this);
    what->right->Do(this);
    return what;
}


Tree *EnvironmentScan::DoPrefix(Prefix *what)
// ----------------------------------------------------------------------------
//   For prefix expressions, simply test left then right
// ----------------------------------------------------------------------------
{
    what->left->Do(this);
    what->right->Do(this);
    return what;
}


Tree *EnvironmentScan::DoPostfix(Postfix *what)
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


Tree *BuildChildren::DoPrefix(Prefix *what)
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


Tree *BuildChildren::DoPostfix(Postfix *what)
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


Tree *BuildChildren::DoInfix(Infix *what)
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


Tree *BuildChildren::DoBlock(Block *what)
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

Tree *DeclarationAction::Do(Tree *what)
// ----------------------------------------------------------------------------
//   Default is to leave trees alone (for native trees)
// ----------------------------------------------------------------------------
{
    return what;
}


Tree *DeclarationAction::DoInteger(Integer *what)
// ----------------------------------------------------------------------------
//   Integers evaluate directly
// ----------------------------------------------------------------------------
{
    return what;
}


Tree *DeclarationAction::DoReal(Real *what)
// ----------------------------------------------------------------------------
//   Reals evaluate directly
// ----------------------------------------------------------------------------
{
    return what;
}


Tree *DeclarationAction::DoText(Text *what)
// ----------------------------------------------------------------------------
//   Text evaluates directly
// ----------------------------------------------------------------------------
{
    return what;
}


Tree *DeclarationAction::DoName(Name *what)
// ----------------------------------------------------------------------------
//   Build a unique reference in the context for the entity
// ----------------------------------------------------------------------------
{
    return what;
}


Tree *DeclarationAction::DoBlock(Block *what)
// ----------------------------------------------------------------------------
//   Declarations in a block belong to the child, not to us
// ----------------------------------------------------------------------------
{
    return what->child->Do(this);
}


Tree *DeclarationAction::DoInfix(Infix *what)
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


Tree *DeclarationAction::DoPrefix(Prefix *what)
// ----------------------------------------------------------------------------
//    All prefix operations translate into a rewrite
// ----------------------------------------------------------------------------
{
    // Deal with 'data' declarations and 'load' statements
    if (Name *name = what->left->AsName())
    {
        if (name->value == "data")
        {
            EnterRewrite(what->right, NULL);
            return what;
        }
        if (name->value == "load")
        {
            Text *file = what->right->AsText();
            if (!file)
                return Error("Argument '$1' to 'load' is not a text",
                             what->right);
            return xl_load(file->value);
        }
    }

    return what;
}


Tree *DeclarationAction::DoPostfix(Postfix *what)
// ----------------------------------------------------------------------------
//    All postfix operations translate into a rewrite
// ----------------------------------------------------------------------------
{
    return what;
}


void DeclarationAction::EnterRewrite(Tree *defined, Tree *definition)
// ----------------------------------------------------------------------------
//   Add a definition in the current context
// ----------------------------------------------------------------------------
{
    if (Name *name = defined->AsName())
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
    : symbols(s), unit(u), needed(), nullIfBad(nib), keepAlternatives(ka)
{}


Tree *CompileAction::Do(Tree *what)
// ----------------------------------------------------------------------------
//   Default is to leave trees alone (for native trees)
// ----------------------------------------------------------------------------
{
    return what;
}


Tree *CompileAction::DoInteger(Integer *what)
// ----------------------------------------------------------------------------
//   Integers evaluate directly
// ----------------------------------------------------------------------------
{
    unit.ConstantInteger(what);
    return what;
}


Tree *CompileAction::DoReal(Real *what)
// ----------------------------------------------------------------------------
//   Reals evaluate directly
// ----------------------------------------------------------------------------
{
    unit.ConstantReal(what);
    return what;
}


Tree *CompileAction::DoText(Text *what)
// ----------------------------------------------------------------------------
//   Text evaluates directly
// ----------------------------------------------------------------------------
{
    unit.ConstantText(what);
    return what;
}


Tree *CompileAction::DoName(Name *what)
// ----------------------------------------------------------------------------
//   Build a unique reference in the context for the entity
// ----------------------------------------------------------------------------
{
    // Normally, the name should have been declared in ParameterMatch
    if (Tree *result = symbols->Named(what->value))
    {
        // Try to compile the definition of the name
        if (!result->AsName())
        {
            Rewrite rw(symbols, what, result);
            if (!what->symbols)
                what->SetSymbols(symbols);
            result = rw.Compile();
        }

        // Check if there is code we need to call
        Compiler *compiler = Context::context->compiler;
        if (compiler->functions.count(result) &&
            compiler->functions[result] != unit.function)
        {
            // Case of "Name -> Foo": Invoke Name
            tree_list noArgs;
            unit.NeedStorage(what);
            unit.Invoke(what, result, noArgs);
            return what;
        }
        else if (unit.value.count(result))
        {
            // Case of "Foo(A,B) -> B" with B: evaluate B lazily
            unit.Copy(result, what, false);
            return result;
        }
        else
        {
            // Return the name itself by default
            unit.ConstantTree(result);
            unit.Copy(result, what);
            if (!result->symbols)
                result->SetSymbols(symbols);
        }

        return result;
    }
    if (nullIfBad)
    {
        unit.ConstantTree(what);
        return what;
    }
    return Error("Name '$1' does not exist", what);
}


Tree *CompileAction::DoBlock(Block *what)
// ----------------------------------------------------------------------------
//   Optimize away indent or parenthese blocks, evaluate others
// ----------------------------------------------------------------------------
{
    if ((what->opening == Block::indent && what->closing == Block::unindent) ||
        (what->opening == "(" && what->closing == ")"))
    {
        if (unit.IsKnown(what))
            unit.Copy(what, what->child, false);
        Tree *result = what->child->Do(this);
        if (!result)
            return NULL;
        if (unit.IsKnown(what->child))
        {
            if (!what->child->symbols)
                what->child->SetSymbols(symbols);
            unit.CallEvaluate(what->child);
        }
        unit.Copy(result, what);
        return what;
    }

    // In other cases, we need to evaluate rewrites
    return Rewrites(what);
}


Tree *CompileAction::DoInfix(Infix *what)
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
            if (!what->left->symbols)
                what->left->SetSymbols(symbols);
            unit.CallEvaluate(what->left);
        }
        if (!what->right->Do(this))
            return NULL;
        if (unit.IsKnown(what->right))
        {
            if (!what->right->symbols)
                what->right->SetSymbols(symbols);
            unit.CallEvaluate(what->right);
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


Tree *CompileAction::DoPrefix(Prefix *what)
// ----------------------------------------------------------------------------
//    Deal with data declarations, otherwise translate as a rewrite
// ----------------------------------------------------------------------------
{
    if (Name *name = what->left->AsName())
    {
        if (name->value == "data")
            return what;
    }
    return Rewrites(what);
}


Tree *CompileAction::DoPostfix(Postfix *what)
// ----------------------------------------------------------------------------
//    All postfix operations translate into a rewrite
// ----------------------------------------------------------------------------
{
    return Rewrites(what);
}


Tree * CompileAction::Rewrites(Tree *what)
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

    for (Symbols *s = symbols; s && !foundUnconditional; s = s->Parent())
    {
        if (!visited.count(s))
        {
            lookups.push_back(s);
            visited.insert(s);
            symbols_set::iterator si;
            for (si = s->imported.begin(); si != s->imported.end(); si++)
            {
                visited.insert(*si);
                lookups.push_back(*si);
            }
        }
    }

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
                                        this, needed);
                Tree *argsTest = candidate->from->Do(matchArgs);
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
                        Symbols &parms = *candidate->from->symbols;
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
                                Tree *parm = parms.Named(name);
                                std::cerr << "   " << name
                                          << " = " << parm << "\n";
                            }
                            std::cerr << "Args:\n";
                            for (a = args.names.begin();
                                 a != args.names.end();
                                 a++)
                            {
                                text name = (*a).first;
                                Tree *arg = args.Named(name);
                                std::cerr << "   " << name
                                          << " = " << arg << "\n";
                            }
                        }

                        // Map the arguments we found in parameter order
                        // (actually, in reverse order, which is what we want)
                        tree_list argsList;
                        tree_list::iterator p;
                        tree_list &order = candidate->parameters;
                        for (p = order.begin(); p != order.end(); p++)
                        {
                            Name *name = (*p)->AsName();
                            Tree *argValue = args.Named(name->value);
                            argsList.push_back(argValue);
                        }

                        // Compile the candidate
                        Tree *code = candidate->Compile();

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
        return Error("No rewrite candidate for '$1'", what);
    }
    return what;
}



// ============================================================================
//
//    Error handling
//
// ============================================================================

Tree * Context::Error(text message, Tree *arg1, Tree *arg2, Tree *arg3)
// ----------------------------------------------------------------------------
//   Execute the innermost error handler
// ----------------------------------------------------------------------------
{
    Tree *handler = ErrorHandler();
    if (handler)
    {
        Tree *arg0 = new Text (message);
        Tree *info = arg3;
        if (arg2)
            info = info ? new Infix(",", arg2, info) : arg2;
        if (arg1)
            info = info ? new Infix(",", arg1, info) : arg1;
        info = info ? new Infix(",", arg0, info) : arg0;

        Prefix *errorCall = new Prefix(handler, info);
        return Run(errorCall);
    }

    // No handler: terminate
    errors.Error(message, arg1, arg2, arg3);
    std::exit(1);
}


Tree *Context::ErrorHandler()
// ----------------------------------------------------------------------------
//    Return the innermost error handler
// ----------------------------------------------------------------------------
{
    return error_handler;
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


Tree *Rewrite::Do(Action &a)
// ----------------------------------------------------------------------------
//   Apply an action to the 'from' and 'to' fields and all referenced trees
// ----------------------------------------------------------------------------
{
    Tree *result = from->Do(a);
    if (to)
        result = to->Do(a);
    for (rewrite_table::iterator i = hash.begin(); i != hash.end(); i++)
        result = (*i).second->Do(a);
    for (tree_list::iterator p=parameters.begin(); p!=parameters.end(); p++)
        result = (*p)->Do(a);
    return result;
}


Tree *Rewrite::Compile(void)
// ----------------------------------------------------------------------------
//   Compile code for the 'to' form
// ----------------------------------------------------------------------------
//   This is similar to Context::Compile, except that it may generate a
//   function with more parameters, i.e. Tree *f(Tree *, Tree *, ...),
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
    if (!from->symbols)
        return Error("Internal: No symbols for '$1'", from);

    // Create local symbols
    Symbols *locals = new Symbols (from->symbols);

    // Record rewrites and data declarations in the current context
    DeclarationAction declare(locals);
    Tree *toDecl = to->Do(declare);
    if (!toDecl)
        return Error("Internal: Declaration error for '$1'", to);

    // Compile the body of the rewrite
    CompileAction compile(locals, unit, false, false);
    Tree *result = to->Do(compile);
    if (!result)
        return Error("Unable to compile '$1'", to);

    // Even if technically, this is not an 'eval_fn' (it has more args),
    // we still record it to avoid recompiling multiple times
    eval_fn fn = compile.unit.Finalize();
    to->code = fn;

    return to;
}



// ============================================================================
//
//   Global error handler
//
// ============================================================================

Tree *Error (text message, Tree *a1, Tree *a2, Tree *a3)
// ----------------------------------------------------------------------------
//    Use the error handler in the global context
// ----------------------------------------------------------------------------
{
    return Context::context->Error(message, a1, a2, a3);
}

XL_END


extern "C" void debugs(XL::Symbols *s)
// ----------------------------------------------------------------------------
//   For the debugger, dump a symbol table
// ----------------------------------------------------------------------------
{
    using namespace XL;
    std::cerr << "SYMBOLS AT " << s << "\n";
    symbol_table::iterator i;
    for (i = s->names.begin(); i != s->names.end(); i++)
        std::cerr << (*i).first << ": " << (*i).second << "\n";
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
