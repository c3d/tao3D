// ****************************************************************************
//  glyph_cache.cpp                                                 Tao project
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
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "glyph_cache.h"
#include "tao_utf8.h"
#include <QtOpenGL>
#include <QFontMetricsF>

TAO_BEGIN

// ============================================================================
//
//   Per-font glyph cache
//
// ============================================================================
//   We have single glyph and word-based variants, because we allow
//   the cache to contain complete rendered words for the complicated
//   cases where we wouldn't know how to layout individual glyphs side by side
//   without the help of Qt. This is known as the "Dosa problem",
//   see http://labs.trolltech.com/blogs/2007/03/30/working-towards-a-unified-
//   text-layout-engine-for-the-free-desktop-software-stack/
//
//   The entries we put and retrieve from the per-font caches are
//   in texel coordinates, so that they don't change when we extend the
//   texture because we run out of space.
//   The entries we return from Find, on the other hand, are OpenGL texture
//   coordinates, i.e. we normalize them in the 0..1 range.

PerFontGlyphCache::PerFontGlyphCache(const QFont &font)
// ----------------------------------------------------------------------------
//   Construct an empty per-font glyph cache
// ----------------------------------------------------------------------------
    : font(font), ascent(0), descent(0), leading(0)
{
    QFontMetricsF fm(font);
    ascent = fm.ascent();
    descent = fm.descent();
    leading = fm.leading();
    baseSize = font.pointSizeF();
}


PerFontGlyphCache::~PerFontGlyphCache()
// ----------------------------------------------------------------------------
//   Clear the per-font glyph cache
// ----------------------------------------------------------------------------
{
    if (GlyphCache::lastFont == this)
        GlyphCache::lastFont = NULL;
}


bool PerFontGlyphCache::Find(uint code, GlyphEntry &entry)
// ----------------------------------------------------------------------------
//   For a given Unicode, find per-font glyph cache entry if it exists
// ----------------------------------------------------------------------------
{
    CodeMap::iterator it = codes.find(code);
    if (it != codes.end())
    {
        entry = (*it).second;
        return true;
    }
    return false;
}


bool PerFontGlyphCache::Find(text word, GlyphEntry &entry)
// ----------------------------------------------------------------------------
//   For a complete UTF-8 word, find per-font glyph cache entry if it exists
// ----------------------------------------------------------------------------
{
    TextMap::iterator it = texts.find(word);
    if (it != texts.end())
    {
        entry = (*it).second;
        return true;
    }
    return false;
}


void PerFontGlyphCache::Insert(uint code, const GlyphEntry &entry)
// ----------------------------------------------------------------------------
//   Insert glyph entry for a single Unicode glyph
// ----------------------------------------------------------------------------
{
    codes[code] = entry;
}


void PerFontGlyphCache::Insert(text word, const GlyphEntry &entry)
// ----------------------------------------------------------------------------
//   Insert glyph entry for a combination of Unicode glyphs
// ----------------------------------------------------------------------------
{
    texts[word] = entry;
}



// ============================================================================
//
//   Unified glyph and word cache
//
// ============================================================================

int GlyphCache::maxFontSize = 64;
uint GlyphCache::defaultSize = 128;
uint GlyphCache::antiAliasMargin = 2;
scale GlyphCache::fontScaling = 2.0;
PerFontGlyphCache *GlyphCache::lastFont = NULL;

GlyphCache::GlyphCache()
// ----------------------------------------------------------------------------
//   Default constructor creates an empty glyph cache with a texture
// ----------------------------------------------------------------------------
    : cache(),
      packer(defaultSize, defaultSize),
      texture(0),
      image(defaultSize, defaultSize, QImage::Format_ARGB32)
{
    glGenTextures(1, &texture);
    image.fill(0);
}


GlyphCache::~GlyphCache()
// ----------------------------------------------------------------------------
//   Release the texture we were using
// ----------------------------------------------------------------------------
{
    glDeleteTextures(1, &texture);
    for (FontMap::iterator it = cache.begin(); it != cache.end(); it++)
        delete (*it).second;
}


