#ifndef TREE_H
#define TREE_H
// ****************************************************************************
//  tree.h                          (C) 1992-2003 Christophe de Dinechin (ddd)
//                                                            XL2 project
// ****************************************************************************
//
//   File Description:
//
//     Basic representation of parse tree.
//
//     See the big comment at the top of parser.h for details about
//     the basics of XL tree representation
//
//
//
//
//
//
// ****************************************************************************
// This program is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html for details
// ****************************************************************************
// * File       : $RCSFile$
// * Revision   : $Revision$
// * Date       : $Date$
// ****************************************************************************

#include "base.h"
#include <map>
#include <vector>
#include <cassert>


XL_BEGIN

// ============================================================================
//
//    The types being defined or used to define XL trees
//
// ============================================================================

struct Tree;                                    // Base tree
struct Integer;                                 // Integer: 0, 3, 8
struct Real;                                    // Real: 3.2, 1.6e4
struct Text;                                    // Text: "ABC"
struct Name;                                    // Name / symbol: ABC, ++-
struct Prefix;                                  // Prefix: sin X
struct Postfix;                                 // Postfix: 3!
struct Infix;                                   // Infix: A+B, newline
struct Block;                                   // Block: (A), {A}
struct Action;                                  // Action on trees
struct Info;                                    // Information in trees
struct Symbols;                                 // Symbol table
struct Sha1;                                    // Hash used for id-ing trees



// ============================================================================
// 
//    Pointer and structure types
// 
// ============================================================================

typedef Tree            *Tree_p;
typedef Integer         *Integer_p;
typedef Real            *Real_p;
typedef Text            *Text_p;
typedef Name            *Name_p;
typedef Prefix          *Prefix_p;
typedef Postfix         *Postfix_p;
typedef Infix           *Infix_p;
typedef Block           *Block_p;

typedef ulong tree_position;                    // Position in context
typedef std::vector<Tree_p> TreeList;           // A list of trees
typedef Tree_p (*eval_fn) (Tree_p);             // Compiled evaluation code


// ============================================================================
//
//    The Tree class
//
// ============================================================================

enum kind
// ----------------------------------------------------------------------------
//   The kinds of tree that compose an XL parse tree
// ----------------------------------------------------------------------------
{
    INTEGER, REAL, TEXT, NAME,                  // Leaf nodes
    BLOCK, PREFIX, POSTFIX, INFIX,              // Non-leaf nodes

    KIND_FIRST          = INTEGER,
    KIND_LAST           = INFIX,
    KIND_LEAF_FIRST     = INTEGER,
    KIND_LEAF_LAST      = NAME,
    KIND_NLEAF_FIRST    = BLOCK,
    KIND_NLEAF_LAST     = INFIX
};


struct Info
// ----------------------------------------------------------------------------
//   Information associated with a tree
// ----------------------------------------------------------------------------
{
                        Info(): next(NULL) {}
                        Info(const Info &) : next(NULL) {}
    virtual             ~Info() {}
    virtual Info *      Copy() { return next ? next->Copy() : NULL; }
    Info *next;
};


struct Tree
// ----------------------------------------------------------------------------
//   The base class for all XL trees
// ----------------------------------------------------------------------------
{
    enum { KINDBITS = 3, KINDMASK=7 };
    enum { NOWHERE = ~0UL };

    // Constructor and destructor
    Tree (kind k, tree_position pos = NOWHERE):
        tag((pos<<KINDBITS) | k), code(NULL), info(NULL), source(NULL) {}
    Tree(kind k, Tree_p from):
        tag(from->tag), code(from->code),
        info(from->info ? from->info->Copy() : NULL), source(from)
    {
        assert(k == Kind());
    }
    ~Tree();

    // Perform recursive actions on a tree
    Tree_p               Do(Action *action);
    Tree_p               Do(Action &action)    { return Do(&action); }

    // Attributes
    kind                Kind()                { return kind(tag & KINDMASK); }
    tree_position       Position()            { return (long) tag>>KINDBITS; }
    bool                IsLeaf()              { return Kind() <= NAME; }
    bool                IsConstant()          { return Kind() <= TEXT; }

    // Info
    template<class I>
    typename I::data_t  Get();
    template<class I>
    void                Set(typename I::data_t data);
    template<class I>
    void                Set2(typename I::data_t data);
    template<class I>
    I*                  GetInfo();
    template<class I>
    void                SetInfo(I *);
    template<class I>
    bool                Exists();
    template <class I>
    bool                Purge();
    template <class I>
    I*                  Remove();
    template <class I>
    I*                  Remove(I *);


    // Safe cast to an appropriate subclass
    Integer_p           AsInteger();
    Real_p              AsReal();
    Text_p              AsText();
    Name_p              AsName();
    Block_p             AsBlock();
    Infix_p             AsInfix();
    Prefix_p            AsPrefix();
    Postfix_p           AsPostfix();

    // Conversion to text
                        operator text();

    // Operator new to record the tree in the garbage collector
    void *              operator new(size_t sz);

public:
    ulong       tag;                            // Position + kind
    eval_fn     code;                           // Compiled code
    Info *      info;                           // Information for tree
    Tree_p      source;                         // Source for the tree

private:
    Tree (const Tree &o):
        tag(o.tag), code(NULL), info(NULL) {}
};


