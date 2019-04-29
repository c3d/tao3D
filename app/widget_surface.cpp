// *****************************************************************************
// widget_surface.cpp                                              Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2012, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2010-2012, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2011,2013-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2013, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

#include "widget_surface.h"
#include "gl_keepers.h"
#include "runtime.h"
#include "tao_utf8.h"
#include "application.h"
#include "apply_changes.h"
#include "tree_cloning.h"
#include "html_converter.h"
#include "widget.h"
#include <cstring>
#include <string>

#include <QLineEdit>
#include <QTextEdit>
#ifndef CFG_NO_QTWEBKIT
#include <QtWebKit>
#if QT_VERSION >= 0x050000
#include <QtWebKitWidgets/QWebView>
#endif
#endif

RECORDER(widget_surface, 16, "Widget surface (from Qt widget to texture)");

TAO_BEGIN

// ============================================================================
//
//    WidgetSurface - Surface for any kind of QWidget
//
// ============================================================================

WidgetSurface::WidgetSurface(Scope   *scope,
                             Tree    *self,
                             QWidget *widget,
                             Widget  *display)
// ----------------------------------------------------------------------------
//   Create a renderer with the right size
// ----------------------------------------------------------------------------
    : widget(widget), parentDisplay(display),
      textureId(0), scope(scope), self(self), dirty(true)
{
    widget->hide();
    GL.GenTextures(1, &textureId);
    record(widget_surface, "Constructing %p class %s widget %p texture %u",
           this, widget->metaObject()->className(), widget, textureId);
}


WidgetSurface::~WidgetSurface()
// ----------------------------------------------------------------------------
//   When deleting the info, delete all renderers we have
// ----------------------------------------------------------------------------
{
    record(widget_surface, "Delete %p widget %p texture %u",
           this, widget, textureId);
    if (widget)
    {
        if (Widget *disp = display())
            disp->deleteFocus(widget);
        delete widget;
        widget = NULL;
    }
    GL.DeleteTextures(1, &textureId);
}


Tree* WidgetSurface::evaluate(Scope *scope, Tree *code)
// ----------------------------------------------------------------------------
//   Set the executing environment and evaluate t
// ----------------------------------------------------------------------------
{
    if (Widget::current)
        return xl_evaluate(scope, code);

    // Set the environment
    if (Widget *disp = display())
    {
        // Evaluate the input tree within saved Tao widget
        Widget::TaoSave saveCurrent(Widget::current, disp);
        return xl_evaluate(scope, code);
    }
    return XL::xl_nil;
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
        QImage image(widget->width(), widget->height(), QImage::Format_ARGB32);
        image.fill(0);
        widget->setAutoFillBackground(false);
        widget->render(&image);

        // Generate the GL texture
        image = QGLWidget::convertToGLFormat(image);
        GL.BindTexture(GL_TEXTURE_2D, textureId);
        GL.TexImage2D(GL_TEXTURE_2D, 0, 4,
                     image.width(), image.height(), 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, image.bits());

        dirty = false;
    }

    // Bind the texture that we got
    GL.BindTexture(GL_TEXTURE_2D, textureId);
    GL.TexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL.TexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GL.Enable(GL_TEXTURE_2D);
    if (TaoApp->hasGLMultisample)
        GL.Enable(GL_MULTISAMPLE);
    return textureId;
}


bool WidgetSurface::requestFocus(Layout *layout, coord x, coord y)
// ----------------------------------------------------------------------------
//    If the widget is selected, request focus
// ----------------------------------------------------------------------------
{
    // Request focus for this widget
    if (Widget *disp = display())
        if (disp->focused(layout))
            return disp->requestFocus(widget, x, y);
    return false;
}


void WidgetSurface::repaint()
// ----------------------------------------------------------------------------
//   The page requests a repaint, repaint in the frame buffer object
// ----------------------------------------------------------------------------
{
    dirty = true;
    if (widget)
    {
        if (Widget *disp = display())
        {
            // disp->updateGL();
            disp->refreshNow();
        }
    }
}



