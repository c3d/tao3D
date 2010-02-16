// ****************************************************************************
//  gl_keepers.cpp                                                  XLR project
// ****************************************************************************
// 
//   File Description:
// 
//   Helper classes to save and restore OpenGL selected attributes and/or the
//   current matrix.
// 
// 
// 
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

#include "gl_keepers.h"


GLAttribKeeper::GLAttribKeeper(GLbitfield bits)
// ----------------------------------------------------------------------------
//    Save selected OpenGL attributes
// ----------------------------------------------------------------------------
{
    glPushAttrib(bits);
}


GLAttribKeeper::~GLAttribKeeper()
// ----------------------------------------------------------------------------
//    Restore saved attributes
// ----------------------------------------------------------------------------
{
    glPopAttrib();
}


GLMatrixKeeper::GLMatrixKeeper()
// ----------------------------------------------------------------------------
//    Save the current matrix
// ----------------------------------------------------------------------------
{
    glPushMatrix();
}


GLMatrixKeeper::~GLMatrixKeeper()
// ----------------------------------------------------------------------------
//    Restore the saved matrix
// ----------------------------------------------------------------------------
{
    glPopMatrix();
}


GLStateKeeper::GLStateKeeper(GLbitfield bits)
// ----------------------------------------------------------------------------
//    Save both selected attributes and the current matrix
// ----------------------------------------------------------------------------
    : attribKeeper(bits), matrixKeeper()
{}

GLStateKeeper::GLStateKeeper()
    : attribKeeper(), matrixKeeper()
{}


// ----------------------------------------------------------------------------
//    restore saved attributes and matrix
// ----------------------------------------------------------------------------
GLStateKeeper::~GLStateKeeper()
{}

