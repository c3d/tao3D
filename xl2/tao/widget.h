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

#include <GL/glew.h>
#include <QtOpenGL>
#include <QImage>
#include <QTimeLine>
#include <QTimer>
#include <QSvgRenderer>
#include <QList>
#include <iostream>
#include "main.h"
#include "tao.h"
#include "text_flow.h"
#include "treeholder.h"
#include "coords3d.h"

namespace Tao {

struct Window;
struct Frame;
struct FrameInfo;
struct Activity;


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
    void        keyPressEvent(QKeyEvent *event);
    void        keyReleaseEvent(QKeyEvent *event);
    void        mousePressEvent(QMouseEvent *);
    void        mouseReleaseEvent(QMouseEvent *);
    void        mouseDoubleClickEvent(QMouseEvent *);
    void        mouseMoveEvent(QMouseEvent *);
    void        timerEvent(QTimerEvent *);
    void        wheelEvent(QWheelEvent *);
    
public:
    void        initializeGL();
    void        resizeGL(int width, int height);
    void        paintGL();
    void        setup(double w, double h, Box *picking = NULL);
    void        setupGL();
    Point3      unproject (coord x, coord y, coord z = 0.0);
    void        initMenu();
    void        updateProgram(XL::SourceFile *sf);
    GLuint      shapeId();
    bool        selected();
    void        select();
    void        drawSelection(const Box3 &bounds);
    void        loadName(bool load);

public slots:
    void        draw();
    void        runProgram();
    void        clearActions();
    void        appFocusChanged(QWidget *prev, QWidget *next);

public:
    typedef XL::Tree    Tree;
    typedef XL::Integer Integer;
    typedef XL::Real    Real;
    typedef XL::Text    Text;
    typedef XL::Name    Name;

    // XLR entry points
    static Widget *Tao() { return current; }
    Tree *status(Tree *self, text t);

    Tree *rotate(Tree *self, double ra, double rx, double ry, double rz);
    Tree *translate(Tree *self, double x, double y, double z);
    Tree *scale(Tree *self, double x, double y, double z);

    Tree *locally(Tree *self, Tree *t);
    Tree *pagesize(Tree *self, uint w, uint h);
    Tree *page(Tree *self, Tree *p);
    Integer *page_width(Tree *self);
    Integer *page_height(Tree *self);

    Tree *refresh(Tree *self, double delay);
    Tree *time(Tree *self);
    Tree *page_time(Tree *self);
    Name *selectable(Tree *self, bool selectable);

    Tree *color(Tree *self, double r, double g, double b, double a);
    Tree *textColor(Tree *self, double r,double g,double b,double a, bool fg);
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
    Tree *framePaint(Tree *self, double x, double y, double w, double h);
    Tree *urlTexture(Tree *self, double x, double y, Text *s, Integer *p);
    Tree *urlPaint(Tree *self, double x, double y, double w, double h,
                   Text *s, Integer *p);
    Tree *lineEditTexture(Tree *self, double x, double y, Text *s);
    Tree *lineEdit(Tree *self, double x, double y, double w, double h, Text *s);

    Tree *qtrectangle(Tree *self, double x, double y, double w, double h);
    Tree *qttext(Tree *self, double x, double y, text s);

    Tree *KmoveTo(Tree *self, double x, double y);
    Tree *Ktext(Tree *self, text s);
    Tree *Krectangle(Tree *self, double x, double y, double w, double h);
    Tree *Kstroke(Tree *self);
    Tree *Kclear(Tree *self);
    Tree *KlayoutText(Tree *self, text s);
    Tree *KlayoutMarkup(Tree *self, text s);

    Tree *menuItem(Tree *self, text s, Tree *t);
    Tree *focus(Tree *self);

    // Focus management
    void              setFocus();
    void              requestFocus(QWidget *widget);

private:
    void widgetVertex(double x, double y, double tx, double ty);
    void circularVertex(double cx, double cy, double r,
                double x, double y,
                double tx0, double ty0, double tx1, double ty1);
    void circularSectorN(double cx, double cy, double r,
                double tx0, double ty0, double tx1, double ty1,
                int sq, int nq);

public:
    // XL Runtime
    XL::SourceFile   *xlProgram;
    QTimer            timer;
    QMenu             contextMenu;
    QList<TreeHolder> actions;
    Frame *           frame;
    Frame *           mainFrame;
    Activity *        activities;
    double            page_start_time;
    GLuint            id, capacity;
    std::set<GLuint>  selection, savedSelection;
    QEvent *          event;

    // Timing for drawing
    ulonglong         tmin, tmax, tsum, tcount;
    ulonglong         now();
    ulonglong         elapsed(ulonglong since, bool stats=true, bool show=true);

    struct State
    // ------------------------------------------------------------------------
    //    State that is preserved by 'locally'
    // ------------------------------------------------------------------------
    {
        GLuint          polygonMode;
        GLuint          frameWidth, frameHeight;
        TextFlow *      flow;
        QTextCharFormat charFormat;  // Font, color, ...
        QPaintDevice *  paintDevice;
        bool            selectable;
    } state;

    static Widget    *current;

    typedef XL::LocalSave<QEvent *> EventSave;
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


inline QString Utf8(text utf8, uint index = 0)
// ----------------------------------------------------------------------------
//    Convert our internal UTF-8 encoded strings to QString format
// ----------------------------------------------------------------------------
{
    kstring data = utf8.data();
    uint len = utf8.length();
    len = len > index ? len - index : 0;
    index = index < len ? index : 0;
    return QString::fromUtf8(data + index, len);
}


inline double CurrentTime()
// ----------------------------------------------------------------------------
//    Return the current time
// ----------------------------------------------------------------------------
{
    QTime t = QTime::currentTime();
    double d = (3600.0	 * t.hour()
		+ 60.0	 * t.minute()
		+	   t.second()
		+  0.001 * t.msec());
    return d;
}


#define TAO(x)  (Tao::Widget::Tao() ? Tao::Widget::Tao()->x : 0)
#define RTAO(x) return TAO(x)

} // namespace Tao


#endif // TAO_WIDGET_H
