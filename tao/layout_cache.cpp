// ****************************************************************************
//  layout_cache.cpp                                               Tao project
// ****************************************************************************
//
//   File Description:
//
//     Layout cache implementation.
//
//
//
//
//
//
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "layout_cache.h"
#include "layout.h"
#include "sha1_ostream.h"
#include "runtime.h"
#include <sstream>
#include <QCryptographicHash>

TAO_BEGIN

typedef XL::Sha1                    tree_hash;



// ============================================================================
//
//   Helper functions
//
// ============================================================================

static text layoutId(Layout *layout)
// ----------------------------------------------------------------------------
//   Helper function to display a layout pointer
// ----------------------------------------------------------------------------
{
    return (layout ? layout->PrettyId() : "NULL");
}


static tree_hash treeHash(Tree *what)
// ----------------------------------------------------------------------------
//   Return the hash value of a XL tree
// ----------------------------------------------------------------------------
{
    if (!what->Exists< XL::HashInfo<> >())
    {
        XL::TreeHashAction<> h_action(XL::TreeHashAction<>::Force);
        what->Do(h_action);
    }
    XL::Sha1 hash = what->Get< XL::HashInfo<> >();
    return hash;
}


static QByteArray toQByteArray(tree_hash hash)
// ----------------------------------------------------------------------------
//   Convert a tree_hash into a QByteArray
// ----------------------------------------------------------------------------
{
    return QByteArray((char *)hash.hash, hash.SIZE);
}


static QByteArray dumpRewrite(XL::Rewrite *r)
// ----------------------------------------------------------------------------
//   Dump a rewrite
// ----------------------------------------------------------------------------
{
    QByteArray ret;
    if (r)
    {
        std::stringstream sstr;
        if (r->native == XL::xl_assigned_value)
            sstr << r->from << " := " << r->to << "\n";
        else
            sstr << r->from << " -> " << r->to << "\n";
        ret.append(sstr.str().c_str(), sstr.str().length());
        XL::rewrite_table::iterator i;
        for (i = r->hash.begin(); i != r->hash.end(); i++)
            ret.append(dumpRewrite((*i).second));
    }
    return ret;
}


static QByteArray dumpRewrites(Context *c)
// ----------------------------------------------------------------------------
//   Dump all the rewrites in a context
// ----------------------------------------------------------------------------
{
    using namespace XL;

    uint debugsm = 10;
    QByteArray ret;
    XL::rewrite_table::iterator i;
    uint n = 0;
    for (i = c->rewrites.begin(); i != c->rewrites.end() && n++ < debugsm; i++)
        ret.append(dumpRewrite((*i).second));
    return ret;
}


static QByteArray dumpContext(Context *c)
// ----------------------------------------------------------------------------
//   Dump a context using NORMAL_LOOKUP rule
// ----------------------------------------------------------------------------
{
    using namespace XL;

    QByteArray ret;
    context_list list;
    c->Contexts(Context::NORMAL_LOOKUP, list);
    for (context_list::iterator i = list.begin(); i != list.end(); i++)
        ret.append(dumpRewrites(*i));
    return ret;
}


static QByteArray treeContextHash(Tree *what, QByteArray context_hash)
// ----------------------------------------------------------------------------
//   Compute hash value (signature) for a pair (XL source tree, context hash)
// ----------------------------------------------------------------------------
{
    tree_hash h = treeHash(what);
    QByteArray hash = toQByteArray(h);
    hash.append(context_hash);
    return hash;
}


static QByteArray treeContextHash(Tree *what, Context *context)
// ----------------------------------------------------------------------------
//   Compute hash value (signature) for a pair (XL source tree, eval context)
// ----------------------------------------------------------------------------
{
    QByteArray c_hash = LayoutCache::contextHash(context);
    return treeContextHash(what, c_hash);
}


static std::ostream& operator <<(std::ostream &out, const QByteArray &ba)
// ----------------------------------------------------------------------------
//   Dump a byte array (hex encoded)
// ----------------------------------------------------------------------------
{
    QString str(ba.toHex());
    out << str.toStdString();
    return out;
}



// ============================================================================
//
//   Layout cache
//
// ============================================================================

Layout * LayoutCache::take(Tree *what, Context *context)
// ----------------------------------------------------------------------------
//   Look up a layout from the cache, remove it if found
// ----------------------------------------------------------------------------
{
    if (isEmpty())
    {
        IFTRACE(layoutcache)
            debug() << "Take: cache is empty\n";
        return NULL;
    }
    QByteArray h = treeContextHash(what, context);
    Layout *layout = LayoutCacheBase::take(h);
    IFTRACE(layoutcache)
        debug() << "Take " << h << ": " << layoutId(layout) << "\n";
    return layout;
}


bool LayoutCache::insert(Layout *layout)
// ----------------------------------------------------------------------------
//   Insert a layout into the cache
// ----------------------------------------------------------------------------
{
    if (!layout || !layout->body || !layout->ctx)
        return false;
    // NB: do not hash layout->ctx now because it may have changed since the
    // layout was created - use the hash value computed at that time
    QByteArray h = treeContextHash(layout->body, layout->ctxHash);
    IFTRACE(layoutcache)
        debug() << "Insert " << layoutId(layout) << " @ " << h << "\n";
    return LayoutCacheBase::insert(h, layout);
}


QByteArray LayoutCache::contextHash(Context *context)
// ----------------------------------------------------------------------------
//   Hash a XL context
// ----------------------------------------------------------------------------
{
    QByteArray hash;
    QByteArray dump = dumpContext(context);
    hash = QCryptographicHash::hash(dump, QCryptographicHash::Sha1);
    return hash;
}


std::ostream & LayoutCache::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[Layout Cache] ";
    return std::cerr;
}

TAO_END
