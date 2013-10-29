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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "main.h"
#include "tao.h"
#include "tao_tree.h"
#include "save.h"
#include "tree-clone.h"
#include "coords3d.h"
#include "matrix.h"
#include "opcodes.h"
#include "drawing.h"
#include "activity.h"
#include "menuinfo.h"
#include "color.h"
#include "glyph_cache.h"
#include "runtime.h"
#include "font_file_manager.h"
#include "layout.h"
#include "page_layout.h"
#include "tao_gl.h"
#include "statistics.h"
#include "file_monitor.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QGridLayout>
#include <QTextCursor>
#include <QImage>
#include <QTimeLine>
#include <QTimer>
#include <QSvgRenderer>
#include <QList>
#include <QColorDialog>
#include <QFontDialog>
#include <QMutex>
#include <QPrinter>
#include <iostream>
#include <map>
#include <set>

#if defined(Q_OS_MACX) && !defined(CFG_NODISPLAYLINK)
#define MACOSX_DISPLAYLINK 1
#endif

#ifdef MACOSX_DISPLAYLINK
typedef struct __CVDisplayLink *CVDisplayLinkRef;
#endif

namespace Tao {

struct Window;
struct FrameInfo;
struct PageLayout;
struct SpaceLayout;
struct GraphicPath;
struct Table;
struct Repository;
struct Drag;
struct TextSelect;
struct WidgetSurface;
struct CoordinatesInfo;
struct MouseFocusTracker;
struct DisplayDriver;
struct OpenGLState;
struct ShaderProgramInfo;

// ----------------------------------------------------------------------------
// Name of fixed menu.
// ----------------------------------------------------------------------------
// Menus then may be retrieved by
//   QMenu * view = window->findChild<QMenu*>(VIEW_MENU_NAME)
#define FILE_MENU_NAME  "TAO_FILE_MENU"
#define EDIT_MENU_NAME  "TAO_EDIT_MENU"
#define SHARE_MENU_NAME "TAO_SHARE_MENU"
#define VIEW_MENU_NAME  "TAO_VIEW_MENU"
#define TOOLBAR_MENU_NAME  "TAO_VIEW_TOOLBAR_MENU"
#define HELP_MENU_NAME  "TAO_HELP_MENU"

#define GUI_FEATURE "GUI"

struct Widget : QGLWidget
// ----------------------------------------------------------------------------
//   This is the widget we use to display XL programs output
// ----------------------------------------------------------------------------
{
private:
    Q_OBJECT
public:
    typedef std::vector<double>         attribute_args;
    typedef std::map<GLuint, uint>      selection_map;
    enum ShaderKind { VERTEX, FRAGMENT, GEOMETRY };

public:
    Widget(QWidget *parent, SourceFile *sf = NULL);
    Widget(Widget &other, const QGLFormat &format);
    ~Widget();

public slots:
    // Slots
    void        dawdle();
    void        draw();
    void        runProgram();
    void        runProgramOnce();
    void        print(QPrinter *printer);
    void        appFocusChanged(QWidget *prev, QWidget *next);
    void        userMenu(QAction *action);
    void        refresh(double delay = 0.0);
    bool        refreshNow();
    bool        refreshNow(QEvent *event);
#ifndef CFG_NOGIT
    void        commitSuccess(QString id, QString msg);
#endif
    void        colorChosen(const QColor &);
    void        colorChanged(const QColor &);
    void        colorRejected();
    void        updateColorDialog();
    void        fontChosen(const QFont &);
    void        updateFontDialog();
    void        updateDialogs()                { mustUpdateDialogs = true; }
    void        fileChosen(const QString & filename);
    void        copy();
    void        cut();
    void        paste();
    void        enableAnimations(bool enable);
    void        showHandCursor(bool enabled);
    void        hideCursor();
    void        setCursor(const QCursor &);
    QCursor     cursor() const;
    void        resetView();
    void        resetViewAndRefresh();
    void        zoomIn();
    void        zoomOut();
    void        saveAndCommit();
    void        renderFrames(int w, int h, double startT, double endT,
                             QString dir, double fps = 25.0, int page = 0,
                             double time_offset = 0.0,
                             QString displayName = "",
                             QString fileName = "frame%0d.png",
                             int firstFrame = 1);
    void        cancelRenderFrames(int s = 1) { renderFramesCanceled = s; }
    void        addToReloadList(const QString &path) { toReload.append(path); }
#ifdef MACOSX_DISPLAYLINK
    void        sendTimerEvent();
#endif


signals:
    // Signals
    void        copyAvailableAndNotReadOnly(bool yes);
    void        copyAvailable(bool hasSelection);
    void        renderFramesProgress(int percent);
    void        renderFramesDone();
    void        runGC();
    void        displayModeChanged(QString newMode);
#ifdef CFG_TIMED_FULLSCREEN
    void        userActivity();
#endif

public:
    // OpenGL and drawing
    void        initializeGL();
    void        resizeGL(int width, int height);
    void        paintGL();
    void        setup(double w, double h, const Box *picking = NULL);
    void        reset();
    void        resetModelviewMatrix();
    void        setupGL();
    void        setupPage();
    void        identifySelection();
    void        updateSelection();
    uint        showGlErrors();
    QFont &     currentFont();
    QPrinter *  currentPrinter() { return printer; }
    double      printerScaling() { return printer ? printOverscaling : 1; }
    double      scalingFactorFromCamera();
    void        legacyDraw();
    void        drawStereoIdent();
    void        drawScene();
    void        drawSelection();
    void        drawActivities();
    void        setGlClearColor();
    void        getCamera(Point3 *position, Point3 *target, Vector3 *upVector,
                          double *toScreen);
    bool        stereoIdentEnabled(void) { return stereoIdent; }

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
    virtual
    bool        event(QEvent *event);
#ifdef MACOSX_DISPLAYLINK
    void        displayLinkEvent();
#endif
    void        startPanning(QMouseEvent *);
    void        doPanning(QMouseEvent *);
    void        endPanning(QMouseEvent *);

    // XL program management
    bool        isBeingDestroyed() { return xlProgram == NULL; }
    bool        sourceChanged();
    void        normalizeProgram();
    void        updateProgram(SourceFile *sf);
    int         loadFile(text file);
    void        loadContextFiles(XL::source_names &files);
    void        reloadProgram(Tree *newProg = NULL);
    void        refreshProgram();
    void        preloadSelectionCode();
    void        updateProgramSource(bool notWhenHidden = true);
    bool        markChange(text reason);
    void        finishChanges();
    void        selectStatements(Tree *tree);
    bool        writeIfChanged(SourceFile &sf);
    bool        setDragging(bool on);
    bool        doSave(ulonglong tick);
#ifndef CFG_NOGIT
    bool        doPull(ulonglong tick);
    bool        doCommit(ulonglong tick);
#endif
    Repository *repository();
    Tree *      get(Tree *shape, text name, text sh = "group,shape");
    bool        set(Tree *shape, text n, Tree *value, text sh = "group,shape");
    bool        get(Tree *shape, text n, TreeList &a, text sh = "group,shape");
    bool        set(Tree *shape, text n, TreeList &a, text sh = "group,shape");
    bool        get(Tree *shape, text n, attribute_args &a,
                    text sh = "group,shape");
    bool        isReadOnly();
    QStringList listNames();
#ifndef CFG_NO_DOC_SIGNATURE
#ifndef TAO_PLAYER
    QString      signDocument(text path);
#endif
    bool         checkDocumentSigned();
#endif

    // Timing
    ulonglong   now();
    void        printStatistics();
    void        logStatistics();
    bool        hasAnimations(void)     { return animated; }
    void        resetTimes();

    // Selection
    GLuint      shapeId()               { return ++id; }
    GLuint      selectionId()           { return ++id | SHAPE_SELECTED; }
    GLuint      selectionCurrentId()    { return id; }
    GLuint      selectionHandleId()     { return handleId; }
    GLuint      selectionCapacity()     { return maxId * (maxIdDepth + 3); }

    enum
    {
        SHAPE_SELECTED     =  0x10000000, // Normal shape selection
        HANDLE_SELECTED    =  0x20000000, // A handle is selected
        CHARACTER_SELECTED =  0x30000000, // A character was selected
        CONTAINER_OPENED   =  0x40000000, // A shape container was opened
        CONTAINER_SELECTED =  0x50000000, // Container is selected
        SELECTION_MASK     =  0x70000000  // Mask for "regular" selection
    };
    void        select(uint id, uint count = 1);
    uint        selected(uint i);
    uint        selected(Layout *);
    void        reselect(Tree *from, Tree *to);

    void        select(Tree *tree)      { selectionTrees.insert(tree); }
    void        deselect(Tree *tree)    { selectionTrees.erase(tree); }
    bool        selected(Tree* tree)    { return selectionTrees.count(tree); }
    bool        hasSelection()          { return !selectionTrees.empty(); }

    void        selectionContainerPush();
    void        selectionContainerPop();

    void        saveSelectionColorAndFont(Layout *where);

    bool        focused(Layout *);
    void        deleteFocus(QWidget *widget);
    bool        requestFocus(QWidget *widget, coord x, coord y);
    void        recordProjection(GLdouble *projection,
                                 GLdouble *model,
                                 GLint *viewport);
    void        recordProjection();
    Point3      unproject (coord x, coord y, coord z,
                           GLdouble *projection,
                           GLdouble *model,
                           GLint *viewport);
    Point3      unproject (coord x, coord y, coord z = 0.0);
    Point3      unprojectLastMouse(GLdouble *projection,
                                   GLdouble *model,
                                   GLint *viewport);
    Point3      unprojectLastMouse();
    Point3      project (coord x, coord y, coord z);
    Point3      project (coord x, coord y, coord z,
                         GLdouble *proj, GLdouble *model, GLint *viewport);
    Point3      objectToWorld(coord x, coord y,
                              GLdouble *proj, GLdouble *model, GLint *viewport);
    Point3      windowToWorld(coord x, coord y,
                              GLdouble *proj, GLdouble *model, GLint *viewport);
    uint        lastModifiers()         { return keyboardModifiers; }
    Drag *      drag();
    TextSelect *textSelection();
    void        drawSelection(Layout *, const Box3 &, text name, uint id=0);
    void        drawHandle(Layout *, const Point3 &, text name, uint id=0);
    Layout *    drawTree(Layout *where, Context *context, Tree_p code);
    void        drawCall(Layout *, XL::XLCall &call, uint id=0);
    bool        mouseTracking() { return doMouseTracking; }
    bool        inMouseMove()   { return w_event &&
                                  w_event->type() == QEvent::MouseMove; }

    template<class Activity>
    Activity *  active();
    void        checkCopyAvailable();
    bool        canPaste();

    Tree *      shapeAction(text n, GLuint id, int x, int y);
    Tree_p      saveAndEvaluate(Context *context, Tree_p code);

    // Text flows and text management
    TextFlow *  pageLayoutFlow(text f)  { return flows[f]; }
    void        eraseFlow(text f)       { flows.erase(f);}
    GlyphCache &glyphs()                { return glyphCache; }
    QStringList fontFiles();

    void        purgeTaoInfo();

    FileMonitor & fileMonitor()         { return srcFileMonitor; }
#ifndef CFG_NO_DOC_SIGNATURE
    bool        isDocSigned() { return isDocumentSigned; }
#endif

public:
    static Widget *Tao()                { assert(current); return current; }
    static Widget *TaoExists()          { return current; }
    static Widget *findTaoWidget();
    Context *   formulasContext()       { return formulas; }
    static int  screenNumber() { return qApp->desktop()->screenNumber(Tao()); }

    // XLR entry points

    // Page definition and attributes
    Text_p      page(Context *context, Text_p name, Tree_p body);
    Text_p      pageLink(Tree_p self, text key, text name);
    Real_p      pageSetPrintTime(Tree_p self, double t);
    Text_p      gotoPage(Tree_p self, text page, bool abortTransition=false);
    Text_p      pageLabel(Tree_p self);
    Integer_p   pageNumber(Tree_p self);
    Integer_p   pageCount(Tree_p self);
    Text_p      pageNameAtIndex(Tree_p self, uint index);
    Real_p      pageWidth(Tree_p self);
    Real_p      pageHeight(Tree_p self);
    Text_p      prevPageLabel(Tree_p self);
    Integer_p   prevPageNumber(Tree_p self);

    // Transitions
    Tree_p      transition(Context *, Tree_p self, double dur, Tree_p body);
    Real_p      transitionTime(Tree_p self);
    Real_p      transitionDuration(Tree_p self);
    Real_p      transitionRatio(Tree_p self);
    Tree_p      transitionCurrentPage(Context *, Tree_p self);
    Tree_p      transitionNextPage(Context *, Tree_p self);
    Tree_p      runTransition(Context *);

    // Frame definition and transitions
    Real_p      frameWidth(Tree_p self);
    Real_p      frameHeight(Tree_p self);
    Real_p      frameDepth(Tree_p self);
    int         width();
    int         height();
    Real_p      windowWidth(Tree_p self);
    Real_p      windowHeight(Tree_p self);
    Real_p      time(Tree_p self);
    Real_p      pageTime(Tree_p self);
    Integer_p   pageSeconds(Tree_p self);
    Real_p      after(Context *context, double delay, Tree_p code);
    Real_p      every(Context *context, double delay, double duration, Tree_p code);
    Real_p      mouseX(Tree_p self);
    Real_p      mouseY(Tree_p self);
    Integer_p   screenMouseX(Tree_p self);
    Integer_p   screenMouseY(Tree_p self);
    Integer_p   mouseButtons(Tree_p self);
    Tree_p      shapeAction(Tree_p self, Context_p context, text name, Tree_p action);

    // Preserving attributes
    Tree_p      locally(Context *context, Tree_p self, Tree_p t);
    Tree_p      shape(Context *context, Tree_p self, Tree_p t);
    Tree_p      activeWidget(Context *context, Tree_p self, Tree_p t);
    Tree_p      anchor(Context *context, Tree_p self, Tree_p t, bool abs = false);
    Tree_p      stereoViewpoints(Context *ctx,Tree_p self,Integer_p e,Tree_p t);
    Integer_p   stereoViewpoints();

    // Transforms
    Tree_p      resetTransform(Tree_p self);
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
    Tree_p      clipPlane(Tree_p self, int plane,
                          double a, double b, double c, double d);

    // Setting attributes
    Tree_p      windowSize(Tree_p self, Integer_p width, Integer_p height);
    Name_p      depthTest(Tree_p self, bool enable);
    Name_p      depthMask(Tree_p self, bool enable);
    Name_p      depthFunction(XL::Tree_p self, text func);
    Name_p      blendFunction(Tree_p self, text src, text dst);
    Name_p      blendFunctionSeparate(Tree_p self,
                                      text src, text dst,
                                      text srca, text dsta);
    Name_p      blendEquation(Tree_p self, text eq);
    Tree_p      refresh(Tree_p self, double delay);
    Tree_p      refreshOn(Tree_p self, int eventType);
    Tree_p      noRefreshOn(Tree_p self, int eventType);
    Tree_p      defaultRefresh(Tree_p self, double delay);
    Real_p      refreshTime(Tree_p self);
    Tree_p      postEvent(int eventType, bool once = false);
    Integer_p   registerUserEvent(text name);
    Integer_p   seconds(Tree_p self, double t);
    Integer_p   minutes(Tree_p self, double t);
    Integer_p   hours(Tree_p self, double t);
    Integer_p   day(Tree_p self, double t);
    Integer_p   weekDay(Tree_p self, double t);
    Integer_p   yearDay(Tree_p self, double t);
    Integer_p   month(Tree_p self, double t);
    Integer_p   year(Tree_p self, double t);
#ifndef CFG_NOSRCEDIT
    Name_p      showSource(Tree_p self, bool show);
#endif
    Name_p      fullScreen(Tree_p self, bool fs);
    Name_p      toggleFullScreen(Tree_p self);
    Name_p      slideShow(XL::Tree_p self, bool ss);
    Name_p      toggleSlideShow(Tree_p self);
    Name_p      blankScreen(XL::Tree_p self, bool bs);
    Name_p      toggleBlankScreen(Tree_p self);
    Name_p      stereoIdentify(XL::Tree_p self, bool bs);
    Name_p      toggleStereoIdentify(Tree_p self);
    Name_p      toggleHandCursor(Tree_p self);
    Name_p      autoHideCursor(XL::Tree_p self, bool autoHide);
    Name_p      enableMouseCursor(XL::Tree_p self, bool on);
    Name_p      toggleAutoHideCursor(XL::Tree_p self);
    Name_p      showStatistics(Tree_p self, bool ss);
    Name_p      toggleShowStatistics(Tree_p self);
    Name_p      logStatistics(Tree_p self, bool ss);
    Name_p      toggleLogStatistics(Tree_p self);
    Integer_p   frameCount(Tree_p self);
    Name_p      resetViewAndRefresh(Tree_p self);
    Name_p      panView(Tree_p self, coord dx, coord dy);
    Real_p      currentZoom(Tree_p self);
    Name_p      setZoom(Tree_p self, scale z);
    Real_p      currentScaling(Tree_p self);
    Name_p      setScaling(Tree_p self, scale z);
    Infix_p     currentCameraPosition(Tree_p self);
    Name_p      setCameraPosition(Tree_p self, coord x, coord y, coord z);
    Infix_p     currentCameraTarget(Tree_p self);
    Name_p      setCameraTarget(Tree_p self, coord x, coord y, coord z);
    Infix_p     currentCameraUpVector(Tree_p self);
    Name_p      setCameraUpVector(Tree_p self, coord x, coord y, coord z);
    Name_p      setEyeDistance(Tree_p self, double eyeD);
    Real_p      getEyeDistance(Tree_p self);
    Name_p      setZNear(Tree_p self, double zn);
    Real_p      getZNear(Tree_p self);
    Name_p      setZFar(Tree_p self, double zf);
    Real_p      getZFar(Tree_p self);
    Name_p      setCameraToScreen(Tree_p self, double d);
    Real_p      getCameraToScreen(Tree_p self);
    Infix_p     currentModelMatrix(Tree_p self);
    Integer_p   lastModifiers(Tree_p self);

    Name_p      enableAnimations(Tree_p self, bool fs);
    Name_p      enableSelectionRectangle(Tree_p self, bool enable);
    Name_p      setDisplayMode(XL::Tree_p self, text name);
    Name_p      addDisplayModeToMenu(XL::Tree_p self, text mode, text label);
    Name_p      enableStereoscopy(Tree_p self, Name_p name);
    Name_p      enableStereoscopyText(Tree_p self, text name);
    Integer_p   polygonOffset(Tree_p self,
                              double f0, double f1, double u0, double u1);
    Name_p      enableVSync(Tree_p self, bool enable);
    double      optimalDefaultRefresh();
    bool        VSyncEnabled();
    bool        VSyncSupported();

    // Graphic attributes
    Tree_p      clearColor(Tree_p self, double r, double g, double b, double a);
    Tree_p      motionBlur(Tree_p self, double f);
    Tree_p      lineColorName(Tree_p self, text name, double a);
    Tree_p      lineColorRgb(Tree_p self, double r, double g, double b, double a);
    Tree_p      lineColorHsl(Tree_p self, double h, double s, double l, double a);
    Tree_p      lineColorHsv(Tree_p self, double h, double s, double v, double a);
    Tree_p      lineColorCmyk(Tree_p self, double c, double m, double y, double k, double a);
    Tree_p      visibility(Tree_p self, double lw);
    Tree_p      lineWidth(Tree_p self, double lw);
    Tree_p      lineStipple(Tree_p self, uint16 pattern, uint16 scale);
    Tree_p      fillColorName(Tree_p self, text name, double a);
    Tree_p      fillColorRgb(Tree_p self,
                             double r, double g, double b, double a);
    Tree_p      fillColorHsl(Tree_p self,
                             double h, double s, double l, double a);
    Tree_p      fillColorHsv(Tree_p self,
                             double h, double s, double v, double a);
    Tree_p      fillColorCmyk(Tree_p self,
                              double c, double m, double y, double k, double a);
    Tree_p      fillColorGradient(Tree_p self,
                                  Real_p pos, double r, double g, double b, double a);

    Integer*    fillTextureUnit(Tree_p self, GLuint texUnit);
    Integer*    fillTextureId(Tree_p self, GLuint texId);
    Integer*    fillTexture(Context *, Tree_p self, text fileName);
    Integer*    fillAnimatedTexture(Context *, Tree_p self, text fileName);
    Integer*    fillTextureFromSVG(Context *, Tree_p self, text svg);
    Tree_p      textureWrap(Tree_p self, bool s, bool t);
    Tree_p      textureMode(Tree_p self, text mode);
    Tree_p      textureMinFilter(Tree_p self, text filter);
    Tree_p      textureMagFilter(Tree_p self, text filter);
    Tree_p      textureTransform(Context *context, Tree_p self, Tree_p code);
    Integer*    textureWidth(Tree_p self);
    Integer*    textureHeight(Tree_p self);
    Integer*    textureType(Tree_p self);
    Text_p      textureMode(Tree_p self);
    Integer*    textureId(Tree_p self);
    Integer*    textureUnit(Tree_p self);
    Tree_p      hasTexture(Tree_p self, GLuint unit);
    Tree_p      extrudeDepth(Tree_p self, float depth);
    Tree_p      extrudeRadius(Tree_p self, float radius);
    Tree_p      extrudeCount(Tree_p self, int count);

    Integer_p   lightsMask(Tree_p self);
    Tree_p      perPixelLighting(Tree_p self,  bool enable);
    Tree_p      lightId(Tree_p self, GLuint id, bool enable);
    Tree_p      light(Tree_p self, GLenum function, GLfloat value);
    Tree_p      light(Tree_p self, GLenum function,
                      GLfloat x, GLfloat y, GLfloat z);
    Tree_p      light(Tree_p self, GLenum function,
                      GLfloat a, GLfloat b, GLfloat c, GLfloat d);
    Tree_p      material(Tree_p self, GLenum face, GLenum function, GLfloat d);
    Tree_p      material(Tree_p self, GLenum face, GLenum function,
                         GLfloat a, GLfloat b, GLfloat c, GLfloat d);
    Tree_p      shaderProgram(Context *, Tree_p self, Tree_p code);
    Tree_p      shaderFromSource(Tree_p self, ShaderKind kind, text source);
    Tree_p      shaderFromFile(Tree_p self, ShaderKind kind, text file);
    Tree_p      shaderSet(Context *, Tree_p self, Tree_p code);
    Text_p      shaderLog(Tree_p self);
    Name_p      setGeometryInputType(Tree_p self, uint inputType);
    Integer*    geometryInputType(Tree_p self);
    Name_p      setGeometryOutputType(Tree_p self, uint outputType);
    Integer*    geometryOutputType(Tree_p self);
    Name_p      setGeometryOutputCount(Tree_p self, uint outputCount);
    Integer*    geometryOutputCount(Tree_p self);

    // Generating a path
    Tree_p      newPath(Context *c, Tree_p self, Tree_p t);
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
    Tree_p      endpointsStyle(Tree_p self,symbol_r s,symbol_r e);

    // 2D primitive that can be in a path or standalone
    Tree_p      fixedSizePoint(Tree_p self, coord x,coord y,coord z, coord s);
    Tree_p      rectangle(Tree_p self, Real_p x, Real_p y, Real_p w, Real_p h);
    Tree_p      plane(Tree_p tree, Real_p x, Real_p y, Real_p w,
                      Real_p h, Integer_p lines_nb, Integer_p columns_nb);
    Tree_p      isoscelesTriangle(Tree_p self,
                                  Real_p x, Real_p y, Real_p w, Real_p h);
    Tree_p      rightTriangle(Tree_p self,
                              Real_p x, Real_p y, Real_p w, Real_p h);
    Tree_p      ellipse(Tree_p self, Real_p x, Real_p y,
                        Real_p w, Real_p h);
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

    Integer*    picturePacker(Tree_p self,
                              uint tw, uint th,
                              uint iw, uint ih,
                              uint pw, uint ph,
                              Tree_p t);
    Tree_p      debugParameters(Tree_p self,
                                double x, double y,
                                double w, double h);

    // 3D primitives
    Tree_p      sphere(Tree_p self,
                       Real_p cx, Real_p cy, Real_p cz,
                       Real_p w, Real_p, Real_p d,
                       Integer_p nslices, Integer_p nstacks);
    Tree_p      torus(Tree_p self,
                       Real_p x, Real_p y, Real_p z,
                       Real_p w, Real_p h, Real_p d,
                       Integer_p nslices, Integer_p nstacks, double ratio);
    Tree_p      cube(Tree_p self, Real_p cx, Real_p cy, Real_p cz,
                     Real_p w, Real_p h, Real_p d);
    Tree_p      cone(Tree_p self, Real_p cx, Real_p cy, Real_p cz,
                     Real_p w, Real_p h, Real_p d,
                     double ratio);

    // Text and font
    Tree_p      textBox(Context *context, Tree_p self,
                        Real_p x, Real_p y, Real_p w, Real_p h,
                        Tree_p body);
    Tree_p      textFlow(Context *context, Tree_p self, Text_p name, Tree_p child);
    Tree_p      textFlow(Context *context, Tree_p self, Text_p name);
    Text_p      textFlow(Context *context, Tree_p self);
    Name_p      textFlowExists(Context *context, Tree_p self, Text_p name);
    Tree_p      textSpan(Context *context, Tree_p self, Tree_p child);
    Tree_p      textUnit(Tree_p self, Text_p content);
    Box3        textSize(Tree_p self, Text_p content);
    Tree_p      textFormula(Tree_p self, Tree_p value);
    Tree_p      textValue(Context *, Tree_p self, Tree_p value);
    Tree_p      font(Context *context, Tree_p self,Tree_p dscr,Tree_p d2=NULL);
    Tree_p      fontFamily(Context *, Tree_p self, Text_p family);
    Tree_p      fontFamily(Context *, Tree_p self, Text_p family, Real_p size);
    Tree_p      fontSize(Tree_p self, double size);
    Tree_p      fontPlain(Tree_p self);
    Tree_p      fontItalic(Tree_p self, scale amount = 1);
    Tree_p      fontBold(Tree_p self, scale amount = 1);
    Tree_p      fontUnderline(Tree_p self, scale amount = 1);
    Tree_p      fontOverline(Tree_p self, scale amount = 1);
    Tree_p      fontStrikeout(Tree_p self, scale amount = 1);
    Tree_p      fontStretch(Tree_p self, scale amount = 1);
    Tree_p      align(Tree_p self,
                      scale center, scale justify, scale spread,
                      scale fullJustify, uint axis);
    Tree_p      spacing(Tree_p self, scale amount, uint axis);
    Tree_p      minimumSpace(Tree_p self, coord before, coord after, uint ax);
    Tree_p      horizontalMargins(Tree_p self, coord left, coord right);
    Tree_p      verticalMargins(Tree_p self, coord top, coord bottom);
    Tree_p      drawingBreak(Tree_p self, BreakOrder order);
    Name_p      textEditKey(Tree_p self, text key);
    Text_p      loremIpsum(Tree_p self, Integer_p nwords);
    Text_p      loadText(Tree_p self, text file, text encoding);
    Text_p      taoLanguage(Tree_p self);
    Text_p      taoVersion(Tree_p self);
    Text_p      taoEdition(Tree_p self);
    Text_p      docVersion(Tree_p self);
    Name_p      enableGlyphCache(Tree_p self, bool enable);
    Tree_p      glyphCacheSizeRange(Tree_p self, double min, double max);
    Tree_p      glyphCacheScaling(Tree_p self, double scaling, double minSize);
    Integer_p   glyphCacheTexture(Tree_p self);
    Text_p      unicodeChar(Tree_p self, int code);
    Text_p      unicodeCharText(Tree_p self, text code);

    // Tables
    Tree_p      newTable(Context *context, Tree_p self,
                         Real_p x, Real_p y,
                         Integer_p r, Integer_p c, Tree_p body);
    Tree_p      newTable(Context *context, Tree_p self,
                         Integer_p r, Integer_p c, Tree_p body);
    Tree_p      tableCell(Context *, Tree_p self,
                          Real_p w, Real_p h, Tree_p body);
    Tree_p      tableCell(Context *, Tree_p self, Tree_p body);
    Tree_p      tableMargins(Tree_p self,
                             Real_p x, Real_p y, Real_p w, Real_p h);
    Tree_p      tableMargins(Tree_p self,
                             Real_p w, Real_p h);
    Tree_p      tableFill(Tree_p self, Tree_p code);
    Tree_p	tableBorder(Tree_p self, Tree_p code);
    Real_p      tableCellX(Tree_p self);
    Real_p      tableCellY(Tree_p self);
    Real_p      tableCellW(Tree_p self);
    Real_p      tableCellH(Tree_p self);
    Integer_p   tableRow(Tree_p self);
    Integer_p   tableColumn(Tree_p self);
    Integer_p   tableRows(Tree_p self);
    Integer_p   tableColumns(Tree_p self);

    // Frames and widgets
    Tree_p      status(Tree_p self, text t, float timeout);
    Integer*    framePaint(Context *context, Tree_p self,
                           Real_p x, Real_p y, Real_p w, Real_p h,
                           Tree_p prog);
    Integer*    frameTexture(Context *context, Tree_p self,
                             double w, double h, Tree_p prog, text name = "",
                             Integer_p depth=NULL, bool canvas=false);
    Integer *   framePixelCount(Tree_p self, float alphaMin);
    Tree*       drawingCache(Context *context, Tree_p self,
                             double version, Tree_p prog);
    Integer*    thumbnail(Context *, Tree_p self, scale s, double i, text page);
    Name_p      saveThumbnail(Context *context, Tree_p self, int w, int h,
                              int page, text file, double pageTime = 0.0);
    Integer*    linearGradient(Context *context, Tree_p self,
                               Real_p start_x, Real_p start_y, Real_p end_x, Real_p end_y,
                               double w, double h, Tree_p prog);
    Integer*    radialGradient(Context *context, Tree_p self,
                               Real_p center_x, Real_p center_y, Real_p radius,
                               double w, double h, Tree_p prog);
    Integer*    conicalGradient(Context *context, Tree_p self,
                                Real_p center_x, Real_p center_y, Real_p angle,
                                double w, double h, Tree_p prog);

    Name_p      offlineRendering(Tree_p self);

    Tree_p      urlPaint(Tree_p self, Real_p x, Real_p y, Real_p w, Real_p h,
                         text_p s, integer_p p);
    Integer*    urlTexture(Tree_p self,
                           double x, double y,
                           Text_p s, Integer_p p);

    Tree_p      lineEdit(Tree_p self, Real_p x,Real_p y,
                         Real_p w,Real_p h, text_p s);
    Integer*    lineEditTexture(Tree_p self, double x, double y, Text_p s);

    Tree_p      textEdit(Context *context, Tree_p self,
                         Real_p x, Real_p y, Real_p w, Real_p h, Tree_p prog);
    Tree_p      textEditTexture(Context *context, Tree_p self,
                                double w, double h, Tree_p prog);

    Tree_p      abstractButton(Tree_p self, Text_p name,
                               Real_p x, Real_p y, Real_p w, Real_p h);
    Tree_p      pushButton(Tree_p self, Real_p x, Real_p y, Real_p w, Real_p h,
                           text_p name, text_p lbl, Tree_p act);
    Integer*    pushButtonTexture(Tree_p self, double w, double h,
                                  text_p name, Text_p lbl, Tree_p act);
    Tree_p      radioButton(Tree_p self, Real_p x,Real_p y, Real_p w,Real_p h,
                            text_p name, text_p lbl,
                            Text_p selected, Tree_p act);
    Integer*    radioButtonTexture(Tree_p self, double w, double h,
                                   text_p name, Text_p lbl,
                                   Text_p selected, Tree_p act);
    Tree_p      checkBoxButton(Tree_p self,
                               Real_p x,Real_p y, Real_p w, Real_p h,
                               text_p name, text_p lbl, Text_p  marked,
                               Tree_p act);
    Integer*    checkBoxButtonTexture(Tree_p self,
                                      double w, double h,
                                      text_p name, Text_p lbl,
                                      Text_p  marked, Tree_p act);
    Tree_p      buttonGroup(Context *context, Tree_p self,
                            bool exclusive, Tree_p buttons);
    Tree_p      setButtonGroupAction(Tree_p self, Tree_p action);

    Tree_p      colorChooser(Tree_p self, text name, Tree_p action);

    Tree_p      fontChooser(Tree_p self, text name, Tree_p action);
    Tree_p      fileChooser(Tree_p self, Tree_p action);
    Tree_p      setFileDialogAction(Tree_p self, Tree_p action);
    Tree_p      setFileDialogDirectory(Tree_p self, text dirname);
    Tree_p      setFileDialogFilter(Tree_p self, text filters);
    Tree_p      setFileDialogLabel(Tree_p self, text label, text value);

    Tree_p      groupBox(Context *context, Tree_p self,
                         Real_p x,Real_p y, Real_p w,Real_p h,
                         text_p lbl, Tree_p buttons);
    Integer*    groupBoxTexture(Tree_p self,
                                double w, double h,
                                Text_p lbl);

    Integer*    image(Context *context,
                      Tree_p self, Real_p x, Real_p y, Real_p w, Real_p h,
                      text filename);
    Tree_p      listFiles(Context *context, Tree_p self, Tree_p pattern);
    Infix_p     imageSize(Context *context,
                          Tree_p self, text filename);

    // Menus and widgets
    Tree_p      chooser(Context *, Tree_p self, text caption);
    Tree_p      chooserChoice(Tree_p self, text caption, Tree_p command);
    Tree_p      chooserCommands(Tree_p self, text prefix, text label);
    Tree_p      chooserPages(Tree_p self, Name_p prefix, text label);
    Tree_p      chooserBranches(Tree_p self, Name_p prefix, text label);
    Tree_p      chooserCommits(Tree_p self, text branch, Name_p prefix, text label);
    Tree_p      checkout(Tree_p self, text what);
    Name_p      currentRepository(Tree_p self);
    Tree_p      closeCurrentDocument(Tree_p self);
    Tree_p      quitTao(Tree_p self);

    static Tree_p runtimeError(Tree_p self, text msg, Tree_p src);
    static Tree_p formulaRuntimeError(Tree_p self, text msg, Tree_p src);
    void        clearErrors();
    void        checkErrors(bool clear);
    Tree_p      menuItem(Tree_p self, text name, text lbl, text iconFileName,
                         bool isCheckable, Text_p isChecked, Tree_p t);
    Tree_p      menuItemEnable(Tree_p self, text name, bool enable);
    Tree_p      menu(Tree_p self, text name, text lbl, text iconFileName,
                     bool isSubmenu=false);

    // The location is the prefered location for the toolbar.
    // The supported values are North, East, South, West or N, E, S, W
    Tree_p      toolBar(Tree_p self, text name, text title, bool isFloatable,
                        text location);

    Tree_p      menuBar(Tree_p self);
    Tree_p      separator(Tree_p self);

    // Tree management
    Name_p      insert(Tree_p self, Tree_p toInsert, text msg ="Inserted tree");
    void        deleteSelection();
    Name_p      deleteSelection(Tree_p self, text key);
    Name_p      setAttribute(Tree_p self, text name, Tree_p attribute, text sh);
    Tree_p      copySelection();

    // Unit conversions
    Real_p      fromCm(Tree_p self, double cm);
    Real_p      fromMm(Tree_p self, double mm);
    Real_p      fromIn(Tree_p self, double in);
    Real_p      fromPt(Tree_p self, double pt);
    Real_p      fromPx(Tree_p self, double px);

    Tree_p      constant(Tree_p self, Tree_p tree);

    // Misc
    Name_p      taoFeatureAvailable(Tree_p self, Name_p name);
    Text_p      GLVersion(XL::Tree_p self);
    Name_p      isGLExtensionAvailable(Tree_p self, text name);
    Name_p      hasDisplayMode(Tree_p self, Name_p name);
    Real_p      getWorldZ(Tree_p, Real_p x, Real_p y);
    Real_p      getWorldCoordinates(Tree_p, Real_p x, Real_p y,
                                    Real_p wx, Real_p wy, Real_p wz);
    Name_p      hasDisplayModeText(Tree_p self, text name);
    Name_p      displaySet(Context *context, Tree_p self, Tree_p code);
    Text_p      displayMode();
    Name_p      readOnly();
    Text_p      baseName(Tree_p, text filename);
    Text_p      dirName(Tree_p, text filename);
    Name_p      openUrl(Tree_p, text url);
    Name_p      screenShot(Tree_p, text filename, bool withAlpha);

    // License checks
    Name_p      hasLicense(Tree_p self, Text_p feature);
    Name_p      checkLicense(Tree_p self, Text_p feature, Name_p critical);
    Name_p      checkImpressOrLicense(Tree_p self, Text_p feature);
    Name_p      blink(Tree_p self, Real_p on, Real_p off, Real_p after);

    // z order management
    Name_p      bringToFront(Tree_p self);
    Name_p      sendToBack(Tree_p self);
    Name_p      bringForward(Tree_p self);
    Name_p      sendBackward(Tree_p self);

    // group management
    Tree_p      group(Context *context, Tree_p self, Tree_p shapes);
    Name_p      groupSelection(Tree_p self);
    Name_p      ungroupSelection(Tree_p self);

    //Documentation
    Text_p generateDoc(Tree_p self, Tree_p tree, text defGrp = "");
    Text_p generateAllDoc(Tree_p self, text filename);

    // Text translation
    Text_p xlTrAdd(Tree_p self, text from, text to);
    Text_p xlTr(Tree_p self, text t);

private:
    friend struct Window;
    friend struct Activity;
    friend struct Identify;
    friend struct Selection;
    friend struct MouseFocusTracker;
    friend struct Drag;
    friend struct TextSelect;
    friend struct TextSplit;
    friend struct Manipulator;
    friend struct ControlPoint;
    friend struct Renormalize;
    friend struct Table;
    friend struct DeleteSelectionAction;
    friend struct ModuleRenderer;
    friend struct Layout;
    friend struct StereoLayout;
    friend struct PageLayout;
    friend struct DisplayDriver;
    friend struct GCThread;
    friend struct WidgetSurface;

    struct ContextAndCode
    {
        ContextAndCode() : context(0), code(0) {}
        ContextAndCode(XL::Context_p context, XL::Tree_p code)
            : context(context), code(code) {}
        ~ContextAndCode() { context = 0; code = 0; }
        XL::Context_p context;
        XL::Tree_p    code;
    };

    typedef XL::Save<QEvent *>               EventSave;
    typedef XL::Save<Widget *>               TaoSave;
    typedef std::map<text, TextFlow*>        flow_map;
    typedef std::map<text, text>             page_map;
    typedef std::vector<text>                page_list;
    typedef std::map<GLuint, Tree_p>         perId_action_map;
    typedef std::map<text, perId_action_map> action_map;
    typedef std::map<Tree_p, ContextAndCode> page_action_map;
    typedef std::map<Tree_p, GLuint>         GLid_map;
    typedef std::set<Tree_p>                 tree_set;

    struct StereoIdentTexture
    {
        StereoIdentTexture(int w, int h, GLuint tex) : w(w), h(h), tex(tex) {}
        int    w, h;
        GLuint tex;
    };

    // XL Runtime
    SourceFile           *xlProgram;
    Context_p             formulas;
    bool                  inError;
    bool                  mustUpdateDialogs;
    bool                  runOnNextDraw;
    bool                  contextFilesLoaded;
    FileMonitor           srcFileMonitor;
    QStringList           toReload;

    // Rendering
    OpenGLState           gl;   // Must appear first (ctor and dtor order)
    QGradient*            gradient;
    QColor                clearCol;
    SpaceLayout *         space;
    Layout *              layout;
    FrameInfo *           frameInfo;
    GraphicPath *         path;
    Table *               table;
    scale                 pageW, pageH, blurFactor;
    text                  currentFlowName;
    flow_map              flows;
    text                  prevPageName, pageName, lastPageName;
    text                  gotoPageName, transitionPageName;
    page_map              pageLinks;
    page_list             pageNames, newPageNames;
    uint                  pageId, pageFound, prevPageShown, pageShown, pageTotal, pageToPrint;
    Tree_p                pageTree, transitionTree;
    double                transitionStartTime, transitionDurationValue;
    Tree_p                currentShape;
    QGridLayout *         currentGridLayout;
    ShaderProgramInfo *   currentShaderProgram;
    GroupInfo   *         currentGroup;
    GlyphCache            glyphCache;
    FontFileManager *     fontFileMgr;
    std::vector<StereoIdentTexture>
                          stereoIdentPatterns;
    bool                  drawAllPages;
    bool                  animated;
    bool                  blanked;
    bool                  stereoIdent;
    bool                  selectionRectangleEnabled;
    bool                  doMouseTracking;
    bool                  runningTransitionCode;
    GLint                 mouseTrackingViewport[4];
    int                   stereoPlane, stereoPlanes;
    DisplayDriver *       displayDriver;
    GLuint                watermark;
    text                  watermarkText;
    int                   watermarkWidth, watermarkHeight;
    bool                  showingEvaluationWatermark;
#ifdef Q_OS_MACX
    bool                  frameBufferReady();
    char                  bFrameBufferReady;
#else
    bool                  frameBufferReady() { return true; }
#endif
    QString               screenShotPath;
    bool                  screenShotWithAlpha;
#ifdef Q_OS_LINUX
    bool                  vsyncState;
#endif

    // Selection
    Activity *            activities;
    GLuint                id, focusId, maxId, idDepth, maxIdDepth, handleId;
    selection_map         selection;
    tree_set              selectionTrees, selectNextTime;
    action_map            actionMap;
    page_action_map       pageChangeActions;
    bool                  hadSelection;
    bool                  selectionChanged;
    QEvent *              w_event;
    QWidget *             focusWidget;
    GLdouble              focusProjection[16], focusModel[16];
    GLint                 focusViewport[4];
    uint                  keyboardModifiers;


    // Menus and widgets
    QMenu                *currentMenu;
    QMenuBar             *currentMenuBar;
    QToolBar             *currentToolBar;
    QVector<MenuInfo*>    menuItems;
    int                   menuCount;
    Tree_p                colorAction, fontAction;
    text                  colorName;
    std::map<text,Color>  selectionColor;
    QFont                 selectionFont;
    QColor                originalColor;
    int                   lastMouseX, lastMouseY, lastMouseButtons;
    CoordinatesInfo*      mouseCoordinatesInfo;
    MouseFocusTracker *   mouseFocusTracker;

    // Timing
#ifdef MACOSX_DISPLAYLINK
    QMutex                displayLinkMutex;
    CVDisplayLinkRef      displayLink;
    bool                  displayLinkStarted;
    bool                  pendingDisplayLinkEvent;
    bool                  stereoBuffersEnabled;
    int                   stereoSkip;
    bool                  holdOff;
    unsigned int          droppedFramesLocked();
    unsigned int          droppedFrames;
#else
    QBasicTimer           timer;
#endif
    double                dfltRefresh;
    QTimer                idleTimer;
    double                pageStartTime, frozenTime, startTime, currentTime;
    Statistics            stats;
    longlong              frameCounter;
    ulonglong             nextSave, nextSync;
#ifndef CFG_NOGIT
    ulonglong             nextCommit, nextPull;
#endif

    // Printing
    double                pagePrintTime;
    uint                  printOverscaling;
    QPrinter             *printer;

    static Widget *       current;
    static QColorDialog * colorDialog;
    static QFontDialog *  fontDialog;
    static QFileDialog *  fileDialog;
           QFileDialog *  currentFileDialog;
    double                zNear, zFar, scaling, zoom, eyeDistance;
    double                cameraToScreen;
    Point3                cameraPosition, cameraTarget;
    Vector3               cameraUpVector;
    int                   panX, panY;
    bool                  dragging;
    bool                  bAutoHideCursor;
    Qt::CursorShape       savedCursorShape;
    QCursor               cachedCursor;
    bool                  mouseCursorHidden;
    int                   renderFramesCanceled;
    bool                  inOfflineRendering;
    int                   offlineRenderingWidth;
    int                   offlineRenderingHeight;
    std::map<text, QFileDialog::DialogLabel> toDialogLabel;
    std::set<int>         pendingEvents;

private:
    Window *              taoWindow();
    StereoIdentTexture    newStereoIdentTexture(int i);
    void                  updateStereoIdentPatterns(int nb);
    void        runPurgeAction(XL::Action &action);
    void        updateFileDialog(Tree *properties, Tree *context);
    Tree_p      updateParentWithGroupInPlaceOfChild(Tree *parent, Tree *child, Tree_p sel);
    bool    updateParentWithChildrenInPlaceOfGroup(Tree *parent, Prefix *group);

    void                  refreshOn(int type,
                                    double nextRefresh = DBL_MAX);
    void                  commitPageChange(bool afterTransition);

public:
    static bool           refreshOnAPI(int event_type, double next_refresh);
    static double         currentTimeAPI();
    static double         currentPageTimeAPI();
    static void           makeGLContextCurrent();
    static bool           addControlBox(Real *x, Real *y, Real *z,
                                        Real *w, Real *h, Real *d);
    static bool           isGLExtensionAvailable(text name);
    static text           currentDocumentFolder();
    static bool           blink(double on, double off, double after);
    void                  setWatermarkText(text t, int w, int h);
    static void           setWatermarkTextAPI(text t, int w, int h);
    void                  drawFullScreenTexture(int texw, int texh, GLuint tex,
                                                bool centered = false);
    void                  drawWatermark();
    static void           drawWatermarkAPI();
    static double         trueCurrentTime();
    static void           postEventAPI(int eventType);
    static bool           postEventOnceAPI(int eventType);
    static bool           offlineRenderingAPI();

private:
    void                  processProgramEvents();
    void                  startRefreshTimer(bool on = true);
    double                CurrentTime();
    void                  setCurrentTime();
    bool inDraw;
    text                  changeReason;

    QTextCursor          * editCursor;
    void                  updateCursor(Text_p t);

    std::map<text, text>  xlTranslations;
    bool                  isInvalid;
#ifndef CFG_NO_DOC_SIGNATURE
    bool                  isDocumentSigned;
#endif
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
        return updateRef(what, new Integer(what->value, what->Position()));
    }
    Tree *DoReal(Real *what)
    {
        if (widget->selected(what))
            return NULL;
        return updateRef(what, new Real(what->value, what->Position()));

    }
    Tree *DoText(Text *what)
    {
        if (widget->selected(what))
            return NULL;
        return updateRef(what, new Text(what->value, what->opening, what->closing,
                        what->Position()));
    }
    Tree *DoName(Name *what)
    {
        if (widget->selected(what))
            return NULL;
        return updateRef(what, new Name(what->value, what->Position()));
    }

    Tree *DoBlock(Block *what)
    {
        if (widget->selected(what))
            return NULL;
        Tree *child = what->child->Do(this);
        if (!child)
            return NULL;
        return updateRef(what, new Block(child, what->opening, what->closing,
                                         what->Position()));
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
        return updateRef(what, new Infix(what->name, left, right,
                                         what->Position()));
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
        return updateRef(what, new Prefix(left, right, what->Position()));
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
        return updateRef(what, new Postfix(left, right, what->Position()));
    }
    Tree *Do(Tree *what)
    {
        return what;            // ??? Should not happen
    }
    Widget *widget;
    Tree *updateRef(Tree *from, Tree *to)
    {
        // Check if we are possibly changing the page tree reference
        if (widget->pageTree == from)
            widget->pageTree = to;

        return to;

    }
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

} // namespace Tao



// ============================================================================
//
//   Qt interface for XL types
//
// ============================================================================

#define TREEPOINTER_TYPE 383 // (QVariant::UserType | 0x100)
Q_DECLARE_METATYPE(XL::Tree_p)

#endif // TAO_WIDGET_H
