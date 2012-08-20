// ****************************************************************************
//  texture_cache.cpp                                              Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "texture_cache.h"
#include "base.h"
#include "tao_utf8.h"
#include "preferences_pages.h"
#include "license.h"

namespace Tao {

// ============================================================================
// 
//    Helpers
// 
// ============================================================================

TextureCache * TextureCache::textureCache = NULL;


static text bytesToText(qint64 size)
// ----------------------------------------------------------------------------
//   Convert size in bytes into a human readable string
// ----------------------------------------------------------------------------
{
    if (size == CACHE_UNLIMITED)
        return +QString("unlimited");
    if (size < CACHE_KB)
        return +QString("%1 B").arg(size);
    else if (size < CACHE_MB)
        return +QString("%1 KiB").arg((double)size/CACHE_KB, 0, 'f', 2);
    else if (size < CACHE_GB)
        return +QString("%1 MiB").arg((double)size/CACHE_MB, 0, 'f', 2);
    else
        return +QString("%1 GiB").arg((double)size/CACHE_GB, 0, 'f', 2);
}


// ----------------------------------------------------------------------------
//   Primitives to enable or disable settings
// ----------------------------------------------------------------------------
#define BOOL_SETTER(fn, attr)                                               \
XL::Name_p TextureCache::fn(bool enable)                                    \
{                                                                           \
    TextureCache * tc = TextureCache::instance();                           \
    bool &attr = tc->attr, prev = attr;                                     \
                                                                            \
    if (attr != enable)                                                     \
    {                                                                       \
        attr = enable;                                                      \
        IFTRACE(texturecache)                                               \
            tc->debug() << #attr << " " << prev << " -> " << attr << "\n";  \
    }                                                                       \
                                                                            \
    return prev ? XL::xl_true : XL::xl_false;                               \
}

BOOL_SETTER(textureMipmap, mipmap)
BOOL_SETTER(textureCompress, compress)


// ----------------------------------------------------------------------------
//   Primitives to change cache size limits
// ----------------------------------------------------------------------------
#define SIZE_SETTER(fn, attr)                                               \
XL::Integer_p TextureCache::fn(qint64 val)                                  \
{                                                                           \
    TextureCache * tc = TextureCache::instance();                           \
    qint64 &attr = tc->attr, prev = attr;                                   \
                                                                            \
    if (attr != val)                                                        \
    {                                                                       \
        attr = val;                                                         \
        IFTRACE(texturecache)                                               \
            tc->debug() << #attr << " " << bytesToText(prev) << " -> "      \
                        << bytesToText(attr) << "\n";                       \
    }                                                                       \
                                                                            \
    return new XL::Integer(prev);                                           \
}

SIZE_SETTER(textureCacheMemSize, maxMemSize)
SIZE_SETTER(textureCacheGLSize, maxGLSize)



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
      network(NULL)
                           
{
    statTimer.setSingleShot(true);
    connect(&statTimer, SIGNAL(timeout()), this, SLOT(doPrintStatistics()));
}


void TextureCache::doPrintStatistics()
// ----------------------------------------------------------------------------
//   Print cache statistics
// ----------------------------------------------------------------------------
{
    IFTRACE(texturecache)
        debug() << "Used: Mem " << bytesToText(memSize) << "/"
                                << bytesToText(maxMemSize)
                << ", GL " << bytesToText(GLSize) << "/"
                           << bytesToText(maxGLSize)
                << "\n";

#ifndef QT_NO_DEBUG
    qint64 checkMemSize = 0, checkGLSize = 0;
    QList<GLuint> ids = fromId.keys();
    foreach (GLuint id, ids)
    {
        CachedTexture * tex = fromId[id];
        checkMemSize += tex->image.byteCount();
        checkGLSize += tex->GLsize;
    }
    Q_ASSERT(checkMemSize == memSize);
    Q_ASSERT(checkGLSize == GLSize);
#endif
}

void TextureCache::printStatistics()
// ----------------------------------------------------------------------------
//   Print cache statistics not more often that once per second
// ----------------------------------------------------------------------------
{
    IFTRACE(texturecache)
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
    QString qimg(img);
    if (!img.contains("://") && QDir::isRelativePath(img) &&
        QRegExp("^[a-z]+:").indexIn(img) == -1)
    {
        qimg = docPath + "/" + img;
        if (!QFileInfo(qimg).exists())
            qimg = "texture:" + img; // Backward-compatibility
    }
    // qimg is either a URL, full path or a prefixed path ("image:file.jpg")
    // It cannot be a relative path.
    TextureName name(qimg, docPath); // REVISIT: remove docPath
    CachedTexture * cached = fromName.value(name);
    if (!cached)
    {
        cached = new CachedTexture(*this, qimg, docPath, mipmap, compress);
        GLuint id = cached->id;
        fromId[id] = fromName[name] = cached;
        if (memSize > maxMemSize)
            purgeMem();
        cached->load();
        insert(cached, memLRU);
        if (GLSize < maxGLSize)
        {
            cached->transfer();
            insert(cached, GL_LRU);
        }
        printStatistics();
    }

    return cached;
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
            cached->load();
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


void TextureCache::setMinMagFilters(GLuint id)
// ----------------------------------------------------------------------------
//   Set GL texture filters to the values currently configured in cache
// ----------------------------------------------------------------------------
{
    setMinFilter(id, minFilter());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter());
}


void TextureCache::setMinFilter(GLuint id, GLenum filter)
// ----------------------------------------------------------------------------
//   Set GL texture minifying filter for texture 'id'
// ----------------------------------------------------------------------------
{
    CachedTexture * cached = fromId[id];
    Q_ASSERT(cached);

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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
}


void TextureCache::purgeMem()
// ----------------------------------------------------------------------------
//   Drop the least recently used textures from main memory
// ----------------------------------------------------------------------------
{
    while (memSize > (maxMemSize * purgeRatio))
    {
        if (!memLRU.last)
        {
            IFTRACE(texturecache)
                debug() << "Nothing left to purge!\n";
            break;
        }

        CachedTexture * tex = memLRU.last->tex;
        unlink(tex, memLRU);
        tex->unload();

        Q_ASSERT(memSize >= 0);
    }
}


void TextureCache::purgeGLMem()
// ----------------------------------------------------------------------------
//   Drop the least recently used items from texture memory
// ----------------------------------------------------------------------------
{
    while (GLSize > (maxGLSize * purgeRatio))
    {
        printStatistics();
        Q_ASSERT(GL_LRU.last);

        CachedTexture * tex = GL_LRU.last->tex;
        unlink(tex, GL_LRU);
        tex->purgeGL();

        Q_ASSERT(GLSize >= 0);
    }
}


void TextureCache::clear()
// ----------------------------------------------------------------------------
//   Empty cache
// ----------------------------------------------------------------------------
{
    IFTRACE(texturecache)
        debug() << "Clearing\n";

    QList<GLuint> ids = fromId.keys();
    foreach (GLuint id, ids)
    {
        CachedTexture * tex = fromId.take(id);
        Q_ASSERT(tex);
        fromName.remove(TextureName(tex->path, tex->docPath));
        unlink(tex, memLRU);
        unlink(tex, GL_LRU);
        delete tex;
    }
    Q_ASSERT(fromId.isEmpty());
    Q_ASSERT(fromName.isEmpty());
}


void TextureCache::purge()
// ----------------------------------------------------------------------------
//   Purge all textures in cache (free memory but keep texture id valid)
// ----------------------------------------------------------------------------
{
    IFTRACE(texturecache)
        debug() << "Purging\n";

    QList<GLuint> ids = fromId.keys();
    foreach (GLuint id, ids)
        fromId[id]->purge();
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

    Q_ASSERT(!"Invalid LRU pointer");
    return NULL;  // not reached
}


void TextureCache::insert(CachedTexture *tex, LRU &lru)
// ----------------------------------------------------------------------------
//   Insert texture at beginning of LRU list
// ----------------------------------------------------------------------------
{
    CachedTexture::Links *t = texLinksForLRU(tex, lru);

    if (lru.first) lru.first->prev = t;
    t->next = lru.first;
    lru.first = t;
    if (!lru.last) lru.last = lru.first;
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
    Q_ASSERT(lru.first);
    Q_ASSERT(lru.last);
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


std::ostream & TextureCache::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[TextureCache] ";
    return std::cerr;
}



// ============================================================================
// 
//    CachedTexture class
// 
// ============================================================================

CachedTexture::CachedTexture(TextureCache &cache, const QString &path,
                             const QString &docPath,
                             bool mipmap, bool compress, bool cacheCompressed)
// ----------------------------------------------------------------------------
//   Allocate GL texture ID to image
// ----------------------------------------------------------------------------
    : path(path), docPath(docPath), width(0), height(0),  mipmap(mipmap),
      compress(compress), isDefaultTexture(false), cache(cache), GLsize(0),
      memLRU(this), GLmemLRU(this), cacheCompressed(cacheCompressed),
      networked(path.contains("://")), networkReply(NULL), inLoad(false)
{
    glGenTextures(1, &id);
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
    glDeleteTextures(1, &id);
    if (networkReply)
        networkReply->deleteLater();
    if (canonicalPath != "")
    {
        // REVISIT Bug if 2 CachedTextures have the same absolutePath
        cache.fileMonitor.removePath(canonicalPath);
    }
}


void CachedTexture::load()
// ----------------------------------------------------------------------------
//   Load file into memory and update cached size
// ----------------------------------------------------------------------------
{
    Q_ASSERT(!loaded());

    // Update load parameters (in case cache settings changed)
    mipmap = cache.mipmap;
    compress = cache.compress;

    int size = 0;
    bool inProgress = false;
    if (networked)
    {
        if (networkReply == NULL)
        {
            Licenses::CheckImpressOrLicense("NetworkAccess 1.0");
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
            image.load(canonicalPath);
    }
    if (image.isNull())
    {
        if (!networked)
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

    IFTRACE2(texturecache, fileload)
    {
        if (networked)
        {
            if (inProgress)
                debug() << "Load in progress: '" << +path << "'\n";
        }
        else
        {
            if (isDefaultTexture)
                debug() << "Failed to load: '" << +path << "'\n";
            else
                debug() << "File->Mem  +" << bytesToText(size)
                        << " (" << width << "x" << height << " pixels, "
                        << "'" << +path << "' ['" << +canonicalPath << "'])\n";
        }
    }

    cache.memSize += size;
}


void CachedTexture::unload()
// ----------------------------------------------------------------------------
//   Remove image data from memory and update cached size
// ----------------------------------------------------------------------------
{
    int purged = image.byteCount();
    image.clear();

    IFTRACE(texturecache)
        debug() << "Mem -" << bytesToText(purged) << "\n";

    cache.memSize -= purged;
}


void CachedTexture::purge()
// ----------------------------------------------------------------------------
//   Remove texture data from memory and GL memory
// ----------------------------------------------------------------------------
{
    if (loaded())
        unload();
    if (transferred())
        purgeGL();
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

    Q_ASSERT(loaded());
    Q_ASSERT(!transferred());
    Q_ASSERT(id);

    int before = image.byteCount();

    bool copiedCompressed = false, didNotCompress = false;
    int copiedSize = 0;

    if (compress)
    {
        // Want compressed texture

        if (image.compressed)
        {
            // Already have compressed texture data

            glBindTexture(GL_TEXTURE_2D, id);
            if (mipmap)
                glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
            glCompressedTexImage2D(GL_TEXTURE_2D, 0, image.fmt, width, height,
                                   0, image.byteCount(), image.compressed);
            if (mipmap)
                glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
            copiedSize = GLsize = image.byteCount();
            copiedCompressed = true;
            ADJUST_FOR_MIPMAP_OVERHEAD(GLsize);
        }
        else
        {
            // Transfer uncompressed image data and request compression.

            QImage texture = QGLWidget::convertToGLFormat(image.raw);
            bool hasAlpha = image.raw.hasAlphaChannel();
            GLenum internalFmt = hasAlpha ? GL_COMPRESSED_RGBA : GL_COMPRESSED_RGB;
            glBindTexture(GL_TEXTURE_2D, id);
            if (mipmap)
                glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
            glTexImage2D(GL_TEXTURE_2D, 0, internalFmt,
                         width, height, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, texture.bits());
            if (mipmap)
                glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
            copiedSize = width * height * 4;

            GLint cmp = GL_FALSE, cmpsz = copiedSize;
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED,
                                     &cmp);
            if (cmp)
                glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
                                         GL_TEXTURE_COMPRESSED_IMAGE_SIZE,
                                         &cmpsz);

            if (cacheCompressed)
            {
                if (cmp && cmpsz)
                {
                    // Cache compressed data for next time
                    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
                                             GL_TEXTURE_INTERNAL_FORMAT,
                                             &image.fmt);
                    glGetCompressedTexImage(GL_TEXTURE_2D, 0,
                                            image.allocateCompressed(cmpsz));
                }
                else
                {
                    // We will not try to compress this one again
                    didNotCompress = true;
                }
            }

            GLsize = cmpsz;
            ADJUST_FOR_MIPMAP_OVERHEAD(GLsize);
        }
    }
    else
    {
        // Want uncompressed texture

        Q_ASSERT(!image.compressed);

        QImage texture = QGLWidget::convertToGLFormat(image.raw);
        bool hasAlpha = image.raw.hasAlphaChannel();
        GLenum internalFmt = hasAlpha ? GL_RGBA : GL_RGB;
        int bytesPerPixel = hasAlpha ? 4 : 3;

        // Generate the GL texture
        glBindTexture(GL_TEXTURE_2D, id);
        if (mipmap)
            glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFmt,
                     width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, texture.bits());
        if (mipmap)
            glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);

        Q_ASSERT(GLsize == 0);
        GLsize = width * height * bytesPerPixel;
        copiedSize = width * height * 4;
        ADJUST_FOR_MIPMAP_OVERHEAD(GLsize);
    }

    IFTRACE(texturecache)
    {
        debug() << "Mem->GL +" << bytesToText(GLsize)
                << " (copied " << bytesToText(copiedSize) << " "
                << (char*)(copiedCompressed ? "" : "not ") << "compressed, "
                << (char*)((compress && !copiedCompressed) ?
                           "compression requested, " : "")
                << (char*)(mipmap ? "" : "no ") << "mipmap)\n";
    }

    int after = image.byteCount();
    int saved = (before - after);
    if (saved)
    {
        IFTRACE(texturecache)
            debug() << "Compression saved " << bytesToText(saved)
                    << " Mem\n";
        cache.memSize -= saved;
    }
    else
    {
        if (compress && didNotCompress)
        {
            IFTRACE(texturecache)
                debug() << "GL did not compress - will not ask again\n";
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
    Q_ASSERT(id);

    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    int purged = GLsize;
    GLsize = 0;

    IFTRACE(texturecache)
        debug() << "GL -" << bytesToText(purged) << "\n";
    cache.GLSize -= purged;
}


GLuint CachedTexture::bind()
// ----------------------------------------------------------------------------
//   Bind texture
// ----------------------------------------------------------------------------
{
    if (networked && !transferred())
        return 0;

    Q_ASSERT(id);
    Q_ASSERT(transferred());
    glBindTexture(GL_TEXTURE_2D, id);
    return id;
}


void CachedTexture::checkReply(QNetworkReply *reply)
// ----------------------------------------------------------------------------
//   Check if network reply is complete, if so then load image
// ----------------------------------------------------------------------------
{
    Q_ASSERT(reply == networkReply);

    if (networked &&
        image.isNull() &&
        reply->error() == QNetworkReply::NoError)
    {
        IFTRACE(texturecache)
            debug() << "Received: '" << +path << "'\n";
        reload();
        reply->deleteLater();
        networkReply = NULL;
    }
}


void CachedTexture::reload()
// ----------------------------------------------------------------------------
//   Reload texture e.g., when file was changed
// ----------------------------------------------------------------------------
{
    // FIXME: check cache limits

    IFTRACE(texturecache)
        debug() << "Reloading\n";
    purge();
    load();
    transfer();
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
            reload();
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
    }
}


std::ostream & CachedTexture::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[CachedTexture " << id << "] ";
    return std::cerr;
}

}
