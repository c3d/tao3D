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

Tree_p xl_identity(Tree_p what)
// ----------------------------------------------------------------------------
//   Return the input tree unchanged
// ----------------------------------------------------------------------------
{
    return what;
}


Tree_p xl_evaluate(Tree_p what)
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
    Tree_p result = symbols->Run(what);
    if (result != what)
        result->source = xl_source(what);
    return result;
}


Tree_p xl_repeat(Tree_p self, Tree_p what, longlong count)
// ----------------------------------------------------------------------------
//   Compile the tree if necessary, then evaluate it count times
// ----------------------------------------------------------------------------
{
    if (!what)
        return what;
    Symbols *symbols = self->Symbols();
    if (!symbols)
        symbols = Symbols::symbols;
    Tree_p result = what;

    while (count-- > 0)
        result = symbols->Run(what);
    if (result != what)
        result->source = xl_source(what);
    return result;
}


bool xl_same_text(Tree_p what, const char *ref)
// ----------------------------------------------------------------------------
//   Compile the tree if necessary, then evaluate it
// ----------------------------------------------------------------------------
{
    Text_p tval = what->AsText(); assert(tval);
    return tval->value == text(ref);
}


bool xl_same_shape(Tree_p left, Tree_p right)
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


Tree_p xl_infix_match_check(Tree_p value, Infix_p ref)
// ----------------------------------------------------------------------------
//   Check if the value is matching the given infix
// ----------------------------------------------------------------------------
{
    while (Block_p block = value->AsBlock())
        if (block->opening == "(" && block->closing == ")")
            value = block->child;
    if (Infix_p infix = value->AsInfix())
        if (infix->name == ref->name)
            return infix;
    return NULL;
}


Tree_p xl_type_check(Tree_p value, Tree_p type)
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
        return false;
    }

    // Check if this is a closure or something we want to evaluate
    Tree_p original = value;
    if (!value->IsConstant() && value->code)
        value = value->code(value);

    Infix_p typeExpr = Symbols::symbols->CompileTypeTest(type);
    typecheck_fn typecheck = (typecheck_fn) typeExpr->code;
    Tree_p afterTypeCast = typecheck(typeExpr, value);
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

Tree_p xl_new_integer(longlong value)
// ----------------------------------------------------------------------------
//    Called by generated code to build a new Integer
// ----------------------------------------------------------------------------
{
    Tree_p result = new Integer(value);
    result->code = xl_identity;
    return result;
}


Tree_p xl_new_real(double value)
// ----------------------------------------------------------------------------
//    Called by generated code to build a new Real
// ----------------------------------------------------------------------------
{
    Tree_p result = new Real (value);
    result->code = xl_identity;
    return result;
}


Tree_p xl_new_character(kstring value)
// ----------------------------------------------------------------------------
//    Called by generated code to build a new single-quoted Text
// ----------------------------------------------------------------------------
{
    Tree_p result = new Text(value, "'", "'");
    result->code = xl_identity;
    return result;
}


Tree_p xl_new_text(kstring value)
// ----------------------------------------------------------------------------
//    Called by generated code to build a new double-quoted Text
// ----------------------------------------------------------------------------
{
    Tree_p result = new Text(text(value));
    result->code = xl_identity;
    return result;
}


Tree_p xl_new_xtext(kstring value, kstring open, kstring close)
// ----------------------------------------------------------------------------
//    Called by generated code to build a new arbitrarily-quoted Text
// ----------------------------------------------------------------------------
{
    Tree_p result = new Text(value, open, close);
    result->code = xl_identity;
    return result;
}


Tree_p xl_new_block(Block_p source, Tree_p child)
// ----------------------------------------------------------------------------
//    Called by generated code to build a new block
// ----------------------------------------------------------------------------
{
    Tree_p result = new Block(source, child);
    result->code = xl_identity;
    return result;
}


