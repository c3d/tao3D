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

#include "main.h"
#include "tao.h"
#include "text_flow.h"
#include "treeholder.h"
#include "coords3d.h"
#include "opcodes.h"

#include <GL/glew.h>
#include <QtOpenGL>
#include <QImage>
#include <QTimeLine>
#include <QTimer>
#include <QSvgRenderer>
#include <QList>
#include <iostream>
#include <map>

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
    bool        forwardEvent(QEvent *event);
    bool        forwardEvent(QMouseEvent *event);
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
    coord       zBuffer(coord z, int pos);
    coord       bringForward(coord z) { return zBuffer(z,1); }
    coord       sendBackward(coord z) { return zBuffer(z,-1); }
    void        updateProgram(XL::SourceFile *sf);
    void        refreshProgram();
    void        markChanged(text reason);

    // Selection
    GLuint      shapeId();
    bool        selected();
    void        select();
    void        requestFocus(QWidget *widget);
    void        recordProjection();
    Point3      unproject (coord x, coord y, coord z = 0.0);
    Vector3     dragDelta();
    text        dragSelector();
    void        drawSelection(const Box3 &bounds, text selector);
    void        loadName(bool load);
    Box3        bbox(coord x, coord y, coord w, coord h);
    Box3        bbox(coord x, coord y, coord z, coord w, coord h, coord d);

private:
    double      z2b(coord z);
    double      b2z(ulong b);

public slots:
    void        dawdle();
    void        draw();
    void        runProgram();
    void        appFocusChanged(QWidget *prev, QWidget *next);
    void        userMenu(QAction *action);

public:
    typedef XL::Tree      Tree;
    typedef XL::Integer   Integer;
    typedef XL::Real      Real;
    typedef XL::Text      Text;
    typedef XL::Name      Name;
    typedef XL::real_r    real_r;
    typedef XL::integer_r integer_r;
    typedef XL::text_r    text_r;
    typedef XL::real_p    real_p;
    typedef XL::integer_p integer_p;
    typedef XL::text_p    text_p;

    // XLR entry points
    static Widget *Tao() { return current; }
    Tree *status(Tree *self, text t);

    Tree *rotate(Tree *self, double ra, double rx, double ry, double rz);
    Tree *translate(Tree *self, double x, double y, double z);
    Tree *scale(Tree *self, double x, double y, double z);
    Tree *depthDelta(Tree *self, double x);
    Name *depthTest(Tree *self, bool enable);

    Tree *locally(Tree *self, Tree *t);
    Tree *pagesize(Tree *self, uint w, uint h);
    Tree *page(Tree *self, Tree *p);
    Integer *page_width(Tree *self);
    Integer *page_height(Tree *self);

    Tree *refresh(Tree *self, double delay);
    Tree *time(Tree *self);
    Tree *page_time(Tree *self);
    Name *selectable(Tree *self, bool selectable);
    Name *selectorName(Tree *self, Text &name);

    Tree *color(Tree *self, double r, double g, double b, double a);
    Tree *textColor(Tree *self, double r,double g,double b,double a, bool fg);
    Tree *filled(Tree *self);
    Tree *hollow(Tree *self);
    Tree *linewidth(Tree *self, double lw);

    Tree *polygon(Tree *self, Tree *t);
    Tree *points(Tree *self, Tree *t);
    Tree *lines(Tree *self, Tree *t);
    Tree *line_strip(Tree *self, Tree *t);
    Tree *line_loop(Tree *self, Tree *t);
    Tree *triangles(Tree *self, Tree *t);
    Tree *triangle_fan(Tree *self, Tree *t);
    Tree *triangle_strip(Tree *self, Tree *t);
    Tree *quads(Tree *self, Tree *t);
    Tree *quad_strip(Tree *self, Tree *t);
    Tree *vertex(Tree *self, double x, double y, double z);
    Tree *sphere(Tree *self,
                 real_r cx, real_r cy, real_r cz, real_r r,
                 integer_r nslices, integer_r nstacks);
    Tree *circle(Tree *self, real_r cx, real_r cy, real_r r);
    Tree *circularSector(Tree *self, real_r cx, real_r cy, real_r r,
                         real_r a, real_r b);
    Tree *roundedRectangle(Tree *self,
                           real_r cx, real_r cy, real_r w, real_r h, real_r r);
    Tree *rectangle(Tree *self, real_r cx, real_r cy, real_r w, real_r h);
    Tree *regularStarPolygon(Tree *self, real_r cx, real_r cy, real_r r,
                integer_r p, integer_r q);

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
    Tree *framePaint(Tree *self, real_r x, real_r y, real_r w, real_r h);
    Tree *urlTexture(Tree *self, double x, double y, Text *s, Integer *p);
    Tree *urlPaint(Tree *self, real_r x, real_r y, real_r w, real_r h,
                   text_p s, integer_p p);
    Tree *lineEditTexture(Tree *self, double x, double y, Text *s);
    Tree *lineEdit(Tree *self, real_r x,real_r y, real_r w,real_r h, text_p s);

    Tree *qtrectangle(Tree *self, real_r x, real_r y, real_r w, real_r h);
    Tree *qttext(Tree *self, double x, double y, text s);

    Tree *moveTo(Tree *self, double x, double y, bool isRelative);
    Tree *Ktext(Tree *self, text s);
    Tree *Krectangle(Tree *self, real_r x, real_r y, real_r w, real_r h);
    Tree *stroke(Tree *self);
    Tree *clear(Tree *self);
    Tree *KlayoutText(Tree *self, text s);
    Tree *KlayoutMarkup(Tree *self, text s);
    Tree *buildPath(Tree *self, Tree *path, int strokeOrFill);
    Tree *arc(Tree *self,
               double x,
               double y,
               double r,
               double a1,
               double a2,
               bool isPositive);
    Tree *curveTo(Tree *self,
                 double x1,
                 double y1,
                 double x2,
                 double y2,
                 double x3,
                 double y3,
                 bool isRelative);
    Tree *lineTo(Tree *self, double x, double y, bool isRelative);
    Tree *closePath(Tree *self);
    Tree *menuItem(Tree *self, text s, Tree *t);
    Tree *menu(Tree *self, text s, bool=false);

    Name *insert(Tree *self, Tree *toInsert);
    Name *deleteSelection(Tree *self);

