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
#include "tao_tree.h"
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
    void        colorRejected();
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
    uint        showGlErrors();

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
    void        applyAction(Action &action);
    void        reloadProgram(Tree *newProg = NULL);
    void        refreshProgram();
    void        updateProgramSource();
    void        markChanged(text reason);
    void        selectStatements(Tree *tree);
    bool        writeIfChanged(XL::SourceFile &sf);
    bool        doCommit(bool immediate = false);
    Repository *repository();
    Tree *      get(Tree *shape, text name, text sh = "shape");
    bool        set(Tree *shape, text n, Tree *value, text sh = "shape");
    bool        get(Tree *shape, text n, TreeList &a, text sh = "shape");
    bool        set(Tree *shape, text n, TreeList &a, text sh = "shape");
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
    uint        selected(Tree* tree)    { return selectionTrees.count(tree); }
    bool        selected()              { return !selectionTrees.empty(); }
    bool        hasSelection()          { return selected(); }
    void        select(Tree *tree)      { selectionTrees.insert(tree); }
    void        deselect(Tree *tree)    { selectionTrees.erase(tree); }
    uint        selected(uint i);
    uint        selected(Layout *);
    static uint singleClicks(uint sel)  { return sel & 0xFFFF; }
    static uint doubleClicks(uint sel)  { return sel >> 16; }
    void        select(uint id, uint count);
    void        deleteFocus(QWidget *widget);
    void        requestFocus(QWidget *widget, coord x, coord y);
    void        recordProjection();
    uint        lastModifiers()         { return keyboardModifiers; }
    Point3      unproject (coord x, coord y, coord z = 0.0);
    Drag *      drag();
    TextSelect *textSelection();
    void        drawSelection(const Box3 &bounds, text name, uint id);
    void        drawHandle(const Point3 &point, text name, uint id);
    template<class Activity>
    Activity *  active();
    void        checkCopyAvailable();
    bool        canPaste();

    // Text flows and text managemen
    PageLayout*&pageLayoutFlow(text name) { return flows[name]; }
    GlyphCache &glyphs()    { return glyphCache; }

