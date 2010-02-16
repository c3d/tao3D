#ifndef GL_KEEPERS_H
#define GL_KEEPERS_H
// ****************************************************************************
//  gl_state_keepr.h                                                XLR project
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
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include <QtOpenGL>


class GLAttribKeeper
// ----------------------------------------------------------------------------
//   Save and restore selected OpenGL attributes
// ----------------------------------------------------------------------------
{
public:
    // By default, all attributes are saved
    GLAttribKeeper(GLbitfield bits = GL_ALL_ATTRIB_BITS);
    ~GLAttribKeeper();
};


class GLMatrixKeeper
// ----------------------------------------------------------------------------
//   Save and restore the current matrix
// ----------------------------------------------------------------------------
{
public:
    GLMatrixKeeper();
    ~GLMatrixKeeper();
};


class GLStateKeeper
// ----------------------------------------------------------------------------
//   Save and restore both selected attributes and the current matrix
// ----------------------------------------------------------------------------
{
public:
    GLStateKeeper(GLbitfield bits);
    // Default constructor will save all attributes
    GLStateKeeper();
    ~GLStateKeeper();

public:
    GLAttribKeeper attribKeeper;
    GLMatrixKeeper matrixKeeper;
};

#endif // GL_KEEPERS_H
