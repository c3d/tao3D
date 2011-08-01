#ifndef TEXTURE_H
#define TEXTURE_H
// ****************************************************************************
//  texture.h                                                       Tao project
// ****************************************************************************
//
//   File Description:
//
//     Loading a texture from an image
//
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
#include "tree.h"
#include "widget.h"
#include "tao_gl.h"
#include "info_trash_can.h"

TAO_BEGIN

struct ImageTextureInfo : XL::Info
// ----------------------------------------------------------------------------
//    Hold information about an image texture
// ----------------------------------------------------------------------------
{
    typedef ImageTextureInfo *data_t;
    struct  Texture { GLuint id, width, height; };
    typedef std::map<text, Texture> texture_map;
    enum { MAX_TEXTURES = 500 };

    ImageTextureInfo();
    ~ImageTextureInfo();

    Texture load(text img);
    GLuint bind(text img);
    operator data_t() { return this; }
    uint        width, height;

    static texture_map textures;
    static Texture &defaultTexture();
};


struct AnimatedTextureInfo : ImageTextureInfo
// ----------------------------------------------------------------------------
//    Hold information about an image texture
// ----------------------------------------------------------------------------
{
    typedef AnimatedTextureInfo *data_t;

    AnimatedTextureInfo();
    ~AnimatedTextureInfo();

    Texture load(text img);
    GLuint bind(text img);
    operator data_t() { return this; }

public:
    QMovie movie;
};

struct TextureIdInfo : XL::Info, InfoTrashCan
// ----------------------------------------------------------------------------
//   Info recording a given texture ID in a tree
// ----------------------------------------------------------------------------
{
    TextureIdInfo(): textureId(0)
    {
        glGenTextures(1, &textureId);
    }
    ~TextureIdInfo()
    {
        glDeleteTextures(1, &textureId);
    }
    virtual void Delete()
    {
        trash.push_back(this);
    }
    GLuint bind()
    {
        glBindTexture(GL_TEXTURE_2D, textureId);
        return textureId;
    }

    GLuint textureId;
};

TAO_END

#endif // texture.h

