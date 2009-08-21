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
// 
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

XL_BEGIN

// ============================================================================
// 
//   Symbols: symbols management
// 
// ============================================================================

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
    names[n->value] = n;
    return n;
}


Rewrite *Symbols::EnterRewrite(Rewrite *rw)
// ----------------------------------------------------------------------------
//   Enter the given rewrite in the rewrites table
// ----------------------------------------------------------------------------
{
    if (rewrites)
        return rewrites->Add(rw);
    rewrites = rw;
    return rw;
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
//   Garbage collection
// 
// ============================================================================
//   This is just a rather simple mark and sweep garbage collector.

ulong Context::gc_increment = 200;
ulong Context::gc_growth_percent = 200;
Context *Context::context = NULL;

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
        for (active_set::iterator a = roots.begin(); a != roots.end(); a++)
            (*a)->Do(gc);
        for (Symbols *s = symbols; s; s = s->parent)
        {
            for (symbol_iter y = s->names.begin(); y != s->names.end(); y++)
                (*y).second->Do(gc);
            if (s->rewrites)
                s->rewrites->Do(gc);
        }
        formats_table::iterator f;
        formats_table &formats = Renderer::renderer->formats;
        for (f = formats.begin(); f != formats.end(); f++)
            (*f).second->Do(gc);

        // Then delete all trees in active set that are no longer referenced
        for (active_set::iterator a = active.begin(); a != active.end(); a++)
        {
            activeCount++;
            if (!gc.alive.count(*a))
            {
                deletedCount++;
                delete *a;
            }
        }
        active = gc.alive;
        gc_threshold = active.size() * gc_growth_percent / 100 + gc_increment;
        IFTRACE(memory)
            std::cerr << "done: Purged " << deletedCount
                      << " out of " << activeCount
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
        return result;
    }
}


Tree *ParameterMatch::DoBlock(Block *what)
// ----------------------------------------------------------------------------
//   Parameters in a block are in its child
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
            return context->Error("Expected a name, got '$1' ", what->left);

        // Check if the name already exists
        if (Tree *existing = symbols->Named(varName->value))
            return context->Error("Typed name '$1' already exists as '$2'",
                                  what->left, existing);

        // Enter the name in symbol table
        Tree *result = symbols->Allocate(varName);
        return result;
    }

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
    Tree *lr = what->left->Do(this);
    if (!lr)
        return NULL;
    Tree *rr = what->right->Do(this);
    if (!rr)
        return NULL;
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
//    Argument matching - Test input parameters
// 
// ============================================================================

Tree *ArgumentMatch::Compile(Tree *source)
// ----------------------------------------------------------------------------
//    Compile the source tree, and record we use the value in expr cache
// ----------------------------------------------------------------------------
{
    // Compile the code
    if (!source->code)
        source = symbols->context->Compile(source, true);
    if (!source)
        return NULL; // No match
 
    // Generate code to only evaluate the result once
    source->Do(compile);
    unit->LazyEvaluation(source);
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
        if (Integer *it = test->AsInteger())
            if (it->value == what->value)
                return what;
        return NULL;
    }

    // Compile the test tree
    Tree *compiled = Compile(test);
    if (!compiled)
        return NULL;

    // Compare at run-time the actual tree value with the test value
    unit->IntegerTest(compiled, what->value);
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
        if (Real *rt = test->AsReal())
            if (rt->value == what->value)
                return what;
        return NULL;
    }

    // Compile the test tree
    Tree *compiled = Compile(test);
    if (!compiled)
        return NULL;

    // Compare at run-time the actual tree value with the test value
    unit->RealTest(compiled, what->value);
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
        if (Text *tt = test->AsText())
            if (tt->value == what->value)
                return what;
        return NULL;
    }

    // Compile the test tree
    Tree *compiled = Compile(test);
    if (!compiled)
        return NULL;

    // Compare at run-time the actual tree value with the test value
    unit->TextTest(compiled, what->value);
    return compiled;
}


