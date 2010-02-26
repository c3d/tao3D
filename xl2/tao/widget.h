#ifndef WIDGET_H
#define WIDGET_H
// ****************************************************************************
//  widget.h                                                       Tao project
// ****************************************************************************
//
//   File Description:
//
//    The primary graphical widget used to display Tao contents
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
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include <QtOpenGL>
#include <QImage>
#include <QTimeLine>
#include <QTimer>
#include <QSvgRenderer>
#include <iostream>
#include "main.h"
#include "tao.h"
#include "text_flow.h"

namespace Tao {

struct Window;

class Widget : public QGLWidget
// ----------------------------------------------------------------------------
//   This is the widget we use to display XL programs output
// ----------------------------------------------------------------------------
{
    Q_OBJECT
public:
    Widget(Window *parent, XL::SourceFile *sf = NULL);
    ~Widget();

    // Events
    void mousePressEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void timerEvent(QTimerEvent *);
    void wheelEvent(QWheelEvent *);
    
public:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();
    void setup(double w, double h);

public slots:
    void draw();

public:
    typedef XL::Tree    Tree;
    typedef XL::Integer Integer;
    typedef XL::Real    Real;
    typedef XL::Text    Text;

    // XLR entry points
    static Widget *Tao() { return current; }
    Tree *status(Tree *self, text t);

    Tree *rotateX(Tree *self, double rx);
    Tree *rotateY(Tree *self, double ry);
    Tree *rotateZ(Tree *self, double rz);
    Tree *rotate(Tree *self, double rx, double ry, double rz, double ra);

    Tree *translateX(Tree *self, double x);
    Tree *translateY(Tree *self, double y);
    Tree *translateZ(Tree *self, double z);
    Tree *translate(Tree *self, double x, double y, double z);

    Tree *scaleX(Tree *self, double x);
    Tree *scaleY(Tree *self, double y);
    Tree *scaleZ(Tree *self, double z);
    Tree *scale(Tree *self, double x, double y, double z);

    Tree *locally(Tree *self, Tree *t);
    Tree *pagesize(Tree *self, uint w, uint h);
    Tree *page(Tree *self, Tree *p);

    Tree *refresh(Tree *self, double delay);
    Tree *time(Tree *self);

    Tree *color(Tree *self, double r, double g, double b, double a);
    Tree *filled(Tree *self);
    Tree *hollow(Tree *self);
    Tree *linewidth(Tree *self, double lw);

    Tree *polygon(Tree *self, Tree *t);
    Tree *vertex(Tree *self, double x, double y, double z);
    Tree *sphere(Tree *self,
                double cx, double cy, double cz,
                double r, int nslices, int nstacks);
    Tree *circle(Tree *self, double cx, double cy, 
                double r);
    Tree *circularSector(Tree *self, double cx, double cy, 
                double r, double a, double b);
    Tree *roundedRectangle(Tree *self, double cx, double cy, 
                double w, double h, double r);
    Tree *rectangle(Tree *self, double cx, double cy, 
                double w, double h);
    Tree *regularPolygon(Tree *self, double cx, double cy, double r, int p);
    Tree *regularStarPolygon(Tree *self, double cx, double cy, double r, 
                int p, int q);

    Tree *texture(Tree *self, text n);
    Tree *svg(Tree *self, text t);
    Tree *texCoord(Tree *self, double x, double y);

    Real *fromCm(Tree *self, double cm);
    Real *fromMm(Tree *self, double mm);
    Real *fromIn(Tree *self, double in);
    Real *fromPt(Tree *self, double pt);
    Real *fromPx(Tree *self, double px);

    Tree *font(Tree *self, text family);
    Tree *fontSize(Tree *self, double size);
    Tree *fontPlain(Tree *self);
    Tree *fontItalic(Tree *self, bool=true);
    Tree *fontBold(Tree *self, bool=true);
    Tree *fontUnderline(Tree *self, bool=true);
    Tree *fontOverline(Tree *self, bool=true);
    Tree *fontStrikeout(Tree *self, bool=true);
    Tree *fontStretch(Tree *self, int stretch);

    Tree *align(Tree *self, int align);
    Tree *textSpan(Tree *self, text content);

    Tree *flow(Tree *self);
    Tree *frameTexture(Tree *self, double w, double h);
    Tree *frame(Tree *self, double x, double y, double w, double h);

public:
    // XL Runtime
    XL::SourceFile *  xlProgram;
    QTimer            timer;
    static Widget    *current;

    struct State
    // ------------------------------------------------------------------------
    //    State that we need to save
    // ------------------------------------------------------------------------
    {
        GLuint          polygonMode;
        GLuint          frameWidth, frameHeight;
        TextFlow     *  flow;
        QTextCharFormat charFormat;
        QTextOption     textOptions;
        QPaintDevice *  paintDevice;
    } state;
};



// ============================================================================
// 
//    Simple utility functions
// 
// ============================================================================

inline void glShowErrors()
// ----------------------------------------------------------------------------
//   Display pending GL errors
// ----------------------------------------------------------------------------
{                
    GLenum err = glGetError();                                         
    while (err != GL_NO_ERROR)
    {
        std::cerr << "GL Error: " << (char *) gluErrorString(err) << "\n";
        err = glGetError();                                     
    }                                                               
}


#define TAO(x)  (Tao::Widget::Tao() ? Tao::Widget::Tao()->x : 0)
#define RTAO(x) return TAO(x)

} // namespace Tao

#endif // TAO_WIDGET_H
