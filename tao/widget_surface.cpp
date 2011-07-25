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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "widget_surface.h"
#include "gl_keepers.h"
#include "runtime.h"
#include "tao_utf8.h"
#include "application.h"
#include "apply_changes.h"
#include "tree_cloning.h"
#include "text_edit.h"
#include <QtWebKit>
#include <phonon>
#include <cstring>
#include <string>

TAO_BEGIN



// ============================================================================
//
//    WidgetSurface - Surface for any kind of QWidget
//
// ============================================================================

WidgetSurface::WidgetSurface(Tree * t, QWidget *widget)
// ----------------------------------------------------------------------------
//   Create a renderer with the right size
// ----------------------------------------------------------------------------
    : widget(widget), textureId(0), dirty(true), tree(t)
{
    IFTRACE(widgets)
    {
        std::cerr << "CONSTRUCTOR this : "<< this << " --- Widget : "
                << widget << "  --- Widget class : \""<<
                widget->metaObject()->className()<< "\"\n";
    }

    widget->hide();
    glGenTextures(1, &textureId);
}


WidgetSurface::~WidgetSurface()
// ----------------------------------------------------------------------------
//   When deleting the info, delete all renderers we have
// ----------------------------------------------------------------------------
{
    if (widget)
    {
        IFTRACE(widgets)
        {
            std::cerr << "DESTRUCTOR this : "<< this << " --- Widget : "
                    << widget << "\n";
        }

        Widget *parent = dynamic_cast<Widget *>(widget->parent());
        if (parent)
            parent->deleteFocus(widget);
        delete widget;
        widget = NULL;
    }
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
        widget->setAutoFillBackground(false);
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
    if (TaoApp->hasGLMultisample)
        glEnable(GL_MULTISAMPLE);
    return textureId;
}


bool WidgetSurface::requestFocus(Layout *layout, coord x, coord y)
// ----------------------------------------------------------------------------
//    If the widget is selected, request focus
// ----------------------------------------------------------------------------
{
    // Request focus for this widget
    Widget *parent = (Widget *) widget->parent();
    if (parent && parent->focused(layout))
        return parent->requestFocus(widget, x, y);
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
        if (Widget *parent = dynamic_cast<Widget *>(widget->parent()))
        {
            //parent->updateGL();
            parent->refreshNow();
        }
    }
}


// ============================================================================
//
//   WebViewSurface - Specialization for QWebView
//
// ============================================================================

WebViewSurface::WebViewSurface(XL::Tree *self, Widget *parent)
// ----------------------------------------------------------------------------
//    Build the QWebView
// ----------------------------------------------------------------------------
    : WidgetSurface(self, new QWebView(parent)),
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


GLuint WebViewSurface::bind(XL::Text *urlTree, XL::Integer_p progressTree)
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

    Widget *parent = (Widget *) widget->parent();
    if (url.Pointer() &&
        url->value != currentUrl &&
        !IsMarkedConstant(url) &&
        !parent->markChange("URL change"))
    {
        // Record the change
        url->value = currentUrl;
    }

    if (progress.Pointer())
        progress->value = progressPercent;
}



// ============================================================================
//
//   LineEditSurface - Specialization for QLineView
//
// ============================================================================

LineEditSurface::LineEditSurface(XL::Tree *t, Widget *parent, bool immed)
// ----------------------------------------------------------------------------
//    Build the QLineEdit
// ----------------------------------------------------------------------------
    : WidgetSurface(t, new QLineEdit(parent)),
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
        (!locallyModified && textTree->value != +lineEdit->text()))
    {
        contents = NULL;
        lineEdit->setText(+textTree->value);
        contents = textTree;
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
            // Record the change
            Widget *parent = (Widget *) widget->parent();
            if (parent->markChange("Line editor text change"))
            {
                contents->value = +text;
                locallyModified = false;
            }
        }
        else
        {
            locallyModified = true;
        }

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
        contents->value = +lineEdit->text();
        locallyModified = false;
    }
    repaint();
}

// ============================================================================
//
//   TextEditSurface - Specialization for QLineView
//
// ============================================================================

TextEditSurface::TextEditSurface(QTextDocument *doc, XL::Block *t,
                                 Widget *parent, bool immed)
