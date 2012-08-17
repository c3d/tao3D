#ifndef TEXTURE_CACHE_H
#define TEXTURE_CACHE_H
// ****************************************************************************
//  texture_cache.h                                                Tao project
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

#include "tao.h"
#include "tao_gl.h"
#include "tao_tree.h"
#include "file_monitor.h"
#include <QMap>
#include <QDateTime>
#include <QtNetwork>
#include <QTime>
#include <QTimer>
#include <iostream>

const qint64 CACHE_KB = 1024LL;
const qint64 CACHE_MB = 1024LL * CACHE_KB;
const qint64 CACHE_GB = 1024LL * CACHE_MB;

// Large size shown as "Unlimited" at the UI level
const qint64 CACHE_UNLIMITED = 999LL * CACHE_GB;


namespace Tao {


struct Image
// ----------------------------------------------------------------------------
//    Hold an image, loaded from disk or read back compressed from GL
// ----------------------------------------------------------------------------
{
    Image() : w(0), h(0), compressed(0), sz(0), fmt(0) {}
    ~Image() { clear(); }

    bool isNull()
    {
        // We never keep both compressed and uncompressed versions of the
        // same image
        Q_ASSERT(!(!raw.isNull() && compressed));

        return (raw.isNull() && !compressed);
    }

    void clear()
    {
        if (compressed)
            free(compressed);
        compressed = 0;
        w = h = sz = 0;
        raw = QImage();
    }

    int byteCount()
    {
        if (compressed)
        {
            Q_ASSERT(sz);
            return sz;
        }
        return raw.byteCount();
    }

    // Load uncompressed image from file
    void load(const QString &path)
    {
        if (compressed)
        {
            Q_ASSERT(sz);
            free(compressed);
            compressed = 0;
            sz = 0;
        }
        raw.load(path);
    }

    // Load uncompressed image from file
    void loadFromData(const QByteArray &data)
    {
        if (compressed)
        {
            Q_ASSERT(sz);
            free(compressed);
            compressed = 0;
            sz = 0;
        }
        raw.loadFromData(data);
    }

    // Used when converting uncompressed to compressed.
    // Reserve space for len bytes in compressed image then return
    // data pointer.
    void * allocateCompressed(int len)
    {
        Q_ASSERT(!compressed);
        raw = QImage();
        return (compressed = malloc(sz = len));
    }

    int  width()   { if (compressed) return w; else return raw.width();  }
    int  height()  { if (compressed) return h; else return raw.height(); }

    int w, h;

    QImage  raw;

    void * compressed;
    int    sz;
    GLint  fmt;
};


class TextureCache;

class CachedTexture : public QObject
// ----------------------------------------------------------------------------
//    2D texture managed by TextureCache
// ----------------------------------------------------------------------------
{
    Q_OBJECT

    friend class TextureCache;

    struct Links
    {
        Links(CachedTexture *tex) : prev(0), next(0), tex(tex) {}
        Links * prev, * next;
        CachedTexture * tex;
    };

public:
    CachedTexture(TextureCache &cache, const QString &path,
                  const QString &docPath, bool mipmap,
                  bool compress, bool cacheCompressed = true);
    ~CachedTexture();

    void            load();
    void            unload();

    void            transfer();
    void            purgeGL();

    void            purge();

    GLuint          bind();

    bool            loaded()
    {
        if (cacheCompressed)
            return (image.compressed || !image.raw.isNull());
        return (!image.raw.isNull());
    }
    bool            transferred() { return (GLsize != 0); }

    void            reload();

public slots:
    void            onFileChanged(const QString &path);

private:
    std::ostream &  debug();
    QString         findPath();
    void            checkFile();

public:
    QString         path, docPath, absolutePath;
    GLuint          id;
    int             width, height;
    bool            mipmap, compress;
    bool            isDefaultTexture;

private:
    TextureCache &  cache;
    int             GLsize;
    Links           memLRU, GLmemLRU;

    Image           image;
    bool            cacheCompressed;
    bool            networked;
    QNetworkReply  *networkReply;

    QTime           pathLastResolved;
};





class TextureCache : public QObject
// ----------------------------------------------------------------------------
//    Singleton. Reads image files from disk, create textures, cache them
// ----------------------------------------------------------------------------
{
    Q_OBJECT

    friend class CachedTexture;

public:
    static TextureCache * instance()
    {
        if (!textureCache)
            textureCache = new TextureCache;
        return textureCache;
    }

public:
    // Primitives
    static XL::Name_p    textureMipmap(bool enable);
    static XL::Name_p    textureCompress(bool enable);

    static XL::Integer_p textureCacheMemSize(qint64 bytes);
    static XL::Integer_p textureCacheGLSize(qint64 bytes);

public:
    TextureCache();
    virtual ~TextureCache() { clear(); }

    CachedTexture * load(const QString &img, const QString &docPath);
    CachedTexture * bind(GLuint id);
    void            setMinMagFilters(GLuint id);
    void            setMinFilter(GLuint id, GLenum filter);
    void            setMagFilter(GLuint id, GLenum filter);

    GLenum          minFilter() { return minFilt; }
    GLenum          magFilter() { return magFilt; }
    qint64          maxMem()    { return maxMemSize; }
    qint64          maxGLMem()  { return maxGLSize; }

public slots:
    void            clear();
    void            purge();
    void            setMaxMemSize(qint64 bytes) { maxMemSize = bytes; }
    void            setMaxGLSize(qint64 bytes)  { maxGLSize  = bytes; }
    void            setMipmap(bool enable)      { mipmap = enable; purge(); }
    void            setCompression(bool enable) { compress = enable; purge(); }
    void            setMinFilter(GLenum filter) { minFilt = filter; }
    void            setMagFilter(GLenum filter) { magFilt = filter; }

private:
    // LRU list management
    struct LRU
    {
        LRU() : first(0), last(0) {}
        CachedTexture::Links * first, * last;
    };

    void            insert(CachedTexture * tex, LRU &lru);
    void            relink(CachedTexture * tex, LRU &lru);
    void            unlink(CachedTexture * tex, LRU &lru);
    CachedTexture::Links *texLinksForLRU(CachedTexture *tex, LRU &lru);
    void            purgeMem();
    void            purgeGLMem();
    void            printStatistics();

    std::ostream &  debug();

private slots:
    void            doPrintStatistics();

private:
    typedef QPair<QString, QString> TextureName; // (file path, doc path)

    QMap <TextureName, CachedTexture *> fromName;
    QMap <GLuint, CachedTexture *>   fromId;
    LRU                              memLRU, GL_LRU;
    qint64                           memSize, GLSize, maxMemSize, maxGLSize;
    float                            purgeRatio;
    QTimer                           statTimer;

    // Default settings for new textures
    bool                             mipmap, compress;
    // Min/mag filters to use
    GLenum                           minFilt, magFilt;

    // Network access manager for all texture network accesses
    QNetworkAccessManager            network;

    // Enables reloading files as they change
    FileMonitor                      fileMonitor;

private:
    static TextureCache * textureCache;
};


struct TextureCacheAPI
// ----------------------------------------------------------------------------
//    Export some TextureCache functions to module API
// ----------------------------------------------------------------------------
{
    static bool bind(GLuint id)
    {
        return TextureCache::instance()->bind(id);
    }
    static void setMinMagFilters(GLuint id)
    {
        TextureCache::instance()->setMinMagFilters(id);
    }
};

}

#endif // texture_cache.h
