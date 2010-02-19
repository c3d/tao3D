#ifndef PAGE_H
#define PAGE_H
// ****************************************************************************
//  page.h                                                          Tao project
// ****************************************************************************
//
//   File Description:
//
//    Off-screen OpenGL rendering to a 2D texture
//
//    A 'PageInfo' associates persistent rendering data to a particular tree
//
//
//
//
//
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao.h"
#include "tree.h"
#include "gl_keepers.h"
#include <map>
#include <QtOpenGL>


TAO_BEGIN

struct PageInfo : XL::Info
// ----------------------------------------------------------------------------
//    Information about a given page being rendered in a dynamic texture
// ----------------------------------------------------------------------------
//    A PageInfo structure tends to be permanently attached to a tree
//    to record information that we don't want to re-create / destroy
//    at each and every evaluation of the tree
{
    typedef PageInfo *data_t;

    PageInfo(uint width = 512, uint height = 512);
    ~PageInfo();

    void resize(uint width, uint height);
    void begin();
    void end();
    void bind();

    QGLFramebufferObject *render_fbo;
    QGLFramebufferObject *texture_fbo;
};


struct PagePainter : QPainter
// ----------------------------------------------------------------------------
//   Paint on a given page, given as a PageInfo
// ----------------------------------------------------------------------------
//   A PagePainter structure is a transient rendering mechanism for a PageInfo
{
    PagePainter(PageInfo *info);
    ~PagePainter();
    PageInfo *info;
    GLStateKeeper save;
};

TAO_END

#endif // PAGE_H