struct Action
// ----------------------------------------------------------------------------
//   An operation we do recursively on trees
// ----------------------------------------------------------------------------
{
    Action () {}
    virtual ~Action() {}
    virtual Tree_p Do (Tree_p what) = 0;

    // Specialization for the canonical nodes, default is to run them
    virtual Tree_p DoInteger(Integer_p what);
    virtual Tree_p DoReal(Real_p what);
    virtual Tree_p DoText(Text_p what);
    virtual Tree_p DoName(Name_p what);
    virtual Tree_p DoPrefix(Prefix_p what);
    virtual Tree_p DoPostfix(Postfix_p what);
    virtual Tree_p DoInfix(Infix_p what);
    virtual Tree_p DoBlock(Block_p what);
};


template <class I> inline typename I::data_t Tree::Get()
// ----------------------------------------------------------------------------
//   Find if we have an information of the right type in 'info'
// ----------------------------------------------------------------------------
{
    for (Info *i = info; i; i = i->next)
        if (I *ic = dynamic_cast<I *> (i))
            return (typename I::data_t) *ic;
    return typename I::data_t();
}


template <class I> inline void Tree::Set(typename I::data_t data)
// ----------------------------------------------------------------------------
//   Set the information given as an argument
// ----------------------------------------------------------------------------
{
    Info *i = new I(data);
    i->next = info;
    info = i;
}


template <class I> inline I* Tree::GetInfo()
// ----------------------------------------------------------------------------
//   Find if we have an information of the right type in 'info'
// ----------------------------------------------------------------------------
{
    for (Info *i = info; i; i = i->next)
        if (I *ic = dynamic_cast<I *> (i))
            return ic;
    return NULL;
}


template <class I> inline void Tree::Set2(typename I::data_t data)
// ----------------------------------------------------------------------------
//   Set the information given as an argument. Do not add new info if exists.
// ----------------------------------------------------------------------------
{
    I *i = GetInfo<I>();
    if (i)
    {
        Info *n = i->next;
        (*i) = data;
        i->next = n;
    }
    else
        Set<I>(data);
}


template <class I> inline void Tree::SetInfo(I *i)
// ----------------------------------------------------------------------------
//   Set the information given as an argument
// ----------------------------------------------------------------------------
{
    Info *last = i;
    while(last->next)
        last = last->next;
    last->next = info;
    info = i;
}


template <class I> inline bool Tree::Exists()
// ----------------------------------------------------------------------------
//   Verifies if the tree already has information of the given type
// ----------------------------------------------------------------------------
{
    for (Info *i = info; i; i = i->next)
        if (dynamic_cast<I *> (i))
            return true;
    return false;
}


template <class I> inline bool Tree::Purge()
// ----------------------------------------------------------------------------
//   Find and purge information of the given type
// ----------------------------------------------------------------------------
{
    Info *last = NULL;
    Info *next = NULL;
    bool purged = false;
    for (Info *i = info; i; i = next)
    {
        next = i->next;
        if (I *ic = dynamic_cast<I *> (i))
        {
            if (last)
                last->next = next;
            else
                info = next;
            delete ic;
            purged = true;
        }
        else
        {
            last = i;
        }
    }
    if (last)
        last->next = NULL;
    return purged;
}