public:
    // XLR entry points
    static Widget *Tao() { assert(current); return current; }
    XL::Symbols *formulaSymbols()       { return symbolTableForFormulas; }

    // Getting attributes
    Text_p      page(Tree_p self, text name, Tree_p body);
    Text_p      pageLink(Tree_p self, text key, text name);
    Text_p      pageLabel(Tree_p self);
    Integer_p   pageNumber(Tree_p self);
    Integer_p   pageCount(Tree_p self);
    Real_p      pageWidth(Tree_p self);
    Real_p      pageHeight(Tree_p self);
    Real_p      frameWidth(Tree_p self);
    Real_p      frameHeight(Tree_p self);
    Real_p      frameDepth(Tree_p self);
    Real_p      windowWidth(Tree_p self);
    Real_p      windowHeight(Tree_p self);
    Real_p      time(Tree_p self);
    Real_p      pageTime(Tree_p self);

    // Preserving attributes
    Tree_p      locally(Tree_p self, Tree_p t);
    Tree_p      shape(Tree_p self, Tree_p t);

    // Transforms
    Tree_p      rotatex(Tree_p self, Real_p rx);
    Tree_p      rotatey(Tree_p self, Real_p ry);
    Tree_p      rotatez(Tree_p self, Real_p rz);
    Tree_p      rotate(Tree_p self, Real_p ra,Real_p rx,Real_p ry,Real_p rz);
    Tree_p      translatex(Tree_p self, Real_p x);
    Tree_p      translatey(Tree_p self, Real_p y);
    Tree_p      translatez(Tree_p self, Real_p z);
    Tree_p      translate(Tree_p self, Real_p x, Real_p y, Real_p z);
    Tree_p      rescalex(Tree_p self, Real_p x);
    Tree_p      rescaley(Tree_p self, Real_p y);
    Tree_p      rescalez(Tree_p self, Real_p z);
    Tree_p      rescale(Tree_p self, Real_p x, Real_p y, Real_p z);

    // Setting attributes
    Name_p      depthTest(Tree_p self, bool enable);
    Tree_p      refresh(Tree_p self, double delay);
    Name_p      fullScreen(Tree_p self, bool fs);
    Name_p      enableAnimations(Tree_p self, bool fs);
    Name_p      toggleFullScreen(Tree_p self);
    Integer_p   polygonOffset(Tree_p self,
                              double f0, double f1, double u0, double u1);

    // Graphic attributes
    Tree_p      lineColor(Tree_p self, double r, double g, double b, double a);
    Tree_p      lineWidth(Tree_p self, double lw);
    Tree_p      lineStipple(Tree_p self, uint16 pattern, uint16 scale);
    Tree_p      fillColor(Tree_p self, double r, double g, double b, double a);
    Tree_p      fillTexture(Tree_p self, text fileName);
    Tree_p      fillTextureFromSVG(Tree_p self, text svg);

    // Generating a path
    Tree_p      newPath(Tree_p self, Tree_p t);
    Tree_p      moveTo(Tree_p self, Real_p x, Real_p y, Real_p z);
    Tree_p      lineTo(Tree_p self, Real_p x, Real_p y, Real_p z);
    Tree_p      curveTo(Tree_p self,
                        Real_p cx, Real_p cy, Real_p cz,
                        Real_p x, Real_p y, Real_p z);
    Tree_p      curveTo(Tree_p self,
                        Real_p c1x, Real_p c1y, Real_p c1z,
                        Real_p c2x, Real_p c2y, Real_p c2z,
                        Real_p x, Real_p y, Real_p z);
    Tree_p      moveToRel(Tree_p self, Real_p x, Real_p y, Real_p z);
    Tree_p      lineToRel(Tree_p self, Real_p x, Real_p y, Real_p z);
    Tree_p      pathTextureCoord(Tree_p self, Real_p x, Real_p y, Real_p r);
    Tree_p      pathColor(Tree_p self, Real_p r, Real_p g, Real_p b, Real_p a);
    Tree_p      closePath(Tree_p self);
    Tree_p      endpointsStyle(Tree_p self,symbolicname_r s,symbolicname_r e);

    // 2D primitive that can be in a path or standalone
    Tree_p      fixedSizePoint(Tree_p self, coord x,coord y,coord z, coord s);
    Tree_p      rectangle(Tree_p self, Real_p x, Real_p y, Real_p w, Real_p h);
    Tree_p      isoscelesTriangle(Tree_p self,
                                  Real_p x, Real_p y, Real_p w, Real_p h);
    Tree_p      rightTriangle(Tree_p self,
                              Real_p x, Real_p y, Real_p w, Real_p h);
    Tree_p      ellipse(Tree_p self, Real_p x, Real_p y, Real_p w, Real_p h);
    Tree_p      ellipseArc(Tree_p self, Real_p x, Real_p y, Real_p w, Real_p h,
                           Real_p start, Real_p sweep);
    Tree_p      roundedRectangle(Tree_p self,
                                 Real_p cx, Real_p cy, Real_p w, Real_p h,
                                 Real_p r);
    Tree_p      ellipticalRectangle(Tree_p self,
                                    Real_p cx, Real_p cy, Real_p w, Real_p h,
                                    Real_p r);
    Tree_p      arrow(Tree_p self, Real_p cx, Real_p cy, Real_p w, Real_p h,
                      Real_p ax, Real_p ary);
    Tree_p      doubleArrow(Tree_p self,
                            Real_p cx, Real_p cy, Real_p w, Real_p h,
                            Real_p ax, Real_p ary);
    Tree_p      starPolygon(Tree_p self,
                            Real_p cx, Real_p cy, Real_p w, Real_p h,
                            Integer_p p, Integer_p q);
    Tree_p      star(Tree_p self, Real_p cx, Real_p cy, Real_p w, Real_p h,
                     Integer_p p, Real_p r);
    Tree_p      speechBalloon(Tree_p self,
                              Real_p cx, Real_p cy, Real_p w, Real_p h,
                              Real_p r, Real_p ax, Real_p ay);
    Tree_p      callout(Tree_p self,
                        Real_p cx, Real_p cy, Real_p w, Real_p h,
                        Real_p r, Real_p ax, Real_p ay, Real_p d);

    Tree_p      debugBinPacker(Tree_p self, uint w, uint h, Tree_p t);
    Tree_p      debugParameters(Tree_p self,
                                double x, double y,
                                double w, double h);

    // 3D primitives
    Tree_p      sphere(Tree_p self,
                       Real_p cx, Real_p cy, Real_p cz,
                       Real_p w, Real_p, Real_p d,
                       Integer_p nslices, Integer_p nstacks);
    Tree_p      cube(Tree_p self, Real_p cx, Real_p cy, Real_p cz,
                     Real_p w, Real_p h, Real_p d);
    Tree_p      cone(Tree_p self, Real_p cx, Real_p cy, Real_p cz,
                     Real_p w, Real_p h, Real_p d);

    // Text and font
    Tree_p      textBox(Tree_p self,
                        Real_p x, Real_p y, Real_p w, Real_p h, Tree_p prog);
    Tree_p      textOverflow(Tree_p self,
                             Real_p x, Real_p y, Real_p w, Real_p h);
    Text_p      textFlow(Tree_p self, text name);
    Tree_p      textSpan(Tree_p self, Text_p content);
    Tree_p      textFormula(Tree_p self, Tree_p value);
    Tree_p      font(Tree_p self, text family);
    Tree_p      fontSize(Tree_p self, double size);
    Tree_p      fontScaling(Tree_p self, double scaling, double minSize);
    Tree_p      fontPlain(Tree_p self);
    Tree_p      fontItalic(Tree_p self, scale amount = 1);
    Tree_p      fontBold(Tree_p self, scale amount = 1);
    Tree_p      fontUnderline(Tree_p self, scale amount = 1);
    Tree_p      fontOverline(Tree_p self, scale amount = 1);
    Tree_p      fontStrikeout(Tree_p self, scale amount = 1);
    Tree_p      fontStretch(Tree_p self, scale amount = 1);
    Tree_p      justify(Tree_p self, scale amount, uint axis);
    Tree_p      center(Tree_p self, scale amount, uint axis);
    Tree_p      spread(Tree_p self, scale amount, uint axis);
    Tree_p      spacing(Tree_p self, scale amount, uint axis);
    Tree_p      drawingBreak(Tree_p self, Drawing::BreakOrder order);
    Name_p      textEditKey(Tree_p self, text key);

    // Frames and widgets
    Tree_p      status(Tree_p self, text t);
    Tree_p      framePaint(Tree_p self, Real_p x, Real_p y, Real_p w, Real_p h,
                           Tree_p prog);
    Tree_p      frameTexture(Tree_p self, double w, double h, Tree_p prog);

    Tree_p      urlPaint(Tree_p self, Real_p x, Real_p y, Real_p w, Real_p h,
                         text_p s, integer_p p);
    Tree_p      urlTexture(Tree_p self,
                           double x, double y,
                           Text_p s, Integer_p p);

    Tree_p      lineEdit(Tree_p self, Real_p x,Real_p y,
                         Real_p w,Real_p h, text_p s);
    Tree_p      lineEditTexture(Tree_p self, double x, double y, Text_p s);

    Tree_p      abstractButton(Tree_p self, Text_p name,
                               Real_p x, Real_p y, Real_p w, Real_p h);
    Tree_p      pushButton(Tree_p self, Real_p x, Real_p y, Real_p w, Real_p h,
                           text_p name, text_p lbl, Tree_p act);
    Tree_p      pushButtonTexture(Tree_p self, double w, double h,
                                  text_p name, Text_p lbl, Tree_p act);
    Tree_p      radioButton(Tree_p self, Real_p x,Real_p y, Real_p w,Real_p h,
                            text_p name, text_p lbl,
                            Text_p selected, Tree_p act);
    Tree_p      radioButtonTexture(Tree_p self, double w, double h,
                                   text_p name, Text_p lbl,
                                   Text_p selected, Tree_p act);
    Tree_p      checkBoxButton(Tree_p self,
                               Real_p x,Real_p y, Real_p w, Real_p h,
                               text_p name, text_p lbl, Text_p  marked,
                               Tree_p act);
    Tree_p      checkBoxButtonTexture(Tree_p self,
                                      double w, double h,
                                      text_p name, Text_p lbl,
                                      Text_p  marked, Tree_p act);
    Tree_p      buttonGroup(Tree_p self, bool exclusive, Tree_p buttons);
    Tree_p      setButtonGroupAction(Tree_p self, Tree_p action);

    Tree_p      colorChooser(Tree_p self, text name, Tree_p action);
    Tree_p      colorChooser(Tree_p self,
                             Real_p x, Real_p y, Real_p w, Real_p h,
                             Tree_p action);
    Tree_p      colorChooserTexture(Tree_p self,
                                    double w, double h,
                                    Tree_p action);

    Tree_p      fontChooser(Tree_p self, Tree_p action);
    Tree_p      fontChooser(Tree_p self,
                            Real_p x, Real_p y, Real_p w, Real_p h,
                            Tree_p action);
    Tree_p      fontChooserTexture(Tree_p self,
                                   double w, double h,
                                   Tree_p action);

    Tree_p      fileChooser(Tree_p self, Tree_p action);
    Tree_p      fileChooser(Tree_p self,
                            Real_p x, Real_p y, Real_p w, Real_p h,
                            Tree_p action);
    Tree_p      fileChooserTexture(Tree_p self,
                                    double w, double h,
                                    Tree_p action);
    Tree_p      setFileDialogAction(Tree_p self, Tree_p action);
    Tree_p      setFileDialogDirectory(Tree_p self, text dirname);
    Tree_p      setFileDialogFilter(Tree_p self, text filters);
    Tree_p      setFileDialogLabel(Tree_p self, text label, text value);

    Tree_p      groupBox(Tree_p self,
                         Real_p x,Real_p y, Real_p w,Real_p h,
                         text_p lbl, Tree_p buttons);
    Tree_p      groupBoxTexture(Tree_p self,
                                double w, double h,
                                Text_p lbl);

    Tree_p      videoPlayer(Tree_p self,
                             Real_p x, Real_p y, Real_p w, Real_p h,
                             Text_p url);

    Tree_p      videoPlayerTexture(Tree_p self, Real_p w, Real_p h, Text_p url);

    Tree_p      image(Tree_p self, Real_p x, Real_p y, Real_p w, Real_p h,
                      text filename);
    Tree_p      image(Tree_p self, Real_p x, Real_p y, text filename);

    // Menus and widgets
    static Tree_p runtimeError(Tree_p self, text msg, Tree_p src);
    static Tree_p formulaRuntimeError(Tree_p self, text msg, Tree_p src);
    Tree_p      menuItem(Tree_p self, text name, text lbl, text iconFileName,
                         bool isCheckable, Text_p isChecked, Tree_p t);
    Tree_p      menu(Tree_p self, text name, text lbl, text iconFileName,
                     bool isSubmenu=false);

    // The location is the prefered location for the toolbar.
    // The supported values are North, East, South, West or N, E, S, W
    Tree_p      toolBar(Tree_p self, text name, text title, bool isFloatable,
                        text location);

    Tree_p      menuBar(Tree_p self);
    Tree_p      separator(Tree_p self);

    // Tree management
    Name_p      insert(Tree_p self, Tree_p toInsert);
    void        deleteSelection();
    Name_p      deleteSelection(Tree_p self, text key);
    Name_p      setAttribute(Tree_p self, text name, Tree_p attribute, text sh);
    Tree_p      removeSelection();

    // Unit conversionsxo
    Real_p      fromCm(Tree_p self, double cm);
    Real_p      fromMm(Tree_p self, double mm);
    Real_p      fromIn(Tree_p self, double in);
    Real_p      fromPt(Tree_p self, double pt);
    Real_p      fromPx(Tree_p self, double px);

    // z order management
    Name_p      bringToFront(Tree_p self);
    Name_p      sendToBack(Tree_p self);