Tree_p xl_new_prefix(Prefix_p source, Tree_p left, Tree_p right)
// ----------------------------------------------------------------------------
//    Called by generated code to build a new Prefix
// ----------------------------------------------------------------------------
{
    Tree_p result = new Prefix(source, left, right);
    result->code = xl_identity;
    return result;
}


Tree_p xl_new_postfix(Postfix_p source, Tree_p left, Tree_p right)
// ----------------------------------------------------------------------------
//    Called by generated code to build a new Postfix
// ----------------------------------------------------------------------------
{
    Tree_p result = new Postfix(source, left, right);
    result->code = xl_identity;
    return result;
}


Tree_p xl_new_infix(Infix_p source, Tree_p left, Tree_p right)
// ----------------------------------------------------------------------------
//    Called by generated code to build a new Infix
// ----------------------------------------------------------------------------
{
    Tree_p result = new Infix(source, left, right);
    result->code = xl_identity;
    return result;
}



// ============================================================================
//
//    Closure management
//
// ============================================================================

Tree_p xl_new_closure(Tree_p expr, uint ntrees, ...)
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
    Prefix_p result = new Prefix(expr, NULL);
    Prefix_p parent = result;
    va_list va;
    va_start(va, ntrees);
    for (uint i = 0; i < ntrees; i++)
    {
        Tree_p arg = va_arg(va, Tree_p);
        IFTRACE(closure)
            std::cerr << "  ARG: " << arg << '\n';
        Prefix_p item = new Prefix(arg, NULL);
        parent->right = item;
        parent = item;
    }
    va_end(va);
    parent->right = xl_false;

    // Generate the code for the arguments
    Compiler * compiler = Context::context->compiler;
    eval_fn fn = compiler->closures[ntrees];
    if (!fn)
    {
        TreeList noParms;
        CompiledUnit unit(compiler, result, noParms);
        unit.CallClosure(result, ntrees);
        fn = unit.Finalize();
        compiler->closures[ntrees] = fn;
        compiler->functions.erase(result);
    }
    result->code = fn;
    xl_set_source(result, expr);

    return result;
}


Tree_p xl_type_error(Tree_p what)
// ----------------------------------------------------------------------------
//   Display message if we have a type error
// ----------------------------------------------------------------------------
{
    Symbols *syms = what->Symbols();
    if (!syms)
        syms = Symbols::symbols;
    LocalSave<Symbols *> saveSyms(Symbols::symbols, syms);
    return Ooops("No form matches '$1'", what);
}



// ============================================================================
//
//    Type matching
//
// ============================================================================

#pragma GCC diagnostic ignored "-Wunused-parameter"