Tree *ArgumentMatch::DoName(Name *what)
// ----------------------------------------------------------------------------
//    Identify the parameters being defined in the shape
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
            Tree *testCode = Compile(test);
            if (!testCode)
                return NULL;
            Tree *thisCode = Compile(existing);
            if (!thisCode)
                return NULL;
            unit->ShapeTest(testCode, thisCode);

            // Return compilation success
            return what;
        }

        // Compile the evaluation of the tested value
        Tree *compiled = Compile(test);
        if (!compiled)
            return NULL;

        // If first occurence of the name, enter it in symbol table
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
        Context *context = symbols->context;
        Name *varName = what->left->AsName();
        if (!varName)
            return context->Error("Expected a name, got '$1' ", what->left);

        // Check if the name already exists
        if (Tree *existing = symbols->Named(varName->value))
            return context->Error("Name '$1' already exists as '$2'",
                                  what->left, existing);

        // Evaluate type expression, e.g. 'integer' in example above
        Tree *typeExpr = Compile(what->right);

        // Compile what we are testing against
        Tree *compiled = Compile(test);

        // Insert a run-time type test
        unit->TypeTest(compiled, typeExpr);

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
//   Optimize away indent or parenthese blocks, evaluate others
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
    // Deal with 'data' declarations
    if (Name *name = what->left->AsName())
    {
        if (name->value == "data")
        {
            EnterRewrite(what->right, NULL);
            return what;
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
    Rewrite *rewrite = new Rewrite(symbols, defined, definition);
    symbols->EnterRewrite(rewrite);
}



// ============================================================================
// 
//   Compilation action - Generation of "optimized" native trees
// 
// ============================================================================

CompileAction::CompileAction(Compiler *c, Symbols *s,
                             Tree *source, tree_list parms, bool nib)
// ----------------------------------------------------------------------------
//   Constructor
// ----------------------------------------------------------------------------
    : symbols(s), unit(NULL), needed(), nullIfBad(nib)
{
    unit = new CompiledUnit(c, source, parms);
}


CompileAction::~CompileAction()
// ----------------------------------------------------------------------------
//   Destructor deletes the CompiledUnit
// ----------------------------------------------------------------------------
{
    delete unit;
}


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
    unit->ConstantInteger(what);
    return what;
}


Tree *CompileAction::DoReal(Real *what)
// ----------------------------------------------------------------------------
//   Reals evaluate directly
// ----------------------------------------------------------------------------
{
    unit->ConstantReal(what);
    return what;
}


Tree *CompileAction::DoText(Text *what)
// ----------------------------------------------------------------------------
//   Text evaluates directly
// ----------------------------------------------------------------------------
{
    unit->ConstantText(what);
    return what;
}


Tree *CompileAction::DoName(Name *what)
// ----------------------------------------------------------------------------
//   Build a unique reference in the context for the entity
// ----------------------------------------------------------------------------
{
    // Normally, the name should have been declared in ParameterMatch
    if (Tree *result = symbols->Named(what->value))
        return result;
    return symbols->context->Error("Name '$1' does not exist", what);
}