private:
    void widgetVertex(double x, double y, double tx, double ty);
    void circularVertex(double cx, double cy, double r,
                double x, double y,
                double tx0, double ty0, double tx1, double ty1);
    void circularSectorN(double cx, double cy, double r,
                double tx0, double ty0, double tx1, double ty1,
                int sq, int nq);
    Tree *evalInGlMode(GLenum mode, Tree *child);

public:
    // XL Runtime
    XL::SourceFile       *xlProgram;
    QTimer                timer, idleTimer;
    QMenu                *currentMenu;
    QMenuBar             *currentMenuBar;
    QList<TreeHolder>     actions;
    Frame *               frame;
    Frame *               mainFrame;
    Activity *            activities;
    double                page_start_time;
    GLuint                id, capacity, selector, activeSelector;
    std::set<GLuint>      selection, savedSelection;
    std::set<XL::Tree *>  selectionTrees;
    std::map<text, uint>  selectors;
    std::vector<text>     selectorNames;
    QEvent *              event;
    GLdouble              depth;
    GLint                 depthBits;
    ulong                 depthBitsMax;
    QWidget *             focusWidget;
    GLdouble              focusProjection[16], focusModel[16];
    GLint                 focusViewport[4];
    text                  whatsNew;
    bool                  reloadProgram;

    // Timing for drawing and saving
    ulonglong         tmin, tmax, tsum, tcount;
    ulonglong         nextSave, nextCommit, nextSync;
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
        GLdouble        depthDelta;
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

#undef TAO // From the command line
#define TAO(x)  (Tao::Widget::Tao() ? Tao::Widget::Tao()->x : 0)
#define RTAO(x) return TAO(x)


} // namespace Tao


#endif // TAO_WIDGET_H