Tree_p xl_boolean_cast(Tree_p source, Tree_p value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as a boolean value (true/false)
// ----------------------------------------------------------------------------
{
    value = xl_evaluate(value);
    if (value == xl_true || value == xl_false)
        return value;
    return NULL;
}


Tree_p xl_integer_cast(Tree_p source, Tree_p value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as an integer
// ----------------------------------------------------------------------------
{
    value = xl_evaluate(value);
    if (Integer_p it = value->AsInteger())
        return it;
    return NULL;
}


Tree_p xl_real_cast(Tree_p source, Tree_p value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as a real
// ----------------------------------------------------------------------------
{
    value = xl_evaluate(value);
    if (Real_p rt = value->AsReal())
        return rt;
    if (Integer_p it = value->AsInteger())
        return new Real(it->value);
    return NULL;
}


Tree_p xl_text_cast(Tree_p source, Tree_p value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as a text
// ----------------------------------------------------------------------------
{
    value = xl_evaluate(value);
    if (Text_p tt = value->AsText())
        if (tt->opening != "'")
            return tt;
    return NULL;
}


Tree_p xl_character_cast(Tree_p source, Tree_p value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as a character
// ----------------------------------------------------------------------------
{
    value = xl_evaluate(value);
    if (Text_p tt = value->AsText())
        if (tt->opening == "'")
            return tt;
    return NULL;
}


Tree_p xl_tree_cast(Tree_p source, Tree_p value)
// ----------------------------------------------------------------------------
//   Don't really check the argument
// ----------------------------------------------------------------------------
{
    return value;
}


Tree_p xl_symbolicname_cast(Tree_p source, Tree_p value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as a name
// ----------------------------------------------------------------------------
{
    if (Name_p nt = value->AsName())
        return nt;
    value = xl_evaluate(value);
    if (Name_p afterEval = value->AsName())
        return afterEval;
    return NULL;
}


Tree_p xl_infix_cast(Tree_p source, Tree_p value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as an infix
// ----------------------------------------------------------------------------
{
    if (Infix_p it = value->AsInfix())
        return it;
    return NULL;
}


Tree_p xl_prefix_cast(Tree_p source, Tree_p value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as a prefix
// ----------------------------------------------------------------------------
{
    if (Prefix_p it = value->AsPrefix())
        return it;
    return NULL;
}


Tree_p xl_postfix_cast(Tree_p source, Tree_p value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as a postfix
// ----------------------------------------------------------------------------
{
    if (Postfix_p it = value->AsPostfix())
        return it;
    return NULL;
}


Tree_p xl_block_cast(Tree_p source, Tree_p value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as a block
// ----------------------------------------------------------------------------
{
    if (Block_p it = value->AsBlock())
        return it;
    return NULL;
}



// ============================================================================
//
//   Managing calls to/from XL
//
// ============================================================================

Tree_p XLCall::operator() (Symbols *syms, bool nullIfBad, bool cached)
// ----------------------------------------------------------------------------
//    Perform the given call in the given context
// ----------------------------------------------------------------------------
{
    if (!syms)
        syms = Symbols::symbols;
    Tree_p callee = syms->CompileCall(name, args, nullIfBad, cached);
    if (callee && callee->code)
        callee = callee->code(callee);
    return callee;
}


Tree_p XLCall::build(Symbols *syms)
// ----------------------------------------------------------------------------
//    Perform the given call in the given context
// ----------------------------------------------------------------------------
{
    if (!syms)
        syms = Symbols::symbols;
    Tree_p callee = syms->CompileCall(name, args);
    return callee;
}


Tree_p xl_invoke(eval_fn toCall, Tree_p src, uint numargs, Tree_p *args)
// ----------------------------------------------------------------------------
//   Invoke a callback with the right number of arguments
// ----------------------------------------------------------------------------
//   We generate a function with the right signature using LLVM
{
    Compiler * compiler = Context::context->compiler;
    adapter_fn fn = compiler->EnterArrayToArgsAdapter(numargs);
    return fn (toCall, src, args);
}



// ============================================================================
//
//    Loading trees from external files
//
// ============================================================================

Tree_p xl_load(text name)
// ----------------------------------------------------------------------------
//    Load a file from disk
// ----------------------------------------------------------------------------
{
    // Check if the file has already been loaded somehwere.
    // If so, return the loaded file
    if (MAIN->files.count(name) > 0)
    {
        SourceFile &sf = MAIN->files[name];
        Symbols::symbols->Import(sf.symbols);
        return sf.tree.tree;
    }

    Parser parser(name.c_str(), MAIN->syntax, MAIN->positions, MAIN->errors);
    Tree_p tree = parser.Parse();
    if (!tree)
        return Ooops("Unable to load file '$1'", new Text(name));

    Symbols *old = Symbols::symbols;
    Symbols *syms = new Symbols(Context::context);
    MAIN->files[name] = SourceFile(name, tree, syms);
    Symbols::symbols = syms;
    tree->SetSymbols(syms);
    tree = syms->CompileAll(tree);
    Symbols::symbols = old;
    old->Import(syms);

    return tree;
}

Tree_p xl_load_csv(text name)
// ----------------------------------------------------------------------------
//    Load a comma-separated file from disk
// ----------------------------------------------------------------------------
{
    // Check if the file has already been loaded somehwere.
    // If so, return the loaded file
    if (MAIN->files.count(name) > 0)
    {
        SourceFile &sf = MAIN->files[name];
        Symbols::symbols->Import(sf.symbols);
        return sf.tree.tree;
    }

    Tree_p tree = NULL;
    Tree_p line = NULL;
    char buffer[256];
    char *ptr = buffer;
    char *end = buffer + sizeof(buffer) - 1;
    bool has_quote = false;
    bool has_nl = false;
    FILE *f = fopen(name.c_str(), "r");

    *end = 0;
    while (!feof(f))
    {
        int c = getc(f);

        if (c == 0xA0)          // Hard space generated by Numbers, skip
            continue;

        has_nl = c == '\n' || c == EOF;
        if (has_nl || (c == ',' && !has_quote))
            c = 0;
        else if (c == '"')
            has_quote = !has_quote;

        if (ptr < end)
            *ptr++ = c;

        if (!c)
        {
            text token;
            Tree_p child = NULL;

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
            if (has_nl)
            {
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
        return Ooops("Unable to load CSV file '$1'", new Text(name));

    // Store that we use the file
    struct stat st;
    stat(name.c_str(), &st);
    Symbols *old = Symbols::symbols;
    Symbols *syms = new Symbols(Context::context);
    MAIN->files[name] = SourceFile(name, tree, syms);
    Symbols::symbols = syms;
    tree->SetSymbols(syms);
    tree = syms->CompileAll(tree);
    Symbols::symbols = old;
    old->Import(syms);

    return tree;
}


Tree_p xl_load_tsv(text name)
// ----------------------------------------------------------------------------
//    Load a tab-separated file from disk
// ----------------------------------------------------------------------------
{
    // Check if the file has already been loaded somehwere.
    // If so, return the loaded file
    if (MAIN->files.count(name) > 0)
    {
        SourceFile &sf = MAIN->files[name];
        Symbols::symbols->Import(sf.symbols);
        return sf.tree.tree;
    }

    Tree_p tree = NULL;
    Tree_p line = NULL;
    char buffer[256];
    char *ptr = buffer;
    char *end = buffer + sizeof(buffer) - 1;
    bool has_nl = false;
    FILE *f = fopen(name.c_str(), "r");

    *end = 0;
    while (!feof(f))
    {
        int c = getc(f);

        if (c == 0xA0)          // Hard space generated by Numbers, skip
            continue;

        has_nl = c == '\n' || c == EOF;
        if (has_nl || c == '\t')
            c = 0;

        if (ptr < end)
            *ptr++ = c;

        if (!c)
        {
            text token;
            Tree_p child = NULL;
            if (ptr > buffer + 1 && buffer[0] == '"' && ptr[-2] == '"')
            {
                token = text(buffer+1, ptr-buffer-2);
                child = new Text(token);
            }
            else if (isdigit(buffer[0]))
            {
                char *ptr2 = NULL;
                longlong l = strtoll(buffer, &ptr2, 10);
                if (ptr2 == ptr-1 || *ptr2 == '%')
                {
                    child = new Integer(l);
                    if (*ptr2 == '%')
                        child = new Postfix (child, new Name ("%"));
                }
                else
                {
                    double d = strtod (buffer, &ptr2);
                    if (ptr2 == ptr-1 || *ptr2 == '%')
                    {
                        child = new Real(d);
                        if (*ptr2 == '%')
                            child = new Postfix (child, new Name ("%"));
                    }
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
            if (has_nl)
            {
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
        return Ooops("Unable to load TSV file '$1'", new Text(name));

    // Store that we use the file
    struct stat st;
    stat(name.c_str(), &st);
    Symbols *old = Symbols::symbols;
    Symbols *syms = new Symbols(Context::context);
    MAIN->files[name] = SourceFile(name, tree, syms);
    Symbols::symbols = syms;
    tree->SetSymbols(syms);
    tree = syms->CompileAll(tree);
    Symbols::symbols = old;
    old->Import(syms);

    return tree;
}

XL_END