template <class I> inline I* Tree::Remove()
// ----------------------------------------------------------------------------
//   Find information and remove it if it exists
// ----------------------------------------------------------------------------
{
    Info *prev = info;
    for (Info *i = info; i; i = i->next)
    {
        if (I *ic = dynamic_cast<I *> (i))
        {
            prev->next = i->next;
            ic->next = NULL;
            return ic;
        }
        prev = i;
    }
    return NULL;
}

template <class I> inline I* Tree::Remove(I *toFind)
// ----------------------------------------------------------------------------
//   Find information matching input and remove it if it exists
// ----------------------------------------------------------------------------
{
    Info *prev = info;
    for (Info *i = info; i; i = i->next)
    {
        I *ic = dynamic_cast<I *> (i);
        if (ic == toFind)
        {
            if (prev == info)
                info = info->next;
            else
                prev->next = i->next;
            ic->next = NULL;
            return ic;
        }
        prev = i;
    }
    return NULL;
}



// ============================================================================
//
//   Leaf nodes (integer, real, name, text)
//
// ============================================================================

struct Integer : Tree
// ----------------------------------------------------------------------------
//   Integer constants
// ----------------------------------------------------------------------------
{
    Integer(longlong i = 0, tree_position pos = NOWHERE):
        Tree(INTEGER, pos), value(i) {}
    Integer(Integer_p i): Tree(INTEGER, i), value(i->value) {}
    longlong            value;
    operator longlong()         { return value; }
};


struct Real : Tree
// ----------------------------------------------------------------------------
//   Real numbers
// ----------------------------------------------------------------------------
{
    Real(double d = 0.0, tree_position pos = NOWHERE):
        Tree(REAL, pos), value(d) {}
    Real(Real_p r): Tree(REAL, r), value(r->value) {}
    double              value;
    operator double()           { return value; }
};


struct Text : Tree
// ----------------------------------------------------------------------------
//   Text, e.g. "Hello World"
// ----------------------------------------------------------------------------
{
    Text(text t, text open="\"", text close="\"", tree_position pos=NOWHERE):
        Tree(TEXT, pos), value(t), opening(open), closing(close) {}
    Text(Text_p t):
        Tree(TEXT, t),
        value(t->value), opening(t->opening), closing(t->closing) {}
    text                value;
    text                opening, closing;
    static text         textQuote, charQuote;
    operator text()             { return value; }
};


struct Name : Tree
// ----------------------------------------------------------------------------
//   A node representing a name or symbol
// ----------------------------------------------------------------------------
{
    Name(text n, tree_position pos = NOWHERE):
        Tree(NAME, pos), value(n) {}
    Name(Name_p n):
        Tree(NAME, n), value(n->value) {}
    text                value;
    operator bool();
};



// ============================================================================
//
//   Structured types: Block, Prefix, Infix
//
// ============================================================================

struct Block : Tree
// ----------------------------------------------------------------------------
//   A block, such as (X), {X}, [X] or indented block
// ----------------------------------------------------------------------------
{
    Block(Tree_p c, text open, text close, tree_position pos = NOWHERE):
        Tree(BLOCK, pos), child(c), opening(open), closing(close) {}
    Block(Block_p b, Tree_p ch):
        Tree(BLOCK, b), child(ch), opening(b->opening), closing(b->closing) {}
    Tree_p              child;
    text                opening, closing;
    static text         indent, unindent;
};


struct Prefix : Tree
// ----------------------------------------------------------------------------
//   A prefix operator, e.g. sin X, +3
// ----------------------------------------------------------------------------
{
    Prefix(Tree_p l, Tree_p r, tree_position pos = NOWHERE):
        Tree(PREFIX, pos), left(l), right(r) {}
    Prefix(Prefix_p p, Tree_p l, Tree_p r):
        Tree(PREFIX, p), left(l), right(r) {}
    Tree_p               left;
    Tree_p               right;
};


struct Postfix : Tree
// ----------------------------------------------------------------------------
//   A postfix operator, e.g. 3!
// ----------------------------------------------------------------------------
{
    Postfix(Tree_p l, Tree_p r, tree_position pos = NOWHERE):
        Tree(POSTFIX, pos), left(l), right(r) {}
    Postfix(Postfix_p p, Tree_p l, Tree_p r):
        Tree(POSTFIX, p), left(l), right(r) {}
    Tree_p              left;
    Tree_p              right;
};


