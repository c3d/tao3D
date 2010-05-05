// ****************************************************************************
//  opcodes_define.h               (C) 1992-2009 Christophe de Dinechin (ddd) 
//                                                                 XL2 project 
// ****************************************************************************
// 
//   File Description:
// 
//     Macros used to declare built-ins.
// 
//     Usage:
//     #include "opcodes_declare.h"
//     #include "builtins.tbl"
// 
//     #include "opcodes_define.h"
//     #include "builtins.tbl"
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

#undef INFIX
#undef PREFIX
#undef POSTFIX
#undef BLOCK
#undef NAME
#undef TYPE
#undef PARM

#ifndef XL_SCOPE
#define XL_SCOPE "xl_"
#endif // XL_SCOPE


#define INFIX(name, rtype, t1, symbol, t2, _code)                       \
    do                                                                  \
    {                                                                   \
        Infix_p ldecl = new Infix(":", new Name("l"), new Name(#t1));   \
        Infix_p rdecl = new Infix(":", new Name("r"), new Name(#t2));   \
        Infix_p from = new Infix(symbol, ldecl, rdecl);                 \
        Name_p to = new Name(symbol);                                   \
        eval_fn fn = (eval_fn) xl_##name;                               \
        Rewrite *rw = c->EnterRewrite(from, to);                        \
        to->code = fn;                                                  \
        to->Set<SymbolsInfo>(c);                                        \
        to->Set<TypeInfo> (rtype##_type);                               \
        compiler->EnterBuiltin(XL_SCOPE #name,                          \
                               to, rw->parameters, fn);                 \
    } while(0);


#define PARM(symbol, type)                                      \
        if (text(#type) == "tree")                              \
        {                                                       \
            Name_p symbol##_decl = new Name(#symbol);           \
            parameters.push_back(symbol##_decl);                \
        }                                                       \
        else                                                    \
        {                                                       \
            Infix_p symbol##_decl = new Infix(":",              \
                                             new Name(#symbol), \
                                             new Name(#type));  \
            parameters.push_back(symbol##_decl);                \
        }


#define PREFIX(name, rtype, symbol, parms, _code)                       \
    do                                                                  \
    {                                                                   \
        TreeList parameters;                                            \
        parms;                                                          \
        eval_fn fn = (eval_fn) xl_##name;                               \
        if (parameters.size())                                          \
        {                                                               \
            Tree_p parmtree = ParametersTree(parameters);               \
            Prefix_p from = new Prefix(new Name(symbol), parmtree);     \
            Name_p to = new Name(symbol);                               \
            Rewrite *rw = c->EnterRewrite(from, to);                    \
            to->code = fn;                                              \
            to->Set<SymbolsInfo> (c);                                   \
            to->Set<TypeInfo> (rtype##_type);                           \
            compiler->EnterBuiltin(XL_SCOPE #name,                      \
                                   to, rw->parameters, fn);             \
        }                                                               \
        else                                                            \
        {                                                               \
            Name_p n  = new Name(symbol);                               \
            n->code = fn;                                               \
            n->Set<SymbolsInfo> (c);                                    \
            n ->Set<TypeInfo> (rtype##_type);                           \
            c->EnterName(symbol, n);                                    \
            TreeList noparms;                                           \
            compiler->EnterBuiltin(XL_SCOPE #name, n, noparms, fn);     \
        }                                                               \
    } while(0);


#define POSTFIX(name, rtype, parms, symbol, _code)                      \
    do                                                                  \
    {                                                                   \
        TreeList  parameters;                                           \
        parms;                                                          \
        Tree_p parmtree = ParametersTree(parameters);                   \
        Postfix_p from = new Postfix(parmtree, new Name(symbol));       \
        Name_p to = new Name(symbol);                                   \
        eval_fn fn = (eval_fn) xl_##name;                               \
        Rewrite *rw = c->EnterRewrite(from, to);                        \
        to->code = fn;                                                  \
        to->Set<SymbolsInfo> (c);                                       \
        to->Set<TypeInfo> (rtype##_type);                               \
        compiler->EnterBuiltin(XL_SCOPE #name,                          \
                               to, rw->parameters, to->code);           \
    } while(0);


#define BLOCK(name, rtype, open, type, close, _code)                    \
    do                                                                  \
    {                                                                   \
        Infix_p parms = new Infix(":", new Name("V"), new Name(#type)); \
        Block_p from = new Block(parms, open, close);                   \
        Name_p to = new Name(#name);                                    \
        eval_fn fn = (eval_fn) xl_##name;                               \
        Rewrite *rw = c->EnterRewrite(from, to);                        \
        to->code = fn;                                                  \
        to->Set<SymbolsInfo> (c);                                       \
        to->Set<TypeInfo> (rtype##_type);                               \
        compiler->EnterBuiltin(XL_SCOPE #name, to,                      \
                               rw->parameters, to->code);               \
    } while (0);



#define NAME(symbol)                            \
    do                                          \
    {                                           \
        Name_p n = new Name(#symbol);           \
        n->code = xl_identity;                  \
        n->Set<SymbolsInfo> (c);                \
        c->EnterName(#symbol, n);               \
        xl_##symbol = n;                        \
        compiler->EnterGlobal(n, &xl_##symbol); \
    } while (0);


#define TYPE(symbol)                                                    \
    do                                                                  \
    {                                                                   \
        /* Type alone evaluates as self */                              \
        Name_p n = new Name(#symbol);                                   \
        eval_fn fn = (eval_fn) xl_identity;                             \
        n->code = fn;                                                   \
        n->Set<SymbolsInfo> (c);                                        \
        c->EnterName(#symbol, n);                                       \
        symbol##_type = n;                                              \
        compiler->EnterGlobal(n, &symbol##_type);                       \
                                                                        \
        /* Type as infix : evaluates to type check, e.g. 0 : integer */ \
        Infix_p from = new Infix(":", new Name("V"), new Name(#symbol)); \
        Name_p to = new Name(#symbol);                                  \
        Rewrite *rw = c->EnterRewrite(from, to);                        \
        eval_fn typeTestFn = (eval_fn) xl_##symbol##_cast;              \
        to->code = typeTestFn;                                          \
        to->Set<SymbolsInfo> (c);                                       \
        compiler->EnterBuiltin(XL_SCOPE #symbol,                        \
                               to, rw->parameters, typeTestFn);         \
                                                                        \
    } while(0);