GlyphCache::PerFont *GlyphCache::FindFont(const QFont &font, bool create)
// ----------------------------------------------------------------------------
//   Find the per-font information in the cache
// ----------------------------------------------------------------------------
{
    PerFontGlyphCache *perFont = lastFont;
    if (!perFont || perFont->font != font)
    {
        Key key(font);
        FontMap::iterator cacheEntry = cache.find(key);
        if (cacheEntry == cache.end())
        {
            // Create font cache entry if necessary
            if (!create)
                return NULL;
            perFont = new PerFontGlyphCache(font);
            cache[key] = perFont;
        }
        else
        {
            // Get the cache entry we want
            perFont = (*cacheEntry).second;
        }
        lastFont = perFont;
    }
    return perFont;
}


bool GlyphCache::Find(const QFont &font,
                      uint code, GlyphEntry &entry,
                      bool create)
// ----------------------------------------------------------------------------
//   Find or create a unicode entry in the cache
// ----------------------------------------------------------------------------
{
    PerFont *perFont = FindFont(font, create);
    if (!perFont)
        return false;
    if (perFont->Find(code, entry))
    {
        ScaleDown(entry, font.pointSizeF() / perFont->baseSize);
        return true;
    }
    if (!create)
        return false;

    // Apply a font with scaling
    QFont scaled(font);
    scaled.setPointSizeF(font.pointSizeF() * fontScaling);
    scaled.setStyleStrategy(QFont::NoAntialias);

    // We need to create a new entry
    QFontMetricsF fm(scaled);
    QChar qc(code);
    QRectF bounds = fm.boundingRect(qc);
    uint width = ceil(bounds.width());
    uint height = ceil(bounds.height());

    // Allocate a rectangle where we will put the texture (may resize us)
    BinPacker::Rect rect;
    uint aam = antiAliasMargin;
    Allocate(width + 2*aam, height + 2*aam, rect);

    // Record glyph information in the entry
    entry.bounds = Box(bounds.x()/fontScaling, bounds.y()/fontScaling,
                       bounds.width()/fontScaling, bounds.height()/fontScaling);
    entry.texture = Box(Point(rect.x1+aam, rect.y2-aam - bounds.height()),
                        Point(rect.x1+aam + bounds.width(), rect.y2-aam));
    entry.advance = fm.width(qc) / fontScaling;

    // Store the new entry
    perFont->Insert(code, entry);

    // Draw the texture portion for the desired word
    qreal x = rect.x1 + aam - bounds.x(); // + (lb < 0 ? lb : 0);
    qreal y = rect.y2 - aam - bounds.bottom();
    QPainter painter(&image);
    painter.setFont(scaled);
    painter.setBrush(Qt::transparent);
    if (0)
    {
        static Qt::GlobalColor debugColors[4] =
            { Qt::darkGreen, Qt::darkCyan, Qt::darkRed, Qt::darkYellow };
        painter.fillRect(rect.x1, rect.y1, rect.x2 - rect.x1, rect.y2 - rect.y1,
                         debugColors[code & 3]);
        painter.fillRect(rect.x1+aam, rect.y1+aam,
                         rect.x2-rect.x1-2*aam, rect.y2-rect.y1-2*aam,
                         debugColors[(code+1) & 3]);
    }
    painter.setPen(Qt::white);
    painter.drawText(QPointF(x, y), QString(qc));
    painter.end();

    // Update the texture
    GenerateTexture();

    // Scale down what we pass back to the caller
    ScaleDown(entry, font.pointSizeF() / perFont->baseSize);
    return true;
}


bool GlyphCache::Find(const QFont &font,
                      text word, GlyphEntry &entry,
                      bool create)
