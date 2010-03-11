#ifndef WEBVIEW_H
#define WEBVIEW_H
// ****************************************************************************
//  webview.h                                                       XLR project
// ****************************************************************************
//
//   File Description:
//
//    Encapsulate a web view that can be rendered as a texture
//
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
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao.h"
#include "tree.h"
#include <QtOpenGL>

class QWebView;

namespace Tao {

struct WebPage : QObject, XL::Info
// ----------------------------------------------------------------------------
//    Hold information about the web page for a given tree
// ----------------------------------------------------------------------------
{
    Q_OBJECT
public:
    typedef WebPage * data_t;
    enum { MAX_PAGES = 20 };

    WebPage(QGLWidget *w, uint width=512, uint height=512);
    ~WebPage();
    operator data_t() { return this; }
    void resize(uint width, uint height);
    void bind(text url);

    QWebView  * webView;
    text        url;
    GLuint      textureId;
    bool        dirty;

protected slots:
    void        pageRepaint(const QRect &dirty);
};

} // namespace Tao

#endif // WEBVIEW_H