// ----------------------------------------------------------------------------
//    Build the QLineEdit
// ----------------------------------------------------------------------------
    : WidgetSurface(t, new QTextEdit(parent)), immediate(immed),
    locallyModified(false)
{
    QTextEdit *textEdit = (QTextEdit *) widget;
    textEdit->setDocument(doc);
    textEdit->setReadOnly(false);

    connect(textEdit, SIGNAL(textChanged()),
            this,     SLOT(textChanged()));
    connect(textEdit, SIGNAL(selectionChanged()),
            this,     SLOT(repaint()));
    connect(textEdit, SIGNAL(cursorPositionChanged()),
            this,     SLOT(repaint()));
    connect(textEdit, SIGNAL(currentCharFormatChanged(QTextCharFormat)),
            this,     SLOT(textChanged()));
}


GLuint TextEditSurface::bind(QTextDocument * doc)
// ----------------------------------------------------------------------------
//    Update text based on text changes
// ----------------------------------------------------------------------------
{
    QTextEdit *textEdit = (QTextEdit *) widget;
    QTextDocument *tmp = textEdit->document();
    if (!locallyModified && tmp != doc)
    {
        textEdit->setDocument(doc);
        textEdit->textCursor().clearSelection();
        delete tmp;
    }

    return WidgetSurface::bind();
}


void TextEditSurface::textChanged()
// ----------------------------------------------------------------------------
//    If the text changed, update the associated XL::Text
// ----------------------------------------------------------------------------
{
    if (XL::Block * prog = tree->AsBlock())
    {
        if (immediate)
        {
            // Record the change
            Widget *parent = (Widget *) widget->parent();
            if (parent->markChange("Text editor text change"))
            {
                QTextEdit *textEdit = (QTextEdit *) widget;
                text_portability p;
                prog->child = p.docToTree(*textEdit->document());
                locallyModified = false;
            }
        }
        else
        {
            locallyModified = true;
        }

    }

    repaint();
}


// ============================================================================
//
//   Abstract Button
//
// ============================================================================

AbstractButtonSurface::AbstractButtonSurface(Tree *t,
                                             QAbstractButton * button,
                                             QString name)
// ----------------------------------------------------------------------------
//    Create the Abstract Button surface
// ----------------------------------------------------------------------------
    : WidgetSurface(t, button), label(), action(XL::xl_false), isMarked(NULL)
{
    button->setObjectName(name);
    connect(button, SIGNAL(toggled(bool)),
            this,   SLOT(toggled(bool)));

    widget->setVisible(true);
}


GLuint AbstractButtonSurface::bind(XL::Text *lbl,
                                   XL::Tree *act,
                                   XL::Text *sel)
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
    IFTRACE (widgets)
    {
        std::cerr << "button "<< label
                  << " was clicked with checked = " << checked << "\n";
    }

    // Evaluate the action. The actual context doesn't matter much, because
    // the action is usually a closure capturing the original context
    if (action)
        XL::MAIN->context->Evaluate(action);
    repaint();
}


void AbstractButtonSurface::toggled(bool checked)
// ----------------------------------------------------------------------------
//    The button has toggled. Evaluate the action, setting 'checked'
// ----------------------------------------------------------------------------
//    Like for the color chooser and other trees, we replace the 'checked'
//    keyword with the value we got from the user. In this case, this is
//    either true (XL::xl_true) or false (XL::xl_false)
{
    IFTRACE (widgets)
    {
        std::cerr << "button "<< label
                  << " has toggled to " << checked << "\n";
    }

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
    XL::Tree *toBeEvaluated = action;
    XL::Symbols *symbols = toBeEvaluated->Symbols();
    toBeEvaluated = toBeEvaluated->Do(replacer);
    toBeEvaluated->SetSymbols(symbols);

    // Evaluate the input tree
    XL::MAIN->context->Evaluate(toBeEvaluated);
    repaint();
}



// ============================================================================
//
//   Color Chooser
//
// ============================================================================

ColorChooserSurface::ColorChooserSurface(XL::Tree *t,
                                         Widget *parent, XL::Tree *act)
// ----------------------------------------------------------------------------
//    Create the Color Chooser surface
// ----------------------------------------------------------------------------
    : WidgetSurface(t, new QColorDialog(parent)), action(act)
{
    QColorDialog *diag = (QColorDialog *) widget;
    diag->setObjectName("colorDialogSurface");
    connect(diag, SIGNAL(colorSelected (const QColor&)),
            this, SLOT(colorChosen(const QColor &)));
    diag->setModal(false);
    diag->setOption(QColorDialog::ShowAlphaChannel, true);
}


