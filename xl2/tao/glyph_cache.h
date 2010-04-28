#ifndef GLYPH_CACHE_H
#define GLYPH_CACHE_H
// ****************************************************************************
//  glyph_cache.h                                                   Tao project
// ****************************************************************************
//
//   File Description:
//
//     Cache transforming glyphs into textures
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
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao.h"
#include "layout.h"
#include <QFont>
#include <map>

TAO_BEGIN

struct GlyphCache
// ----------------------------------------------------------------------------
//    Cache turning glyhs into textures
// ----------------------------------------------------------------------------
{
    GlyphCache() : cache() {}
    ~GlyphCache();

protected:
    struct Key
    {
        Key(const QFont &font, uint code): font(font), code(code) {}
        QFont font;
        uint code;

        uint fontSizeOrder(const QFont &font) const
        {
            int ptSize = font.pointSize();
            if (ptSize < 0)
                ptSize = font.pixelSize();
            uint result = ptSize;
            if (ptSize < 0 || ptSize >= 32)
                result = 64;
            else if (ptSize >= 16)
                result |= 3;
            return result;
        }

        template <class T> int order(T x1, T x2) const
        {
            if (x1 < x2)
                return -1;
            if (x2 < x1)
                return 1;
            return 0;                
        }

        int compare(const QFont &f1, const QFont &f2) const
        {
            if (int sizeCmp = order(fontSizeOrder(f1), fontSizeOrder(f2)))
                return sizeCmp;
            if (int weight = order(f1.weight(), f2.weight()))
                return weight;
            if (int style = order(f1.style(), f2.style()))
                return style;
            if (int stretch = order(f1.stretch(), f2.stretch()))
                return stretch;
            if (int family = order(f1.family(), f2.family()))
                return family;
            return 0;
        }

        bool operator==(const Key &o) const
        {
            return code == o.code && compare(font, o.font) == 0;
        }
        bool operator<(const Key &o) const
        {
            return code <  o.code ||
                  (code == o.code && compare(font, o.font) < 0);
        }
    };
    std::map<Key, uint> cache;

public:
    uint texture(const QFont &font, uint code)
    {
        Key k(font, code);
        return cache.count(k) ? cache[k] : 0;
    }

    void enter(const QFont &font, uint code, uint texId)
    {
        Key k(font, code);
        cache[k] = texId;
    }

public:
    static int          maxFontSize;
};

TAO_END

#endif // GLYPH_CACHE_H
