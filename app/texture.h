#ifndef TEXTURE_H
#define TEXTURE_H
// *****************************************************************************
// texture.h                                                       Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2011-2012, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2011,2013-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2012, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
// (C) 2011-2012, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
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