struct Infix : Tree
// ----------------------------------------------------------------------------
//   Infix operators, e.g. A+B, A and B, A,B,C,D,E
// ----------------------------------------------------------------------------
{
    Infix(text n, Tree_p l, Tree_p r, tree_position pos = NOWHERE):
        Tree(INFIX, pos), left(l), right(r), name(n) {}
    Infix(Infix_p i, Tree_p l, Tree_p r):
        Tree(INFIX, i), left(l), right(r), name(i->name) {}
    Tree_p              left;
    Tree_p              right;
    text                name;
};



// ============================================================================
//
//    Safe casts
//
// ============================================================================

inline Integer_p Tree::AsInteger()
// ----------------------------------------------------------------------------
//    Return a pointer to an Integer or NULL
// ----------------------------------------------------------------------------
{
    if (this && Kind() == INTEGER)
        return (Integer_p) this;
    return NULL;
}


inline Real_p Tree::AsReal()
// ----------------------------------------------------------------------------
//    Return a pointer to an Real or NULL
// ----------------------------------------------------------------------------
{
    if (this && Kind() == REAL)
        return (Real_p) this;
    return NULL;
}


inline Text_p Tree::AsText()
// ----------------------------------------------------------------------------
//    Return a pointer to an Text or NULL
// ----------------------------------------------------------------------------
{
    if (this && Kind() == TEXT)
        return (Text_p) this;
    return NULL;
}


inline Name_p Tree::AsName()
// ----------------------------------------------------------------------------
//    Return a pointer to an Name or NULL
// ----------------------------------------------------------------------------
{
    if (this && Kind() == NAME)
        return (Name_p) this;
    return NULL;
}


inline Block_p Tree::AsBlock()
// ----------------------------------------------------------------------------
//    Return a pointer to an Block or NULL
// ----------------------------------------------------------------------------
{
    if (this && Kind() == BLOCK)
        return (Block_p) this;
    return NULL;
}


inline Infix_p Tree::AsInfix()
// ----------------------------------------------------------------------------
//    Return a pointer to an Infix or NULL
// ----------------------------------------------------------------------------
{
    if (this && Kind() == INFIX)
        return (Infix_p) this;
    return NULL;
}


inline Prefix_p Tree::AsPrefix()
// ----------------------------------------------------------------------------
//    Return a pointer to an Prefix or NULL
// ----------------------------------------------------------------------------
{
    if (this && Kind() == PREFIX)
        return (Prefix_p) this;
    return NULL;
}


inline Postfix_p Tree::AsPostfix()
// ----------------------------------------------------------------------------
//    Return a pointer to an Postfix or NULL
// ----------------------------------------------------------------------------
{
    if (this && Kind() == POSTFIX)
        return (Postfix_p) this;
    return NULL;
}



// ============================================================================
//
//    Garbage collection roots
//
// ============================================================================

struct TreeRoot
// ----------------------------------------------------------------------------
//    A tree that shouldn't be garbage collected until the root dies
// ----------------------------------------------------------------------------
{
    TreeRoot(Tree_p t = NULL);
    TreeRoot(const TreeRoot &o);
    ~TreeRoot();
    operator Tree_p (void) { return tree; }
    bool operator< (const TreeRoot &o) const { return tree < o.tree; }
public:
    Tree_p       tree;
};


struct IntegerRoot : TreeRoot
// ----------------------------------------------------------------------------
//   Looks and behaves like a real_r, but keeps the value around
// ----------------------------------------------------------------------------
{
    IntegerRoot(Integer &r): TreeRoot(&r) {}
    Integer_p operator&()       { return (Integer_p) tree; }
    operator longlong()         { return ((Integer_p) tree)->value; }
};


struct RealRoot : TreeRoot
// ----------------------------------------------------------------------------
//   Looks and behaves like a real_r, but keeps the value around
// ----------------------------------------------------------------------------
{
    RealRoot(Real &r): TreeRoot(&r) {}
    Real_p operator&()          { return (Real_p) tree; }
    operator double()           { return ((Real_p) tree)->value; }
};


