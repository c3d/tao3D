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
    virtual ~WidgetSurface();

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


struct PushButtonSurface : WidgetSurface
// ----------------------------------------------------------------------------
//    Hold information about a QPushButton
// ----------------------------------------------------------------------------
{
    Q_OBJECT;
public:
    typedef PushButtonSurface * data_t;
    PushButtonSurface(Widget *parent);
    operator data_t() { return this; }
    virtual GLuint bind(XL::Text *lbl, XL::Tree *action);

private:
    text         label;
    XL::TreeRoot action;

public slots:
    void clicked(bool checked);
};


struct GroupBoxSurface : WidgetSurface
// ----------------------------------------------------------------------------
//    Hold information about a QPushButton
// ----------------------------------------------------------------------------
{
    Q_OBJECT;
public:
    typedef GroupBoxSurface * data_t;
    GroupBoxSurface(Widget *parent, QGridLayout *l);
    operator data_t() { return this; }
    virtual GLuint bind(XL::Text *lbl);

private:
    text label;
public:
    QGridLayout *grid;
};


struct ColorChooserSurface : WidgetSurface
// ----------------------------------------------------------------------------
//    Hold information about a QColorChooser
// ----------------------------------------------------------------------------
{
    Q_OBJECT;
public:
    typedef ColorChooserSurface * data_t;
    ColorChooserSurface(Widget *parent, XL::Tree *action);
    ~ColorChooserSurface();
    operator data_t() { return this; }
    virtual GLuint bind();

private:
    XL::TreeRoot action;
public slots:
    void colorChosen(const QColor &color);
};


struct FontChooserSurface : WidgetSurface
// ----------------------------------------------------------------------------
//    Hold information about a QFontChooser
// ----------------------------------------------------------------------------
{
    Q_OBJECT;
public:
    typedef FontChooserSurface * data_t;
    FontChooserSurface(Widget *parent, XL::Tree *action);
    ~FontChooserSurface();
    operator data_t() { return this; }
    virtual GLuint bind();

private:
    XL::TreeRoot action;
public slots:
    void fontChosen(const QFont &font);
};


struct VideoPlayerSurface : WidgetSurface
// ----------------------------------------------------------------------------
//    Hold information about a Phonon::VideoPlayer
// ----------------------------------------------------------------------------
{
    Q_OBJECT;
public:
    typedef VideoPlayerSurface * data_t;
    VideoPlayerSurface(Widget *parent);
    ~VideoPlayerSurface();
    operator data_t() { return this; }
    virtual GLuint bind(XL::Text *url);
public:
    text url;
    QGLFramebufferObject *fbo;
};
} // namespace Tao

#endif // WIDGET_SURFACE_H
