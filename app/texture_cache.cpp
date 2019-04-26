// *****************************************************************************
// texture_cache.cpp                                               Tao3D project
// *****************************************************************************
//
// File description:
//
//     Loading a texture from an image file. Image data are cached in RAM and
//     by OpenGL.
//
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2012-2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2012,2014-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2012-2013, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

#include "texture_cache.h"
#include "base.h"
#include "tao_utf8.h"
#include "opengl_state.h"
#include "preferences_pages.h"
#include "license.h"
#include "application.h"
#include "widget.h"
#include "gl_keepers.h"
#include <QtEndian>

RECORDER(textures,              16, "Textures and texture cache");
RECORDER(textures_warning,      16, "Warnings from textures and texture cache");

namespace Tao {

// ============================================================================
//
//    Helpers
//
// ============================================================================

QWeakPointer<TextureCache> TextureCache::textureCache;

#define BOOL_SETTER(fn, attr)                                   \
XL::Name_p TextureCache::fn(bool enable)                        \
/* --------------------------------------------------------- */ \
/*   Primitives to enable or disable boolean settings        */ \
/* --------------------------------------------------------- */ \
{                                                               \
    QSharedPointer<TextureCache> tc = TextureCache::instance(); \
    bool &attr = tc->attr, prev = attr;                         \
                                                                \
    if (attr != enable)                                         \
    {                                                           \
        attr = enable;                                          \
        record(textures, #attr " %+s -> %+s",                   \
               prev ? "on" : "off",                             \
               attr ? "on" : "off");                            \
    }                                                           \
                                                                \
    return prev ? XL::xl_true : XL::xl_false;                   \
}

BOOL_SETTER(textureMipmap, mipmap)
BOOL_SETTER(textureCompress, compress)
BOOL_SETTER(textureSaveCompressed, saveCompressed)


#define SIZE_SETTER(fn, attr)                                   \
XL::Integer_p TextureCache::fn(quint64 val)                     \
/* --------------------------------------------------------- */ \
/*   Primitives to change numerical values in cache          */ \
/* --------------------------------------------------------- */ \
{                                                               \
    QSharedPointer<TextureCache> tc = TextureCache::instance(); \
    quint64 &attr = tc->attr, prev = attr;                      \
                                                                \
    if (attr != val)                                            \
    {                                                           \
        attr = val;                                             \
        record(textures, #attr " %k -> %k", prev, attr);        \
    }                                                           \
                                                                \
    return new XL::Integer(prev);                               \
}

SIZE_SETTER(textureCacheMemSize, maxMemSize)
SIZE_SETTER(textureCacheGLSize, maxGLSize)


XL::Name_p TextureCache::textureCacheRefresh()
// ----------------------------------------------------------------------------
//   Primitive to refresh stale images downloaded over the newtwork
// ----------------------------------------------------------------------------
{
    TextureCache::instance()->refresh();
    return XL::xl_true;
}



// ============================================================================
//
//    TextureCache class
//
// ============================================================================

TextureCache::TextureCache()
// ----------------------------------------------------------------------------
//   Initialize cache
// ----------------------------------------------------------------------------
    : memSize(0), GLSize(0),
      maxMemSize(PerformancesPage::textureCacheMaxMem()),
      maxGLSize(PerformancesPage::textureCacheMaxGLMem()),
      purgeRatio(0.8), mipmap(PerformancesPage::texture2DMipmap()),
      compress(PerformancesPage::texture2DCompress()),
      minFilt(PerformancesPage::texture2DMinFilter()),
      magFilt(PerformancesPage::texture2DMagFilter()),
      network(NULL), texChangedEvent(QEvent::registerEventType()),
      fileMonitor("tex"), saveCompressed(false)

{
    statTimer.setSingleShot(true);
    connect(&statTimer, SIGNAL(timeout()), this, SLOT(doPrintStatistics()));
    record(textures, "Texture cache %p created ev %d", this, texChangedEvent);
    record(layout, "Texture changed event is %d for cahce %p",
           texChangedEvent, this);

    QGLWidget gl;
    gl.makeCurrent();

    GLint *fmt, n = 0;
    if (QGLContext::currentContext()->isValid())
    {
        GL.Get(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &n);
        fmt = (GLint*)malloc(n * sizeof(GLint));
        GL.Get(GL_COMPRESSED_TEXTURE_FORMATS, fmt);
        for (int i = 0; i < n; ++i)
            cmpFormats.insert(fmt[i]);
        free(fmt);
    }
    record(textures, "GL impementation supports compressed texture formats");
}


QSharedPointer<TextureCache> TextureCache::instance()
// ----------------------------------------------------------------------------
//    Return a pointer to the instance of the texture cache.
// ----------------------------------------------------------------------------
{
    QSharedPointer<TextureCache> ptr;
    if (!textureCache)
    {
        ptr = QSharedPointer<TextureCache>(new TextureCache);
        textureCache = ptr.toWeakRef();
    }
    else
    {
        ptr = textureCache.toStrongRef();
        XL_ASSERT(ptr);
    }
    return ptr;
}


void TextureCache::doPrintStatistics()
// ----------------------------------------------------------------------------
//   Print cache statistics
// ----------------------------------------------------------------------------
{
    record(textures, "Used %k / %k, GL %k/%k",
           memSize, maxMemSize, GLSize, maxGLSize);

#ifndef QT_NO_DEBUG
    quint64 checkMemSize = 0, checkGLSize = 0;
    QList<GLuint> ids = fromId.keys();
    foreach (GLuint id, ids)
    {
        CachedTexture * tex = fromId[id];
        checkMemSize += tex->image.byteCount();
        checkGLSize += tex->GLsize;
    }
    XL_ASSERT(checkMemSize == memSize);
    XL_ASSERT(checkGLSize == GLSize);
#endif
}


void TextureCache::printStatistics()
// ----------------------------------------------------------------------------
//   Print cache statistics not more often that once per second
// ----------------------------------------------------------------------------
{
    IFTRACE(textures)
    {
        if (statTimer.isActive())
            return;
        statTimer.start(1000);
    }
}


CachedTexture * TextureCache::load(const QString &img, const QString &docPath)
// ----------------------------------------------------------------------------
//   Load texture file. docPath is used if img is relative.
// ----------------------------------------------------------------------------
{
    record(textures, "Loading %s", img);
    QString name(img);
    if (!name.contains("://") && QDir::isRelativePath(name) &&
        QRegExp("^[a-z]+:").indexIn(name) == -1)
    {
        name = docPath + "/" + img;
        if (!QFileInfo(name).exists())
        {
             // Backward compatibility
            QString qualified = "texture:" + img;
            QFileInfo info(qualified);
            if (info.exists())
                name = info.absoluteFilePath();
        }
    }
    // name is either a URL, full path or a prefixed path ("image:file.jpg").
    // It cannot be a relative path.
    CachedTexture * cached = fromName.value(name);
    if (!cached)
    {
        cached = new CachedTexture(*this, name, mipmap, compress);
        GLuint id = cached->id;
        fromId[id] = fromName[name] = cached;
        if (memSize > maxMemSize)
            purgeMem();
        if (cached->load())
        {
            insert(cached, memLRU);
            if (GLSize < maxGLSize)
            {
                cached->transfer();
                insert(cached, GL_LRU);
            }
            record(textures, "After load %k / %k, GL %k/%k",
                   memSize, maxMemSize, GLSize, maxGLSize);
            printStatistics();
        }
    }

    return cached;
}


CachedTexture * TextureCache::load(text img)
// ----------------------------------------------------------------------------
//    Load texture in the doc path
// ----------------------------------------------------------------------------
{
    TAO(refreshOn(TextureCache::instance()->textureChangedEvent()));
    return load(+img, TAO(documentPath()));
}


CachedTexture * TextureCache::bind(GLuint id)
// ----------------------------------------------------------------------------
//   Bind GL texture if it exists and return object
// ----------------------------------------------------------------------------
{
    CachedTexture * cached = fromId.value(id);
    if (!cached)
        return NULL;

    if (!cached->transferred())
    {
        if (!cached->loaded())
        {
            if (memSize > maxMemSize)
                purgeMem();
            if (!cached->load())
            {
                // Texture download is still in progress
                return cached;
            }
            insert(cached, memLRU);
        }
        else
        {
            relink(cached, memLRU);
        }

        if (GLSize > maxGLSize)
            purgeGLMem();

        cached->transfer();
        insert(cached, GL_LRU);

        printStatistics();

        cached->bind();
        return cached;
    }

    relink(cached, GL_LRU);
    cached->bind();
    return cached;
}


void TextureCache::reload(CachedTexture *tex)
// ----------------------------------------------------------------------------
//   Reload from file or network
// ----------------------------------------------------------------------------
{
    record(textures, "Reload %p: %k / %k, GL %k/%k",
           tex, memSize, maxMemSize, GLSize, maxGLSize);
    tex->purge();
    if (memSize > maxMemSize)
        purgeMem();
    if (tex->load())
    {
        insert(tex, memLRU);
        printStatistics();

        if (GLSize > maxGLSize)
            purgeGLMem();

        tex->transfer();
        insert(tex, GL_LRU);
    }
}


void TextureCache::setMinMagFilters(GLuint id)
// ----------------------------------------------------------------------------
//   Set GL texture filters to the values currently configured in cache
// ----------------------------------------------------------------------------
{
    setMinFilter(id, minFilter());
    GL.TexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter());
}


void TextureCache::setMinFilter(GLuint id, GLenum filter)
// ----------------------------------------------------------------------------
//   Set GL texture minifying filter for texture 'id'
// ----------------------------------------------------------------------------
{
    CachedTexture * cached = fromId[id];
    XL_ASSERT(cached);

    if ( !cached->mipmap &&
         (filter == GL_NEAREST_MIPMAP_NEAREST ||
          filter == GL_LINEAR_MIPMAP_NEAREST  ||
          filter == GL_NEAREST_MIPMAP_LINEAR  ||
          filter == GL_LINEAR_MIPMAP_LINEAR))
    {
        // Fallback to GL_LINEAR when a mipmap filter is requested but texture
        // was not loaded with mipmapping enabled
        filter = GL_LINEAR;
    }
    GL.TexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
}


void TextureCache::purgeMem()
// ----------------------------------------------------------------------------
//   Drop the least recently used textures from main memory
// ----------------------------------------------------------------------------
{
    record(textures,
           "Purge memory in %p: %k / %k",
           this, memSize, maxMemSize);
    while (memSize > (maxMemSize * purgeRatio))
    {
        printStatistics();
        XL_ASSERT(memLRU.last);

        CachedTexture * tex = memLRU.last->tex;
        unlink(tex, memLRU);
        tex->unload();
    }
}


void TextureCache::purgeGLMem()
// ----------------------------------------------------------------------------
//   Drop the least recently used items from texture memory
// ----------------------------------------------------------------------------
{
    record(textures,
           "Purge GL memory in %p: %k / %k",
           this, GLSize, maxGLSize);
    while (GLSize > (maxGLSize * purgeRatio))
    {
        printStatistics();
        XL_ASSERT(GL_LRU.last);

        CachedTexture * tex = GL_LRU.last->tex;
        unlink(tex, GL_LRU);
        tex->purgeGL();
    }
}


void TextureCache::clear()
// ----------------------------------------------------------------------------
//   Empty cache
// ----------------------------------------------------------------------------
{
    record(textures, "Clear %p", this);
    QList<GLuint> ids = fromId.keys();
    foreach (GLuint id, ids)
    {
        CachedTexture * tex = fromId.take(id);
        XL_ASSERT(tex);
        fromName.remove(tex->path);
        unlink(tex, memLRU);
        unlink(tex, GL_LRU);
        delete tex;
    }
    XL_ASSERT(fromId.isEmpty());
    XL_ASSERT(fromName.isEmpty());
    XL_ASSERT(memSize == 0);
    XL_ASSERT(GLSize == 0);
    XL_ASSERT(memLRU.first == NULL);
    XL_ASSERT(memLRU.last == NULL);
    XL_ASSERT(GL_LRU.first == NULL);
    XL_ASSERT(GL_LRU.last == NULL);
}


void TextureCache::purge()
// ----------------------------------------------------------------------------
//   Purge all textures in cache (free memory but keep texture id valid)
// ----------------------------------------------------------------------------
{
    record(textures, "Purge %p", this);
    QList<GLuint> ids = fromId.keys();
    foreach (GLuint id, ids)
        fromId[id]->purge();
}


void TextureCache::refresh()
// ----------------------------------------------------------------------------
//   Purge stale images
// ----------------------------------------------------------------------------
{
    // REVISIT: the current implementation purges all network images
    foreach (GLuint id, fromId.keys())
    {
        CachedTexture * tex = fromId[id];
        if (tex->networked)
            tex->purge();
    }
}


CachedTexture::Links *TextureCache::texLinksForLRU(CachedTexture *tex,
                                                   LRU &lru)
// ----------------------------------------------------------------------------
//   The links in texture 'tex' that should be used for chaining into LRU 'lru'
// ----------------------------------------------------------------------------
{
    if (&lru == &memLRU)
        return &tex->memLRU;
    else if (&lru == &GL_LRU)
        return &tex->GLmemLRU;

    XL_ASSERT(!"Invalid LRU pointer");
    return NULL;  // not reached
}


void TextureCache::insert(CachedTexture *tex, LRU &lru)
// ----------------------------------------------------------------------------
//   Insert texture at beginning of LRU list
// ----------------------------------------------------------------------------
{
    CachedTexture::Links *t = texLinksForLRU(tex, lru);

#ifndef QT_NO_DEBUG
    for (CachedTexture::Links *cur = lru.first; cur; cur = cur->next)
        XL_ASSERT(tex != cur->tex);
#endif

    if (lru.first) lru.first->prev = t;
    t->next = lru.first;
    lru.first = t;
    if (!lru.last) lru.last = lru.first;

    XL_ASSERT(lru.first);
    XL_ASSERT(lru.last);
    XL_ASSERT(lru.last->next == NULL);
}


void TextureCache::relink(CachedTexture *tex, LRU &lru)
// ----------------------------------------------------------------------------
//   Move texture at beginning of LRU list
// ----------------------------------------------------------------------------
{
    CachedTexture::Links *t = texLinksForLRU(tex, lru);

    if (lru.first != t)
    {
        if (t->prev) t->prev->next = t->next;
        if (t->next) t->next->prev = t->prev;
        if (lru.last == t) lru.last = t->prev;
        t->prev = 0;
        t->next = lru.first;
        lru.first->prev = t;
        lru.first = t;
    }
    XL_ASSERT(lru.first);
    XL_ASSERT(lru.last);
    XL_ASSERT(lru.last->next == NULL);
}


void TextureCache::unlink(CachedTexture *tex, LRU &lru)
// ----------------------------------------------------------------------------
//   Remove texture from LRU list
// ----------------------------------------------------------------------------
{
    CachedTexture::Links *t = texLinksForLRU(tex, lru);

    if (t->prev) t->prev->next = t->next;
    if (t->next) t->next->prev = t->prev;
    if (lru.last  == t) lru.last  = t->prev;
    if (lru.first == t) lru.first = t->next;

    t->next = t->prev = 0;
}



// ============================================================================
//
//    CachedTexture class
//
// ============================================================================

CachedTexture::CachedTexture(TextureCache &cache, const QString &path,
                             bool mipmap, bool compress)
// ----------------------------------------------------------------------------
//   Allocate GL texture ID to image
// ----------------------------------------------------------------------------
    : path(path), width(0), height(0),  mipmap(mipmap),
      compress(compress), isDefaultTexture(false),
      networked(path.contains("://")),
      cache(cache), GLsize(0),
      memLRU(this), GLmemLRU(this), saveCompressed(cache.saveCompressed),
      networkReply(NULL), inLoad(false)
{
    GL.GenTextures(1, &id);
    if (networked)
    {
        connect(&cache.network, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(checkReply(QNetworkReply*)));
    }
    else
    {
        connect(&cache.fileMonitor, SIGNAL(created(QString,QString)),
                this, SLOT(onFileCreated(QString,QString)));
        connect(&cache.fileMonitor, SIGNAL(changed(QString,QString)),
                this, SLOT(onFileChanged(QString,QString)));
        connect(&cache.fileMonitor, SIGNAL(deleted(QString,QString)),
                this, SLOT(onFileDeleted(QString)));
    }
}


CachedTexture::~CachedTexture()
// ----------------------------------------------------------------------------
//   Delete texture
// ----------------------------------------------------------------------------
{
    purge();

    if(Tao::OpenGLState::Current())
        GL.DeleteTextures(1, &id);

    if (networkReply)
        networkReply->deleteLater();
    if (path != "")
        cache.fileMonitor.removePath(path);
}


bool CachedTexture::load()
// ----------------------------------------------------------------------------
//   Load file into memory and update cached size. Return true if loaded.
// ----------------------------------------------------------------------------
{
    XL_ASSERT(!loaded());

    // Update load parameters (in case cache settings changed)
    mipmap = cache.mipmap;
    compress = cache.compress;

    int size = 0;
    bool inProgress = false;
    if (networked)
    {
        if (networkReply == NULL)
        {
            QUrl url(path);
            QNetworkRequest req(url);
            networkReply = cache.network.get(req);
            inProgress = true;
        }
        if (networkReply->isFinished() &&
            networkReply->error() == QNetworkReply::NoError)
            image.loadFromData(networkReply->readAll());
    }
    else
    {
        if (canonicalPath == "")
        {
            // If file exists, fileMonitor will emit created() synchronously,
            // which will set canonicalPath
            inLoad = true; // Prevent onFileCreated from calling reload
            cache.fileMonitor.addPath(path);
            inLoad = false;
        }
        if (canonicalPath != "")
            image.load(canonicalPath, compress);
    }
    if (image.isNull())
    {
        if (!networked || (networkReply && networkReply->isFinished() &&
                           networkReply->error() != QNetworkReply::NoError))
            image.load(":/images/default_image.svg");
        isDefaultTexture = true;
    }
    else
    {
        isDefaultTexture = false;
    }
    if (!image.isNull())
    {
        width = image.width();
        height = image.height();
        size = image.byteCount();
    }

    if (networked)
    {
        if (inProgress)
            record(textures, "Load %s in progress", path);
        else if (loaded())
            record(textures,
                   "Network %s: %u x %u pixels, %k",
                   path, width, height, size);
    }
    else
    {
        if (isDefaultTexture)
        {
            record(textures_warning, "Failed to load %s", path);
        }
        else
        {
            QString cpath = image.compressed
                ? Image::toCompressedPath(canonicalPath)
                : canonicalPath;
            record(textures,
                   "Network %s compressed %s: %u x %u pixels, %k",
                   path, cpath, width, height, size);
        }
    }

    cache.memSize += size;
    return (size != 0);
}


void CachedTexture::unload()
// ----------------------------------------------------------------------------
//   Remove image data from memory and update cached size
// ----------------------------------------------------------------------------
{
    XL_ASSERT(loaded());

    int purged = image.byteCount();
    image.clear();
    record(textures, "Unloading %k", purged);
    cache.memSize -= purged;
}


void CachedTexture::purge()
// ----------------------------------------------------------------------------
//   Remove texture data from memory and GL memory
// ----------------------------------------------------------------------------
{
    if (loaded())
    {
        cache.unlink(this, cache.memLRU);
        unload();
    }
    if (transferred())
    {
        cache.unlink(this, cache.GL_LRU);
        purgeGL();
    }
}


void CachedTexture::transfer()
// ----------------------------------------------------------------------------
//   Copy image to GL memory and update cached sizes
// ----------------------------------------------------------------------------
{
#define ADJUST_FOR_MIPMAP_OVERHEAD(sz) \
        do { if (mipmap) { sz *= 1.33; } } while(0)

    if (networked && !loaded())
        return;

    XL_ASSERT(loaded());
    XL_ASSERT(!transferred());
    XL_ASSERT(id);

    int before = image.byteCount();

    bool copiedCompressed = false, didNotCompress = false;
    int copiedSize = 0;

    if (compress)
    {
        // Want compressed texture

        if (image.compressed)
        {
            // Already have compressed texture data

            GL.BindTexture(GL_TEXTURE_2D, id);
            if (mipmap)
                GL.TexParameter(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
            TextureCache::instance()->setMinMagFilters(id);
            GL.CompressedTexImage2D(GL_TEXTURE_2D, 0, image.fmt, width, height,
                                    0, image.byteCount(), image.compressed);
            copiedSize = GLsize = image.byteCount();
            copiedCompressed = true;
            ADJUST_FOR_MIPMAP_OVERHEAD(GLsize);
        }
        else
        {
            // Transfer uncompressed image data and request compression.

            QImage texture = QGLWidget::convertToGLFormat(image.raw);
            bool hasAlpha = image.raw.hasAlphaChannel();
            GLenum internalFmt = hasAlpha?GL_COMPRESSED_RGBA:GL_COMPRESSED_RGB;
            GL.BindTexture(GL_TEXTURE_2D, id);
            if (mipmap)
                GL.TexParameter(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
            TextureCache::instance()->setMinMagFilters(id);
            GL.TexImage2D(GL_TEXTURE_2D, 0, internalFmt,
                          width, height, 0, GL_RGBA,
                          GL_UNSIGNED_BYTE, texture.bits());
            copiedSize = width * height * 4;

            GLint cmp = GL_FALSE, cmpsz = copiedSize;
            GL.GetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED,
                                      &cmp);
            if (cmp)
                GL.GetTexLevelParameteriv(GL_TEXTURE_2D, 0,
                                          GL_TEXTURE_COMPRESSED_IMAGE_SIZE,
                                          &cmpsz);

            if (cmp && cmpsz)
            {
                // Cache compressed data for next time
                GL.GetTexLevelParameteriv(GL_TEXTURE_2D, 0,
                                          GL_TEXTURE_INTERNAL_FORMAT,
                                          &image.fmt);
                GL.GetCompressedTexImage(GL_TEXTURE_2D, 0,
                                         image.allocateCompressed(cmpsz));
                image.w = width;
                image.h = height;
            }
            else
            {
                // We will not try to compress this one again
                didNotCompress = true;
            }

            if (!networked && saveCompressed && image.compressed &&
                !image.loadedFromCompressedFile)
            {
                QString cmpPath = Image::toCompressedPath(canonicalPath);
                if (image.saveCompressed(cmpPath))
                    record(textures, "Saved compressed %s", cmpPath);
            }

            GLsize = cmpsz;
            ADJUST_FOR_MIPMAP_OVERHEAD(GLsize);
        }
    }
    else
    {
        // Want uncompressed texture

        XL_ASSERT(!image.compressed);

        QImage texture = QGLWidget::convertToGLFormat(image.raw);
        bool hasAlpha = image.raw.hasAlphaChannel();
        GLenum internalFmt = hasAlpha ? GL_RGBA : GL_RGB;
        int bytesPerPixel = hasAlpha ? 4 : 3;

        // Generate the GL texture
        GL.BindTexture(GL_TEXTURE_2D, id);
        if (mipmap)
            GL.TexParameter(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
        TextureCache::instance()->setMinMagFilters(id);
        GL.TexImage2D(GL_TEXTURE_2D, 0, internalFmt,
                     width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, texture.bits());

        XL_ASSERT(GLsize == 0);
        GLsize = width * height * bytesPerPixel;
        copiedSize = width * height * 4;
        ADJUST_FOR_MIPMAP_OVERHEAD(GLsize);
    }

    record(textures,
           "Mem->GL %k, %k copied, %+s, %+s",
           GLsize,
           copiedSize,
           copiedCompressed
           ? (compress ? "compressed" : "compressed (unexpected)")
           : (compress ? "failed compression" : "uncompressed"),
           mipmap ? "mipmapped" : "no mipmap");

    int after = image.byteCount();
    int saved = (before - after);
    if (saved)
    {
        record(textures, "Compression saved %k", saved);
        cache.memSize -= saved;
    }
    else
    {
        if (compress && didNotCompress)
        {
            record(textures_warning, "GL did not compress, will not ask again");
            compress = false;
        }
    }

    cache.GLSize += GLsize;
}


void CachedTexture::purgeGL()
// ----------------------------------------------------------------------------
//   Remove texture data from GL memory, keep texture ID, update cached size
// ----------------------------------------------------------------------------
{
    XL_ASSERT(id);

    if (Tao::OpenGLState::Current())
    {
        // Assure we restore a correct GL state after purge.
        // (Purge phase is often out of the evaluation phase)
        GLAllStateKeeper save;

        // Resize the texture as a 1x1 pixel. Resizing at 0x0 triggers a bug in
        // the Nvidia driver on MacOS Mountain Lion (#2622)
        static uint32 zero = 0;
        GL.BindTexture(GL_TEXTURE_2D, id);
        GL.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA,
                      GL_UNSIGNED_BYTE, &zero);

        int purged = GLsize;
        GLsize = 0;
        cache.GLSize -= purged;
        record(textures, "Purged %k GL bytes now %k", purged, cache.GLSize);
    }
}


GLuint CachedTexture::bind()
// ----------------------------------------------------------------------------
//   Bind texture
// ----------------------------------------------------------------------------
{
    if (networked && !transferred())
        return 0;

    XL_ASSERT(id);
    XL_ASSERT(transferred());
    GL.BindTexture(GL_TEXTURE_2D, id);

    return id;
}


void CachedTexture::checkReply(QNetworkReply *reply)
// ----------------------------------------------------------------------------
//   Process network reply completion (success or error)
// ----------------------------------------------------------------------------
{
    // Check if this is for me
    if (reply != networkReply)
        return;

    if (networked &&
        image.isNull())
    {
        QNetworkRequest::Attribute
                attr = QNetworkRequest::HttpStatusCodeAttribute;
        int code = networkReply->attribute(attr).toInt();

        if (reply->error() == QNetworkReply::NoError)
        {
            record(textures, "Received HTTP%d for %s", code, path);
            if (code >= 300 && code <= 400)
            {
                // Redirected
                networkReply->deleteLater();
                QNetworkRequest::Attribute
                        attr = QNetworkRequest::RedirectionTargetAttribute;
                QUrl url = reply->attribute(attr).toUrl();
                record(textures, "Redirected to %s", url.toString());
                QNetworkRequest req(url);
                networkReply = cache.network.get(req);
                return;
            }
        }
        else
        {
            record(textures_warning, "Error %s downloading %s",
                   reply->errorString(), path);
        }
        // Show received image, or error placeholder
        reload();
        networkReply->deleteLater();
        networkReply = NULL;
        // A simple update is not enough here: the texture id has not yet been
        // returned, so a XL refresh is needed
        Widget::postEventAPI(cache.textureChangedEvent());

        emit textureUpdated(this);
    }
}


void CachedTexture::reload()
// ----------------------------------------------------------------------------
//   Reload texture e.g., when file was changed
// ----------------------------------------------------------------------------
{
    record(textures, "Reloading %s", path);
    cache.reload(this);
}


QImage CachedTexture::loadedImage()
// ----------------------------------------------------------------------------
//    Return the image as it was loaded, reload if necessary
// ----------------------------------------------------------------------------
{
    bind();
    return image.raw;
}


void CachedTexture::onFileCreated(const QString &path,
                                  const QString &canonicalPath)
// ----------------------------------------------------------------------------
//   Called by file monitor when a registered path is an existing file
// ----------------------------------------------------------------------------
{
    // Also called when a file is deleted, then re-created
    if (path == this->path)
    {
        this->canonicalPath = canonicalPath;
        if (!inLoad)
        {
            reload();
            Widget::postEventOnceAPI(cache.textureChangedEvent());
        }
    }
}


void CachedTexture::onFileChanged(const QString &path,
                                  const QString &canonicalPath)
// ----------------------------------------------------------------------------
//   Called by file monitor when a file is changed
// ----------------------------------------------------------------------------
{
    if (path == this->path)
    {
        // Canonical path may have changed if path is a prefixed path
        this->canonicalPath = canonicalPath;
        reload();
        Widget::postEventOnceAPI(cache.textureChangedEvent());
    }
}


void CachedTexture::onFileDeleted(const QString &path)
// ----------------------------------------------------------------------------
//   Called by file monitor when the file is deleted
// ----------------------------------------------------------------------------
{
    if (path == this->path)
    {
        this->canonicalPath = "";
        reload();
        Widget::postEventOnceAPI(cache.textureChangedEvent());
    }
}



// ============================================================================
//
//    Image class
//
// ============================================================================



bool Image::isNull()
// ----------------------------------------------------------------------------
//   Is this image non-empty?
// ----------------------------------------------------------------------------
{
    // We never keep both compressed and uncompressed versions of the
    // same image
    XL_ASSERT(!(!raw.isNull() && compressed));

    return (raw.isNull() && !compressed);
}


void Image::clear()
// ----------------------------------------------------------------------------
//   Delete image data
// ----------------------------------------------------------------------------
{
    if (compressed)
        free(compressed);
    compressed = 0;
    w = h = sz = 0;
    raw = QImage();
    loadedFromCompressedFile = false;
}


int Image::width()
// ----------------------------------------------------------------------------
//   The image width in pixels
// ----------------------------------------------------------------------------
{
    return compressed ? w : raw.width();
}


int Image::height()
// ----------------------------------------------------------------------------
//   The image height in pixels
// ----------------------------------------------------------------------------
{
    return compressed ? h : raw.height();
}


int Image::byteCount()
// ----------------------------------------------------------------------------
//   The size in bytes of the image
// ----------------------------------------------------------------------------
{
    if (compressed)
    {
        XL_ASSERT(sz);
        return sz;
    }
    return raw.byteCount();
}


void Image::load(const QString &path, bool trycomp)
// ----------------------------------------------------------------------------
//   Load compressed or uncompressed image from file
// ----------------------------------------------------------------------------
{
    if (compressed)
        clear();

    if (!trycomp || !loadCompressed(path))
        raw.load(path);
}


void Image::loadFromData(const QByteArray &data)
// ----------------------------------------------------------------------------
//   Load uncompressed image from byte array
// ----------------------------------------------------------------------------
{
    if (compressed)
        clear();
    raw.loadFromData(data);
}


void * Image::allocateCompressed(int len)
// ----------------------------------------------------------------------------
//   Reserve space for len bytes in compressed image then return data pointer
// ----------------------------------------------------------------------------
{
    // Used when converting uncompressed to compressed.
    XL_ASSERT(!compressed);
    raw = QImage();
    return (compressed = malloc(sz = len));
}


QString Image::toCompressedPath(const QString &path)
// ----------------------------------------------------------------------------
//   The path for the compressed variant of a texture file
// ----------------------------------------------------------------------------
{
    return path + ".compressed";
}


quint64 Image::CompressedFileHeader::theSignature()
// ----------------------------------------------------------------------------
//   Signature to avoid loading invalid compressed texture files
// ----------------------------------------------------------------------------
{
#define MAKE_QUINT64(a, b, c, d, e, f, g, h)                                  \
    ((quint64)a<<56) | ((quint64)b<<48) | ((quint64)c<<40) | ((quint64)d<<32) \
  | ((quint64)e<<24) | ((quint64)f<<16) | ((quint64)g<< 8) | ((quint64)h)

    return MAKE_QUINT64('T', 'A', 'O', 'C', 'M', 'P', 'T', 'X');

#undef MAKE_QUINT64
}

bool Image::loadCompressed(const QString &path)
// ----------------------------------------------------------------------------
//   Load pre-compressed file if it exists and is more recent than uncompressed
// ----------------------------------------------------------------------------
{
    XL_ASSERT(!compressed);

    QFileInfo uncmp(path), cmp(toCompressedPath(path));
    if (!cmp.exists() || !uncmp.exists())
        return false;
    if (cmp.lastModified() < uncmp.lastModified())
        return false;
    QFile cmpfile(cmp.filePath());
    if (!cmpfile.open(QIODevice::ReadOnly))
        return false;

    QByteArray ba = cmpfile.readAll();
    CompressedFileHeader *hdr = (CompressedFileHeader *)ba.constData();

    int len = ba.size() - sizeof(CompressedFileHeader);
    if (len < 0)
        return false;
    if (hdr->signature != CompressedFileHeader::theSignature())
        return false;
    fmt = qFromBigEndian(hdr->fmt);
    if (!TextureCache::instance()->supported(fmt))
    {
        fmt = 0;
        return false;
    }

    w = qFromBigEndian(hdr->w);
    h = qFromBigEndian(hdr->h);
    memcpy(allocateCompressed(len), &hdr->data, len);

    loadedFromCompressedFile =  true;
    return true;
}


bool Image::saveCompressed(const QString &compressedPath)
// ----------------------------------------------------------------------------
//   Save (cache) compressed texture to specified path
// ----------------------------------------------------------------------------
{
    XL_ASSERT(compressed);
    XL_ASSERT(sz);

    bool ok = false;
    QFile cmpfile(compressedPath);
    if (cmpfile.open(QIODevice::WriteOnly))
    {
        CompressedFileHeader hdr;
        hdr.signature = hdr.theSignature();
        hdr.fmt = qToBigEndian((quint32)fmt);
        hdr.w = qToBigEndian((quint32)w);
        hdr.h = qToBigEndian((quint32)h);

        if (cmpfile.write((const char*)&hdr, sizeof(hdr)) &&
            cmpfile.write((const char*)compressed, sz) == sz)
        {
            ok = true;
        }
        else
        {
            cmpfile.remove();
        }
    }
    return ok;
}

}
