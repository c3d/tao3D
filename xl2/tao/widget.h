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
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "main.h"
#include "tao.h"
#include "coords3d.h"
#include "opcodes.h"
#include "drawing.h"
#include "activity.h"
#include "menuinfo.h"
#include "glyph_cache.h"

#include <GL/glew.h>
#include <QtOpenGL>
#include <QImage>
#include <QTimeLine>
#include <QTimer>
#include <QSvgRenderer>
#include <QList>
#include <QColorDialog>
#include <QFontDialog>
#include <iostream>
#include <map>

namespace Tao {

struct Window;
struct FrameInfo;
struct Layout;
struct PageLayout;
struct SpaceLayout;
struct GraphicPath;
struct Repository;
struct Drag;
struct TextSelect;
struct WidgetSurface;

// ----------------------------------------------------------------------------
// Name of fixed menu. Menus then may be retrieved by
//   QMenu * view = window->findChild<QMenu*>(VIEW_MENU_NAME)
// ----------------------------------------------------------------------------
#define FILE_MENU_NAME  "TAO_FILE_MENU"
#define EDIT_MENU_NAME  "TAO_EDIT_MENU"
#define SHARE_MENU_NAME "TAO_SHARE_MENU"
#define VIEW_MENU_NAME  "TAO_VIEW_MENU"
#define HELP_MENU_NAME  "TAO_HELP_MENU"

class Widget : public QGLWidget
// ----------------------------------------------------------------------------
//   This is the widget we use to display XL programs output
// ----------------------------------------------------------------------------
{
    Q_OBJECT
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
    typedef std::vector<double>   attribute_args;

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
    void        colorChosen(const QColor &);
    void        colorChanged(const QColor &);
    void        updateColorDialog();
    void        fontChosen(const QFont &);
    void        fontChanged(const QFont &);
    void        updateFontDialog();
    void        updateDialogs()                { mustUpdateDialogs = true; }
    void        fileChosen(const QString & filename);
    void        copy();
    void        cut();
    void        paste();
    void        enableAnimations(bool animate);

signals:
    // Signals
    void        copyAvailable(bool yes = true);

public:
    // OpenGL
    void        initializeGL();
    void        resizeGL(int width, int height);
    void        paintGL();
    void        setup(double w, double h, Box *picking = NULL);
    void        setupGL();
    void        identifySelection();
    void        updateSelection();

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
    void        reloadProgram(XL::Tree *newProg = NULL);
    void        renormalizeProgram();
    void        refreshProgram();
    void        markChanged(text reason);
    bool        writeIfChanged(XL::SourceFile &sf);
    bool        doCommit(bool immediate = false);
    Repository *repository();
    Tree *      get(Tree *shape, text name, text sh = "shape");
    bool        set(Tree *shape, text n, Tree *value, text sh = "shape");
    bool        get(Tree *shape, text n, XL::tree_list &a, text sh = "shape");
    bool        set(Tree *shape, text n, XL::tree_list &a, text sh = "shape");
    bool        get(Tree *shape, text n, attribute_args &a, text sh = "shape");
    bool        set(Tree *shape, text n, attribute_args &a, text sh = "shape");

    // Timing
    ulonglong   now();
    ulonglong   elapsed(ulonglong since, ulonglong until,
                        bool stats = true, bool show=true);
    bool        timerIsActive()         { return timer.isActive(); }
    bool        hasAnimations(void)     { return animated; }


    // Selection
    enum { CHAR_ID_BIT = 1U<<31, CHAR_ID_MASK = ~CHAR_ID_BIT };
    GLuint      newId()                 { return ++id; }
    GLuint      currentId()             { return id; }
    GLuint      manipulatorId()         { return manipulator; }
    GLuint      selectionCapacity()     { return capacity; }
    GLuint      newCharId(uint ids = 1) { return charId += ids; }
    GLuint      currentCharId()         { return charId; }
    uint        charSelected(uint i)    { return selected(i | CHAR_ID_BIT); }
    uint        charSelected()          { return charSelected(charId); }
    void        selectChar(uint i,uint c){ select(i|CHAR_ID_BIT, c); }
    uint        selected(Tree *tree)    { return selectionTrees.count(tree); }
    bool        selected()              { return !selectionTrees.empty(); }
    bool        hasSelection()          { return selected(); }
    void        deselect(Tree *tree)    { selectionTrees.erase(tree); }
    uint        selected(uint i);
    uint        selected(Layout *);
    void        select(uint id, uint count);
    void        deleteFocus(QWidget *widget);
    void        requestFocus(QWidget *widget, coord x, coord y);
    void        recordProjection();
    uint        lastModifiers()         { return keyboardModifiers; }
    Point3      unproject (coord x, coord y, coord z = 0.0);
    Drag *      drag();
    TextSelect *textSelection();
    void        drawSelection(const Box3 &bounds, text name);
    void        drawHandle(const Point3 &point, text name);
    template<class Activity>
    Activity *  active();
    void        checkCopyAvailable();
    bool        canPaste();

    // Text flows and text managemen
    PageLayout*&pageLayoutFlow(text name) { return flows[name]; }
    GlyphCache &glyphs()    { return glyphCache; }

public:
    // XLR entry points
    static Widget *Tao() { return current; }

    // Getting attributes
    Text *      page(Tree *self, text name, Tree *body);
    Text *      pageLink(Tree *self, text key, text name);
    Text *      pageLabel(Tree *self);
    Integer *   pageNumber(Tree *self);
    Integer *   pageCount(Tree *self);
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
    Tree *      shape(Tree *self, Tree *t);

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
    Name *      enableAnimations(Tree *self, bool fs);
    Name *      toggleFullScreen(Tree *self);
    Integer *   polygonOffset(Tree *self,
                              double f0, double f1, double u0, double u1);

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
    Tree *      isoscelesTriangle(Tree *self,
                                  real_r x, real_r y, real_r w, real_r h);
    Tree *      rightTriangle(Tree *self,
                              real_r x, real_r y, real_r w, real_r h);
    Tree *      ellipse(Tree *self, real_r x, real_r y, real_r w, real_r h);
    Tree *      ellipseArc(Tree *self, real_r x, real_r y, real_r w, real_r h,
                           real_r start, real_r sweep);
    Tree *      roundedRectangle(Tree *self,
                                 real_r cx, real_r cy, real_r w, real_r h,
                                 real_r r);
    Tree *      ellipticalRectangle(Tree *self,
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
    Tree *      callout(Tree *self,
                        real_r cx, real_r cy, real_r w, real_r h,
                        real_r r, real_r ax, real_r ay, real_r d);

    Tree *      debugBinPacker(Tree *self, uint w, uint h, Tree *t);
    Tree *      debugParameters(Tree *self,
                                double x, double y,
                                double w, double h);

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
    Tree *      fontScaling(Tree *self, double scaling, double minSize);
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
    Name *      textEditKey(Tree *self, text key);

    // Frames and widgets
    Tree *      status(Tree *self, text t);
    Tree *      framePaint(Tree *self, real_r x, real_r y, real_r w, real_r h,
                           Tree *prog);
    Tree *      frameTexture(Tree *self, double w, double h, Tree *prog);

    Tree *      urlPaint(Tree *self, real_r x, real_r y, real_r w, real_r h,
                         text_p s, integer_p p);
    Tree *      urlTexture(Tree *self,
                           double x, double y,
                           Text *s, Integer *p);

    Tree *      lineEdit(Tree *self, real_r x,real_r y,
                         real_r w,real_r h, text_p s);
    Tree *      lineEditTexture(Tree *self, double x, double y, Text *s);

    Tree *      abstractButton(Tree *self, Text *name,
                               real_r x, real_r y, real_r w, real_r h);
    Tree *      pushButton(Tree *self, real_r x, real_r y, real_r w, real_r h,
                           text_p name, text_p lbl, Tree *act);
    Tree *      pushButtonTexture(Tree *self, double w, double h,
                                  text_p name, Text *lbl, Tree *act);
    Tree *      radioButton(Tree *self, real_r x,real_r y, real_r w,real_r h,
                            text_p name, text_p lbl,
                            Text *selected, Tree *act);
    Tree *      radioButtonTexture(Tree *self, double w, double h,
                                   text_p name, Text *lbl,
                                   Text *selected, Tree *act);
    Tree *      checkBoxButton(Tree *self,
                               real_r x,real_r y, real_r w, real_r h,
                               text_p name, text_p lbl, Text* marked,
                               Tree *act);
    Tree *      checkBoxButtonTexture(Tree *self,
                                      double w, double h,
                                      text_p name, Text *lbl,
                                      Text* marked, Tree *act);
    Tree *      buttonGroup(Tree *self, bool exclusive, Tree *buttons);
    Tree *      setButtonGroupAction(Tree *self, Tree *action);

    Tree *      colorChooser(Tree *self, text name, Tree *action);
    Tree *      colorChooser(Tree *self,
                             real_r x, real_r y, real_r w, real_r h,
                             Tree *action);
    Tree *      colorChooserTexture(Tree *self,
                                    double w, double h,
                                    Tree *action);

    Tree *      fontChooser(Tree *self, Tree *action);
    Tree *      fontChooser(Tree *self,
                            real_r x, real_r y, real_r w, real_r h,
                            Tree *action);
    Tree *      fontChooserTexture(Tree *self,
                                   double w, double h,
                                   Tree *action);

    Tree *      fileChooser(Tree *self, Tree *action);
    Tree *      fileChooser(Tree *self,
                            real_r x, real_r y, real_r w, real_r h,
                            Tree *action);
    Tree *      fileChooserTexture(Tree *self,
                                    double w, double h,
                                    Tree *action);
    Tree *      setFileDialogAction(Tree *self, Tree *action);
    Tree *      setFileDialogDirectory(Tree *self, text dirname);
    Tree *      setFileDialogFilter(Tree *self, text filters);
    Tree *      setFileDialogLabel(Tree *self, text label, text value);

    Tree *      groupBox(Tree *self,
                         real_r x,real_r y, real_r w,real_r h,
                         text_p lbl, Tree *buttons);
    Tree *      groupBoxTexture(Tree *self,
                                double w, double h,
                                Text *lbl);

    Tree *      videoPlayer(Tree *self,
                            real_r x, real_r y, real_r w, real_r h, Text *url);

    Tree *      videoPlayerTexture(Tree *self, real_r w, real_r h, Text *url);

    Tree *      image(Tree *self, real_r x, real_r y, real_r w, real_r h,
                      text filename);

    // Menus and widgets
    Tree *      runtimeError(Tree *self, text msg, Tree *src);
    Tree *      menuItem(Tree *self, text name, text lbl, text iconFileName,
                         bool isCheckable, Text *isChecked, Tree *t);
    Tree *      menu(Tree *self, text name, text lbl, text iconFileName,
                     bool isSubmenu=false);

    // The location is the prefered location for the toolbar.
    // The supported values are North, East, South, West or N, E, S, W
    Tree *      toolBar(Tree *self, text name, text title, bool isFloatable,
                        text location);

    Tree *      menuBar(Tree *self);
    Tree *      separator(Tree *self);

    // Tree management
    Name *      insert(Tree *self, Tree *toInsert);
    void        deleteSelection();
    Name *      deleteSelection(Tree *self, text key);
    Name *      setAttribute(Tree *self, text name, Tree *attribute, text sh);

    // Unit conversionsxo
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
    friend class TextSelect;
    friend class Manipulator;
    friend class ControlPoint;

    typedef XL::LocalSave<QEvent *>             EventSave;
    typedef std::map<GLuint, uint>              selection_map;
    typedef std::map<text, PageLayout*>         flow_map;
    typedef std::map<text, text>                page_map;

    // XL Runtime
    XL::SourceFile       *xlProgram;
    bool                  inError;
    bool                  mustUpdateDialogs;

    // Rendering
    SpaceLayout *         space;
    Layout *              layout;
    GraphicPath *         path;
    scale                 pageW, pageH;
    text                  flowName;
    flow_map              flows;
    text                  pageName, lastPageName;
    page_map              pageLinks;
    uint                  pageId, pageShown, pageTotal;
    Tree *                pageTree;
    Tree *                currentShape;
    QGridLayout *         currentGridLayout;
    GroupInfo   *         currentGroup;
    GlyphCache            glyphCache;

    // Selection
    Activity *            activities;
    GLuint                id, charId, capacity, manipulator;
    selection_map         selection, savedSelection;
    std::set<Tree *>      selectionTrees, selectNextTime;
    bool                  wasSelected;
    QEvent *              event;
    QWidget *             focusWidget;
    GLdouble              focusProjection[16], focusModel[16];
    GLint                 focusViewport[4];
    uint                  keyboardModifiers;

    // Menus and widgets
    QMenu                *currentMenu;
    QMenuBar             *currentMenuBar;
    QToolBar             *currentToolBar;
    QVector<MenuInfo*>    orderedMenuElements;
    int                   order;
    XL::TreeRoot          colorAction, fontAction;
    text                  colorName;

    // Timing
    QTimer                timer, idleTimer;
    double                pageStartTime, pageRefresh, frozenTime;
    ulonglong             tmin, tmax, tsum, tcount;
    ulonglong             nextSave, nextCommit, nextSync, nextPull;
    bool                  animated;

    static Widget *       current;
    static QColorDialog * colorDialog;
    static QFontDialog *  fontDialog;
    static QFileDialog *  fileDialog;
           QFileDialog *  currentFileDialog;
    static double         zNear, zFar;

    std::map<text, QFileDialog::DialogLabel> toDialogLabel;
private:
    void        updateFileDialog(Tree *properties);

};


template<class ActivityClass>
inline ActivityClass *Widget::active()
// ----------------------------------------------------------------------------
//   Return an activity of the given type
// ----------------------------------------------------------------------------
{
    for (Activity *a = activities; a; a = a->next)
        if (ActivityClass *result = dynamic_cast<ActivityClass *> (a))
            return result;
    return NULL;
}



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



// ============================================================================
//
//   Action that returns a tree where all the selected trees are removed
//
// ============================================================================

struct DeleteSelectionAction : XL::TreeClone
// ----------------------------------------------------------------------------
//    A specialized clone action that doesn't copy selected trees
// ----------------------------------------------------------------------------
{
    DeleteSelectionAction(Widget *widget): widget(widget) {}
    XL::Tree *DoInfix(XL::Infix *what)
    {
        if (what->name == "\n" || what->name == ";")
        {
            if (widget->selected(what->left))
            {
                if (widget->selected(what->right))
                    return NULL;
                return what->right->Do(this);
            }
            if (widget->selected(what->right))
                return what->left->Do(this);
        }
        XL::Tree *left = what->left->Do(this);
        XL::Tree *right = what->right->Do(this);
        if (left && right)
            return new XL::Infix(what->name, left, right, what->Position());
        else if (left)
            return left;
        return right;
    }
    Widget *widget;
};


struct InsertAtSelectionAction : XL::TreeClone
// ----------------------------------------------------------------------------
//    A specialized clone action that inserts an input
// ----------------------------------------------------------------------------
{
    InsertAtSelectionAction(Widget *widget,
                            XL::Tree *toInsert, XL::Tree *parent)
        : widget(widget), toInsert(toInsert), parent(parent) {}


    XL::Tree *DoName(XL::Name *what)
    {
        if (what == parent)
            parent = NULL;
        return XL::TreeClone::DoName(what);
    }

    XL::Tree *DoPrefix(XL::Prefix *what)
    {
        if (what == parent)
            parent = NULL;
        return XL::TreeClone::DoPrefix(what);
    }

    XL::Tree *DoPostfix(XL::Postfix *what)
    {
        if (what == parent)
            parent = NULL;
        return XL::TreeClone::DoPostfix(what);
    }

    XL::Tree *DoBlock(XL::Block *what)
    {
        if (what == parent)
            parent = NULL;
        return XL::TreeClone::DoBlock(what);
    }

    XL::Tree *DoInfix(XL::Infix *what)
    {
        if (what == parent)
            parent = NULL;

        if (!parent)
        {
            if (what->name == "\n" || what->name == ";")
            {
                // Check if we hit the selection. If so, insert
                if (toInsert && widget->selected(what->left))
                {
                    XL::Tree *ins = toInsert;
                    toInsert = NULL;
                    return new XL::Infix("\n", ins, what->Do(this));
                }
            }
        }
        return XL::TreeClone::DoInfix(what);
    }
    Widget   *widget;
    XL::Tree *toInsert;
    XL::Tree *parent;
};


struct SetAttributeAction : XL::Action
// ----------------------------------------------------------------------------
//    Copy the inserted item as attribute in all selected items
// ----------------------------------------------------------------------------
{
    SetAttributeAction(text name, XL::Tree *attribute,
                       Widget *widget, text shape = "shape")
        : name(name), attribute(attribute), widget(widget), shape(shape) {}

    XL::Tree *Do(XL::Tree *what)
    {
        if (widget->selected(what))
            widget->set(what, name, attribute, shape);
        return what;
    }

    text      name;
    XL::Tree *attribute;
    Widget   *widget;
    text      shape;
};



struct N2NReplacerTreeClone : XL::TreeClone
// ----------------------------------------------------------------------------
//    Replace a name with a Name
// ----------------------------------------------------------------------------
{
    N2NReplacerTreeClone(std::map<text, text> *c) :
             concordance(c){};
    XL::Tree* DoName(XL::Name *what);
    XL::Tree* replace(XL::Tree *original);
    std::map<text, text> *concordance;

};
struct N2TReplacerTreeClone : XL::TreeClone
// ----------------------------------------------------------------------------
//    Replace a name with a Text
// ----------------------------------------------------------------------------
{
    N2TReplacerTreeClone(std::map<text, text> *c) :
             concordance(c){};
    XL::Tree* DoName(XL::Name *what);

    XL::Tree* replace(XL::Tree *original);

    std::map<text, text> *concordance;

};



} // namespace Tao



// ============================================================================
//
//   Qt interface for XL types
//
// ============================================================================

#define TREEROOT_TYPE 383 // (QVariant::UserType | 0x100)
Q_DECLARE_METATYPE(XL::TreeRoot)

#endif // TAO_WIDGET_H