Tree *CompileAction::DoBlock(Block *what)
// ----------------------------------------------------------------------------
//   Optimize away indent or parenthese blocks, evaluate others
// ----------------------------------------------------------------------------
{
    if ((what->opening == Block::indent && what->closing == Block::unindent) ||
        (what->opening == "(" && what->closing == ")"))
        return what->child->Do(this);
    
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
        if (Name *n = what->left->AsName())
            unit->EagerEvaluation(n);
        if (!what->right->Do(this))
            return NULL;
        if (Name *m = what->right->AsName())
            unit->EagerEvaluation(m);
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
//    All prefix operations translate into a rewrite
// ----------------------------------------------------------------------------
{
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

    Context *context = symbols->context;
    for (Symbols *s = symbols; s && !foundUnconditional; s = s->Parent())
    {
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
                // If we find a real match, identify its parameters
                Symbols parms(candidate->symbols);
                ParameterMatch matchParms(&parms);
                Tree *parmsTest = candidate->from->Do(matchParms);
                if (!parmsTest)
                    return context->Error(
                        "Internal: Invokation parameters for '$1'?",
                        candidate->from);

                // Create the invokation point
                llvm::BasicBlock *bb = unit->BeginInvokation();
                Symbols args(context->symbols);
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
                        unit->Return (what);
                        foundUnconditional = !unit->failbb;
                    }
                    else
                    {
                        // We should have same number of args and parms
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

                        // Map the arguments we found in called stack order
                        // (i.e. we take the order from parms ids)
                        tree_list argsList;
                        symbol_iter i;
                        for (i=parms.names.begin(); i!=parms.names.end(); i++)
                        {
                            text name = (*i).first;
                            Tree *argValue = args.Named(name);
                            argsList.push_back(argValue);
                        }

                        // Compile the candidate
                        Tree *code = candidate->Compile();

                        // Invoke the candidate
                        unit->Invoke(code, argsList);

                        // If there was no test code, don't keep testing further
                        foundUnconditional = !unit->failbb;
                        
                        // This is the end of a successful invokation
                        unit->EndInvokation(bb, true);
                    } // if (data form)
                } // Match args
                else
                {
                    // Indicate unsuccessful invokation
                    unit->EndInvokation(bb, false);
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

    // If we didn't find anything, report it
    if (!foundSomething)
    {
        if (nullIfBad)
            return NULL;
        return symbols->context->Error("No rewrite candidate for '$1'", what);
    }
    return what;
}



// ============================================================================
// 
//    Evaluation of trees
// 
// ============================================================================

Tree *Context::Compile(Tree *source, bool nullIfBad)
// ----------------------------------------------------------------------------
//    Return an optimized version of the source tree, ready to run
// ----------------------------------------------------------------------------
//    This associates an eval_fn to the tree, i.e. code that takes a tree
//    as input and returns a tree as output.
{
    // If we already have compiled code, we are done
    if (source->code)
        return source;

    // Record rewrites in the current context
    Symbols parms(this);
    DeclarationAction declare(&parms);
    Tree *result = source->Do(declare);

    // Compile code for that tree
    tree_list parmsList;
    parmsList.push_back(source);
    CompileAction compile(compiler, &parms, source, parmsList, nullIfBad);
    if (compile.unit->IsForwardCall())
        return source;          // Nested compile

    // Generate the code
    result = source->Do(compile);

    // If we didn't compile successfully, report
    if (!result)
    {
        if (nullIfBad)
            return result;
        return Error("Couldn't compile '$1'", source);
    }

    // If we compiled successfully, get the code and store it
    eval_fn code = compile.unit->Finalize();
    source->code = code;

    return source;
}


Tree *Context::Run(Tree *code)
// ----------------------------------------------------------------------------
//   Execute a compiled code tree - Very similar to xl_evaluate
// ----------------------------------------------------------------------------
{
    Tree *result = code;
    if (!result)
        return result;

    if (!result->code)
        result = Compile(result);

    assert(result->code);
    result = result->code(code);
    return result;
}


Rewrite *Context::EnterRewrite(Tree *from, Tree *to)
// ----------------------------------------------------------------------------
//   Create a rewrite for the current context and enter it
// ----------------------------------------------------------------------------
{
    Rewrite *rewrite = new Rewrite(symbols, from, to);
    return symbols->EnterRewrite(rewrite);
}


void Context::ParameterList(Tree *form, tree_list &list)
// ----------------------------------------------------------------------------
//    List the parameters for the form and add them to the list
// ----------------------------------------------------------------------------
{
    // Identify all parameters in 'from'
    Symbols parms(symbols);
    ParameterMatch matchParms(&parms);
    Tree *parmsOK = form->Do(matchParms);
    if (!parmsOK)
        Error("Internal: what parameter list in '$1'?", form);

    // Build the parameter list for 'to'
    symbol_iter p;
    for (p = parms.names.begin(); p != parms.names.end(); p++)
        list.push_back((*p).second);
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
        return context->Run(errorCall);
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
//   Apply an action to the 'from' and 'to' fields and all hash entries
// ----------------------------------------------------------------------------
{
    rewrite_table::iterator i;
    Tree *result = from->Do(a);
    result = to->Do(a);
    for (i = hash.begin(); i != hash.end(); i++)
        result = (*i).second->Do(a);
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

    // Identify all parameters in 'from'
    Context *context = symbols->context;
    Symbols parms(symbols);
    ParameterMatch matchParms(&parms);
    Tree *parmsOK = from->Do(matchParms);
    if (!parmsOK)
        return context->Error("Internal: what parameters in '$1'?", from);

    // Build the parameter list for 'to'
    symbol_iter p;
    tree_list parmsList;
    for (p = parms.names.begin(); p != parms.names.end(); p++)
        parmsList.push_back((*p).second);
    CompileAction compile(context->compiler, &parms, to, parmsList, false);
    if (compile.unit->IsForwardCall())
    {
        // Recursive compilation of that form
        // REVISIT: Can this happen?
        return to;              // We know how to invoke it anyway
    }

    // Compile the body of the rewrite
    Tree *result = to->Do(compile);
    if (!result)
        return context->Error("Unable to compile '$1'", to);

    // Even if technically, this is not an 'eval_fn' (it has more args),
    // we still record it to avoid recompiling multiple times
    eval_fn code = compile.unit->Finalize();
    to->code = code;

    return result;
}

XL_END


extern "C" void debugs(XL::Symbols *symbols)
// ----------------------------------------------------------------------------
//   For the debugger, dump a symbol table
// ----------------------------------------------------------------------------
{
    using namespace XL;
    for (Symbols *s = symbols; s; s = s->Parent())
    {
        std::cerr << "SYMBOLS AT " << s << "\n";
        symbol_table::iterator i;
        for (i = s->names.begin(); i != s->names.end(); i++)
            std::cerr << (*i).first << ": " << (*i).second << "\n";
    }
}