// ----------------------------------------------------------------------------
//   Find or create a word entry in the cache
// ----------------------------------------------------------------------------
{
    PerFont *perFont = FindFont(font, create);
    if (!perFont)
        return false;
    if (perFont->Find(word, entry))
    {
        ScaleDown(entry, font.pointSizeF() / perFont->baseSize);
        return true;
    }
    if (!create)
        return false;

    // Apply a font with scaling
    QFont scaled(font);
    scaled.setPointSizeF(font.pointSizeF() * fontScaling);
    scaled.setStyleStrategy(QFont::NoAntialias);

    // We need to create a new entry
    QFontMetricsF fm(scaled);
    QString qs(+word);
    QRectF bounds = fm.boundingRect(qs);
    uint width = bounds.width();
    uint height = bounds.height();

    // Allocate a rectangle where we will put the texture (may resize us)
    BinPacker::Rect rect;
    uint aam = antiAliasMargin;
    Allocate(width + 2*aam, height + 2*aam, rect);

    // Record glyph information in the entry
    entry.bounds = Box(bounds.x()/fontScaling, bounds.y()/fontScaling,
                       bounds.width()/fontScaling, bounds.height()/fontScaling);
    entry.texture = Box(Point(rect.x1+aam, rect.y1+aam),
                        Point(rect.x2-aam, rect.y2-aam));
    entry.advance = fm.width(qs) / fontScaling;

    // Store the new entry
    perFont->Insert(word, entry);

    // Draw the texture portion for the desired word
    scale leftBearing = qs.length() ? fm.leftBearing(qs[0]) : 0;
    qreal x = rect.x1 - bounds.x() + aam - leftBearing;
    qreal y = rect.y1 - bounds.y() + aam;
    QPainter painter(&image);
    painter.setFont(scaled);
    painter.setBrush(Qt::transparent);
    painter.setPen(Qt::white);
    painter.drawText(QPointF(x, y), qs);
    painter.end();

    // Update the texture
    GenerateTexture();

    // Scale down what we pass back to the caller
    ScaleDown(entry, font.pointSizeF() / perFont->baseSize);
    return true;
}


void GlyphCache::Allocate(uint width, uint height, Rect &rect)
// ----------------------------------------------------------------------------
//   Allocate  a rectangle, resizing it as necessary
// ----------------------------------------------------------------------------
{
    while (!packer.Allocate(width, height, rect))
    {
        uint w = packer.Width();
        uint h = packer.Height();
        do { w <<= 1; } while (w < width);
        do { h <<= 1; } while (h < height);

        // Resize the image and copy in the texture
        image = image.copy(0,0,w,h);
        GenerateTexture();

        // Resize the rectangle from which we do the texture allocation
        packer.Resize(w, h);
    }
}


void GlyphCache::GenerateTexture()
// ----------------------------------------------------------------------------
//   Copy the current image into our GL texture
// ----------------------------------------------------------------------------
{
    image.save("/tmp/glop.png");

    QImage texImg = QGLWidget::convertToGLFormat(image);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 texImg.width(), texImg.height(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, texImg.bits());
}


qreal GlyphCache::Ascent(const QFont &font)
// ----------------------------------------------------------------------------
//   Return the ascent for the font
// ----------------------------------------------------------------------------
{
    return FindFont(font, true)->ascent;
}


qreal GlyphCache::Descent(const QFont &font)
// ----------------------------------------------------------------------------
//   Return the descent for the font
// ----------------------------------------------------------------------------
{
    return FindFont(font, true)->descent;
}


qreal GlyphCache::Leading(const QFont &font)
// ----------------------------------------------------------------------------
//   Return the leading for the font
// ----------------------------------------------------------------------------
{
    return FindFont(font, true)->leading;
}


void GlyphCache::ScaleDown(GlyphEntry &entry, scale fontScale)
// ----------------------------------------------------------------------------
//   Adjust the scale from texel coordinates to GL coordinates
// ----------------------------------------------------------------------------
{
    // Scale the geometry
    entry.bounds *= fontScale;

    // Adjust texture coordinates
    uint width = packer.Width();
    uint height = packer.Height();
    scale xscale = 1.0 / width;
    scale yscale = 1.0 / height;

    Point &l = entry.texture.lower;
    Point &u = entry.texture.upper;

    extern double debugX, debugY, debugW, debugH;
    l.x = xscale * (l.x + debugX);
    u.x = xscale * (u.x + debugW);
    l.y = yscale * (height - (l.y + debugY));
    u.y = yscale * (height - (u.y + debugH));
}

TAO_END
