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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "glyph_cache.h"
#include "tao_utf8.h"
#include "path3d.h"
#include "tao_gl.h"
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

PerFontGlyphCache::PerFontGlyphCache(const QFont &font, const uint64 texUnits)
// ----------------------------------------------------------------------------
//   Construct an empty per-font glyph cache
// ----------------------------------------------------------------------------
    : font(font), texUnits(texUnits), ascent(0), descent(0), leading(0)
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
    for (CodeMap::iterator ci = codes.begin(); ci != codes.end(); ci++)
    {
        GlyphEntry &e = (*ci).second;
        if (e.interior)
            glDeleteLists(e.interior, 1);
        if (e.outline)
            glDeleteLists(e.outline, 1);
    }
    for (TextMap::iterator ti = texts.begin(); ti != texts.end(); ti++)
    {
        GlyphEntry &e = (*ti).second;
        if (e.interior)
            glDeleteLists(e.interior, 1);
        if (e.outline)
            glDeleteLists(e.outline, 1);
    }
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

uint GlyphCache::defaultSize = 128;

GlyphCache::GlyphCache()
// ----------------------------------------------------------------------------
//   Default constructor creates an empty glyph cache with a texture
// ----------------------------------------------------------------------------
    : cache(),
      packer(defaultSize, defaultSize),
      texture(0),
      image(defaultSize, defaultSize, QImage::Format_ARGB32),
      dirty(false),
      minFontSizeForAntialiasing(9),
      maxFontSize(64),
      antiAliasMargin(1),
      fontScaling(2.0),
      lastFont(NULL)
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


void GlyphCache::Clear()
// ----------------------------------------------------------------------------
//   Clear the glyph cache entirely
// ----------------------------------------------------------------------------
{
    for (FontMap::iterator it = cache.begin(); it != cache.end(); it++)
        delete (*it).second;
    cache.clear();
    image.fill(0);
    lastFont = NULL;
}


