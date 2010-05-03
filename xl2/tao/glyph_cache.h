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
#include "coords.h"
#include "binpack.h"

#include <QFont>
#include <QImage>
#include <map>

TAO_BEGIN

struct GlyphCache;

struct PerFontGlyphCache
// ----------------------------------------------------------------------------
//    Cache storing glyph information for a specific font
// ----------------------------------------------------------------------------
{
    PerFontGlyphCache(const QFont &font);
    ~PerFontGlyphCache();

public:
    struct GlyphEntry
    {
        Box             bounds;
        Box             texture;
        coord           advance;
    };

public:
    bool Find(uint code, GlyphEntry &entry);
    bool Find(text word, GlyphEntry &entry);
    void Insert(uint code, const GlyphEntry &entry);
    void Insert(text word, const GlyphEntry &entry);

protected:
    typedef std::map<uint, GlyphEntry>  CodeMap;
    typedef std::map<text, GlyphEntry>  TextMap;

protected:
    friend class GlyphCache;
    QFont       font;
    CodeMap     codes;
    TextMap     texts;
    qreal       ascent, descent, leading;
    qreal       baseSize;
};


struct GlyphCache
// ----------------------------------------------------------------------------
//   Cache storing glyph information irrespective of font
// ----------------------------------------------------------------------------
{
    GlyphCache();
    ~GlyphCache();

public:
    typedef     PerFontGlyphCache::GlyphEntry   GlyphEntry;
    typedef     PerFontGlyphCache               PerFont;
    typedef     BinPacker::Rect                 Rect;

    void        Clear();

    uint        Width()        { return packer.Width(); }
    uint        Height()       { return packer.Height(); }
    uint        Texture()      { if (dirty) GenerateTexture(); return texture; }

    PerFont *   FindFont(const QFont &font, bool create = false);

    bool        Find(const QFont &font, uint code, GlyphEntry&, bool crt=false);
    bool        Find(const QFont &font, text word, GlyphEntry&, bool ctt=false);
    void        Allocate(uint width, uint height, Rect &rect);

    void        GenerateTexture();
    qreal       Ascent(const QFont &font);
    qreal       Descent(const QFont &font);
    qreal       Leading(const QFont &font);
    void        ScaleDown(GlyphEntry &entry, scale fontScale);

protected:
    // We have a special key that distinguish fonts visually
    // Two fonts with slightly differing size are considered equivalent
    struct Key
    {
        Key(const QFont &font): font(font) {}
        QFont font;

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
            return compare(font, o.font) == 0;
        }
        bool operator<(const Key &o) const
        {
            return compare(font, o.font) < 0;
        }
    };

protected:
    typedef std::map<Key, PerFont *> FontMap;
    FontMap     cache;
    BinPacker   packer;
    uint        texture;
    QImage      image;
    bool        dirty;

public:
    static uint defaultSize;
    scale       minFontSizeForAntialiasing;
    uint        maxFontSize;
    uint        antiAliasMargin;
    scale       fontScaling;
    PerFont *   lastFont;
};

TAO_END

#endif // GLYPH_CACHE_H