GLuint ColorChooserSurface::bind()
// ----------------------------------------------------------------------------
//    Activate the widget
// ----------------------------------------------------------------------------
//    At least on MacOSX, the color chooser shows in its own window
{
//    QColorDialog *diag = (QColorDialog *) widget;
//    diag->setOption(QColorDialog::DontUseNativeDialog, false);

    widget->setVisible(true);
    dirty = true;
    return WidgetSurface::bind();

}


void ColorChooserSurface::colorChosen(const QColor &col)
// ----------------------------------------------------------------------------
//    A color was selected. Evaluate the action.
// ----------------------------------------------------------------------------
{
    IFTRACE (widgets)
    {
        std::cerr << "Color "<< +col.name()
                  << "was chosen for reference "<< action
                  <<"\nand action " << action << "\n";
    }


    // Replace "red", "green", "blue" or "alpha".
    // REVISIT: Perform actual definitions of these variables
    ColorTreeClone replacer(col);
    XL::Tree *toBeEvaluated = action;
    XL::Symbols *symbols = toBeEvaluated->Symbols();
    toBeEvaluated = toBeEvaluated->Do(replacer);
    toBeEvaluated->SetSymbols(symbols);

    // Evaluate the input tree
    XL::MAIN->context->Evaluate(toBeEvaluated);

    widget->setVisible(false);
    repaint();

}



// ============================================================================
//
//    Font Chooser
//
// ============================================================================

FontChooserSurface::FontChooserSurface(XL::Tree *t,
                                       Widget *parent,
                                       XL::Tree *act)
// ----------------------------------------------------------------------------
//    Create the Font Chooser surface
// ----------------------------------------------------------------------------
  : WidgetSurface(t, new QFontDialog(parent)), action(act)
{
    QFontDialog *diag = (QFontDialog *) widget;
    diag->setObjectName("fontDialogSurface");
    connect(diag, SIGNAL(fontSelected (const QFont&)),
            this, SLOT(fontChosen(const QFont&)));
    diag->setModal(false);
}


GLuint FontChooserSurface::bind()
// ----------------------------------------------------------------------------
//   Display the font chooser
// ----------------------------------------------------------------------------
{
    widget->setVisible(true);
    return WidgetSurface::bind();
}


void FontChooserSurface::fontChosen(const QFont& ft)
// ----------------------------------------------------------------------------
//    A font was selected. Evaluate the action.
// ----------------------------------------------------------------------------
{
    IFTRACE (widgets)
    {
        std::cerr << "Font "<< +ft.toString()
                  << "was chosen for reference "<< action
                  <<"\nand action " << action << "\n";
    }

    // Replace the various keywords
    FontTreeClone replacer(ft);
    XL::Tree *toBeEvaluated = action;
    XL::Symbols *symbols = toBeEvaluated->Symbols();
    toBeEvaluated = toBeEvaluated->Do(replacer);
    toBeEvaluated->SetSymbols(symbols);

    // Evaluate the input tree
    XL::MAIN->context->Evaluate(toBeEvaluated);

    widget->setVisible(false);
    repaint();
}



// ============================================================================
//
//    File Chooser
//
// ============================================================================

FileChooserSurface::FileChooserSurface(XL::Tree *t, Widget *parent)
// ----------------------------------------------------------------------------
//    Create the File Chooser surface
// ----------------------------------------------------------------------------
  : WidgetSurface(t, new QFileDialog(parent))
{
    QFileDialog *diag = (QFileDialog *) widget;
    diag->setObjectName("fileDialogSurface");
    connect(diag, SIGNAL(fileSelected (const QString&)),
            parent, SLOT(fileChosen(const QString&)));
    connect(diag, SIGNAL(fileSelected (const QString&)),
            this, SLOT(hideWidget()));
    diag->setModal(false);
}


GLuint FileChooserSurface::bind()
// ----------------------------------------------------------------------------
//   Display the file chooser
// ----------------------------------------------------------------------------
{
    widget->setVisible(true);
    return WidgetSurface::bind();
}


void FileChooserSurface::hideWidget()
// ----------------------------------------------------------------------------
//    A file was selected. Evaluate the action.
// ----------------------------------------------------------------------------
{
     widget->setVisible(false);
     repaint();
}

