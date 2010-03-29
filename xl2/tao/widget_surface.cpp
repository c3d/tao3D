// ****************************************************************************
//  widget_surface.cpp                                              Tao project
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

#include "widget_surface.h"
#include "gl_keepers.h"
#include "runtime.h"
#include <QtWebKit>


TAO_BEGIN


// ============================================================================
//
//    WidgetSurface - Surface for any kind of QWidget
//
// ============================================================================

WidgetSurface::WidgetSurface(QWidget *widget)
// ----------------------------------------------------------------------------
//   Create a renderer with the right size
// ----------------------------------------------------------------------------
    : widget(widget), textureId(0), dirty(true)
{
    widget->hide();
    glGenTextures(1, &textureId);
}


WidgetSurface::~WidgetSurface()
// ----------------------------------------------------------------------------
//   When deleting the info, delete all renderers we have
// ----------------------------------------------------------------------------
{
    delete widget;
    glDeleteTextures(1, &textureId);
}


void WidgetSurface::resize(uint w, uint h)
// ----------------------------------------------------------------------------
//   Resize the FBO and if necessary request a repaint
// ----------------------------------------------------------------------------
{
    // Resizing the widget should trigger a repaint
    if (int(w) != widget->width() || int(h) != widget->height())
    {
        widget->resize(w, h);
        dirty = true;
    }
}


