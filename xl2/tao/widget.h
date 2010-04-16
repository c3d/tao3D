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
#include "coords3d.h"
#include "opcodes.h"
#include "drawing.h"


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
struct FrameInfo;
struct Activity;
struct Layout;
struct PageLayout;
struct SpaceLayout;
struct GraphicPath;
struct Repository;
struct Drag;
struct WidgetSurface;

class Widget : public QGLWidget
// ----------------------------------------------------------------------------
//   This is the widget we use to display XL programs output
// ----------------------------------------------------------------------------
{
    Q_OBJECT
public:
    Widget(Window *parent, XL::SourceFile *sf = NULL);
    ~Widget();

public slots:
    // Slots
    void        dawdle();
    void        draw();
    void        runProgram();
    void        appFocusChanged(QWidget *prev, QWidget *next);
    void        userMenu(QAction *action);
    bool        refresh(double delay = 0.0);
    void        commitSuccess(QString id, QString msg);

public:
    // OpenGL
    void        initializeGL();
    void        resizeGL(int width, int height);
    void        paintGL();
    void        setup(double w, double h, Box *picking = NULL);
    void        setupGL();
    void        identifySelection();

    // Events
    bool        forwardEvent(QEvent *event);
    bool        forwardEvent(QMouseEvent *event);
    void        keyPressEvent(QKeyEvent *event);
    void        keyReleaseEvent(QKeyEvent *event);
    void        mousePressEvent(QMouseEvent *);
    void        mouseReleaseEvent(QMouseEvent *);
    void        mouseMoveEvent(QMouseEvent *);
    void        mouseDoubleClickEvent(QMouseEvent *);
    void        wheelEvent(QWheelEvent *);
    void        timerEvent(QTimerEvent *);

    // XL program management
    void        updateProgram(XL::SourceFile *sf);
    void        applyAction(XL::Action &action);
    void        refreshProgram();
    void        markChanged(text reason);
    bool        writeIfChanged(XL::SourceFile &sf);
    bool        doCommit();
    Repository *repository();

    // Timing
    ulonglong   now();
    ulonglong   elapsed(ulonglong since, ulonglong until,
                        bool stats = true, bool show=true);
    bool        timerIsActive()         { return timer.isActive(); }

    // Selection
    GLuint      newId()                 { return ++id; }
    GLuint      currentId()             { return id; }
    GLuint      manipulatorId()         { return manipulator; }
    GLuint      selectionCapacity()     { return capacity; }
    uint        selected();
    void        select(uint id, uint count);
    void        deleteFocus(QWidget *widget);
    void        requestFocus(QWidget *widget, coord x, coord y);
    void        recordProjection();
    Point3      unproject (coord x, coord y, coord z = 0.0);
    Drag *      drag();
    void        drawSelection(const Box3 &bounds, text name);
    void        drawHandle(const Point3 &point, text name);

    // Text flows
    PageLayout*&pageLayoutFlow(text name) { return flows[name]; }

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

    // Getting attributes
    Real *      pageWidth(Tree *self);
    Real *      pageHeight(Tree *self);
    Real *      frameWidth(Tree *self);
    Real *      frameHeight(Tree *self);
    Real *      frameDepth(Tree *self);
    Real *      windowWidth(Tree *self);
    Real *      windowHeight(Tree *self);
    Real *      time(Tree *self);
    Real *      pageTime(Tree *self);

    // Preserving attributes
    Tree *      locally(Tree *self, Tree *t);

    // Transforms
    Tree *      rotatex(Tree *self, real_r rx);
    Tree *      rotatey(Tree *self, real_r ry);
    Tree *      rotatez(Tree *self, real_r rz);
    Tree *      rotate(Tree *self, real_r ra, real_r rx, real_r ry, real_r rz);
    Tree *      translatex(Tree *self, real_r x);
    Tree *      translatey(Tree *self, real_r y);
    Tree *      translatez(Tree *self, real_r z);
    Tree *      translate(Tree *self, real_r x, real_r y, real_r z);
    Tree *      rescalex(Tree *self, real_r x);
    Tree *      rescaley(Tree *self, real_r y);
    Tree *      rescalez(Tree *self, real_r z);
    Tree *      rescale(Tree *self, real_r x, real_r y, real_r z);

    // Setting attributes
    Name *      depthTest(Tree *self, bool enable);
    Tree *      refresh(Tree *self, double delay);
    Name *      fullScreen(Tree *self, bool fs);
    Name *      toggleFullScreen(Tree *self);

    // Graphic attributes
    Tree *      lineColor(Tree *self, double r, double g, double b, double a);
    Tree *      lineWidth(Tree *self, double lw);
    Tree *      lineStipple(Tree *self, uint16 pattern, uint16 scale);
    Tree *      fillColor(Tree *self, double r, double g, double b, double a);
    Tree *      fillTexture(Tree *self, text fileName);
    Tree *      fillTextureFromSVG(Tree *self, text svg);

    // Generating a path
    Tree *      newPath(Tree *self, Tree *t);
    Tree *      moveTo(Tree *self, real_r x, real_r y, real_r z);
    Tree *      lineTo(Tree *self, real_r x, real_r y, real_r z);
    Tree *      curveTo(Tree *self,
                        real_r cx, real_r cy, real_r cz,
                        real_r x, real_r y, real_r z);
    Tree *      curveTo(Tree *self,
                        real_r c1x, real_r c1y, real_r c1z,
                        real_r c2x, real_r c2y, real_r c2z,
                        real_r x, real_r y, real_r z);
    Tree *      moveToRel(Tree *self, real_r x, real_r y, real_r z);
    Tree *      lineToRel(Tree *self, real_r x, real_r y, real_r z);
    Tree *      pathTextureCoord(Tree *self, real_r x, real_r y, real_r r);
    Tree *      pathColor(Tree *self, real_r r, real_r g, real_r b, real_r a);
    Tree *      closePath(Tree *self);

    // 2D primitive that can be in a path or standalone
    Tree *      rectangle(Tree *self, real_r x, real_r y, real_r w, real_r h);
    Tree *      isoscelesTriangle(Tree *self, real_r x, real_r y, real_r w, real_r h);
    Tree *      rightTriangle(Tree *self, real_r x, real_r y, real_r w, real_r h);
    Tree *      ellipse(Tree *self, real_r x, real_r y, real_r w, real_r h);
    Tree *      ellipseArc(Tree *self, real_r x, real_r y, real_r w, real_r h,
                           real_r start, real_r sweep);
    Tree *      roundedRectangle(Tree *self,
                                 real_r cx, real_r cy, real_r w, real_r h,
                                 real_r r);
    Tree *      arrow(Tree *self, real_r cx, real_r cy, real_r w, real_r h,
                      real_r ax, real_r ary);
    Tree *      doubleArrow(Tree *self,
                            real_r cx, real_r cy, real_r w, real_r h,
                            real_r ax, real_r ary);
    Tree *      starPolygon(Tree *self,
                            real_r cx, real_r cy, real_r w, real_r h,
                            integer_r p, integer_r q);
    Tree *      star(Tree *self, real_r cx, real_r cy, real_r w, real_r h,
                     integer_r p, real_r r);
    Tree *      speechBalloon(Tree *self,
                              real_r cx, real_r cy, real_r w, real_r h,
                              real_r r, real_r ax, real_r ay);

    // 3D primitives
    Tree *      sphere(Tree *self,
                       real_r cx, real_r cy, real_r cz,
                       real_r w, real_r, real_r d,
                       integer_r nslices, integer_r nstacks);
    Tree *      cube(Tree *self, real_r cx, real_r cy, real_r cz,
                     real_r w, real_r h, real_r d);
    Tree *      cone(Tree *self, real_r cx, real_r cy, real_r cz,
                     real_r w, real_r h, real_r d);

    // Text and font
    Tree *      textBox(Tree *self,
                        real_r x, real_r y, real_r w, real_r h, Tree *prog);
    Tree *      textOverflow(Tree *self,
                             real_r x, real_r y, real_r w, real_r h);
    Text *      textFlow(Tree *self, text name);
    Tree *      textSpan(Tree *self, text_r content);
    Tree *      font(Tree *self, text family);
    Tree *      fontSize(Tree *self, double size);
    Tree *      fontPlain(Tree *self);
    Tree *      fontItalic(Tree *self, scale amount = 1);
    Tree *      fontBold(Tree *self, scale amount = 1);
    Tree *      fontUnderline(Tree *self, scale amount = 1);
    Tree *      fontOverline(Tree *self, scale amount = 1);
    Tree *      fontStrikeout(Tree *self, scale amount = 1);
    Tree *      fontStretch(Tree *self, scale amount = 1);
    Tree *      justify(Tree *self, scale amount, uint axis);
    Tree *      center(Tree *self, scale amount, uint axis);
    Tree *      spread(Tree *self, scale amount, uint axis);
    Tree *      spacing(Tree *self, scale amount, uint axis);
    Tree *      drawingBreak(Tree *self, Drawing::BreakOrder order);

    // Frames and widgets
    Tree *      status(Tree *self, text t);
    Tree *      framePaint(Tree *self, real_r x, real_r y, real_r w, real_r h,
                           Tree *prog);
    Tree *      frameTexture(Tree *self, double w, double h, Tree *prog);

    Tree *      urlPaint(Tree *self, real_r x, real_r y, real_r w, real_r h,
                         text_p s, integer_p p);
    Tree *      urlTexture(Tree *self, double x, double y, Text *s, Integer *p);

    Tree *      lineEdit(Tree *self, real_r x,real_r y,
                         real_r w,real_r h, text_p s);
    Tree *      lineEditTexture(Tree *self, double x, double y, Text *s);

    Tree *      abstractButton(Tree *self,
                               real_r x, real_r y, real_r w, real_r h);
    Tree *      pushButton(Tree *self, real_r x, real_r y, real_r w, real_r h,
                           text_p lbl, Tree *act);
    Tree *      pushButtonTexture(Tree *self, double w, double h,
                                  Text *lbl, Tree *act);
    Tree *      radioButton(Tree *self, real_r x,real_r y, real_r w,real_r h,
                           text_p lbl, Text *selected, Tree *act);
    Tree *      radioButtonTexture(Tree *self, double w, double h,
                                  Text *lbl, Text *selected, Tree *act);
    Tree *      checkBoxButton(Tree *self, real_r x,real_r y, real_r w,real_r h,
                               text_p lbl, Text* marked,
                               Tree *act);
    Tree *      checkBoxButtonTexture(Tree *self, double w, double h,
                                      Text *lbl, Text* marked, Tree *act);
    Tree *      buttonGroup(Tree *self, Tree *buttons, bool exclusive = true);

    Tree *      colorChooser(Tree *self, real_r x, real_r y, real_r w, real_r h,
                             Tree *action);
    Tree *      colorChooserTexture(Tree *self,double w, double h,
                                    Tree *action);

    Tree *      fontChooser(Tree *self, real_r x, real_r y, real_r w, real_r h,
                             Tree *action);
    Tree *      fontChooserTexture(Tree *self,double w, double h,
                                    Tree *action);

    Tree *      groupBox(Tree *self,
                         real_r x,real_r y, real_r w,real_r h,
                         text_p lbl, Tree *buttons);
    Tree *      groupBoxTexture(Tree *self,
                                double w, double h,
                                Text *lbl);

    Tree *      videoPlayer(Tree *self,
                            real_r x, real_r y, real_r w, real_r h, Text *url);

    Tree *      videoPlayerTexture(Tree *self, real_r w, real_r h, Text *url);

    // Menus
    Tree *      menuItem(Tree *self, Text *s, Tree *t);
    Tree *      menu(Tree *self, Text *s, bool=false);

    // Tree management
    Name *      insert(Tree *self, Tree *toInsert);
    Name *      deleteSelection(Tree *self);

    // Unit conversions
    Real *      fromCm(Tree *self, double cm);
    Real *      fromMm(Tree *self, double mm);
    Real *      fromIn(Tree *self, double in);
    Real *      fromPt(Tree *self, double pt);
    Real *      fromPx(Tree *self, double px);

private:
    friend class Window;
    friend class Activity;
    friend class Selection;
    friend class Drag;
    friend class Manipulator;
    friend class ControlPoint;
    friend class AbstractButtonSurface;

    typedef XL::LocalSave<QEvent *> EventSave;
    typedef std::map<GLuint, uint>  selection_map;
    typedef std::map<text, PageLayout*> flow_map;

    // XL Runtime
    XL::SourceFile       *xlProgram;

    // Rendering
    SpaceLayout *         space;
    Layout *              layout;
    GraphicPath *         path;
    scale                 pageW, pageH;
    text                  flowName;
    flow_map              flows;
    QGridLayout *         currentGridLayout;
    QButtonGroup *        currentGroup;

    // Selection
    Activity *            activities;
    GLuint                id, capacity, manipulator;
    selection_map         selection, savedSelection;
    std::set<XL::Tree *>  selectionTrees;
    QEvent *              event;
    QWidget *             focusWidget;
    GLdouble              focusProjection[16], focusModel[16];
    GLint                 focusViewport[4];

    // Menus
    QMenu                *currentMenu;
    QMenuBar             *currentMenuBar;

    // Program changes
    bool                  reloadProgram;

    // Timing
    QTimer                timer, idleTimer;
    double                pageStartTime, pageRefresh;
    ulonglong             tmin, tmax, tsum, tcount;
    ulonglong             nextSave, nextCommit, nextSync, nextPull;

    static Widget *       current;
    static double         zNear, zFar;
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



// ============================================================================
//
//   Qt interface for XL types
//
// ============================================================================

#define TREEROOT_TYPE 383 // (QVariant::UserType | 0x100)
Q_DECLARE_METATYPE(XL::TreeRoot)

#endif // TAO_WIDGET_H
