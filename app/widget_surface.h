#ifndef WIDGET_SURFACE_H
#define WIDGET_SURFACE_H
// *****************************************************************************
// widget_surface.h                                                Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010-2012, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2011,2013-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2013, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
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

#include "tao.h"
#include "tao_tree.h"
#include "coords.h"
#include "tao_gl.h"
#include "info_trash_can.h"
#include "tao/tao_info.h"
#include <QtGui>
#include <QAbstractButton>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QAbstractSlider>

namespace Tao {

struct Widget;
struct Layout;


struct WidgetSurface : QObject, Tao::Info, InfoTrashCan
// ----------------------------------------------------------------------------
//    Hold information about the the widget associated to a given tree
// ----------------------------------------------------------------------------
{
    Q_OBJECT
public:
    WidgetSurface(Scope *scope, Tree *self, QWidget *widget, Widget *display);
    virtual ~WidgetSurface();
    virtual void Delete() { trash.push_back(this); }

    typedef WidgetSurface * data_t;
    operator data_t() { return this; }

    void                resize(uint width, uint height);
    virtual GLuint      bind();
    virtual bool        requestFocus(Layout *l, coord x, coord y);
    int                 width()         { return widget->width(); }
    int                 height()        { return widget->height(); }
    Tree *              evaluate(Scope *scope, Tree *code);
    Widget *            display()       { return parentDisplay; }

public:
    QWidget *           widget;
    Widget *            parentDisplay;
    GLuint              textureId;
    Scope_p             scope;
    Tree_p              self;
    bool                dirty;

protected slots:
    void                repaint();
};


#ifndef CFG_NO_QTWEBKIT
struct WebViewSurface : WidgetSurface
// ----------------------------------------------------------------------------
//    Hold information about a QWebView
// ----------------------------------------------------------------------------
{
    Q_OBJECT;
public:
    WebViewSurface(Scope *scope, Tree *self, Widget *parent);
    virtual GLuint      bindURL(Text *url, Integer_p progress=NULL);

    typedef WebViewSurface * data_t;
    operator data_t() { return this; }

private:
    Text_p              url;
    Integer_p           progress;
    text                inputUrl, currentUrl;

protected slots:
    void                finishedLoading(bool loadedOK);
    void                loadProgress(int progress);
};
#endif


struct LineEditSurface : WidgetSurface
// ----------------------------------------------------------------------------
//    Hold information about a QLineEdit
// ----------------------------------------------------------------------------
{
    Q_OBJECT;
public:
    LineEditSurface(Scope *scope, Text *self, Widget *parent);
    GLuint              bindText(Text *text);

    typedef LineEditSurface * data_t;
    operator data_t() { return this; }

private:
    Text_p              contents;

public slots:
    void                textChanged(const QString &text);
    void                inputValidated();
};


struct TextEditSurface : WidgetSurface
// ----------------------------------------------------------------------------
//    Hold information about a QLineEdit
// ----------------------------------------------------------------------------
{
    Q_OBJECT;
public:
    TextEditSurface(Scope *scope, Text *html, Widget *parent);
    GLuint              bindHTML(Text *html);

    typedef TextEditSurface * data_t;
    operator data_t() { return this; }

private:
    Text_p              contents;

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
    AbstractButtonSurface(Scope *scope, Tree *self,
                          QAbstractButton *button, QString name);
    virtual GLuint      bindButton(Text *lbl, Tree *action, Text *sel);

    virtual operator    data_t() { return this; }

private:
    text                label;
    Tree_p              action;
    Text_p              isMarked;

public slots:
    void                clicked(bool checked);
    void                toggled(bool checked);

};


struct PushButtonSurface : AbstractButtonSurface
// ----------------------------------------------------------------------------
//    Hold information about a QPushButton
// ----------------------------------------------------------------------------
{
    Q_OBJECT;
public:
    PushButtonSurface(Scope *scope, Tree *self, QWidget *parent, QString name):
        AbstractButtonSurface(scope, self,new QPushButton(parent), name)
    {
        connect((QPushButton*)widget, SIGNAL(clicked(bool)),
                this,                 SLOT(clicked(bool)));
    }

    typedef PushButtonSurface * data_t;
    operator data_t() { return this; }
};


struct RadioButtonSurface : AbstractButtonSurface
// ----------------------------------------------------------------------------
//    Hold information about a QPushButton
// ----------------------------------------------------------------------------
{
    Q_OBJECT;
public:
    RadioButtonSurface(Scope *scope, Tree *self, QWidget *parent, QString name):
        AbstractButtonSurface(scope, self, new QRadioButton(parent), name){}

    typedef RadioButtonSurface * data_t;
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
    CheckBoxSurface(Scope *scope, Tree *self, QWidget *parent, QString name):
        AbstractButtonSurface(scope, self, new QCheckBox(parent), name){}
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
    GroupBoxSurface(Scope *scope, Tree *self, Widget *parent);
    virtual ~GroupBoxSurface();

    operator data_t() { return this; }
    virtual GLuint bindButton(Text *lbl);
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
    GridGroupBox(QWidget* p): QGroupBox(p)
    {
        QGridLayout *grid = new QGridLayout(this);
        grid->setObjectName("groupBox layout");
        setLayout(grid);
    }
protected:
    virtual bool event(QEvent *event);

};


struct AbstractSliderSurface : WidgetSurface
// ----------------------------------------------------------------------------
//    Hold information about a Slider
// ----------------------------------------------------------------------------
{
    Q_OBJECT;
public:
    typedef AbstractSliderSurface * data_t;
    AbstractSliderSurface(Scope *scope, Tree *self, QAbstractSlider *parent);
    operator data_t() { return this; }

private:
    int min, max;
    Integer_p value;
public slots:
    void valueChanged(int new_value);

};

} // namespace Tao

#endif // WIDGET_SURFACE_H
