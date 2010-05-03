#ifndef COMPILER_H
#define COMPILER_H
// ****************************************************************************
//  compiler.h                       (C) 1992-2009 Christophe de Dinechin (ddd) 
//                                                                 XL2 project 
// ****************************************************************************
// 
//   File Description:
// 
//    Just-in-time compiler for the trees
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

#include "tree.h"
#include <map>
#include <set>
#include <llvm/Support/IRBuilder.h>


namespace llvm
// ----------------------------------------------------------------------------
//   Forward classes from LLVM
// ----------------------------------------------------------------------------
{
    class LLVMContext;
    class Module;
    class ExecutionEngine;
    class FunctionPassManager;
    class StructType;
    class PointerType;
    class FunctionType;
    class BasicBlock;
    class Value;
};

XL_BEGIN

struct CompiledUnit;
struct Options;
typedef std::map<text, llvm::Function *>    builtins_map;
typedef std::map<Tree_p , llvm::Value *>    value_map;
typedef std::map<Tree_p , llvm::Function *> function_map;
typedef std::map<uint, eval_fn>             closure_map;
typedef std::set<Tree_p>                    closure_set;
typedef std::set<Tree_p>                    data_set;
typedef std::set<Tree_p>                    deleted_set;
typedef Tree_p (*adapter_fn) (eval_fn callee, Tree_p src, Tree_p *args);


struct Compiler
// ----------------------------------------------------------------------------
//   Just-in-time compiler data
// ----------------------------------------------------------------------------
{
    Compiler(kstring moduleName = "xl", uint optimize_level = 999);
    ~Compiler();

    llvm::Function *          EnterBuiltin(text name,
                                           Tree_p to,
                                           TreeList parms,
                                           eval_fn code);
    llvm::Function *          ExternFunction(kstring name, void *address,
                                             const llvm::Type *retType,
                                             int parmCount, ...);
    adapter_fn                EnterArrayToArgsAdapter(uint numtrees);
    llvm::Value *             EnterGlobal(Name_p name, Name_p *address);
    llvm::Value *             EnterConstant(Tree_p constant);
    bool                      IsKnown(Tree_p value);
    llvm::Value *             Known(Tree_p value);

    void                      FreeResources(Tree_p tree);
    void                      FreeResources();

    void                      Reset();

public:
    llvm::LLVMContext         *context;
    llvm::Module              *module;
    llvm::ExecutionEngine     *runtime;
    llvm::FunctionPassManager *optimizer;
    llvm::StructType          *treeTy;
    llvm::PointerType         *treePtrTy;
    llvm::PointerType         *treePtrPtrTy;
    llvm::StructType          *integerTreeTy;
    llvm::PointerType         *integerTreePtrTy;
    llvm::StructType          *realTreeTy;
    llvm::PointerType         *realTreePtrTy;
    llvm::StructType          *prefixTreeTy;
    llvm::PointerType         *prefixTreePtrTy;
    llvm::FunctionType        *evalTy;
    llvm::PointerType         *evalFnTy;
    llvm::PointerType         *symbolPtrTy;
    llvm::PointerType         *infoPtrTy;
    llvm::PointerType         *charPtrTy;
    llvm::Function            *xl_evaluate;
    llvm::Function            *xl_same_text;
    llvm::Function            *xl_same_shape;
    llvm::Function            *xl_infix_match_check;
    llvm::Function            *xl_type_check;
    llvm::Function            *xl_type_error;
    llvm::Function            *xl_new_integer;
    llvm::Function            *xl_new_real;
    llvm::Function            *xl_new_character;
    llvm::Function            *xl_new_text;
    llvm::Function            *xl_new_xtext;
    llvm::Function            *xl_new_block;
    llvm::Function            *xl_new_prefix;
    llvm::Function            *xl_new_postfix;
    llvm::Function            *xl_new_infix;
    llvm::Function            *xl_new_closure;
    builtins_map               builtins;
    function_map               functions;
    value_map                  globals;
    closure_map                closures;
    deleted_set                deleted;
    closure_map                array_to_args_adapters;
};


