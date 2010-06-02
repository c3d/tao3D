// ****************************************************************************
//  runtime.cpp                     (C) 1992-2009 Christophe de Dinechin (ddd)
//                                                                 XL2 project
// ****************************************************************************
//
//   File Description:
//
//     Runtime functions necessary to execute XL programs
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

#include "runtime.h"
#include "tree.h"
#include "parser.h"
#include "scanner.h"
#include "renderer.h"
#include "context.h"
#include "options.h"
#include "opcodes.h"
#include "compiler.h"
#include "main.h"

#include <iostream>
#include <cstdarg>
#include <cstdio>
#include <sys/stat.h>


XL_BEGIN

Tree *xl_identity(Tree *what)
// ----------------------------------------------------------------------------
//   Return the input tree unchanged
// ----------------------------------------------------------------------------
{
    return what;
}


Tree *xl_evaluate(Tree *what)
// ----------------------------------------------------------------------------
//   Compile the tree if necessary, then evaluate it
// ----------------------------------------------------------------------------
// This is similar to Context::Run, but we save stack space for recursion
{
    if (!what)
        return what;
    Symbols *symbols = what->Symbols();
    if (!symbols)
        symbols = Symbols::symbols;
    
    StackDepthCheck depthCheck(what);
    if (depthCheck)
        return what;

    Tree *result = symbols->Run(what);
    if (result != what)
        result->source = xl_source(what);
    return result;
}


Tree *xl_repeat(Tree *self, Tree *what, longlong count)
// ----------------------------------------------------------------------------
//   Compile the tree if necessary, then evaluate it count times
// ----------------------------------------------------------------------------
{
    if (!what)
        return what;
    Symbols *symbols = self->Symbols();
    if (!symbols)
        symbols = Symbols::symbols;
    Tree *result = what;

    while (count-- > 0)
        result = symbols->Run(what);
    if (result != what)
        result->source = xl_source(what);
    return result;
}


Tree *xl_map(Tree *data, Tree *code, text row, text column)
// ----------------------------------------------------------------------------
//   Apply the code on each piece of data
// ----------------------------------------------------------------------------
{
    bool evaluate = true;

    if (Name *name = data->AsName())
        data = xl_evaluate(name);

    if (Block *block = data->AsBlock())
    {
        data = block->child;
        if (!data->Symbols())
            data->SetSymbols(block->Symbols());
        if (!data->code)
            data->code = xl_evaluate_children;
    }

    if (Block *codeBlock = code->AsBlock())
    {
        code = codeBlock->child;
        if (!code->Symbols())
            code->SetSymbols(codeBlock->Symbols());
        if (codeBlock->opening == "[" && codeBlock->closing == "]")
            evaluate = false;
    }

    MapAction map(code, row, column, evaluate);
    Tree *result = map.Map(data);
    return result;
}


bool xl_same_text(Tree *what, const char *ref)
// ----------------------------------------------------------------------------
//   Compile the tree if necessary, then evaluate it
// ----------------------------------------------------------------------------
{
    Text *tval = what->AsText(); assert(tval);
    return tval->value == text(ref);
}


bool xl_same_shape(Tree *left, Tree *right)
// ----------------------------------------------------------------------------
//   Check equality of two trees
// ----------------------------------------------------------------------------
{
    if (right)
    {
        XL::TreeMatch compareForEquality(right);
        if (left && left->Do(compareForEquality))
            return true;
    }
    return false;
}


Tree *xl_infix_match_check(Tree *value, kstring name)
// ----------------------------------------------------------------------------
//   Check if the value is matching the given infix
// ----------------------------------------------------------------------------
{
    while (Block *block = value->AsBlock())
        if (block->opening == "(" && block->closing == ")")
            value = block->child;
    if (Infix *infix = value->AsInfix())
        if (infix->name == name)
            return infix;
    return NULL;
}