GlyphCache::PerFont *GlyphCache::FindFont(const QFont &font, const uint64 texUnits, bool create)
// ----------------------------------------------------------------------------
//   Find the per-font information in the cache
// ----------------------------------------------------------------------------
{
    PerFontGlyphCache *perFont = lastFont;
    if (!perFont || perFont->font != font || perFont->texUnits != texUnits)
    {
        Key key(font, texUnits);
        FontMap::iterator cacheEntry = cache.find(key);
        if (cacheEntry == cache.end())
        {
            // Create font cache entry if necessary
            if (!create)
                return NULL;
            perFont = new PerFontGlyphCache(font, texUnits);
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


bool GlyphCache::Find(const QFont &font, const uint64 texUnits,
                      uint code, GlyphEntry &entry,
                      bool create, bool interior, scale lineWidth)
// ----------------------------------------------------------------------------
//   Find or create a unicode entry in the cache
// ----------------------------------------------------------------------------
{
    PerFont *perFont = FindFont(font, texUnits, create);
    if (!perFont)
        return false;
    bool found = perFont->Find(code, entry);
    if (!found && !create)
        return false;

    if (!found)
    {
        // Apply a font with scaling
        scale fs = fontScaling;
        uint aam = antiAliasMargin;
        if (font.pointSizeF() <= minFontSizeForAntialiasing)
        {
            fs = 1;
            aam = 0;
        }
        QFont scaled(font);
        scaled.setPointSizeF(font.pointSizeF() * fs);
        if (!aam)
        {
            scaled.setStyleStrategy(QFont::NoAntialias);
            aam = 1;
        }

        // We need to create a new entry
        QFontMetricsF fm(scaled);
        QChar qc(code);
        QRectF bounds = fm.boundingRect(qc);
        uint width = ceil(bounds.width());
        uint height = ceil(bounds.height());

        // Allocate a rectangle where we will put the texture (may resize us)
        BinPacker::Rect rect;
        Allocate(width + 2*aam, height + 2*aam, rect);

        // Record glyph information in the entry
        entry.bounds = Box(bounds.x()/fs, bounds.y()/fs,
                           bounds.width()/fs, bounds.height()/fs);
        entry.texture = Box(Point(rect.x1+aam, rect.y2-aam - bounds.height()),
                            Point(rect.x1+aam + bounds.width(), rect.y2-aam));
        entry.advance = fm.width(qc) / fs;
        entry.lineWidth = 0;
        entry.interior = 0;
        entry.outline = 0;
        entry.scalingFactor = fs;

        // Store the new entry
        perFont->Insert(code, entry);

        // Draw the texture portion for the desired word
        qreal x = rect.x1 + aam - bounds.x(); // + (lb < 0 ? lb : 0);
        qreal y = rect.y2 - aam - bounds.bottom();
        QPainter painter(&image);
        painter.setFont(scaled);
        painter.setBrush(Qt::transparent);
        painter.setPen(Qt::white);
        painter.drawText(QPointF(x, y), QString(qc));
        painter.end();

        // We will need to update the texture
        dirty = true;
    }

    // Line width should remain identical even if we scale the font
    lineWidth /= font.pointSizeF() / perFont->baseSize;

    // Check if we want OpenGL display lists
    if ((interior && !entry.interior) ||
        (lineWidth > 0 && (!entry.outline || lineWidth != entry.lineWidth)))
    {
        // Reset font to original size
        QFont scaled(font);
        scaled.setPointSizeF(perFont->baseSize);

        // Draw glyph into a path
        QPainterPath qtPath;
        GraphicPath path;

        qtPath.addText(0, 0, scaled, QString(QChar(code)));
        path.addQtPath(qtPath, -1);
        if (interior)
        {
            // Create an OpenGL display list for the glyph
            if (!entry.interior)
                entry.interior = glGenLists(1);
            glNewList(entry.interior, GL_COMPILE);
            path.Draw(Vector3(0,0,0), texUnits, GL_POLYGON, GLU_TESS_WINDING_ODD);
            glEndList();
        }

        if (!entry.outline || entry.lineWidth != lineWidth)
        {
            if (!entry.outline)
                entry.outline = glGenLists(1);
            entry.lineWidth = lineWidth;

            // Render outline in a GL list
            QPainterPathStroker stroker;
            stroker.setWidth(lineWidth);
            stroker.setCapStyle(Qt::FlatCap);
            stroker.setJoinStyle(Qt::RoundJoin);
            stroker.setDashPattern(Qt::SolidLine);
            QPainterPath stroke = stroker.createStroke(qtPath);
            GraphicPath strokePath;
            strokePath.addQtPath(stroke, -1);
            glNewList(entry.outline, GL_COMPILE);
            strokePath.Draw(Vector3(0,0,0), texUnits, GL_POLYGON,
                            GLU_TESS_WINDING_POSITIVE);
            glEndList();
        }

        // Store the new or updated entry
        perFont->Insert(code, entry);
    }

    // Scale down what we pass back to the caller
    ScaleDown(entry, font.pointSizeF() / perFont->baseSize);
    return true;
}


bool GlyphCache::Find(const QFont &font, const uint64 texUnits,
                      text code, GlyphEntry &entry,
                      bool create, bool interior, scale lineWidth)
// ----------------------------------------------------------------------------
//   Find or create a word entry in the cache
// ----------------------------------------------------------------------------
{
    PerFont *perFont = FindFont(font, texUnits, create);
    if (!perFont)
        return false;
    bool found = perFont->Find(code, entry);
    if (!found && !create)
        return false;

    if (!found)
    {
        // Apply a font with scaling
        scale fs = fontScaling;
        uint aam = antiAliasMargin;
        if (font.pointSizeF() <= minFontSizeForAntialiasing)
        {
            fs = 1;
            aam = 0;
        }
        QFont scaled(font);
        scaled.setPointSizeF(font.pointSizeF() * fs);
        if (!aam)
        {
            scaled.setStyleStrategy(QFont::NoAntialias);
            aam = 1;
        }

        // We need to create a new entry
        QFontMetricsF fm(scaled);
        QString qs(+code);
        QRectF bounds = fm.boundingRect(qs);
        uint width = ceil(bounds.width());
        uint height = ceil(bounds.height());

        // Allocate a rectangle where we will put the texture (may resize us)
        BinPacker::Rect rect;
        Allocate(width + 2*aam, height + 2*aam, rect);

        // Record glyph information in the entry
        entry.bounds = Box(bounds.x()/fs, bounds.y()/fs,
                           bounds.width()/fs, bounds.height()/fs);
        entry.texture = Box(Point(rect.x1+aam, rect.y2-aam - bounds.height()),
                            Point(rect.x1+aam + bounds.width(), rect.y2-aam));
        entry.advance = fm.width(qs) / fs;
        entry.lineWidth = 0;
        entry.interior = 0;
        entry.outline = 0;
        entry.scalingFactor = fs;

        // Store the new entry
        perFont->Insert(code, entry);

        // Draw the texture portion for the desired word
        qreal x = rect.x1 + aam - bounds.x(); // + (lb < 0 ? lb : 0);
        qreal y = rect.y2 - aam - bounds.bottom();
        QPainter painter(&image);
        painter.setFont(scaled);
        painter.setBrush(Qt::transparent);
        painter.setPen(Qt::white);
        painter.drawText(QPointF(x, y), qs);
        painter.end();

        // We will need to update the texture
        dirty = true;
    }

    // Line width should remain identical even if we scale the font
    lineWidth /= font.pointSizeF() / perFont->baseSize;

    // Check if we want OpenGL display lists
    if ((interior && !entry.interior) ||
        (lineWidth > 0 && (!entry.outline || lineWidth != entry.lineWidth)))
    {
        // Reset font to original size
        QFont scaled(font);
        scaled.setPointSizeF(perFont->baseSize);

        // Draw glyph into a path
        QPainterPath qtPath;
        GraphicPath path;

        qtPath.addText(0, 0, scaled, QString(+code));
        path.addQtPath(qtPath, -1);

        if (interior)
        {
            // Create an OpenGL display list for the glyph
            if (!entry.interior)
                entry.interior = glGenLists(1);
            glNewList(entry.interior, GL_COMPILE);
            path.Draw(Vector3(0,0,0), texUnits, GL_POLYGON, GLU_TESS_WINDING_ODD);
            glEndList();
        }

        if (!entry.outline || entry.lineWidth != lineWidth)
        {
            if (!entry.outline)
                entry.outline = glGenLists(1);
            entry.lineWidth = lineWidth;

            // Render outline in a GL list
            QPainterPathStroker stroker;
            stroker.setWidth(lineWidth);
            stroker.setCapStyle(Qt::FlatCap);
            stroker.setJoinStyle(Qt::RoundJoin);
            stroker.setDashPattern(Qt::SolidLine);
            QPainterPath stroke = stroker.createStroke(qtPath);
            GraphicPath strokePath;
            strokePath.addQtPath(stroke, -1);
            glNewList(entry.outline, GL_COMPILE);
            strokePath.Draw(Vector3(0,0,0), texUnits, GL_POLYGON,
                            GLU_TESS_WINDING_POSITIVE);
            glEndList();
        }

        // Store the new or updated entry
        perFont->Insert(code, entry);
    }

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
        dirty = true;

        // Resize the rectangle from which we do the texture allocation
        packer.Resize(w, h);
    }
}


void GlyphCache::GenerateTexture()
// ----------------------------------------------------------------------------
//   Copy the current image into our GL texture
// ----------------------------------------------------------------------------
{
    QImage texImg = QGLWidget::convertToGLFormat(image).mirrored(false, true);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texture);
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA,
                 texImg.width(), texImg.height(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, texImg.bits());

    dirty = false;
}


qreal GlyphCache::Ascent(const QFont &font, const uint64 texUnits)
// ----------------------------------------------------------------------------
//   Return the ascent for the font
// ----------------------------------------------------------------------------
{
    return FindFont(font, texUnits, true)->ascent;
}


qreal GlyphCache::Descent(const QFont &font, const uint64 texUnits)
// ----------------------------------------------------------------------------
//   Return the descent for the font
// ----------------------------------------------------------------------------
{
    return FindFont(font, texUnits, true)->descent;
}


qreal GlyphCache::Leading(const QFont &font, const uint64 texUnits)
// ----------------------------------------------------------------------------
//   Return the leading for the font
// ----------------------------------------------------------------------------
{
    return FindFont(font, texUnits, true)->leading;
}


void GlyphCache::ScaleDown(GlyphEntry &entry, scale fontScale)
// ----------------------------------------------------------------------------
//   Adjust the scale
// ----------------------------------------------------------------------------
{
    // Scale the geometry
    entry.bounds.lower.x *= fontScale;
    entry.bounds.upper.x *= fontScale;
    entry.bounds.lower.y *= fontScale;
    entry.bounds.upper.x *= fontScale;
    entry.advance *= fontScale;
    entry.scalingFactor = fontScale;
}

TAO_END
