// ****************************************************************************
//  webview.cpp                                                     XLR project
// ****************************************************************************
//
//   File Description:
//
//     Render a web page into a texture
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

#include "webview.h"
#include "gl_keepers.h"
#include <QtWebKit>


TAO_BEGIN

WebPage::WebPage(Widget *w, uint width, uint height)
// ----------------------------------------------------------------------------
//   Create a renderer with the right size
// ----------------------------------------------------------------------------
    : webView(NULL), url(""),
      textureId(0), dirty(true)
{
    webView = new QWebView(w);
    connect(webView->page(),
            SIGNAL(repaintRequested(const QRect &)),
            this,
            SLOT(pageRepaint(const QRect &)));

    glGenTextures(1, &textureId);
}


WebPage::~WebPage()
// ----------------------------------------------------------------------------
//   When deleting the info, delete all renderers we have
// ----------------------------------------------------------------------------
{
    delete webView;
    glDeleteTextures(1, &textureId);
}


void WebPage::resize(uint w, uint h)
// ----------------------------------------------------------------------------
//   Resize the FBO and if necessary request a repaint
// ----------------------------------------------------------------------------
{
    // Resizing the web view should trigger a repaint
    if (w != webView->width() || h != webView->height())
        webView->resize(w, h);
}


void WebPage::bind (text url)
// ----------------------------------------------------------------------------
//    Activate a given SVG renderer
// ----------------------------------------------------------------------------
{
    if (url != this->url)
    {
        webView->load(QUrl(QString::fromStdString(url)));
        this->url = url;
    }

    if (dirty)
    {
        QImage image(webView->width(), webView->height(),
                     QImage::Format_ARGB32);
        webView->render(&image);

        // Generate the GL texture
        image = QGLWidget::convertToGLFormat(image);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, 3,
                     image.width(), image.height(), 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, image.bits());

        dirty = false;
    }

    // Bind the texture that we got
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glEnable(GL_TEXTURE_2D);
#ifdef GL_MULTISAMPLE   // Not supported on Windows
    glEnable(GL_MULTISAMPLE);
#endif

    // Set the focus for the parent
    Widget *parent = (Widget *) webView->parent();
    parent->requestFocus(webView);
}


void WebPage::pageRepaint(const QRect &dirtyRect)
// ----------------------------------------------------------------------------
//   The page requests a repaint, repaint in the frame buffer object
// ----------------------------------------------------------------------------
{
    dirty = true;
}

TAO_END