Tree *xl_type_check(Tree *value, Tree *type)
// ----------------------------------------------------------------------------
//   Check if value has the type of 'type'
// ----------------------------------------------------------------------------
{
    IFTRACE(typecheck)
        std::cerr << "Type check " << value << " against " << type << ':';
    if (!value || !type->code)
    {
        IFTRACE(typecheck)
            std::cerr << "Failed (no value / no code)\n";
        return NULL;
    }

    // Check if this is a closure or something we want to evaluate
    Tree *original = value;
    StackDepthCheck typeDepthCheck(value);
    if (typeDepthCheck)
        return NULL;

    Infix *typeExpr = Symbols::symbols->CompileTypeTest(type);
    typecheck_fn typecheck = (typecheck_fn) typeExpr->code;
    Tree *afterTypeCast = typecheck(typeExpr, value);
    if (afterTypeCast && afterTypeCast != original)
        xl_set_source(afterTypeCast, value);
    IFTRACE(typecheck)
    {
        if (afterTypeCast)
            std::cerr << "Success\n";
        else
            std::cerr << "Failed (not same type)\n";
    }
    return afterTypeCast;
}



// ========================================================================
//
//    Creating entities (callbacks for compiled code)
//
// ========================================================================

Tree *xl_new_integer(longlong value)
// ----------------------------------------------------------------------------
//    Called by generated code to build a new Integer
// ----------------------------------------------------------------------------
{
    Tree *result = new Integer(value);
    result->code = xl_identity;
    return result;
}


Tree *xl_new_real(double value)
// ----------------------------------------------------------------------------
//    Called by generated code to build a new Real
// ----------------------------------------------------------------------------
{
    Tree *result = new Real (value);
    result->code = xl_identity;
    return result;
}


Tree *xl_new_character(kstring value)
// ----------------------------------------------------------------------------
//    Called by generated code to build a new single-quoted Text
// ----------------------------------------------------------------------------
{
    Tree *result = new Text(value, "'", "'");
    result->code = xl_identity;
    return result;
}


Tree *xl_new_text(kstring value)
// ----------------------------------------------------------------------------
//    Called by generated code to build a new double-quoted Text
// ----------------------------------------------------------------------------
{
    Tree *result = new Text(text(value));
    result->code = xl_identity;
    return result;
}


Tree *xl_new_xtext(kstring value, kstring open, kstring close)
// ----------------------------------------------------------------------------
//    Called by generated code to build a new arbitrarily-quoted Text
// ----------------------------------------------------------------------------
{
    Tree *result = new Text(value, open, close);
    result->code = xl_identity;
    return result;
}


Tree *xl_new_block(Block *source, Tree *child)
// ----------------------------------------------------------------------------
//    Called by generated code to build a new block
// ----------------------------------------------------------------------------
{
    Tree *result = new Block(source, child);
    result->code = xl_identity;
    return result;
}


Tree *xl_new_prefix(Prefix *source, Tree *left, Tree *right)
// ----------------------------------------------------------------------------
//    Called by generated code to build a new Prefix
// ----------------------------------------------------------------------------
{
    Tree *result = new Prefix(source, left, right);
    result->code = xl_identity;
    return result;
}


Tree *xl_new_postfix(Postfix *source, Tree *left, Tree *right)
// ----------------------------------------------------------------------------
//    Called by generated code to build a new Postfix
// ----------------------------------------------------------------------------
{
    Tree *result = new Postfix(source, left, right);
    result->code = xl_identity;
    return result;
}


Tree *xl_new_infix(Infix *source, Tree *left, Tree *right)
// ----------------------------------------------------------------------------
//    Called by generated code to build a new Infix
// ----------------------------------------------------------------------------
{
    Tree *result = new Infix(source, left, right);
    result->code = xl_identity;
    return result;
}



// ============================================================================
//
//    Closure management
//
// ============================================================================