//    Name_p      bringForward(Tree_p self);
//    Name_p      sendBackward(Tree_p self);

private:
    friend class Window;
    friend class Activity;
    friend class Selection;
    friend class Drag;
    friend class TextSelect;
    friend class Manipulator;
    friend class ControlPoint;
    friend class Renormalize;

    typedef XL::LocalSave<QEvent *>             EventSave;
    typedef XL::LocalSave<Widget *>             TaoSave;
    typedef std::map<GLuint, uint>              selection_map;
    typedef std::map<text, PageLayout*>         flow_map;
    typedef std::map<text, text>                page_map;

    // XL Runtime
    XL::SourceFile       *xlProgram;
    XL::Symbols          *symbolTableForFormulas;
    Tree_p                symbolTableRoot;
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
    Tree_p                pageTree;
    Tree_p                currentShape;
    QGridLayout *         currentGridLayout;
    GroupInfo   *         currentGroup;
    GlyphCache            glyphCache;

    // Selection
    Activity *            activities;
    GLuint                id, charId, capacity, manipulator;
    selection_map         selection, savedSelection;
    std::set<Tree_p >     selectionTrees, selectNextTime;
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
    Tree_p                colorAction, fontAction;
    text                  colorName;
    QColor                originalColor;

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

#undef TAO // From the command line
#define TAO(x)          Tao::Widget::Tao()->x
#define RTAO(x)         return Tao::Widget::Tao()->x;