// ============================================================================
//
//   WebViewSurface - Specialization for QWebView
//
// ============================================================================

#ifndef CFG_NO_QTWEBKIT

WebViewSurface::WebViewSurface(Scope *scope, Tree *self, Widget *display)
// ----------------------------------------------------------------------------
//    Build the QWebView
// ----------------------------------------------------------------------------
    : WidgetSurface(scope, self, new QWebView(NULL), display),
      url(NULL), progress(0), inputUrl(""), currentUrl("")
{
    QWebView *webView = (QWebView *) widget;
    webView->setObjectName("WebView");
    connect(webView->page(),    SIGNAL(repaintRequested(const QRect &)),
            this,               SLOT(repaint()));
    connect(webView,            SIGNAL(loadFinished(bool)),
            this,               SLOT(finishedLoading(bool)));
    connect(webView,            SIGNAL(loadProgress(int)),
            this,               SLOT(loadProgress(int)));
}


GLuint WebViewSurface::bindURL(Text *urlTree, Integer_p progressTree)
// ----------------------------------------------------------------------------
//    Update depending on URL changes, then bind texture
// ----------------------------------------------------------------------------
{
    progress = progressTree;
    url = urlTree;

    if (urlTree->value != inputUrl)
    {
        inputUrl = currentUrl = urlTree->value;
        QWebView *webView = (QWebView *) widget;
        webView->load(QUrl(+inputUrl));
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
    QWebView *webView = (QWebView *) widget;
    if (webView->url().isValid() && progressPercent >= 20)
        currentUrl = +webView->url().toString();

    if (Widget *disp = display())
    {
        if (url.Pointer() &&
            url->value != currentUrl &&
            !IsMarkedConstant(url) &&
            disp->markChange("URL change"))
        {
            // Record the change
            url->value = currentUrl;
        }
    }

    if (progress.Pointer())
        progress->value = progressPercent;
}

#endif


// ============================================================================
//
//   LineEditSurface - Specialization for QLineView
//
// ============================================================================

LineEditSurface::LineEditSurface(Scope *scope, Text *text, Widget *display)
// ----------------------------------------------------------------------------
//    Build the QLineEdit
// ----------------------------------------------------------------------------
    : WidgetSurface(scope, text, new QLineEdit(NULL), display), contents(text)
{
    QLineEdit *lineEdit = (QLineEdit *) widget;
    lineEdit->setFrame(true);
    lineEdit->setReadOnly(false);
    lineEdit->setText(+text->value);
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


GLuint LineEditSurface::bindText(Text *newText)
// ----------------------------------------------------------------------------
//    Update text based on text changes
// ----------------------------------------------------------------------------
{
    QLineEdit *lineEdit = (QLineEdit *) widget;
    if (newText && newText != contents && newText->value != contents->value)
    {
        contents = NULL;
        lineEdit->setText(+newText->value);
        contents = newText;
    }

    return WidgetSurface::bind();
}


void LineEditSurface::textChanged(const QString &)
// ----------------------------------------------------------------------------
//    If the text changed, repaint the widget (update XL text on validation)
// ----------------------------------------------------------------------------
{
    repaint();
}


void LineEditSurface::inputValidated()
// ----------------------------------------------------------------------------
//    If the text changed, update the associated Text
// ----------------------------------------------------------------------------
{
    QLineEdit *lineEdit = (QLineEdit *) widget;
    text validated = +lineEdit->text();
    if (Widget *disp = display())
    {
        if (contents &&
            contents->value != validated &&
            !IsMarkedConstant(contents) &&
            disp->markChange("Line editor text change"))
        {
            contents->value = validated;
        }
    }
    repaint();
}



// ============================================================================
//
//   TextEditSurface - Specialization for QLineView
//
// ============================================================================

TextEditSurface::TextEditSurface(Scope *scope, Text *html, Widget *display)
// ----------------------------------------------------------------------------
//    Build the QLineEdit
// ----------------------------------------------------------------------------
    : WidgetSurface(scope, html, new QTextEdit(NULL), display), contents(html)
{
    QTextEdit *textEdit = (QTextEdit *) widget;
    textEdit->setReadOnly(false);
    textEdit->setHtml(+html->value);
    textEdit->selectAll();

    connect(textEdit, SIGNAL(textChanged()),
            this,     SLOT(textChanged()));
    connect(textEdit, SIGNAL(selectionChanged()),
            this,     SLOT(repaint()));
    connect(textEdit, SIGNAL(cursorPositionChanged()),
            this,     SLOT(repaint()));
    connect(textEdit, SIGNAL(currentCharFormatChanged(QTextCharFormat)),
            this,     SLOT(textChanged()));
}


GLuint TextEditSurface::bindHTML(Text *html)
// ----------------------------------------------------------------------------
//    Update text based on text changes
// ----------------------------------------------------------------------------
{
    QTextEdit *textEdit = (QTextEdit *) widget;
    if (html && html != contents && html->value != contents->value)
    {
        contents = NULL;
        textEdit->setHtml(+html->value);
        contents = html;
    }

    return WidgetSurface::bind();
}


void TextEditSurface::textChanged()
// ----------------------------------------------------------------------------
//    If the text changed, update the associated Text
// ----------------------------------------------------------------------------
{
    // Record the change
    if (Widget *disp = display())
    {
        QTextEdit *textEdit = (QTextEdit *) widget;
        text html = +textEdit->toHtml();

        if (contents &&
            contents->value != html &&
            !IsMarkedConstant(contents) &&
            disp->markChange("Text editor text change"))
        {
            contents->value = html;
        }
    }
    repaint();
}



// ============================================================================
//
//   Abstract Button
//
// ============================================================================

AbstractButtonSurface::AbstractButtonSurface(Scope *scope, Tree *self,
                                             QAbstractButton * button,
                                             QString name)
// ----------------------------------------------------------------------------
//    Create the Abstract Button surface
// ----------------------------------------------------------------------------
    : WidgetSurface(scope, self, button, NULL),
      label(), action(XL::xl_false), isMarked(NULL)
{
    button->setObjectName(name);
    connect(button, SIGNAL(toggled(bool)),
            this,   SLOT(toggled(bool)));
    connect(button, SIGNAL(pressed()), this, SLOT(repaint())); // Bug #2420-1

    // Bug #2420
    // Widget MUST NOT be shown. It will be rendered in an image and then
    // transformed in a texture (WidgetSurface::bind procedure)
    widget->setVisible(false);
}


GLuint AbstractButtonSurface::bindButton(Text *lbl,
                                         Tree *act,
                                         Text *sel)
// ----------------------------------------------------------------------------
//    If the label or associated action changes
// ----------------------------------------------------------------------------
{
    QAbstractButton *button = (QAbstractButton *) widget;
    if (lbl->value != label)
    {
        label = lbl->value;
        button->setText(+label);
    }
    dirty = true;
    action = act;

    if (sel && button->isCheckable() &&
        strcasecmp(sel->value.c_str(),
                   isMarked ? isMarked->value.c_str():"") != 0)
    {
        button->setChecked(strcasecmp(sel->value.c_str(), "true")==0);
        isMarked = sel;
    }

    return WidgetSurface::bind();
}


void AbstractButtonSurface::clicked(bool checked)
// ----------------------------------------------------------------------------
//    The button was clicked. Evaluate the action.
// ----------------------------------------------------------------------------
{
    record(widget_surface, "Button %s was clicked (%+s)",
           label, checked ? "checked" : "not checked");

    // Evaluate the action. The actual context doesn't matter much, because
    // the action is usually a closure capturing the original context
    if (action)
        evaluate(scope, action);
    repaint();
}


void AbstractButtonSurface::toggled(bool checked)
// ----------------------------------------------------------------------------
//    The button has toggled. Evaluate the action, setting 'checked'
// ----------------------------------------------------------------------------
//    Like for the color chooser and other trees, we replace the 'checked'
//    keyword with the value we got from the user. In this case, this is
//    either true (xl_true) or false (xl_false)
{
    record(widget_surface,
           "Button %s has toggled to %+s",
           label, checked ? "checked" : "not checked");

    if (isMarked)
    {
        if (checked)
            isMarked->value = "true";
        else
            isMarked->value = "false";
    }

    if (!action)
    {
        repaint();
        return;
    }

    // Replace "checked" with true or false in input tree
    // REVISIT: Replace with a declaration of "checked" in the tree
    ToggleTreeClone replacer(checked);
    Tree *code = action->Do(replacer);

    // Evaluate the input tree
    evaluate(scope, code);
    repaint();
}



// ============================================================================
//
//   Group Box
//
// ============================================================================

GroupBoxSurface::GroupBoxSurface(Scope *scope, Tree *self, Widget *display)
// ----------------------------------------------------------------------------
//    Create the Group Box surface
// ----------------------------------------------------------------------------
    : WidgetSurface(scope, self, new GridGroupBox(NULL), display)
{}


GroupBoxSurface::~GroupBoxSurface()
// ----------------------------------------------------------------------------
//    delete the Group Box surface
// ----------------------------------------------------------------------------
{
    while (QLayoutItem *child = grid()->takeAt(0))
    {
        child->widget()->setParent(NULL);
        delete child;
    }
}


GLuint GroupBoxSurface::bindButton(Text *lbl)
// ----------------------------------------------------------------------------
//   Display the group box
// ----------------------------------------------------------------------------
{
    record(widget_surface, "Bind button label %s value %s", label, lbl->value);
    if (lbl->value != label)
    {
        label = lbl->value;
        QGroupBox *gbox = (QGroupBox *) widget;
        gbox->setObjectName(+label);
        gbox->setTitle(+label);
        dirty = true;
        record(widget_surface,
               "Bind button %s layout %s",
               label, gbox->layout()->objectName());
    }
    return WidgetSurface::bind();
}


bool GridGroupBox::event(QEvent *event)
// ----------------------------------------------------------------------------
//   In case of MouseEvent it forward the event to the desired widget
// ----------------------------------------------------------------------------
{
    record(widget_surface, "GridGroupBox receives event %d", event->type());

    switch (event->type()){
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
        {
            QGridLayout * grid = static_cast<QGridLayout *>(layout());
            QMouseEvent *evt = static_cast<QMouseEvent*>(event);

            int nx = evt->x();
            int ny = evt->y();

            for (int col = 0; col < grid->columnCount(); col++)
            {
                for(int row = 0; row < grid->rowCount(); row++)
                {
                    QRect rect = grid->cellRect(row,col);
                    if (rect.contains(nx, ny))
                    {
                        if (QLayoutItem * item = grid->itemAtPosition(row, col))
                        {
                            int cx = nx - rect.x();
                            int cy = ny - rect.y();
                            record(widget_surface,
                                   "nx=%d ny=%d cx=%d cy=%d", nx, ny, cx, cy);
                            QMouseEvent clocal(evt->type(), QPoint(cx, cy ),
                                               evt->button(), evt->buttons(),
                                               evt->modifiers());
                            bool ret = ((QObject*)item->widget())->event(&clocal);
                            ((Widget*)parent())->refreshNow();
                            return ret;
                        }
                        return QGroupBox::event(event);
                    }
                }

            }
        }
        /* Fall through */

    default:
        return QGroupBox::event(event);

    }
}



// ============================================================================
//
//   Slider TO BE DONE
//
// ============================================================================

AbstractSliderSurface::AbstractSliderSurface(Scope *scope, Tree *self,
                                             QAbstractSlider *slide)
// ----------------------------------------------------------------------------
//   Abstract slider
// ----------------------------------------------------------------------------
    : WidgetSurface(scope, self, slide, NULL), min(0), max(0), value(NULL)
{

}


void AbstractSliderSurface::valueChanged(int /* new_value */)
// ----------------------------------------------------------------------------
//   Update when the slider value changed
// ----------------------------------------------------------------------------
{

}

TAO_END