struct TextRoot : TreeRoot
// ----------------------------------------------------------------------------
//   Looks and behaves like a text_p, but keeps the value around
// ----------------------------------------------------------------------------
{
    TextRoot(Text_p t): TreeRoot(t) {}
    operator Text_p ()          { return (Text_p) tree; }
    text &      Value()         { return ((Text_p) tree)->value; }
};



// ============================================================================
//
//    Tree cloning
//
// ============================================================================

struct DeepCopyCloneMode;       // Child nodes are cloned too (default)
struct ShallowCopyCloneMode;    // Child nodes are referenced
struct NodeOnlyCloneMode;       // Child nodes are left NULL


template <typename mode> struct TreeCloneTemplate : Action
// ----------------------------------------------------------------------------
//   Clone a tree
// ----------------------------------------------------------------------------
{
    TreeCloneTemplate() {}
    virtual ~TreeCloneTemplate() {}

    Tree_p DoInteger(Integer_p what)
    {
        return new Integer(what->value, what->Position());
    }
    Tree_p DoReal(Real_p what)
    {
        return new Real(what->value, what->Position());

    }
    Tree_p DoText(Text_p what)
    {
        return new Text(what->value, what->opening, what->closing,
                        what->Position());
    }
    Tree_p DoName(Name_p what)
    {
        return new Name(what->value, what->Position());
    }

    Tree_p DoBlock(Block_p what)
    {
        return new Block(Clone(what->child), what->opening, what->closing,
                         what->Position());
    }
    Tree_p DoInfix(Infix_p what)
    {
        return new Infix (what->name, Clone(what->left), Clone(what->right),
                          what->Position());
    }
    Tree_p DoPrefix(Prefix_p what)
    {
        return new Prefix(Clone(what->left), Clone(what->right),
                          what->Position());
    }
    Tree_p DoPostfix(Postfix_p what)
    {
        return new Postfix(Clone(what->left), Clone(what->right),
                           what->Position());
    }
    Tree_p Do(Tree_p what)
    {
        return what;            // ??? Should not happen
    }
protected:
    // Default is to do a deep copy
    Tree_p  Clone(Tree_p t) { return t->Do(this); }
};


template<> inline
Tree_p TreeCloneTemplate<ShallowCopyCloneMode>::Clone(Tree_p t)
// ----------------------------------------------------------------------------
//   Specialization for the shallow copy clone
// ----------------------------------------------------------------------------
{
    return t;
}


template<> inline
Tree_p TreeCloneTemplate<NodeOnlyCloneMode>::Clone(Tree_p)
// ----------------------------------------------------------------------------
//   Specialization for the node-only clone
// ----------------------------------------------------------------------------
{
    return NULL;
}


typedef struct TreeCloneTemplate<DeepCopyCloneMode>     TreeClone;
typedef struct TreeCloneTemplate<ShallowCopyCloneMode>  ShallowCopyTreeClone;
typedef struct TreeCloneTemplate<NodeOnlyCloneMode>     NodeOnlyTreeClone;



// ============================================================================
//
//    Tree copying
//
// ============================================================================

enum CopyMode
// ----------------------------------------------------------------------------
//   Several ways of copying a tree.
// ----------------------------------------------------------------------------
{
    CM_RECURSIVE = 1,    // Copy child nodes (as long as their kind match)
    CM_NODE_ONLY         // Copy only one node
};


