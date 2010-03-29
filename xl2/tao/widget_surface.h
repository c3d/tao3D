#ifndef WIDGET_SURFACE_H
#define WIDGET_SURFACE_H
// ****************************************************************************
//  widget_surface.h                                                Tao project
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
#include "coords.h"
#include <GL/glew.h>
#include <QtOpenGL>

class QWidget;

namespace Tao {

struct Widget;


struct WidgetSurface : QObject, XL::Info
// ----------------------------------------------------------------------------
//    Hold information about the the widget associated to a given tree
// ----------------------------------------------------------------------------
{
    Q_OBJECT
public:
    typedef WidgetSurface * data_t;

    WidgetSurface(QWidget *widget);
    ~WidgetSurface();

    operator            data_t() { return this; }
    void                resize(uint width, uint height);
    virtual GLuint      bind();
    virtual void        requestFocus(coord x, coord y);

    QWidget *           widget;
    GLuint              textureId;
    bool                dirty;

protected slots:
    void                repaint();
};


struct WebViewSurface : WidgetSurface
// ----------------------------------------------------------------------------
//    Hold information about a QWebView
// ----------------------------------------------------------------------------
{
    Q_OBJECT;
public:
    typedef WebViewSurface * data_t;
    WebViewSurface(Widget *parent);
    operator data_t() { return this; }
    virtual GLuint bind(XL::Text *url, XL::Integer *progress=NULL);

private:
    XL::Text    *urlTree;
    text         url;
    XL::Integer *progress;

protected slots:
    void        finishedLoading(bool loadedOK);
    void        loadProgress(int progress);
};


struct LineEditSurface : WidgetSurface
// ----------------------------------------------------------------------------
//    Hold information about a QLineEdit
// ----------------------------------------------------------------------------
{
    Q_OBJECT;
public:
    typedef LineEditSurface * data_t;
    LineEditSurface(Widget *parent, bool immed=false);
    operator data_t() { return this; }
    virtual GLuint bind(XL::Text *contents);

private:
    XL::Text *contents;
    bool      immediate;
    bool      locallyModified;

public slots:
    void textChanged(const QString &text);
    void inputValidated();
};

} // namespace Tao

#endif // WIDGET_SURFACE_H
