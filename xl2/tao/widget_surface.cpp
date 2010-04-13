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
#include "tao_utf8.h"
#include <QtWebKit>
#include <phonon>
#include <cstring>
#include <string>

TAO_BEGIN


// ============================================================================
//
//   Names we use from XL namespace
//
// ============================================================================

typedef XL::SymbolsInfo SymbolsInfo;
typedef XL::Symbols     Symbols;



// ============================================================================
//
//    WidgetSurface - Surface for any kind of QWidget
//
// ============================================================================

WidgetSurface::WidgetSurface(XL::Tree *t, QWidget *widget)
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
    if (tree)
    {
        tree->Remove<WidgetSurface>(this);
    }
    if (widget)
    {
        IFTRACE(widgets)
        {
            std::cerr << "DESTRUCTOR this : "<< this << " --- Widget : "
                    << widget << "\n";
        }
        Widget *parent = (Widget *) widget->parent();
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

WebViewSurface::WebViewSurface(XL::Tree *self, Widget *parent)
// ----------------------------------------------------------------------------
//    Build the QWebView
// ----------------------------------------------------------------------------
    : WidgetSurface(self, new QWebView(parent)),
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
        webView->load(QUrl(+url));
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
        (!locallyModified && textTree->value != lineEdit->text().toStdString()))
    {
        contents = NULL;
        lineEdit->setText(+textTree->value);
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



// ============================================================================
//
//   Abstract Button
//
// ============================================================================
AbstractButtonSurface::AbstractButtonSurface(XL::Tree *t,
                                             QAbstractButton * button)
// ----------------------------------------------------------------------------
//    Create the Abstract Button surface
// ----------------------------------------------------------------------------
    : WidgetSurface(t, button), label(), action(XL::xl_false), isMarked(NULL)
{

    connect(button, SIGNAL(clicked(bool)),
            this,   SLOT(clicked(bool)));

    connect(button, SIGNAL(toggled(bool)),
            this,   SLOT(toggled(bool)));

    widget->setVisible(true);
}


GLuint AbstractButtonSurface::bind(XL::Text *lbl, XL::Tree *act, XL::Text *sel)
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
    action.tree = act;

    if (sel && button->isCheckable() && sel != isMarked)
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

    if (action.tree)
        xl_evaluate(action);
}

void AbstractButtonSurface::toggled(bool checked)
// ----------------------------------------------------------------------------
//    The button was clicked. Evaluate the action.
// ----------------------------------------------------------------------------
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

}


// ============================================================================
//
//   Color Chooser
//
// ============================================================================

ColorChooserSurface::ColorChooserSurface(XL::Tree *t, Widget *parent, XL::Tree *act)
// ----------------------------------------------------------------------------
//    Create the Color Chooser surface
// ----------------------------------------------------------------------------
    : WidgetSurface(t, new QColorDialog(parent)), action(act)
{
    QColorDialog *diag = (QColorDialog *) widget;
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
        std::cerr << "Color "<< col.name().toStdString()
                  << "was chosen for reference "<< action.tree
                  <<"\nand action " << action << "\n";
    }

    // We override names 'red', 'green', 'blue' and 'alpha' in the input tree
    struct ColorTreeClone : XL::TreeClone
    {
        ColorTreeClone(const QColor &c) : color(c){}
        XL::Tree *DoName(XL::Name *what)
        {
            if (what->value == "red")
                return new XL::Real(color.redF(), what->Position());
            if (what->value == "green")
                return new XL::Real(color.greenF(), what->Position());
            if (what->value == "blue")
                return new XL::Real(color.blueF(), what->Position());
            if (what->value == "alpha")
                return new XL::Real(color.alphaF(), what->Position());

            return new XL::Name(what->value, what->Position());
        }
        QColor color;
    } replacer(col);

    // The tree to be evaluated needs its own symbol table before evaluation
    XL::Tree *toBeEvaluated = action.tree;
    XL::Symbols *syms = toBeEvaluated->Get<SymbolsInfo>();
    if (!syms)
        syms = XL::Symbols::symbols;
    syms = new Symbols(syms);
    toBeEvaluated = toBeEvaluated->Do(replacer);
    toBeEvaluated->Set<SymbolsInfo>(syms);

    // Evaluate the input tree
    xl_evaluate(toBeEvaluated);

    widget->setVisible(false);
}


// ============================================================================
//
//    Font Chooser
//
// ============================================================================

FontChooserSurface::FontChooserSurface(XL::Tree *t, Widget *parent,
                                       XL::Tree *act)