Tree *xl_new_closure(Tree *expr, uint ntrees, ...)
// ----------------------------------------------------------------------------
//   Create a new closure at runtime, capturing the various trees
// ----------------------------------------------------------------------------
{
    // Immediately return anything we could evaluate at no cost
    if (!expr || expr->IsConstant() || !expr->code || !ntrees)
        return expr;

    IFTRACE(closure)
        std::cerr << "CLOSURE: Arity " << ntrees
                  << " code " << (void *) expr->code
                  << " [" << expr << "]\n";

    // Build the prefix with all the arguments
    Prefix *result = new Prefix(expr, NULL);
    Prefix *parent = result;
    va_list va;
    va_start(va, ntrees);
    for (uint i = 0; i < ntrees; i++)
    {
        Tree *arg = va_arg(va, Tree *);
        IFTRACE(closure)
            std::cerr << "  ARG: " << arg << '\n';
        Prefix *item = new Prefix(arg, NULL);
        parent->right = item;
        parent = item;
    }
    va_end(va);
    parent->right = xl_false;

    // Generate the code for the arguments
    Compiler *compiler = Context::context->compiler;
    eval_fn fn = compiler->closures[ntrees];
    if (!fn)
    {
        TreeList noParms;
        CompiledUnit unit(compiler, result, noParms);
        unit.CallClosure(result, ntrees);
        fn = unit.Finalize();
        compiler->closures[ntrees] = fn;
        compiler->SetTreeFunction(result, NULL); // Now owned by closures[n]
    }
    result->code = fn;
    xl_set_source(result, expr);

    return result;
}


Tree *xl_type_error(Tree *what)
// ----------------------------------------------------------------------------
//   Display message if we have a type error
// ----------------------------------------------------------------------------
{
    bool quickExit = false;
    if (quickExit)
        return what;
    Symbols *syms = what->Symbols();
    if (!syms)
        syms = Symbols::symbols;
    LocalSave<Symbols_p> saveSyms(Symbols::symbols, syms);
    return Ooops("No form matches '$1'", what);
}


Tree *xl_evaluate_children(Tree *what)
// ----------------------------------------------------------------------------
//   Reconstruct a similar tree evaluating children
// ----------------------------------------------------------------------------
{
    EvaluateChildren eval(what->Symbols());
    Tree *result = what->Do(eval);
    if (!result->Symbols())
        result->SetSymbols(what->Symbols());
    return result;
}



// ============================================================================
//
//    Type matching
//
// ============================================================================

#pragma GCC diagnostic ignored "-Wunused-parameter"

