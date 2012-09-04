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
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QTimer>
#include <QtNetwork>
#include <QSet>
#include <QSharedPointer>
#include <QWeakPointer>
#include <iostream>

const quint64 CACHE_KB = 1024LL;
const quint64 CACHE_MB = 1024LL * CACHE_KB;
const quint64 CACHE_GB = 1024LL * CACHE_MB;

// Large size shown as "Unlimited" at the UI level
const quint64 CACHE_UNLIMITED = 999LL * CACHE_GB;


namespace Tao {


struct Image
// ----------------------------------------------------------------------------
//    Hold an image, loaded from disk/network or read back compressed from GL
// ----------------------------------------------------------------------------
{
    Image() : w(0), h(0), compressed(0), sz(0), fmt(0) {}
    ~Image() { clear(); }

    bool      isNull();
    void      clear();
    int       byteCount();
    int       width();
    int       height();

    void      load(const QString &path, bool trycomp = false);
    void      loadFromData(const QByteArray &data);
    void *    allocateCompressed(int len);

    static
    QString   toCompressedPath(const QString &path);
    bool      loadCompressed(const QString &path);
    bool      saveCompressed(const QString &compressedPath);

    struct CompressedFileHeader
    // ------------------------------------------------------------------------
    //    Private header used when saving a compressed texture to disk
    // ------------------------------------------------------------------------
    {
        quint64 signature;
        quint32 fmt;
        quint32 w;
        quint32 h;
        char    data[0];

        static quint64 theSignature();
    }
    __attribute__((packed));

    int     w, h;

    QImage  raw;

    void *  compressed;
    int     sz;
    GLint   fmt;
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
                  bool mipmap, bool compress);
    ~CachedTexture();

    bool            load();
    void            unload();

    void            transfer();
    void            purgeGL();

    void            purge();

    GLuint          bind();

    bool            loaded()
    {
        return (image.compressed || !image.raw.isNull());
    }
    bool            transferred() { return (GLsize != 0); }

    void            reload();

public slots:
    void            onFileCreated(const QString &path,
                                  const QString &canonicalPath);
    void            onFileChanged(const QString &path,
                                  const QString &canonicalPath);
    void            onFileDeleted(const QString &path);

private slots:
    void            checkReply(QNetworkReply *reply);

private:
    std::ostream &  debug();
    void            savedCompressedTexture();

public:
    QString         path, canonicalPath;
    GLuint          id;
    int             width, height;
    bool            mipmap, compress;
    bool            isDefaultTexture;

private:
    TextureCache &  cache;
    int             GLsize;
    Links           memLRU, GLmemLRU;

    Image           image;
    bool            saveCompressed;
    bool            networked;
    QNetworkReply  *networkReply;

    bool            inLoad;
};





class TextureCache : public QObject
// ----------------------------------------------------------------------------
//    Singleton. Reads image files from disk, create textures, cache them
// ----------------------------------------------------------------------------
{
    Q_OBJECT

    friend class CachedTexture;

public:
    static QSharedPointer<TextureCache> instance();

public:
    // Primitives
    static XL::Name_p    textureMipmap(bool enable);
    static XL::Name_p    textureCompress(bool enable);

    static XL::Integer_p textureCacheMemSize(quint64 bytes);
    static XL::Integer_p textureCacheGLSize(quint64 bytes);

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
    quint64         maxMem()    { return maxMemSize; }
    quint64         maxGLMem()  { return maxGLSize; }

    int             textureChangedEvent() { return texChangedEvent; }

    bool            supported(GLuint fmt) { return cmpFormats.contains(fmt); }

public slots:
    void            clear();
    void            purge();
    void            setMaxMemSize(quint64 bytes){ maxMemSize = bytes; }
    void            setMaxGLSize(quint64 bytes) { maxGLSize  = bytes; }
    void            setMipmap(bool enable)      { mipmap = enable; purge(); }
    void            setCompression(bool enable) { compress = enable; purge(); }
    void            setSaveCompressed(bool enable) { saveCompressed = enable;
                                                     if (saveCompressed)
                                                         compress = true;
                                                     purge(); }
    void            setMinFilter(GLenum filter) { minFilt = filter; }
    void            setMagFilter(GLenum filter) { magFilt = filter; }

private:
    // LRU list management
    struct LRU
    {
        LRU() : first(0), last(0) {}
        CachedTexture::Links * first, * last;
    };

    void            reload(CachedTexture * tex);
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
    QMap <QString, CachedTexture *>  fromName;
    QMap <GLuint, CachedTexture *>   fromId;
    LRU                              memLRU, GL_LRU;
    quint64                          memSize, GLSize, maxMemSize, maxGLSize;
    float                            purgeRatio;
    QTimer                           statTimer;

    // Default settings for new textures
    bool                             mipmap, compress;
    // Min/mag filters to use
    GLenum                           minFilt, magFilt;

    // Network access manager for all texture network accesses
    QNetworkAccessManager            network;
    int                              texChangedEvent;

    // Enables reloading files as they change
    FileMonitor                      fileMonitor;

    // Caching compressed textures to disk
    bool                             saveCompressed;
    QSet<GLuint>                     cmpFormats;

private:
    static QWeakPointer<TextureCache> textureCache;
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
