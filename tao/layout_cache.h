#ifndef LAYOUT_CACHE_H
#define LAYOUT_CACHE_H
// ****************************************************************************
//  layout_cache.h                                                  Tao project
// ****************************************************************************
//
//   File Description:
//
//     A container for Layout objects.
//     This cache is used by the layout refresh code. When the Tao source code
//     of a layout has to be reevaluated due to an event, caching may avoid
//     re-creating child layouts that are not modified by that event.
//     Ownership of layouts added into the cache is transferred to the cache,
//     and thus objects may deleted at any time.
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

#include "tao.h"
#include "tao_tree.h"
#include "hash.h"
#include <QCache>
#include <QByteArray>

TAO_BEGIN

struct Layout;

typedef QCache<QByteArray, Layout>  LayoutCacheBase;


struct LayoutCache : public LayoutCacheBase
// ----------------------------------------------------------------------------
//   Cache Layout pointers, indexed by hash of their XL tree and a context
// ----------------------------------------------------------------------------
{
                        LayoutCache() {}
                       ~LayoutCache() {}

public:
    Layout *            take(Tree *t, Context *context);
    bool                insert(Layout *layout);

public:
    static QByteArray   contextHash(Context *context);

protected:
    std::ostream &      debug();
 };

TAO_END

#endif // LAYOUT_CACHE_H