GLuint WidgetSurface::bind ()
// ----------------------------------------------------------------------------
//    Activate a given widget
// ----------------------------------------------------------------------------
{
    if (dirty)
    {
        QImage image(widget->width(), widget->height(),
                     QImage::Format_ARGB32);
        widget->render(&image);

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

    return textureId;
}


void WidgetSurface::requestFocus(coord x, coord y)
// ----------------------------------------------------------------------------
//    If the widget is selected, request focus
// ----------------------------------------------------------------------------
{
    // Request focus for this widget
    Widget *parent = (Widget *) widget->parent();
    if (parent->selected())
        parent->requestFocus(widget, x, y);
}


void WidgetSurface::repaint()
// ----------------------------------------------------------------------------
//   The page requests a repaint, repaint in the frame buffer object
// ----------------------------------------------------------------------------
{
    dirty = true;
}



// ============================================================================
//
//   WebViewSurface - Specialization for QWebView
//
// ============================================================================

WebViewSurface::WebViewSurface(Widget *parent)
// ----------------------------------------------------------------------------
//    Build the QWebView
// ----------------------------------------------------------------------------
    : WidgetSurface(new QWebView(parent)),
      urlTree(NULL), url(""), progress(0)
{
    QWebView *webView = (QWebView *) widget;
    connect(webView->page(),    SIGNAL(repaintRequested(const QRect &)),
            this,               SLOT(repaint()));
    connect(webView,            SIGNAL(loadFinished(bool)),
            this,               SLOT(finishedLoading(bool)));
    connect(webView,            SIGNAL(loadProgress(int)),
            this,               SLOT(loadProgress(int)));
}


GLuint WebViewSurface::bind(XL::Text *urlTree, XL::Integer *progressTree)
// ----------------------------------------------------------------------------
//    Update depending on URL changes, then bind texture
// ----------------------------------------------------------------------------
{
    if (progress != progressTree)
        progress = progressTree;

    if (urlTree != this->urlTree || urlTree->value != url)
    {
        QWebView *webView = (QWebView *) widget;
        url = urlTree->value;
        this->urlTree = urlTree;
        webView->load(QUrl(QString::fromStdString(url)));
        loadProgress(0);
    }

    return WidgetSurface::bind();
}


void WebViewSurface::finishedLoading (bool loadedOK)
// ----------------------------------------------------------------------------
//   When we loaded the surface, update the URL
// ----------------------------------------------------------------------------
{
    loadProgress(loadedOK ? 100 : 100);
}


void WebViewSurface::loadProgress(int progressPercent)
// ----------------------------------------------------------------------------
//   If we have a progress status, update it
// ----------------------------------------------------------------------------
{
    if (urlTree)
    {
        QWebView *webView = (QWebView *) widget;
        if (webView->url().isValid() && progressPercent >= 20)
        {
            url = webView->url().toString().toStdString();
            urlTree->value = url;
        }
    }
    if (progress)
        progress->value = progressPercent;
}



// ============================================================================
//
//   LineEditSurface - Specialization for QLineView
//
// ============================================================================

LineEditSurface::LineEditSurface(Widget *parent, bool immed)
// ----------------------------------------------------------------------------
//    Build the QLineEdit
// ----------------------------------------------------------------------------
    : WidgetSurface(new QLineEdit(parent)),
      contents(NULL), immediate(immed), locallyModified(false)
{
    QLineEdit *lineEdit = (QLineEdit *) widget;
    lineEdit->setFrame(true);
    lineEdit->setReadOnly(false);
    lineEdit->selectAll();
    connect(lineEdit, SIGNAL(textChanged(const QString &)),
            this,     SLOT(textChanged(const QString &)));
    connect(lineEdit, SIGNAL(editingFinished()),
            this,     SLOT(repaint()));
    connect(lineEdit, SIGNAL(selectionChanged()),
            this,     SLOT(repaint()));
    connect(lineEdit, SIGNAL(cursorPositionChanged(int, int)),
            this,     SLOT(repaint()));
    connect(lineEdit, SIGNAL(returnPressed()),
            this,     SLOT(inputValidated()));
}


GLuint LineEditSurface::bind(XL::Text *textTree)
// ----------------------------------------------------------------------------
//    Update text based on text changes
// ----------------------------------------------------------------------------
{
    QLineEdit *lineEdit = (QLineEdit *) widget;
    if (contents != textTree ||
        (!locallyModified && textTree->value != lineEdit->text().toStdString()))
    {
        contents = NULL;
        lineEdit->setText(QString::fromStdString(textTree->value));
        contents = textTree;

        // Record the change
        Widget *parent = (Widget *) widget->parent();
        parent->markChanged("Text change");
    }

    return WidgetSurface::bind();
}


void LineEditSurface::textChanged(const QString &text)
// ----------------------------------------------------------------------------
//    If the text changed, update the associated XL::Text
// ----------------------------------------------------------------------------
{
    if (contents)
    {
        if (immediate)
        {
            contents->value = text.toStdString();
            locallyModified = false;
        }
        else
        {
            locallyModified = true;
        }

        // Record the change
        Widget *parent = (Widget *) widget->parent();
        parent->markChanged("Text change");
    }

    repaint();
}


void LineEditSurface::inputValidated()
// ----------------------------------------------------------------------------
//    If the text changed, update the associated XL::Text
// ----------------------------------------------------------------------------
{
    if (contents)
    {
        QLineEdit *lineEdit = (QLineEdit *) widget;
        contents->value = lineEdit->text().toStdString();
        locallyModified = false;
    }
    repaint();
}

PushButtonSurface::PushButtonSurface(Widget *parent,
                                     XL::Text* lbl, XL::Tree *act)
// ----------------------------------------------------------------------------
//    Create the Push Button surface
// ----------------------------------------------------------------------------
  : WidgetSurface(new QPushButton(parent)), label(lbl), action(act)
{
    QPushButton *button = (QPushButton *) widget;
    if (lbl) button->setText(QString::fromStdString(lbl->value));
    connect(button, SIGNAL(clicked(bool)),
            this,     SLOT(clicked(bool)));
}

GLuint PushButtonSurface::bind(XL::Text *lbl, XL::Tree *act)
// ----------------------------------------------------------------------------
//    If the label or associated action changes
// ----------------------------------------------------------------------------
{
    QPushButton *button = (QPushButton *) widget;
    button->setText(QString::fromStdString(lbl ?lbl->value : ""));
    label = lbl;
    action = act;

    return WidgetSurface::bind();

}

#pragma GCC diagnostic ignored "-Wunused-parameter"
void PushButtonSurface::clicked(bool checked)
// ----------------------------------------------------------------------------
//    The button was clicked. Evaluate the action.
// ----------------------------------------------------------------------------
{
    IFTRACE (widgets)
    {
        std::cerr << "button "<< label->value << " was cliked\n";
    }

    if (action)
        xl_evaluate(action);
}

ColorChooserSurface::ColorChooserSurface(Widget *parent)
// ----------------------------------------------------------------------------
//    Create the Push Button surface
// ----------------------------------------------------------------------------
  : WidgetSurface(new QColorDialog(parent))
{
    QColorDialog *diag = (QColorDialog *) widget;
    connect(diag, SIGNAL(colorSelected (const QColor&)),
            this,   SLOT(colorchosen(const QColor &)));
    diag->setModal(false);
    diag->setOption(QColorDialog::ShowAlphaChannel, true);
    diag->setVisible(true);
}


void ColorChooserSurface::colorchosen(const QColor &color)
// ----------------------------------------------------------------------------
//    The button was clicked. Evaluate the action.
// ----------------------------------------------------------------------------
{
    IFTRACE (widgets)
    {
        std::cerr << "Color was chosen "<< color.name().toStdString() << "\n";
    }
    ((Widget *)parent())->lineColor(NULL,
                                color.redF(),
                                color.greenF(),
                                color.blueF(),
                                color.alphaF());
    widget->setVisible(false);
}

TAO_END
