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

#include <GL/glew.h>
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
    GLAttribKeeper(GLbitfield bits = GL_ALL_ATTRIB_BITS) { glPushAttrib(bits); }
    ~GLAttribKeeper()    { glPopAttrib(); }

private:
    GLAttribKeeper(const GLAttribKeeper &other) {}
};


struct GLMatrixKeeper
// ----------------------------------------------------------------------------
//   Save and restore the current matrix
// ----------------------------------------------------------------------------
//   Caller is responsible for current matrix mode (model or projection view)
{
    GLMatrixKeeper()    { glPushMatrix(); }
    ~GLMatrixKeeper()   { glPopMatrix(); }

private:
    GLMatrixKeeper(const GLMatrixKeeper &other) {}
};


struct GLStateKeeper
// ----------------------------------------------------------------------------
//   Save and restore both selected attributes and the current matrix
// ----------------------------------------------------------------------------
{
    GLStateKeeper(GLbitfield bits = GL_ALL_ATTRIB_BITS):
        attribs(bits), matrix() {}
    ~GLStateKeeper() {}

public:
    GLAttribKeeper attribs;
    GLMatrixKeeper matrix;

private:
    GLStateKeeper(const GLStateKeeper &other);
};


struct WidgetStateKeeper
// ----------------------------------------------------------------------------
//    Save custom attributes we have in our widget
// ----------------------------------------------------------------------------
{
    WidgetStateKeeper(Widget *w): widget(w), state(w->state) {}
    ~WidgetStateKeeper() { widget->state = state; }
    Widget *widget;
    Widget::State state;
};


struct GLAndWidgetKeeper
// ----------------------------------------------------------------------------
//    Save the state of a widget as well as the GL state
// ----------------------------------------------------------------------------
{
    GLAndWidgetKeeper(Widget *w, GLbitfield bits = GL_ALL_ATTRIB_BITS):
        widget(w), gl(bits) {}
    WidgetStateKeeper   widget;
    GLStateKeeper       gl;
};

TAO_END

#endif // GL_KEEPERS_H
