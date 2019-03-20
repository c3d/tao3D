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
// This software is licensed under the GNU General Public License v3.
// See file COPYING for details.
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao.h"
#include "tree.h"
#include "widget.h"
#include "tao_gl.h"
#include "info_trash_can.h"
#include <QMovie>

TAO_BEGIN

struct AnimatedTextureInfo : XL::Info
// ----------------------------------------------------------------------------
//    Hold information about an image texture
// ----------------------------------------------------------------------------
{
    typedef AnimatedTextureInfo *data_t;

    AnimatedTextureInfo();
    ~AnimatedTextureInfo();

    GLuint load(text img);
    GLuint bind(text img);
    operator data_t() { return this; }

public:
    GLuint id;
    uint   width, height;
    QMovie movie;
};


struct TextureIdInfo : XL::Info, InfoTrashCan
// ----------------------------------------------------------------------------
//   Info recording a given texture ID in a tree
// ----------------------------------------------------------------------------
{
    TextureIdInfo(): textureId(0)
    {
        GL.GenTextures(1, &textureId);
    }
    ~TextureIdInfo()
    {
        GL.DeleteTextures(1, &textureId);
    }
    virtual void Delete()
    {
        trash.push_back(this);
    }
    GLuint bind()
    {
        GL.BindTexture(GL_TEXTURE_2D, textureId);
        return textureId;
    }

    GLuint textureId;
};

TAO_END

#endif // texture.h