// ----------------------------------------------------------------------------
//    Create the Color Chooser surface
// ----------------------------------------------------------------------------
  : WidgetSurface(t, new QFontDialog(parent)), action(act)
{
    QFontDialog *diag = (QFontDialog *) widget;
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
        std::cerr << "Font "<< ft.toString().toStdString()
                  << "was chosen for reference "<< action.tree
                  <<"\nand action " << action << "\n";
    }

    struct FontTreeClone : XL::TreeClone
    {
        FontTreeClone(const QFont &f) : font(f){}
        XL::Tree *DoName(XL::Name *what)
        {
            if (what->value == "family")
                return new XL::Text(font.family().toStdString(),
                                    "\"" ,"\"",what->Position());
            if (what->value == "pointSize")
                return new XL::Integer(font.pointSize(), what->Position());
            if (what->value == "weight")
                return new XL::Integer(font.weight(), what->Position());
            if (what->value == "italic")
            {
                return new XL::Name(font.italic() ?
                                      XL::xl_true->value :
                                      XL::xl_false->value,
                                      what->Position());
            }

            return new XL::Name(what->value, what->Position());
        }
        QFont font;
    } replacer(ft);

    // The tree to be evaluated needs its own symbol table before evaluation
    XL::Tree *toBeEvaluated = action.tree;
    XL::Symbols *syms = toBeEvaluated->Get<SymbolsInfo>();
    if (!syms)
        syms = XL::Symbols::symbols;
    syms = new Symbols(syms);
    toBeEvaluated = toBeEvaluated->Do(replacer);
    toBeEvaluated->Set<SymbolsInfo>(syms);

    // Evaluate the input tree
    xl_evaluate(toBeEvaluated);

    widget->setVisible(false);
}

// ============================================================================
//
//   Group Box
//
// ============================================================================

GroupBoxSurface::GroupBoxSurface(XL::Tree *t, Widget *parent, QGridLayout *l)
// ----------------------------------------------------------------------------
//    Create the Group Box surface
// ----------------------------------------------------------------------------
    : WidgetSurface(t, new QGroupBox(parent)), grid(l)
{
    QGroupBox *gbox = (QGroupBox *) widget;
    grid->setParent(gbox);
    gbox->setLayout(grid);
}

GroupBoxSurface::~GroupBoxSurface()
// ----------------------------------------------------------------------------
//    delete the Group Box surface
// ----------------------------------------------------------------------------
{
    QLayoutItem *child;
    while ((child = grid->takeAt(0)) != 0) {
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
        gbox->setTitle(+label);
        dirty = true;
        IFTRACE(widgets)
        {
            QLayout *l = gbox->layout();
            std::cerr << "Layout : "<<(l?l->objectName().toStdString():"No Layout")<<"\n";
        }
    }
    return WidgetSurface::bind();
}



// ============================================================================
//
//   Video Player
//
// ============================================================================

VideoPlayerSurface::VideoPlayerSurface(XL::Tree *t, Widget *parent)
// ----------------------------------------------------------------------------
//   Create the video player
// ----------------------------------------------------------------------------
    : WidgetSurface(t, new Phonon::VideoPlayer(Phonon::VideoCategory, NULL)),
      fbo(NULL)
{
    (void) parent;              // REVISIT
    widget->setVisible(true);
    widget->setAttribute(Qt::WA_DontShowOnScreen);
}


VideoPlayerSurface::~VideoPlayerSurface()
// ----------------------------------------------------------------------------
//    Stop the player and delete the frame buffer object
// ----------------------------------------------------------------------------
{
   Phonon::VideoPlayer *player = (Phonon::VideoPlayer*) widget;
   player->stop();
   delete fbo;
}


GLuint VideoPlayerSurface::bind(XL::Text *urlTree)
// ----------------------------------------------------------------------------
//    Bind the surface to the texture
// ----------------------------------------------------------------------------
{
    Phonon::VideoPlayer *player = (Phonon::VideoPlayer*) widget;
    if (!fbo ||
        fbo->width() != player->width() ||
        fbo->height() != player->height())
    {
        delete fbo;
        fbo = new QGLFramebufferObject(player->width(), player->height(),
                                       GL_TEXTURE_2D);
    }

    if (urlTree->value != url)
    {
        url = urlTree->value;
        player->play(Phonon::MediaSource(QUrl(+url)));
    }
    dirty = true;

    fbo->bind();
    widget->render(fbo);
    fbo->release();

    return fbo->texture();
}

TAO_END
