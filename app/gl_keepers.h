#ifndef GL_KEEPERS_H
#define GL_KEEPERS_H
// *****************************************************************************
// gl_keepers.h                                                    Tao3D project
// *****************************************************************************
//
// File description:
//
//   Helper classes to save and restore OpenGL selected attributes and/or the
//   current matrix.
//
//   The mask used to indicates which groups of state variables to save on the
//   attribute stack is the same than the one of glPushAttrib.
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2011-2012, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2012,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2012, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
// (C) 2011-2012, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
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

#include "tao.h"
#include "widget.h"
#include "tao_gl.h"
#include "application.h"
#include "opengl_save.h"

TAO_BEGIN

struct GLAttribKeeper : OpenGLSave
// ----------------------------------------------------------------------------
//   Save and restore selected OpenGL attributes
// ----------------------------------------------------------------------------
//   By default, all attributes are saved (GL_ALL_ATTRIB_BITS)
{
    GLAttribKeeper(GLbitfield bits = GL_ALL_ATTRIB_BITS)
        : OpenGLSave(~(STATE_mvMatrix | STATE_projMatrix | STATE_viewport))
    { (void) bits; }
};


struct GLMatrixKeeper : OpenGLSave
// ----------------------------------------------------------------------------
//   Save and restore the current matrix
// ----------------------------------------------------------------------------
//   Caller is responsible for current matrix mode (model or projection view)
{
    GLMatrixKeeper(bool save=true)
        : OpenGLSave(save ? (STATE_mvMatrix | STATE_projMatrix) : 0)
    { }
};


struct GLStateKeeper : OpenGLSave
// ----------------------------------------------------------------------------
//   Save and restore both selected attributes and the current matrix
// ----------------------------------------------------------------------------
{
    GLStateKeeper(GLbitfield bits = GL_ALL_ATTRIB_BITS, bool save = true):
        OpenGLSave(save ? OpenGLSave_All : ~(STATE_mvMatrix | STATE_projMatrix))
    { (void) bits; }
};


struct GLAllStateKeeper : OpenGLSave
// ----------------------------------------------------------------------------
//   Save and restore both selected attributes and the current matrices
// ----------------------------------------------------------------------------
{
    GLAllStateKeeper(GLbitfield bits = GL_ALL_ATTRIB_BITS,
                     bool saveModel = true,
                     bool saveProjection = true,
                     uint64 saveTextureMatrix = ~0UL)
        : OpenGLSave((saveModel         ? OpenGLSave_All : ~STATE_mvMatrix) &
                     (saveProjection    ? OpenGLSave_All : ~STATE_projMatrix))
    { (void) bits; (void) saveTextureMatrix; }
};

TAO_END

#endif // GL_KEEPERS_H
