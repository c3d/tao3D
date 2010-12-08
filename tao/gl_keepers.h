#ifndef GL_KEEPERS_H
#define GL_KEEPERS_H
// ****************************************************************************
//  gl_state_keepr.h                                                Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Christophe de Dinechin <ddd@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include <QtOpenGL>
#include "tao.h"
#include "widget.h"


TAO_BEGIN

struct GLAttribKeeper
// ----------------------------------------------------------------------------
//   Save and restore selected OpenGL attributes
// ----------------------------------------------------------------------------
//   By default, all attributes are saved (GL_ALL_ATTRIB_BITS)
{
    GLAttribKeeper(GLbitfield bits = GL_ALL_ATTRIB_BITS)
        : bits(bits)
    {
        if (bits)
            glPushAttrib(bits);
    }
    ~GLAttribKeeper()
    {
        if (bits)
            glPopAttrib();
    }

private:
    GLbitfield  bits;
    GLAttribKeeper(const GLAttribKeeper &) {}
};


struct GLMatrixKeeper
// ----------------------------------------------------------------------------
//   Save and restore the current matrix
// ----------------------------------------------------------------------------
//   Caller is responsible for current matrix mode (model or projection view)
{
    GLMatrixKeeper(bool save=true)
        : save(save)
    {
        if (save)
            glPushMatrix();
    }
    ~GLMatrixKeeper()
    {
        if (save)
            glPopMatrix();
    }

private:
    bool        save;
    GLMatrixKeeper(const GLMatrixKeeper &) {}
};


struct GLStateKeeper
// ----------------------------------------------------------------------------
//   Save and restore both selected attributes and the current matrix
// ----------------------------------------------------------------------------
{
    GLStateKeeper(GLbitfield bits = GL_ALL_ATTRIB_BITS, bool save = true):
        attribs(bits), matrix(save) {}
    ~GLStateKeeper() {}

public:
    GLAttribKeeper attribs;
    GLMatrixKeeper matrix;

private:
    GLStateKeeper(const GLStateKeeper &other);
};


struct GLAllStateKeeper : GLStateKeeper
// ----------------------------------------------------------------------------
//   Save and restore both selected attributes and the current matrices
// ----------------------------------------------------------------------------
{
    GLAllStateKeeper(GLbitfield bits = GL_ALL_ATTRIB_BITS,
                     bool saveModel = true,
                     bool saveProjection = true,
                     bool saveTextureMatrix = true)
        : GLStateKeeper(bits, saveModel),
          saveProjection(saveProjection),
          saveTextureMatrix(saveTextureMatrix)
    {
        if (saveProjection)
        {
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
        }
        if (saveTextureMatrix)
        {
            glMatrixMode(GL_TEXTURE);
            glPushMatrix();
        }
        glMatrixMode(GL_MODELVIEW);
    }
    ~GLAllStateKeeper()
    {
        if (saveTextureMatrix)
        {
            glMatrixMode(GL_TEXTURE);
            glPopMatrix();
        }
        if (saveProjection)
        {
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
        }
        glMatrixMode(GL_MODELVIEW);
    }

private:
    bool        saveProjection;
    bool        saveTextureMatrix;
    GLAllStateKeeper(const GLStateKeeper &other);
};

TAO_END

#endif // GL_KEEPERS_H