Tree *xl_boolean_cast(Tree *source, Tree *value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as a boolean value (true/false)
// ----------------------------------------------------------------------------
{
    value = xl_evaluate(value);
    if (value == xl_true || value == xl_false)
        return value;
    return NULL;
}


Tree *xl_integer_cast(Tree *source, Tree *value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as an integer
// ----------------------------------------------------------------------------
{
    value = xl_evaluate(value);
    if (Integer *it = value->AsInteger())
        return it;
    return NULL;
}


Tree *xl_real_cast(Tree *source, Tree *value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as a real
// ----------------------------------------------------------------------------
{
    value = xl_evaluate(value);
    if (Real *rt = value->AsReal())
        return rt;
    if (Integer *it = value->AsInteger())
        return new Real(it->value);
    return NULL;
}


Tree *xl_text_cast(Tree *source, Tree *value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as a text
// ----------------------------------------------------------------------------
{
    value = xl_evaluate(value);
    if (Text *tt = value->AsText())
        if (tt->opening != "'")
            return tt;
    return NULL;
}


Tree *xl_character_cast(Tree *source, Tree *value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as a character
// ----------------------------------------------------------------------------
{
    value = xl_evaluate(value);
    if (Text *tt = value->AsText())
        if (tt->opening == "'")
            return tt;
    return NULL;
}


Tree *xl_tree_cast(Tree *source, Tree *value)
// ----------------------------------------------------------------------------
//   Don't really check the argument
// ----------------------------------------------------------------------------
{
    return value;
}


Tree *xl_symbolicname_cast(Tree *source, Tree *value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as a name
// ----------------------------------------------------------------------------
{
    if (Name *nt = value->AsName())
        return nt;
    value = xl_evaluate(value);
    if (Name *afterEval = value->AsName())
        return afterEval;
    return NULL;
}


Tree *xl_infix_cast(Tree *source, Tree *value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as an infix
// ----------------------------------------------------------------------------
{
    if (Infix *it = value->AsInfix())
        return it;
    return NULL;
}


Tree *xl_prefix_cast(Tree *source, Tree *value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as a prefix
// ----------------------------------------------------------------------------
{
    if (Prefix *it = value->AsPrefix())
        return it;
    return NULL;
}


Tree *xl_postfix_cast(Tree *source, Tree *value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as a postfix
// ----------------------------------------------------------------------------
{
    if (Postfix *it = value->AsPostfix())
        return it;
    return NULL;
}


Tree *xl_block_cast(Tree *source, Tree *value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as a block
// ----------------------------------------------------------------------------
{
    if (Block *it = value->AsBlock())
        return it;
    return NULL;
}



// ============================================================================
//
//   Managing calls to/from XL
//
// ============================================================================

Tree *XLCall::operator() (Symbols *syms, bool nullIfBad, bool cached)
// ----------------------------------------------------------------------------
//    Perform the given call in the given context
// ----------------------------------------------------------------------------
{
    assert(syms);
    Tree *callee = syms->CompileCall(name, args, nullIfBad, cached);
    if (callee && callee->code)
        callee = callee->code(callee);
    return callee;
}


Tree *XLCall::build(Symbols *syms)
// ----------------------------------------------------------------------------
//    Perform the given call in the given context
// ----------------------------------------------------------------------------
{
    assert(syms);
    Tree *callee = syms->CompileCall(name, args);
    return callee;
}


Tree *xl_invoke(eval_fn toCall, Tree *src, TreeList &args)
// ----------------------------------------------------------------------------
//   Invoke a callback with the right number of arguments
// ----------------------------------------------------------------------------
//   We generate a function with the right signature using LLVM
{
    Compiler *compiler = Context::context->compiler;
    adapter_fn fn = compiler->EnterArrayToArgsAdapter(args.size());

    // REVISIT: The following assumes that Tree_p and Tree * have the
    // same memory representation in a std::vector<Tree_p>
    return fn (toCall, src, (Tree **) &args[0]);
}



// ============================================================================
//
//    Loading trees from external files
//
// ============================================================================

Tree *xl_load(text name)
// ----------------------------------------------------------------------------
//    Load a file from disk
// ----------------------------------------------------------------------------
{
    text path = MAIN->SearchFile(name);
    if (path == "")
        return Ooops("Source file '$1' not found", new Text(name));

    // Check if the file has already been loaded somehwere.
    // If so, return the loaded file
    if (MAIN->files.count(path) > 0)
    {
        SourceFile &sf = MAIN->files[path];
        Symbols::symbols->Import(sf.symbols);
        return sf.tree;
    }

    Parser parser(path.c_str(), MAIN->syntax, MAIN->positions, MAIN->errors);
    Tree *tree = parser.Parse();
    if (!tree)
        return Ooops("Unable to load file '$1'", new Text(path));

    Symbols_p old = Symbols::symbols;
    Symbols_p syms = new Symbols(old);
    MAIN->files[path] = SourceFile(path, tree, syms);
    Symbols::symbols = syms;
    tree->SetSymbols(syms);
    tree = syms->CompileAll(tree);
    Symbols::symbols = old;
    old->Import(syms);

    return tree;
}


Tree *xl_load_data(text name, text prefix, text fieldSeps, text recordSeps)
// ----------------------------------------------------------------------------
//    Load a comma-separated or tab-separated file from disk
// ----------------------------------------------------------------------------
{
    text path = MAIN->SearchFile(name);
    if (path == "")
        return Ooops("CSV file '$1' not found", new Text(name));

    // Check if the file has already been loaded somehwere.
    // If so, return the loaded file
    if (MAIN->files.count(path) > 0)
    {
        SourceFile &sf = MAIN->files[path];
        Symbols::symbols->Import(sf.symbols);
        return sf.tree;
    }

    Tree *tree = NULL;
    Tree *line = NULL;
    char buffer[256];
    char *ptr = buffer;
    char *end = buffer + sizeof(buffer) - 1;
    bool hasQuote = false;
    bool hasRecord = false;
    bool hasField = false;
    FILE *f = fopen(path.c_str(), "r");

    *end = 0;
    while (!feof(f))
    {
        int c = getc(f);

        if (c == 0xA0)          // Hard space generated by Numbers, skip
            continue;

        if (hasQuote)
        {
            hasRecord = false;
            hasField = false;
        }
        else
        {
            hasRecord = recordSeps.find(c) != recordSeps.npos || c == EOF;
            hasField = fieldSeps.find(c) != fieldSeps.npos;
        }
        if (hasRecord || hasField)
            c = 0;
        else if (c == '"')
            hasQuote = !hasQuote;

        if (ptr < end)
            *ptr++ = c;

        if (!c)
        {
            text token;
            Tree *child = NULL;

            if (isdigit(ptr[0]))
            {
                char *ptr2 = NULL;
                longlong l = strtoll(buffer, &ptr2, 10);
                if (ptr2 == ptr-1)
                {
                    child = new Integer(l);
                }
                else
                {
                    double d = strtod (buffer, &ptr2);
                    if (ptr2 == ptr-1)
                        child = new Real(d);
                }
            }
            if (child == NULL)
            {
                token = text(buffer, ptr - buffer - 1);
                child = new Text(buffer);
            }

            // Combine to line
            if (line)
                line = new Infix(",", line, child);
            else
                line = child;

            // Combine as line if necessary
            if (hasRecord)
            {
                if (prefix != "")
                    line = new Prefix(new Name(prefix), line);
                if (tree)
                    tree = new Infix("\n", tree, line);
                else
                    tree = line;
                line = NULL;
            }
            ptr = buffer;
        }
    }
    fclose(f);
    if (!tree)
        return Ooops("Unable to load data from '$1'", new Text(path));

    // Store that we use the file
    struct stat st;
    stat(path.c_str(), &st);
    Symbols_p old = Symbols::symbols;
    Symbols_p syms = new Symbols(old);
    MAIN->files[path] = SourceFile(path, tree, syms);
    Symbols::symbols = syms;
    tree->SetSymbols(syms);
    tree = syms->CompileAll(tree);
    Symbols::symbols = old;
    old->Import(syms);

    return tree;
}



// ============================================================================
// 
//   Map the given action on the code
// 
// ============================================================================

MapAction::MapAction(Tree *code, bool eval)
// ----------------------------------------------------------------------------
//   Compile the code for the action
// ----------------------------------------------------------------------------
    : code(code), function(NULL), evaluate(eval)
{}


MapAction::MapAction(Tree *code, text separator, bool eval)
// ----------------------------------------------------------------------------
//   Compile the code for the action
// ----------------------------------------------------------------------------
    : code(code), function(NULL), evaluate(eval)
{
    separators.insert(separator);
}


MapAction::MapAction(Tree *code, text row, text column, bool eval)
// ----------------------------------------------------------------------------
//   Compile the code for the action
// ----------------------------------------------------------------------------
    : code(code), function(NULL), evaluate(eval)
{
    separators.insert(row);
    separators.insert(column);
}


Tree *MapAction::Map(Tree *data)
// ----------------------------------------------------------------------------
//   Compile the code as a one-argument function
// ----------------------------------------------------------------------------
{
    CurryFunctionInfo *curry = code->GetInfo<CurryFunctionInfo>();
    if (!curry)
    {
        // Symbols and function we are going to create
        Symbols *symbols = new Symbols(code->Symbols());
        eval_fn fn = NULL;

        // Stuff to compile
        Tree *toCompile = code;
        Name *parameter = new Name("_");
        TreeList parameters;
        parameters.push_back(parameter);

        // Check the case where we get x->sin x as input
        if (Infix *infix = code->AsInfix())
        {
            if (infix->name == "->")
            {
                if (Name *name = infix->left->AsName())
                {
                    parameter->value = name->value;
                    toCompile = infix->right;
                }
            }
        }


        // Need to compile the prefix according to its type
        if (Name *name = code->AsName())
            // For a name Foo, we compile Foo _
            toCompile = new Prefix(name, parameter);
        else if (code->IsConstant())
            // For a constant like 17, we compile 17
            toCompile = code;
        else if (toCompile == code)
            // For something like X -> 2+X, we finish with the parameter
            toCompile = new Infix("\n", toCompile, parameter);

        // Create a compile unit;
        Compiler *compiler = Context::context->compiler;
        CompiledUnit unit(compiler, toCompile, parameters);
        assert (!unit.IsForwardCall() || !"Forward call in map function");

        // Record internal declarations if any
        symbols->Allocate(parameter);
        DeclarationAction declare(symbols);
        Tree *toDecl = toCompile->Do(declare);
        assert(toDecl);

        // Compile the body we generated
        CompileAction compile(symbols, unit, true, true);
        Tree *compiled = toCompile->Do(compile);

        // Record success or failure
        if (compiled)
            fn = unit.Finalize();

        // Record the generated function
        curry = new CurryFunctionInfo;
        code->SetInfo<CurryFunctionInfo>(curry);
        curry->code = compiled;
        curry->symbols = symbols;
        curry->function = fn;

        // Check for errors (emit message only once)
        if (!compiled)
            return Ooops("Unable to compile '$1' for map function", code);
    }

    // The function we are going to use is the one we found
    function = (map_fn) curry->function;
    if (!function)
        return Ooops("Unable to map with '$1'", code);

    // Now that we have a compiled function, we can map our data
    Tree *result = data->Do(this);
    return result;
}


Tree *MapAction::Do(Tree *what)
// ----------------------------------------------------------------------------
//   Apply the code to the given tree
// ----------------------------------------------------------------------------
{
    if (evaluate)
    {
        if (!what->Symbols())
            what->SetSymbols(code->Symbols());
        what = xl_evaluate(what);
    }
    return function(code, what);
}


Tree *MapAction::DoInfix(Infix *infix)
// ----------------------------------------------------------------------------
//   Check if this is a separator, if so evaluate left and right
// ----------------------------------------------------------------------------
{
    if (separators.count(infix->name))
    {
        Tree *left = infix->left->Do(this);
        Tree *right = infix->right->Do(this);
        if (left != infix->left || right != infix->right)
        {
            infix = new Infix(infix->name, left, right, infix->Position());
            infix->code = xl_identity;
        }
        return infix;
    }

    // Otherwise simply apply the function to the infix
    return Do(infix);
}


Tree *MapAction::DoPrefix(Prefix *prefix)
// ----------------------------------------------------------------------------
//   Apply to the whole prefix (don't decompose)
// ----------------------------------------------------------------------------
{
    return Do(prefix);
}


Tree *MapAction::DoPostfix(Postfix *postfix)
// ----------------------------------------------------------------------------
//   Apply to the whole postfix (don't decompose)
// ----------------------------------------------------------------------------
{
    return Do(postfix);
}


Tree *MapAction::DoBlock(Block *block)
// ----------------------------------------------------------------------------
//   Apply to the whole block (don't decompose)
// ----------------------------------------------------------------------------
{
    return Do(block);
}



// ============================================================================
// 
//   Stack depth check
// 
// ============================================================================

uint StackDepthCheck::stack_depth      = 0;
uint StackDepthCheck::max_stack_depth  = 0;
bool StackDepthCheck::in_error_handler = false;
bool StackDepthCheck::in_error         = false;


void StackDepthCheck::StackOverflow(Tree *what)
// ----------------------------------------------------------------------------
//   We have a stack overflow, bummer
// ----------------------------------------------------------------------------
{
    if (!max_stack_depth)
    {
        max_stack_depth = Options::options->stack_depth;
        if (stack_depth <= max_stack_depth)
            return;
    }
    if (in_error_handler)
    {
        Error("Double stack overflow in '$1'", what, NULL, NULL).Display();
        in_error_handler = false;
    }
    else
    {
        in_error = true;
        LocalSave<bool> overflow(in_error_handler, true);
        LocalSave<uint> depth(stack_depth, 1);
        Ooops("Stack overflow evaluating '$1'", what);
    }
}

XL_END