struct CompiledUnit
// ----------------------------------------------------------------------------
//  A compilation unit, which typically corresponds to an expression
// ----------------------------------------------------------------------------
{
    CompiledUnit(Compiler *comp, Tree_p source, TreeList parms);
    ~CompiledUnit();

    bool                IsForwardCall()         { return entrybb == NULL; }
    eval_fn             Finalize();

    enum { knowAll = -1, knowGlobals = 1, knowLocals = 2, knowValues = 4 };

    llvm::Value *       NeedStorage(Tree_p tree);
    bool                IsKnown(Tree_p tree, uint which = knowAll);
    llvm::Value *       Known(Tree_p tree, uint which = knowAll );

    llvm::Value *       ConstantInteger(Integer_p what);
    llvm::Value *       ConstantReal(Real_p what);
    llvm::Value *       ConstantText(Text_p what);
    llvm::Value *       ConstantTree(Tree_p what);

    llvm::Value *       NeedLazy(Tree_p subexpr);
    llvm::Value *       MarkComputed(Tree_p subexpr, llvm::Value *value);
    llvm::BasicBlock *  BeginLazy(Tree_p subexpr);
    void                EndLazy(Tree_p subexpr, llvm::BasicBlock *skip);

    llvm::BasicBlock *  NeedTest();
    llvm::Value *       Left(Tree_p);
    llvm::Value *       Right(Tree_p);
    llvm::Value *       Copy(Tree_p src, Tree_p dst, bool markDone=true);
    llvm::Value *       Invoke(Tree_p subexpr, Tree_p callee, TreeList args);
    llvm::Value *       CallEvaluate(Tree_p);
    llvm::Value *       CallNewBlock(Block_p);
    llvm::Value *       CallNewPrefix(Prefix_p);
    llvm::Value *       CallNewPostfix(Postfix_p);
    llvm::Value *       CallNewInfix(Infix_p);
    llvm::Value *       CreateClosure(Tree_p callee, TreeList &args);
    llvm::Value *       CallClosure(Tree_p callee, uint ntrees);
    llvm::Value *       CallTypeError(Tree_p what);

    llvm::BasicBlock *  TagTest(Tree_p code, ulong tag);
    llvm::BasicBlock *  IntegerTest(Tree_p code, longlong value);
    llvm::BasicBlock *  RealTest(Tree_p code, double value);
    llvm::BasicBlock *  TextTest(Tree_p code, text value);
    llvm::BasicBlock *  ShapeTest(Tree_p code, Tree_p other);
    llvm::BasicBlock *  InfixMatchTest(Tree_p code, Infix_p ref);
    llvm::BasicBlock *  TypeTest(Tree_p code, Tree_p type);

public:
    Compiler *          compiler;       // The compiler environment we use
    llvm::LLVMContext * context;        // The context we got from compiler
    Tree_p              source;         // The original source we compile

    llvm::IRBuilder<> * code;           // Instruction builder for code
    llvm::IRBuilder<> * data;           // Instruction builder for data
    llvm::Function *    function;       // Function we generate

    llvm::BasicBlock *  allocabb;       // Function entry point, allocas
    llvm::BasicBlock *  entrybb;        // Entry point for that code
    llvm::BasicBlock *  exitbb;         // Exit point for that code
    llvm::BasicBlock *  failbb;         // Where we go if tests fail

    value_map           value;          // Map tree -> LLVM value
    value_map           storage;        // Map tree -> LLVM alloca space
    value_map           computed;       // Map tree -> LLVM "computed" flag
    data_set            noeval;         // Data expressions we don't evaluate
};


struct ExpressionReduction
// ----------------------------------------------------------------------------
//   Record compilation state around a specific expression reduction
// ----------------------------------------------------------------------------
{
    ExpressionReduction(CompiledUnit &unit, Tree_p source);
    ~ExpressionReduction();

    void                NewForm();
    void                Succeeded();
    void                Failed();

public:
    CompiledUnit &      unit;           // Compilation unit we use
    Tree_p              source;         // Tree we build (mostly for debugging)
    llvm::LLVMContext * context;        // Inherited context

    llvm::Value *       storage;        // Storage for expression value
    llvm::Value *       computed;       // Flag telling if value was computed

    llvm::BasicBlock *  savedfailbb;    // Saved location of failbb

    llvm::BasicBlock *  entrybb;        // Entry point to subcase
    llvm::BasicBlock *  savedbb;        // Saved position before subcase
    llvm::BasicBlock *  successbb;      // Successful completion of expression

    value_map           savedvalue;     // Saved compile unit value map
};


#define LLVM_INTTYPE(t)         llvm::IntegerType::get(*context, sizeof(t) * 8)
#define LLVM_BOOLTYPE           llvm::Type::getInt1Ty(*context)

XL_END

#endif // COMPILER_H