inline void glShowErrors()
// ----------------------------------------------------------------------------
//   Display pending GL errors
// ----------------------------------------------------------------------------
{
    TAO(showGlErrors());
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



// ============================================================================
//
//   Action that returns a tree where all the selected trees are removed
//
// ============================================================================

struct DeleteSelectionAction : XL::Action
// ----------------------------------------------------------------------------
//    A specialized clone action that doesn't copy selected trees
// ----------------------------------------------------------------------------
{
    DeleteSelectionAction(Widget *widget): widget(widget) {}
    Tree *DoInteger(Integer *what)
    {
        if (widget->selected(what))
            return NULL;
        return new Integer(what->value, what->Position());
    }
    Tree *DoReal(Real *what)
    {
        if (widget->selected(what))
            return NULL;
        return new Real(what->value, what->Position());

    }
    Tree *DoText(Text *what)
    {
        if (widget->selected(what))
            return NULL;
        return new Text(what->value, what->opening, what->closing,
                        what->Position());
    }
    Tree *DoName(Name *what)
    {
        if (widget->selected(what))
            return NULL;
        return new Name(what->value, what->Position());
    }

    Tree *DoBlock(Block *what)
    {
        if (widget->selected(what))
            return NULL;
        Tree *child = what->child->Do(this);
        if (!child)
            return NULL;
        return new Block(child, what->opening, what->closing, what->Position());
    }
    Tree *DoInfix(XL::Infix *what)
    {
        if (widget->selected(what))
            return NULL;
        Tree *left = what->left->Do(this);
        Tree *right = what->right->Do(this);
        if (!right)
            return left;
        if (!left)
            return right;
        return new Infix(what->name, left, right, what->Position());
    }
    Tree *DoPrefix(Prefix *what)
    {
        if (widget->selected(what))
            return NULL;
        Tree *left = what->left->Do(this);
        Tree *right = what->right->Do(this);
        if (!right)
            return left;
        if (!left)
            return right;
        return new Prefix(left, right, what->Position());
    }
    Tree *DoPostfix(Postfix *what)
    {
        if (widget->selected(what))
            return NULL;
        Tree *left = what->left->Do(this);
        Tree *right = what->right->Do(this);
        if (!right)
            return left;
        if (!left)
            return right;
        return new Postfix(left, right, what->Position());
    }
    Tree *Do(Tree *what)
    {
        return what;            // ??? Should not happen
    }
    Widget *widget;
};


struct SetAttributeAction : XL::Action
// ----------------------------------------------------------------------------
//    Copy the inserted item as attribute in all selected items
// ----------------------------------------------------------------------------
{
    SetAttributeAction(text name, Tree *attribute,
                       Widget *widget, text shape = "shape")
        : name(name), attribute(attribute), widget(widget), shape(shape) {}

    Tree *Do(Tree *what)
    {
        if (widget->selected(what))
            widget->set(what, name, attribute, shape);
        return what;
    }

    text      name;
    Tree_p    attribute;
    Widget   *widget;
    text      shape;
};


struct NameToNameReplacement : XL::TreeClone
// ----------------------------------------------------------------------------
//    Replace specific names with names (e.g. alternate spellings)
// ----------------------------------------------------------------------------
{
    NameToNameReplacement(){}

    Tree *  DoName(XL::Name *what);
    Tree *  Replace(Tree *original);
    text &      operator[] (text index)         { return map[index]; }

    std::map<text, text> map;
};


struct NameToTextReplacement : NameToNameReplacement
// ----------------------------------------------------------------------------
//    Replace specific names with a text
// ----------------------------------------------------------------------------
{
    NameToTextReplacement(): NameToNameReplacement() {}
    Tree *  DoName(XL::Name *what);
};


struct InsertImageWidthAndHeightAction : XL::Action
// ----------------------------------------------------------------------------
// Action to insert the width and height of the image in the source.
// ----------------------------------------------------------------------------
{
    InsertImageWidthAndHeightAction(double w, double h)
        :ww(w), hh(h), done(false) {}
    Tree *Do (Tree *what) { return what;}
    Tree *DoInfix(Infix *what);
    double ww,hh;
    bool   done;
};


} // namespace Tao



// ============================================================================
//
//   Qt interface for XL types
//
// ============================================================================

#define TREEPOINTER_TYPE 383 // (QVariant::UserType | 0x100)
Q_DECLARE_METATYPE(XL::Tree_p)

#endif // TAO_WIDGET_H