template <CopyMode mode> struct TreeCopyTemplate : Action
// ----------------------------------------------------------------------------
//   Copy a tree into another tree. Node values are copied, infos are not.
// ----------------------------------------------------------------------------
{
    TreeCopyTemplate(Tree_p dest): dest(dest) {}
    virtual ~TreeCopyTemplate() {}

    Tree_p DoInteger(Integer_p what)
    {
        if (Integer_p it = dest->AsInteger())
        {
            it->value = what->value;
            it->tag = ((what->Position()<<Tree::KINDBITS) | it->Kind());
            return what;
        }
        return NULL;
    }
    Tree_p DoReal(Real_p what)
    {
        if (Real_p rt = dest->AsReal())
        {
            rt->value = what->value;
            rt->tag = ((what->Position()<<Tree::KINDBITS) | rt->Kind());
            return what;
        }
        return NULL;
    }
    Tree_p DoText(Text_p what)
    {
        if (Text_p tt = dest->AsText())
        {
            tt->value = what->value;
            tt->tag = ((what->Position()<<Tree::KINDBITS) | tt->Kind());
            return what;
        }
        return NULL;
    }
    Tree_p DoName(Name_p what)
    {
        if (Name_p nt = dest->AsName())
        {
            nt->value = what->value;
            nt->tag = ((what->Position()<<Tree::KINDBITS) | nt->Kind());
            return what;
        }
        return NULL;
    }

    Tree_p DoBlock(Block_p what)
    {
        if (Block_p bt = dest->AsBlock())
        {
            bt->opening = what->opening;
            bt->closing = what->closing;
            bt->tag = ((what->Position()<<Tree::KINDBITS) | bt->Kind());
            if (mode == CM_RECURSIVE)
            {
                dest = bt->child;
                Tree_p  br = what->child->Do(this);
                dest = bt;
                return br;
            }
            return what;
        }
        return NULL;
    }
    Tree_p DoInfix(Infix_p what)
    {
        if (Infix_p it = dest->AsInfix())
        {
            it->name = what->name;
            it->tag = ((what->Position()<<Tree::KINDBITS) | it->Kind());
            if (mode == CM_RECURSIVE)
            {
                dest = it->left;
                Tree_p lr = what->left->Do(this);
                dest = it;
                if (!lr)
                    return NULL;
                dest = it->right;
                Tree_p rr = what->right->Do(this);
                dest = it;
                if (!rr)
                    return NULL;
            }
            return what;
        }
        return NULL;
    }
    Tree_p DoPrefix(Prefix_p what)
    {
        if (Prefix_p pt = dest->AsPrefix())
        {
            pt->tag = ((what->Position()<<Tree::KINDBITS) | pt->Kind());
            if (mode == CM_RECURSIVE)
            {
                dest = pt->left;
                Tree_p lr = what->left->Do(this);
                dest = pt;
                if (!lr)
                    return NULL;
                dest = pt->right;
                Tree_p rr = what->right->Do(this);
                dest = pt;
                if (!rr)
                    return NULL;
            }
            return what;
        }
        return NULL;
    }
    Tree_p DoPostfix(Postfix_p what)
    {
        if (Postfix_p pt = dest->AsPostfix())
        {
            pt->tag = ((what->Position()<<Tree::KINDBITS) | pt->Kind());
            if (mode == CM_RECURSIVE)
            {
                dest = pt->left;
                Tree_p lr = what->left->Do(this);
                dest = pt;
                if (!lr)
                    return NULL;
                dest = pt->right;
                Tree_p rr = what->right->Do(this);
                dest = pt;
                if (!rr)
                    return NULL;
            }
            return what;
        }
        return NULL;
    }
    Tree_p Do(Tree_p what)
    {
        return what;            // ??? Should not happen
    }
    Tree_p dest;
};



// ============================================================================
//
//    Tree shape equality comparison
//
// ============================================================================

enum TreeMatchMode
// ----------------------------------------------------------------------------
//   The ways of comparing trees
// ----------------------------------------------------------------------------
{
    TM_RECURSIVE = 1,  // Compare whole tree
    TM_NODE_ONLY       // Compare one node only
};


