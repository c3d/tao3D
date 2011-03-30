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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao.h"
#include "tao_tree.h"
#include "coords.h"
#include "tao_gl.h"
#include <phonon>

class QWidget;

namespace Tao {

struct Widget;
struct Layout;


struct WidgetSurface : QObject, XL::Info
// ----------------------------------------------------------------------------
//    Hold information about the the widget associated to a given tree
// ----------------------------------------------------------------------------
{
    Q_OBJECT
public:
    typedef WidgetSurface * data_t;

    WidgetSurface(Tree *t, QWidget *widget);
    virtual ~WidgetSurface();

    operator            data_t() { return this; }
    void                resize(uint width, uint height);
    virtual GLuint      bind();
    virtual bool        requestFocus(Layout *l, coord x, coord y);
    int                 width()         { return widget->width(); }
    int                 height()        { return widget->height(); }

    QWidget *           widget;
    GLuint              textureId;
    bool                dirty;
    XL::Tree_p          tree;

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
    WebViewSurface(XL::Tree *t, Widget *parent);
    operator data_t() { return this; }
    virtual GLuint bind(XL::Text *url, XL::Integer_p progress=NULL);

private:
    XL::Text_p    url;
    XL::Integer_p progress;
    text          inputUrl, currentUrl;

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
    LineEditSurface(XL::Tree *t, Widget *parent, bool immed=false);
    operator data_t() { return this; }
    virtual GLuint bind(XL::Text *contents);

private:
    XL::Text_p contents;
    bool      immediate;
    bool      locallyModified;

public slots:
    void textChanged(const QString &text);
    void inputValidated();
};

struct TextEditSurface : WidgetSurface
// ----------------------------------------------------------------------------
//    Hold information about a QLineEdit
// ----------------------------------------------------------------------------
{
    Q_OBJECT;
public:
    typedef TextEditSurface * data_t;
    TextEditSurface(QTextDocument *doc, XL::Block *prog,
                    Widget *parent, bool immed=false);
    operator data_t() { return this; }
    virtual GLuint bind(QTextDocument *doc);

private:
    bool      immediate;
    bool      locallyModified;

public slots:
    void textChanged();
};



struct AbstractButtonSurface : WidgetSurface
// ----------------------------------------------------------------------------
//    Hold information about a Button
// ----------------------------------------------------------------------------
{
    Q_OBJECT;
public:
//    typedef AbstractButtonSurface * data_t;
    AbstractButtonSurface(XL::Tree *t, QAbstractButton *button, QString name);
    virtual GLuint bind(XL::Text *lbl, XL::Tree *action, XL::Text *sel);
    virtual operator data_t() { return this; }

private:
    text       label;
    XL::Tree_p action;
    XL::Text_p isMarked;

public slots:
    void clicked(bool checked);
    void toggled(bool checked);

};


struct PushButtonSurface : AbstractButtonSurface
// ----------------------------------------------------------------------------
//    Hold information about a QPushButton
// ----------------------------------------------------------------------------
{
    Q_OBJECT;
public:
    typedef PushButtonSurface * data_t;
    PushButtonSurface(XL::Tree *t, QWidget *parent, QString name):
        AbstractButtonSurface(t,new QPushButton(parent), name)
    {
        connect((QPushButton*)widget, SIGNAL(clicked(bool)),
                this,                 SLOT(clicked(bool)));
    }
    operator data_t() { return this; }
};


struct RadioButtonSurface : AbstractButtonSurface
// ----------------------------------------------------------------------------
//    Hold information about a QPushButton
// ----------------------------------------------------------------------------
{
    Q_OBJECT;
public:
    typedef RadioButtonSurface * data_t;
    RadioButtonSurface(XL::Tree *t, QWidget *parent, QString name):
        AbstractButtonSurface(t, new QRadioButton(parent), name){}
    operator data_t() { return this; }
};


struct CheckBoxSurface : AbstractButtonSurface
// ----------------------------------------------------------------------------
//    Hold information about a QCheckBox button
// ----------------------------------------------------------------------------
{
    Q_OBJECT;
public:
    typedef CheckBoxSurface * data_t;
    CheckBoxSurface(XL::Tree *t, QWidget *parent, QString name):
        AbstractButtonSurface(t, new QCheckBox(parent), name){}
    operator data_t() { return this; }
};


struct GroupBoxSurface : WidgetSurface
// ----------------------------------------------------------------------------
//    Hold information about a QGroupBox
// ----------------------------------------------------------------------------
{
    Q_OBJECT;
public:
    typedef GroupBoxSurface * data_t;
    GroupBoxSurface(XL::Tree *t, Widget *parent);
    virtual ~GroupBoxSurface();

    operator data_t() { return this; }
    virtual GLuint bind(XL::Text *lbl);
    QGridLayout *grid(){
        return (QGridLayout*)widget->layout();
     }


private:
    text label;
};


struct GridGroupBox : QGroupBox
// ----------------------------------------------------------------------------
//    A specialized QGroupBox with a QGridLayout
// ----------------------------------------------------------------------------
{
    GridGroupBox(QWidget* p):
            QGroupBox(p)
    {
        QGridLayout *grid = new QGridLayout(this);
        grid->setObjectName("groupBox layout");
        setLayout(grid);
    }
protected:
    virtual bool event(QEvent *event);

};


struct ColorChooserSurface : WidgetSurface
// ----------------------------------------------------------------------------
//    Hold information about a QColorChooser
// ----------------------------------------------------------------------------
{
    Q_OBJECT;
public:
    typedef ColorChooserSurface * data_t;
    ColorChooserSurface(XL::Tree *t, Widget *parent, XL::Tree *action);
    operator data_t() { return this; }
    virtual GLuint bind();

private:
    XL::Tree_p action;
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
    FontChooserSurface(XL::Tree *t, Widget *parent, XL::Tree *action);
    operator data_t() { return this; }
    virtual GLuint bind();

private:
    XL::Tree_p action;
public slots:
    void fontChosen(const QFont &font);
};


struct FileChooserSurface : WidgetSurface
// ----------------------------------------------------------------------------
//    Hold information about a QFontChooser
// ----------------------------------------------------------------------------
{
    Q_OBJECT;
public:
    typedef FileChooserSurface * data_t;
    FileChooserSurface(XL::Tree *t, Widget *parent);
    operator data_t() { return this; }
    virtual GLuint bind();

public slots:
    void hideWidget();
};


struct VideoSurface : WidgetSurface
// ----------------------------------------------------------------------------
//    Display a Phonon::VideoWidget
// ----------------------------------------------------------------------------
{
    Q_OBJECT;
public:
    typedef VideoSurface * data_t;
    VideoSurface(XL::Tree *t, Widget *parent);
    ~VideoSurface();
    operator data_t() { return this; }
    virtual GLuint bind(XL::Text *url);
public:
    text url;
    QGLFramebufferObject *fbo;
    Phonon::AudioOutput *audio;
    Phonon::MediaObject *media;
};


struct AbstractSliderSurface : WidgetSurface
// ----------------------------------------------------------------------------
//    Hold information about a Slider
// ----------------------------------------------------------------------------
{
    Q_OBJECT;
public:
    typedef AbstractSliderSurface * data_t;
    AbstractSliderSurface(XL::Tree *t, QAbstractSlider *parent);
    operator data_t() { return this; }
//    virtual GLuint bind();

private:
    int min, max;
    XL::Integer_p value;
public slots:
    void valueChanged(int new_value);

};

} // namespace Tao

#endif // WIDGET_SURFACE_H