// ============================================================================
//
//   Group Box
//
// ============================================================================

GroupBoxSurface::GroupBoxSurface(XL::Tree *t, Widget *parent)
// ----------------------------------------------------------------------------
//    Create the Group Box surface
// ----------------------------------------------------------------------------
    : WidgetSurface(t, new GridGroupBox(parent)){}

GroupBoxSurface::~GroupBoxSurface()
// ----------------------------------------------------------------------------
//    delete the Group Box surface
// ----------------------------------------------------------------------------
{
    QLayoutItem *child;
    while ((child = grid()->takeAt(0)) != 0) {
        child->widget()->setParent(NULL);
        delete child;
    }
}


GLuint GroupBoxSurface::bind(XL::Text *lbl)
// ----------------------------------------------------------------------------
//   Display the group box
// ----------------------------------------------------------------------------
{
    if (lbl->value != label)
    {
        label = lbl->value;
        QGroupBox *gbox = (QGroupBox *) widget;
        gbox->setObjectName(+label);
        gbox->setTitle(+label);
        dirty = true;
        IFTRACE(widgets)
        {
            QLayout *l = gbox->layout();
            std::cerr << "Layout : "
                      << (l ? +l->objectName() : "No Layout")
                      <<"\n";
        }
    }
    return WidgetSurface::bind();
}


bool GridGroupBox::event(QEvent *event)
// ----------------------------------------------------------------------------
//   In case of MouseEvent it forward the event to the desired widget
// ----------------------------------------------------------------------------
{
    IFTRACE(widgets)
            std::cerr << "GridGroupBox::event\n";
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
                            IFTRACE(widgets)
                                    std::cerr << "GridGroupBox::event cx = "
                                    << cx <<" cy = " << cy << std::endl;
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

    default:
        return QGroupBox::event(event);

    }
}



// ============================================================================
//
//   Video Player
//
// ============================================================================

VideoSurface::VideoSurface(XL::Tree *t, Widget *parent)
// ----------------------------------------------------------------------------
//   Create the video player
// ----------------------------------------------------------------------------
    : WidgetSurface(t, new Phonon::VideoWidget(NULL)),
      audio(new Phonon::AudioOutput(Phonon::VideoCategory, NULL)),
      media(new Phonon::MediaObject(NULL))
{
    (void) parent;
    Phonon::createPath(media, audio);
    Phonon::createPath(media, (Phonon::VideoWidget *) widget);
    widget->setAttribute(Qt::WA_DontShowOnScreen);
    widget->setVisible(true);
}


VideoSurface::~VideoSurface()
// ----------------------------------------------------------------------------
//    Stop the player and delete the frame buffer object
// ----------------------------------------------------------------------------
{
   media->stop();
   delete audio;
   delete media;
}


GLuint VideoSurface::bind(XL::Text *urlTree)
// ----------------------------------------------------------------------------
//    Bind the surface to the texture
// ----------------------------------------------------------------------------
{
    Phonon::VideoWidget *player = (Phonon::VideoWidget *) widget;

    if (urlTree->value != url)
    {
        url = urlTree->value;
        media->stop();
        media->setCurrentSource(Phonon::MediaSource(QUrl(+url)));
        media->play();
        Tao::Widget::Tao()->makeCurrent();
    }

    QSize hint = player->sizeHint();
    if (hint.isValid())
    {
        if (hint.width() != widget->width() ||
            hint.height() != widget->height())
            resize(hint.width(), hint.height());

        QImage image(widget->width(), widget->height(),
                     QImage::Format_ARGB32);
        widget->setAutoFillBackground(false);
        widget->render(&image);

        // Generate the GL texture
        image = QGLWidget::convertToGLFormat(image);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, 3,
                     image.width(), image.height(), 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, image.bits());

        return textureId;
    }

    // Default is to return no texture
    return 0;
}



// ============================================================================
//
//   Slider TO BE DONE
//
// ============================================================================

AbstractSliderSurface::AbstractSliderSurface(XL::Tree *t,
                                             QAbstractSlider *slide) :
    WidgetSurface(t, slide), min(0), max(0), value(NULL)
{

}

void AbstractSliderSurface::valueChanged(int /* new_value */)
{

}

TAO_END