template <TreeMatchMode mode> struct TreeMatchTemplate : Action
// ----------------------------------------------------------------------------
//   Check if two trees match in structure
// ----------------------------------------------------------------------------
{
    TreeMatchTemplate (Tree_p t): test(t) {}
    Tree_p DoInteger(Integer_p what)
    {
        if (Integer_p it = test->AsInteger())
            if (it->value == what->value)
                return what;
        return NULL;
    }
    Tree_p DoReal(Real_p what)
    {
        if (Real_p rt = test->AsReal())
            if (rt->value == what->value)
                return what;
        return NULL;
    }
    Tree_p DoText(Text_p what)
    {
        if (Text_p tt = test->AsText())
            if (tt->value == what->value)
                return what;
        return NULL;
    }
    Tree_p DoName(Name_p what)
    {
        if (Name_p nt = test->AsName())
            if (nt->value == what->value)
                return what;
        return NULL;
    }

    Tree_p DoBlock(Block_p what)
    {
        // Test if we exactly match the block, i.e. the reference is a block
        if (Block_p bt = test->AsBlock())
        {
            if (bt->opening == what->opening &&
                bt->closing == what->closing)
            {
                if (mode == TM_NODE_ONLY)
                    return what;
                test = bt->child;
                Tree_p br = what->child->Do(this);
                test = bt;
                if (br)
                    return br;
            }
        }
        return NULL;
    }
    Tree_p DoInfix(Infix_p what)
    {
        if (Infix_p it = test->AsInfix())
        {
            // Check if we match the tree, e.g. A+B vs 2+3
            if (it->name == what->name)
            {
                if (mode == TM_NODE_ONLY)
                    return what;
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
        return NULL;
    }
    Tree_p DoPrefix(Prefix_p what)
    {
        if (Prefix_p pt = test->AsPrefix())
        {
            if (mode == TM_NODE_ONLY)
                return what;
            // Check if we match the tree, e.g. f(A) vs. f(2)
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
            return what;
        }
        return NULL;
    }
    Tree_p DoPostfix(Postfix_p what)
    {
        if (Postfix_p pt = test->AsPostfix())
        {
            if (mode == TM_NODE_ONLY)
                return what;
            // Check if we match the tree, e.g. A! vs 2!
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
    Tree_p Do(Tree_p)
    {
        return NULL;
    }

    Tree_p       test;
};

typedef struct TreeMatchTemplate<TM_RECURSIVE> TreeMatch;



// ============================================================================
//
//    Hash key for tree rewrite
//
// ============================================================================
//  We use this hashing key to quickly determine if two trees "look the same"

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

    Tree_p DoInteger(Integer_p what)
    {
        key = (key << 3) ^ Hash(0, what->value);
        return what;
    }
    Tree_p DoReal(Real_p what)
    {
        key = (key << 3) ^ Hash(1, *((ulong *) &what->value));
        return what;
    }
    Tree_p DoText(Text_p what)
    {
        key = (key << 3) ^ Hash(2, what->value);
        return what;
    }
    Tree_p DoName(Name_p what)
    {
        key = (key << 3) ^ Hash(3, what->value);
        return what;
    }

    Tree_p DoBlock(Block_p what)
    {
        key = (key << 3) ^ Hash(4, what->opening + what->closing);
        return what;
    }
    Tree_p DoInfix(Infix_p what)
    {
        key = (key << 3) ^ Hash(5, what->name);
        return what;
    }
    Tree_p DoPrefix(Prefix_p what)
    {
        ulong old = key;
        key = 0; what->left->Do(this);
        key = (old << 3) ^ Hash(6, key);
        return what;
    }
    Tree_p DoPostfix(Postfix_p what)
    {
        ulong old = key;
        key = 0; what->right->Do(this);
        key = (old << 3) ^ Hash(7, key);
        return what;
    }
    Tree_p Do(Tree_p what)
    {
        key = (key << 3) ^ Hash(1, (ulong) what);
        return what;
    }

    ulong key;
};


extern text     sha1(Tree_p t);
extern Name_p   xl_true;
extern Name_p   xl_false;


typedef long node_id;              // A node identifier


struct NodeIdInfo : Info
// ----------------------------------------------------------------------------
//   Node identifier information
// ----------------------------------------------------------------------------
{
    NodeIdInfo(node_id id): id(id) {}

    typedef node_id data_t;
    operator data_t() { return id; }
    node_id id;
};


struct SimpleAction : Action
// ----------------------------------------------------------------------------
//   Holds a method to be run on any kind of tree node
// ----------------------------------------------------------------------------
{
    SimpleAction() {}
    virtual ~SimpleAction() {}
    Tree_p DoBlock(Block_p what)
    {
        return Do(what);
    }
    Tree_p DoInfix(Infix_p what)
    {
        return Do(what);
    }
    Tree_p DoPrefix(Prefix_p what)
    {
        return Do(what);
    }
    Tree_p DoPostfix(Postfix_p what)
    {
        return Do(what);
    }
    virtual Tree_p  Do(Tree_p what) = 0;
};


struct SetNodeIdAction : SimpleAction
// ------------------------------------------------------------------------
//   Set an integer node ID to each node.
// ------------------------------------------------------------------------
{
    SetNodeIdAction(node_id from_id = 1, node_id step = 1)
        : id(from_id), step(step) {}
    virtual Tree_p Do(Tree_p what)
    {
        what->Set<NodeIdInfo>(id);
        id += step;
        return NULL;
    }
    node_id id;
    node_id step;
};

XL_END

#endif // TREE_H
