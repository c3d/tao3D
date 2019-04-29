#ifndef WIDGET_H
#define WIDGET_H
// *****************************************************************************
// widget.h                                                        Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2011-2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2010-2012, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2014, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
// (C) 2011-2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
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
#include "preview.h"
#include "tao_process.h"

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

RECORDER_DECLARE(focus);
RECORDER_DECLARE(primitive);
RECORDER_DECLARE(widget);


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
struct XLSourceEdit;

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
    void        renderFrames(int w, int h, double start_time, double duration,
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
    double      printerScaling() { return (printer
                                           ? printOverscaling
                                           : devicePixelRatio); }
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
    int         renderWidth()  { return width()  * devicePixelRatio; }
    int         renderHeight() { return height() * devicePixelRatio; }

    // Events
    bool        forwardEvent(QEvent *event);
    bool        forwardEvent(QMouseEvent *event);
    void        keyPressEvent(QKeyEvent *event);
    void        keyReleaseEvent(QKeyEvent *event);
    void        handleKeyEvent(QKeyEvent *event, bool keypress);
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
    void        checkEditorInstructionsFile();
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
    void        listNames(XLSourceEdit *editor);
#ifndef CFG_NO_DOC_SIGNATURE
#ifndef TAO_PLAYER
    QString     signDocument(text path);
#endif
    bool        checkDocumentSigned();
    void        excludeFromSignature(text path);
#endif
    QString     documentPath();
    void        addSearchPath(Scope *scope, text prefix, text path);
    text        resolvePrefixedPath(Scope *scope, text prefixedPath);

    // Timing
    ulonglong   now();
    void        printStatistics();
    void        printPerLayoutStatistics();
    void        logStatistics();
    bool        hasAnimations(void)     { return animated; }
    void        resetTimes();

    // Selection
    GLuint      shapeId()               { return ++id; }
    GLuint      selectionId()           { return ++id | SHAPE_SELECTED; }
    GLuint      selectionCurrentId()    { return id; }
    GLuint      selectionHandleId()     { return handleId; }
    GLuint      selectionCapacity()     { return maxId * (maxIdDepth + 3); }
    Layout *    currentLayout()         { return layout; }

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
    void        drawSelection(const Box3 &, text name,Layout * = 0);
    void        drawHandle(const Point3 &, text name, Layout * = 0, uint id=0);
    Layout *    drawTree(Scope *scope, Tree *code, Layout * = 0);
    void        drawCall(XL::XLCall &call, Layout * = 0);
    bool        mouseTracking() { return doMouseTracking; }
    bool        inMouseMove()   { return w_event &&
                                  w_event->type() == QEvent::MouseMove; }

    template<class Activity>
    Activity *  active();
    void        checkCopyAvailable();
    bool        canPaste();

    Tree *      shapeAction(text n, GLuint id, int x, int y);
    Tree *      saveAndEvaluate(Scope *scope, Tree *code);

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
    Text *      page(Scope *scope, Text *name, Tree *body);
    Text *      pageUniqueName(Tree *self, text name);
    Text *      pageLink(Tree *self, text key, text name);
    Real *      pageSetPrintTime(Tree *self, double t);
    Text *      gotoPage(Tree *self, text page, bool abortTransition=false);
    Text *      gotoPage(Tree *self, Text *page, bool abortTransition=false);
    Text *      pageLabel(Tree *self);
    Integer *   pageNumber(Tree *self);
    Integer *   pageCount(Tree *self);
    Text *      pageNameAtIndex(Tree *self, uint index);
    Real *      pageWidth(Tree *self);
    Real *      pageHeight(Tree *self);
    Text *      prevPageLabel(Tree *self);
    Integer *   prevPageNumber(Tree *self);

    // Transitions
    Tree *      transition(Scope *, Tree *self, double dur, Tree *body);
    Real *      transitionTime(Tree *self);
    Real *      transitionDuration(Tree *self);
    Real *      transitionRatio(Tree *self);
    Tree *      transitionCurrentPage(Scope *, Tree *self);
    Tree *      transitionNextPage(Scope *, Tree *self);
    Tree *      runTransition(Scope *);

    // Frame definition and transitions
    Real *      frameWidth(Tree *self);
    Real *      frameHeight(Tree *self);
    Real *      frameDepth(Tree *self);
    int         width();
    int         height();
    Real *      windowWidth(Tree *self);
    Real *      windowHeight(Tree *self);
    Real *      time(Tree *self);
    Real *      pageTime(Tree *self);
    Integer *   pageSeconds(Tree *self);
    Real *      after(Scope *scope, double delay, Tree *code);
    Real *      every(Scope *scope, double delay, double duration, Tree *code);
    Name *      once(Scope *scope, Tree *self, Tree *prog);
    Real *      mouseX(Tree *self);
    Real *      mouseY(Tree *self);
    Integer *   screenMouseX(Tree *self);
    Integer *   screenMouseY(Tree *self);
    Integer *   mouseButtons(Tree *self);
    Tree *      shapeAction(Scope *, Tree *self, text name, Tree *action);

    // Preserving attributes
    Tree *      locally(Scope *scope, Tree *self, Tree *body);
    Tree *      shape(Scope *scope, Tree *self, Tree *body);
    Tree *      activeWidget(Scope *scope, Tree *self, Tree *t);
    Tree *      anchor(Scope *scope, Tree *self, Tree *t, bool abs=false);
    Tree *      stereoViewpoints(Scope *ctx,Tree *self,Integer *e,Tree *t);
    Integer *   stereoViewpoints();

    // Transforms
    Tree *      resetTransform(Tree *self);
    Tree *      rotatex(Tree *self, Real *rx);
    Tree *      rotatey(Tree *self, Real *ry);
    Tree *      rotatez(Tree *self, Real *rz);
    Tree *      rotate(Tree *self, Real *ra,Real *rx,Real *ry,Real *rz);
    Tree *      translatex(Tree *self, Real *x);
    Tree *      translatey(Tree *self, Real *y);
    Tree *      translatez(Tree *self, Real *z);
    Tree *      translate(Tree *self, Real *x, Real *y, Real *z);
    Tree *      rescalex(Tree *self, Real *x);
    Tree *      rescaley(Tree *self, Real *y);
    Tree *      rescalez(Tree *self, Real *z);
    Tree *      rescale(Tree *self, Real *x, Real *y, Real *z);
    Tree *      clipPlane(Tree *self, int plane,
                          double a, double b, double c, double d);

    // Setting attributes
    Tree *      windowSize(Tree *self, Integer *width, Integer *height);
    Name *      depthTest(Tree *self, bool enable);
    Name *      depthMask(Tree *self, bool enable);
    Name *      depthFunction(XL::Tree *self, text func);
    Name *      blendFunction(Tree *self, text src, text dst);
    Name *      blendFunctionSeparate(Tree *self,
                                      text src, text dst,
                                      text srca, text dsta);
    Name *      blendEquation(Tree *self, text eq);
    Tree *      refresh(Tree *self, double delay);
    Tree *      refreshOn(Tree *self, int eventType);
    void        refreshOn(int type, double nextRefresh = DBL_MAX);
    Tree *      noRefreshOn(Tree *self, int eventType);
    Tree *      defaultRefresh(Tree *self, double delay);
    Real *      refreshTime(Tree *self);
    Real *      defaultCurveSteps(Tree *self,scale min,scale incr,scale max);
    Tree *      postEvent(int eventType, bool once = false);
    Integer *   registerUserEvent(text name);
    Name *      addLayoutName(text name);
    Name *      refreshAlso(text name);
    Integer *   seconds(Tree *self, double t);
    Integer *   minutes(Tree *self, double t);
    Integer *   hours(Tree *self, double t);
    Integer *   day(Tree *self, double t);
    Integer *   weekDay(Tree *self, double t);
    Integer *   yearDay(Tree *self, double t);
    Integer *   month(Tree *self, double t);
    Integer *   year(Tree *self, double t);
#ifndef CFG_NOSRCEDIT
    Name *      showSource(Tree *self, bool show);
#endif
    Name *      fullScreen(Tree *self, bool fs);
    Name *      toggleFullScreen(Tree *self);
    Name *      slideShow(XL::Tree *self, bool ss);
    Name *      toggleSlideShow(Tree *self);
    Name *      blankScreen(XL::Tree *self, bool bs);
    Name *      toggleBlankScreen(Tree *self);
    Name *      stereoIdentify(XL::Tree *self, bool bs);
    Name *      toggleStereoIdentify(Tree *self);
    Name *      toggleHandCursor(Tree *self);
    Name *      autoHideCursor(XL::Tree *self, bool autoHide);
    Name *      enableMouseCursor(XL::Tree *self, bool on);
    Name *      toggleAutoHideCursor(XL::Tree *self);
    Name *      showStatistics(Tree *self, bool ss);
    Name *      toggleShowStatistics(Tree *self);
    Name *      logStatistics(Tree *self, bool ss);
    Name *      toggleLogStatistics(Tree *self);
    Integer *   frameCount(Tree *self);
    Name *      resetViewAndRefresh(Tree *self);
    Name *      panView(Tree *self, coord dx, coord dy);
    Real *      currentZoom(Tree *self);
    Name *      setZoom(Tree *self, scale z);
    Real *      currentScaling(Tree *self);
    Name *      setScaling(Tree *self, scale z);
    Infix *     currentCameraPosition(Tree *self);
    Name *      setCameraPosition(Tree *self, coord x, coord y, coord z);
    Infix *     currentCameraTarget(Tree *self);
    Name *      setCameraTarget(Tree *self, coord x, coord y, coord z);
    Infix *     currentCameraUpVector(Tree *self);
    Name *      setCameraUpVector(Tree *self, coord x, coord y, coord z);
    Name *      setEyeDistance(Tree *self, double eyeD);
    Real *      getEyeDistance(Tree *self);
    Name *      setZNear(Tree *self, double zn);
    Real *      getZNear(Tree *self);
    Name *      setZFar(Tree *self, double zf);
    Real *      getZFar(Tree *self);
    Name *      setCameraToScreen(Tree *self, double d);
    Real *      getCameraToScreen(Tree *self);
    Infix *     currentModelMatrix(Tree *self);
    Integer *   lastModifiers(Tree *self);

    Name *      enableAnimations(Tree *self, bool fs);
    Name *      enableSelectionRectangle(Tree *self, bool enable);
    Name *      setDisplayMode(XL::Tree *self, text name);
    Name *      addDisplayModeToMenu(XL::Tree *self, text mode, text label);
    Name *      enableStereoscopy(Tree *self, Name *name);
    Name *      enableStereoscopyText(Tree *self, text name);
    Integer *   polygonOffset(Tree *self,
                              double f0, double f1, double u0, double u1);
    Name *      enableVSync(Tree *self, bool enable);
    double      optimalDefaultRefresh();
    bool        VSyncEnabled();
    bool        VSyncSupported();

    // Graphic attributes
    static QColor colorByName(text name, double alpha=1.0);
    static QColor colorRGB(scale r, scale g, scale b, scale a = 1.0);
    static QColor colorHSV(scale h, scale s, scale v, scale a = 1.0);
    static QColor colorHSL(scale h, scale s, scale l, scale a = 1.0);
    static QColor colorCMYK(scale c, scale m, scale y, scale k, scale a = 1.0);
    Tree *      clearColor(Tree *self, QColor c);
    Tree *      fillColor(Tree *self, QColor c);
    Tree *      lineColor(Tree *self, QColor c);
    Tree *      gradientColor(Tree *self, double pos, QColor c);
    Tree *      motionBlur(Tree *self, double f);
    Tree *      visibility(Tree *self, double lw);
    Tree *      lineWidth(Tree *self, double lw);
    Tree *      lineStipple(Tree *self, uint16 pattern, uint16 scale);
    Tree *      lineStipple(Tree *self, text nae);

    Integer*    fillTextureUnit(Tree *self, GLuint texUnit);
    Integer*    fillTextureId(Tree *self, GLuint texId);
    Integer*    fillTexture(Scope *, Tree *self, text fileName);
    Integer*    fillAnimatedTexture(Scope *, Tree *self, text fileName);
    Integer*    fillTextureFromSVG(Scope *, Tree *self, text svg);
    Tree *      textureWrap(Tree *self, bool s, bool t);
    Tree *      textureMode(Tree *self, text mode);
    Tree *      textureMinFilter(Tree *self, text filter);
    Tree *      textureMagFilter(Tree *self, text filter);
    Tree *      textureTransform(Scope *scope, Tree *self, Tree *code);
    Integer*    textureWidth(Tree *self);
    Integer*    textureHeight(Tree *self);
    Integer*    textureType(Tree *self);
    Text *      textureMode(Tree *self);
    Integer*    textureId(Tree *self);
    Integer*    textureUnit(Tree *self);
    Tree *      hasTexture(Tree *self, GLuint unit);
    Tree *      extrudeDepth(Tree *self, float depth);
    Tree *      extrudeRadius(Tree *self, float radius);
    Tree *      extrudeCount(Tree *self, int count);

    Integer *   lightsMask(Tree *self);
    Tree *      perPixelLighting(Tree *self,  bool enable);
    Tree *      lightId(Tree *self, GLuint id, bool enable);
    Tree *      light(Tree *self, GLenum function, GLfloat value);
    Tree *      light(Tree *self, GLenum function,
                      GLfloat x, GLfloat y, GLfloat z);
    Tree *      light(Tree *self, GLenum function,
                      GLfloat a, GLfloat b, GLfloat c, GLfloat d);
    Tree *      material(Tree *self, GLenum face, GLenum function, GLfloat d);
    Tree *      material(Tree *self, GLenum face, GLenum function,
                         GLfloat a, GLfloat b, GLfloat c, GLfloat d);

#define LIGHT_ADAPT(name,arg)                                   \
    Tree * name(Tree *self, QColor c)                           \
    {                                                           \
        return light(self, arg,                                 \
                     c.redF(),c.greenF(),c.blueF(),c.alphaF()); \
    }
    LIGHT_ADAPT(lightAmbient, GL_AMBIENT)
    LIGHT_ADAPT(lightDiffuse, GL_DIFFUSE)
    LIGHT_ADAPT(lightSpecular, GL_SPECULAR)
#undef LIGHT_ADAPT

#define MATERIAL_ADAPT(name,arg)                                        \
    Tree * name(Tree *self, QColor c)                                   \
    {                                                                   \
        return material(self, GL_FRONT_AND_BACK, arg,                   \
                        c.redF(),c.greenF(),c.blueF(),c.alphaF());      \
    }                                                                   \
    Tree * name##F(Tree *self, QColor c)                                \
    {                                                                   \
        return material(self, GL_FRONT, arg,                            \
                        c.redF(),c.greenF(),c.blueF(),c.alphaF());      \
    }                                                                   \
    Tree * name##B(Tree *self, QColor c)                                \
    {                                                                   \
        return material(self, GL_BACK, arg,                             \
                        c.redF(),c.greenF(),c.blueF(),c.alphaF());      \
    }
    MATERIAL_ADAPT(materialAmbient, GL_AMBIENT)
    MATERIAL_ADAPT(materialDiffuse, GL_DIFFUSE)
    MATERIAL_ADAPT(materialSpecular, GL_SPECULAR)
    MATERIAL_ADAPT(materialEmission, GL_EMISSION)
#undef MATERIAL_ADAPT

    Tree *      shaderProgram(Scope *, Tree *self, Tree *code);
    Tree *      shaderFromSource(Tree *self, ShaderKind kind, text source);
    Tree *      shaderFromFile(Scope *, Tree *self, ShaderKind k, text file);
    Tree *      shaderSet(Scope *, Tree *self, Tree *code);
    Text *      shaderLog(Tree *self);
    Name *      setGeometryInputType(Tree *self, uint inputType);
    Integer*    geometryInputType(Tree *self);
    Name *      setGeometryOutputType(Tree *self, uint outputType);
    Integer*    geometryOutputType(Tree *self);
    Name *      setGeometryOutputCount(Tree *self, uint outputCount);
    Integer*    geometryOutputCount(Tree *self);

    // Generating a path
    Tree *      newPath(Scope *c, Tree *self, Tree *t);
    Tree *      moveTo(Tree *self, Real *x, Real *y, Real *z);
    Tree *      lineTo(Tree *self, Real *x, Real *y, Real *z);
    Tree *      curveTo(Tree *self,
                        Real *cx, Real *cy, Real *cz,
                        Real *x, Real *y, Real *z);
    Tree *      curveTo(Tree *self,
                        Real *c1x, Real *c1y, Real *c1z,
                        Real *c2x, Real *c2y, Real *c2z,
                        Real *x, Real *y, Real *z);
    Tree *      moveToRel(Tree *self, Real *x, Real *y, Real *z);
    Tree *      lineToRel(Tree *self, Real *x, Real *y, Real *z);
    Tree *      pathTextureCoord(Tree *self, Real *x, Real *y, Real *r);
    Tree *      closePath(Tree *self);
    Tree *      endpointsStyle(Tree *self, Name *s, Name *e);
    Tree *      endpointsStyle(Tree *self, text s, text e);

    // 2D primitive that can be in a path or standalone
    Tree *      fixedSizePoint(Tree *self, coord x,coord y,coord z, coord s);
    Tree *      rectangle(Tree *self, Real *x, Real *y, Real *w, Real *h);
    Tree *      plane(Tree *tree, Real *x, Real *y, Real *w,
                      Real *h, Integer *lines_nb, Integer *columns_nb);
    Tree *      isoscelesTriangle(Tree *self,
                                  Real *x, Real *y, Real *w, Real *h);
    Tree *      rightTriangle(Tree *self,
                              Real *x, Real *y, Real *w, Real *h);
    Tree *      ellipse(Tree *self, Real *x, Real *y,
                        Real *w, Real *h);
    Tree *      ellipseArc(Tree *self, Real *x, Real *y, Real *w, Real *h,
                           Real *start, Real *sweep);
    Tree *      ellipseSector(Tree *self, Real *x, Real *y, Real *w, Real *h,
                             Real *start, Real *sweep);
    Tree *      roundedRectangle(Tree *self,
                                 Real *cx, Real *cy, Real *w, Real *h,
                                 Real *r);
    Tree *      ellipticalRectangle(Tree *self,
                                    Real *cx, Real *cy, Real *w, Real *h,
                                    Real *r);
    Tree *      arrow(Tree *self, Real *cx, Real *cy, Real *w, Real *h,
                      Real *ax, Real *ary);
    Tree *      doubleArrow(Tree *self,
                            Real *cx, Real *cy, Real *w, Real *h,
                            Real *ax, Real *ary);
    Tree *      starPolygon(Tree *self,
                            Real *cx, Real *cy, Real *w, Real *h,
                            Integer *p, Integer *q);
    Tree *      star(Tree *self, Real *cx, Real *cy, Real *w, Real *h,
                     Integer *p, Real *r);
    Tree *      speechBalloon(Tree *self,
                              Real *cx, Real *cy, Real *w, Real *h,
                              Real *r, Real *ax, Real *ay);
    Tree *      callout(Tree *self,
                        Real *cx, Real *cy, Real *w, Real *h,
                        Real *r, Real *ax, Real *ay, Real *d);

    Integer*    picturePacker(Tree *self,
                              uint tw, uint th,
                              uint iw, uint ih,
                              uint pw, uint ph,
                              Tree *t);
    Tree *      debugParameters(Tree *self,
                                double x, double y,
                                double w, double h);

    // 3D primitives
    Tree *      sphere(Tree *self,
                       Real *cx, Real *cy, Real *cz,
                       Real *w, Real *h, Real *d,
                       Integer *nslices, Integer *nstacks);
    Tree *      torus(Tree *self,
                       Real *x, Real *y, Real *z,
                       Real *w, Real *h, Real *d,
                       Integer *nslices, Integer *nstacks, double ratio);
    Tree *      cube(Tree *self, Real *cx, Real *cy, Real *cz,
                     Real *w, Real *h, Real *d);
    Tree *      cone(Tree *self, Real *cx, Real *cy, Real *cz,
                     Real *w, Real *h, Real *d,
                     double ratio);

    // Text and font
    Tree *      textBox(Scope *scope, Tree *self,
                        Real *x, Real *y, Real *w, Real *h,
                        Tree *body);
    Tree *      textFlow(Scope *scope, Tree *self, Text *name, Tree *child);
    Tree *      textFlow(Scope *scope, Tree *self, Text *name);
    Text *      textFlow(Scope *scope, Tree *self);
    Name *      textFlowExists(Scope *scope, Tree *self, Text *name);
    Tree *      textSpan(Scope *scope, Tree *self, Tree *child);
    Tree *      textUnit(Tree *self, Text *content);
    Tree *      htmlTextUnit(Scope *scope,Tree *self,text html,text css);
    Box3        textSize(Tree *self, Text *content);
    Tree *      textFormula(Scope *scope, Tree *self, Tree *value);
    Tree *      textValue(Scope *, Tree *self, Tree *value);
    Tree *      font(Scope *scope, Tree *self,Tree *dscr,Tree *d2=NULL);
    Tree *      fontFamily(Scope *, Tree *self, Text *family);
    Tree *      fontFamily(Scope *, Tree *self, Text *family, Real *size);
    Tree *      fontSize(Tree *self, double size);
    Tree *      fontPlain(Tree *self);
    Tree *      fontItalic(Tree *self, scale amount = 1);
    Tree *      fontBold(Tree *self, scale amount = 1);
    Tree *      fontUnderline(Tree *self, scale amount = 1);
    Tree *      fontOverline(Tree *self, scale amount = 1);
    Tree *      fontStrikeout(Tree *self, scale amount = 1);
    Tree *      fontStretch(Tree *self, scale amount = 1);
    Tree *      align(Tree *self,
                      scale center, scale justify, scale spread,
                      scale fullJustify, uint axis);
    Tree *      spacing(Tree *self, scale amount, uint axis);
    Tree *      minimumSpace(Tree *self, coord before, coord after, uint ax);
    Tree *      horizontalMargins(Tree *self, coord left, coord right);
    Tree *      verticalMargins(Tree *self, coord top, coord bottom);
    Tree *      drawingBreak(Tree *self, BreakOrder order);
    Name *      textEditKey(Scope *, Tree *self, text key);
    Text *      loremIpsum(Scope *, Tree *self, Integer *nwords);
    Text *      loadText(Scope *, Tree *self, text file, text encoding);
    Text *      taoLanguage(Tree *self);
    Text *      taoVersion(Tree *self);
    Text *      taoEdition(Tree *self);
    Text *      docVersion(Tree *self);
    Name *      enableGlyphCache(Tree *self, bool enable);
    Tree *      glyphCacheSizeRange(Tree *self, double min, double max);
    Tree *      glyphCacheScaling(Tree *self, double scaling, double minSize);
    Integer *   glyphCacheTexture(Tree *self);
    Text *      unicodeChar(Tree *self, int code);
    Text *      unicodeCharText(Tree *self, text code);

    // Tables
    Tree *      newTable(Scope *scope, Tree *self,
                         Real *x, Real *y,
                         Integer *r, Integer *c, Tree *body);
    Tree *      newTable(Scope *scope, Tree *self,
                         Integer *r, Integer *c, Tree *body);
    Tree *      tableCell(Scope *, Tree *self,
                          Real *w, Real *h, Tree *body);
    Tree *      tableCell(Scope *, Tree *self, Tree *body);
    Tree *      tableMargins(Tree *self,
                             Real *x, Real *y, Real *w, Real *h);
    Tree *      tableMargins(Tree *self,
                             Real *w, Real *h);
    Tree *      tableFill(Tree *self, Tree *code);
    Tree *	tableBorder(Tree *self, Tree *code);
    Real *      tableCellX(Tree *self);
    Real *      tableCellY(Tree *self);
    Real *      tableCellW(Tree *self);
    Real *      tableCellH(Tree *self);
    Integer *   tableRow(Tree *self);
    Integer *   tableColumn(Tree *self);
    Integer *   tableRows(Tree *self);
    Integer *   tableColumns(Tree *self);

    // Frames and widgets
    Tree *      status(Tree *self, text t, float timeout);
    Integer*    framePaint(Scope *scope, Tree *self,
                           Real *x, Real *y, Real *w, Real *h,
                           Tree *prog);
    Integer*    frameTexture(Scope *scope, Tree *self,
                             double w, double h, Tree *prog, text name = "",
                             Integer *depth=NULL, bool canvas=false);
    Integer *    framePixelCount(Tree *self, float alphaMin);
    Integer *    framePixel(Tree *self, float x, float y,
                           Real *r, Real *g, Real *b, Real *a);
    Tree*       drawingCache(Scope *scope, Tree *self,
                             double version, Tree *prog);
    Integer*    thumbnail(Scope *, Tree *self, scale s, double i, text page);
    Name *      saveThumbnail(int w, int h, int page,
                              text file, double pageTime = 0.0);
    Integer*    linearGradient(Scope *scope, Tree *self,
                               Real *start_x, Real *start_y, Real *end_x, Real *end_y,
                               double w, double h, Tree *prog);
    Integer*    radialGradient(Scope *scope, Tree *self,
                               Real *center_x, Real *center_y, Real *radius,
                               double w, double h, Tree *prog);
    Integer*    conicalGradient(Scope *scope, Tree *self,
                                Real *center_x, Real *center_y, Real *angle,
                                double w, double h, Tree *prog);

    Name *      offlineRendering(Tree *self);

    Tree *      urlPaint(Tree *self,
                         Real *x, Real *y, Real *w, Real *h,
                         Text *url, Integer *p);
    Integer*    urlTexture(Tree *self,
                           double x, double y,
                           Text *s, Integer *p);

    Tree *      lineEdit(Scope *scope, Tree *self,
                         Real *x, Real *y, Real *w, Real *h,
                         Text *txt);
    Integer*    lineEditTexture(Scope *scope, Tree *self,
                                double x, double y, Text *txt);

    Tree *      textEdit(Scope *scope, Tree *self,
                         Real *x, Real *y, Real *w, Real *h, Text *html);
    Tree *      textEditTexture(Scope *scope, Tree *self,
                                double w, double h, Text *html);

    Tree *      abstractButton(Scope *scope, Tree *self, Text *name,
                               Real *x, Real *y, Real *w, Real *h);
    Tree *      pushButton(Scope *scope, Tree *self,
                           Real *x, Real *y, Real *w, Real *h,
                           Text *name, Text *label, Tree *act);
    Integer*    pushButtonTexture(Scope *scope, Tree *self,
                                  double w, double h,
                                  Text *name, Text *label, Tree *act);
    Tree *      radioButton(Scope *scope, Tree *self,
                            Real *x,Real *y, Real *w,Real *h,
                            Text *name, Text *label,
                            Text *selected, Tree *act);
    Integer*    radioButtonTexture(Scope *scope, Tree *self,
                                   double w, double h,
                                   Text *name, Text *label,
                                   Text *selected, Tree *act);
    Tree *      checkBoxButton(Scope *scope, Tree *self,
                               Real *x,Real *y, Real *w, Real *h,
                               Text *name, Text *label, Text * marked,
                               Tree *act);
    Integer*    checkBoxButtonTexture(Scope *scope, Tree *self,
                                      double w, double h,
                                      Text *name, Text *label,
                                      Text * marked, Tree *act);
    Tree *      buttonGroup(Scope *scope, Tree *self,
                            bool exclusive, Tree *buttons);
    Tree *      setButtonGroupAction(Scope *scope, Tree *self, Tree *action);

    Tree *      colorChooser(Scope *, Tree *self, text name, Tree *action);

    Tree *      fontChooser(Scope *, Tree *self, text name, Tree *action);
    Tree *      fileChooser(Scope *, Tree *self, Tree *action);
    Tree *      setFileDialogAction(Scope *, Tree *self, Tree *action);
    Tree *      setFileDialogDirectory(Tree *self, text dirname);
    Tree *      setFileDialogFilter(Tree *self, text filters);
    Tree *      setFileDialogLabel(Tree *self, text label, text value);

    Tree *      groupBox(Scope *scope, Tree *self,
                         Real *x,Real *y, Real *w,Real *h,
                         Text *label, Tree *buttons);
    Integer*    groupBoxTexture(Scope *, Tree *self,
                                double w, double h,
                                Text *label);

    Integer*    image(Scope *scope, Tree *self,
                      Real *x, Real *y, Real *w, Real *h,
                      text filename);
    Tree *      listFiles(Scope *scope, Tree *self, Tree *pattern);
    Infix *     imageSize(Scope *scope,
                          Tree *self, text filename);

    // Menus and widgets
    Tree *      chooser(Scope *, Tree *self,
                        text caption);
    Tree *      chooserChoice(Tree *self,
                              text caption, Tree *command);
    Tree *      chooserCommands(Tree *self,
                                text prefix, text label);
    Tree *      chooserPages(Tree *self,
                             Name *prefix, text label);
    Tree *      chooserBranches(Tree *self,
                                Name *prefix, text label);
    Tree *      chooserCommits(Tree *self,
                               text branch, Name *prefix, text label);
    Tree *      checkout(Tree *self, text what);
    Name *      currentRepository(Tree *self);
    Tree *      closeCurrentDocument(Tree *self);
    Tree *      quitTao(Tree *self);

    static Tree *runtimeError(Scope *, Tree *self, kstring msg, Tree *src);
    static Tree *formulaRuntimeError(Scope *, Tree *self, kstring msg, Tree *src);
    void        clearErrors();
    void        checkErrors();
    Tree *      menuItem(Scope *, Tree *self,
                         text name, text label, text iconFileName,
                         bool isCheckable, Text *isChecked, Tree *t);
    Tree *      menuItemEnable(Tree *self, text name, bool enable);
    Tree *      menu(text name, text label, text iconFileName,
                     bool isSubmenu=false);

    // The location is the prefered location for the toolbar.
    // The supported values are North, East, South, West or N, E, S, W
    Tree *      toolBar(text name, text title, bool isFloatable, text location);

    Tree *      menuBar();
    Tree *      separator();

    // Tree management
    Name *      insert(Scope *, Tree *self, Tree *toInsert, text msg ="Inserted");
    void        deleteSelection();
    Name *      deleteSelection(Scope *scope, Tree *self, text key);
    Name *      setAttribute(text name, Tree *attribute, text sh);
    Tree *      copySelection();

    // Unit conversions
    Real *      fromCm(Tree *self, double cm);
    Real *      fromMm(Tree *self, double mm);
    Real *      fromIn(Tree *self, double in);
    Real *      fromPt(Tree *self, double pt);
    Real *      fromPx(Tree *self, double px);

    Tree *      constant(Tree *self, Tree *tree);

    // Misc
    Name *      taoFeatureAvailable(Tree *self, Name *name);
    Text *      GLVersion(XL::Tree *self);
    Name *      isGLExtensionAvailable(Tree *self, text name);
    Real *      getWorldZ(Tree *, Real *x, Real *y);
    Real *      getWorldCoordinates(Tree *, Real *x, Real *y,
                                    Real *wx, Real *wy, Real *wz);
    Name *      hasDisplayModeText(Tree *self, text name);
    Name *      displaySet(Scope *scope, Tree *self, Tree *code);
    Text *      displayMode();
    Name *      readOnly();
    Text *      baseName(Tree *self, text filename);
    Text *      dirName(Tree *self, text filename);
    Name *      openUrl(text url);
    Name *      screenShot(text filename, scale sz, bool withAlpha);

    // License checks
    Name *      hasLicense(Tree *self, Text *feature);
    Name *      checkLicense(Tree *self, Text *feature, Name *critical);
    Name *      blink(Tree *self, Real *on, Real *off, Real *after);

    // Z order management
    Name *      bringToFront(Tree *self);
    Name *      sendToBack(Tree *self);
    Name *      bringForward(Tree *self);
    Name *      sendBackward(Tree *self);

    // Group management
    Tree *      group(Scope *scope, Tree *self, Tree *shapes);
    Name *      groupSelection(Tree *self);
    Name *      ungroupSelection(Tree *self);

    // Documentation
    Text *      generateDoc(Tree *self, Tree *tree, text defGrp = "");
    Text *      generateAllDoc(Tree *self, text filename);

    // Text translation
    Text * xlTrAdd(Tree *self, text from, text to);
    Text * xlTr(Tree *self, text t);

    // Run process
    Name *      runProcess(Tree *self, text name, text args);
    Name *      runProcess(Tree *self, text name, QStringList &args);
    Name *      writeToProcess(Tree *self, text name, text data);
    Text *      readFromProcess(Tree *self, text name, uint lines = ~0U);
    Name *      dropProcess(Tree *self, text name);

    // Run rsync
    Text *      runRsync(Tree *self, text opts, text source, text dest);

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

    struct ScopeAndCode
    {
        ScopeAndCode(): scope(), code() {}
        ScopeAndCode(Scope *scope, Tree *code): scope(scope), code(code) {}
        ~ScopeAndCode() {}
        Scope_p scope;
        Tree_p  code;
    };

    typedef XL::Save<QEvent *>               EventSave;
    typedef XL::Save<Widget *>               TaoSave;
    typedef std::map<text, TextFlow*>        flow_map;
    typedef std::map<text, text>             page_map;
    typedef std::vector<text>                page_list;
    typedef std::map<GLuint, Tree_p>         perId_action_map;
    typedef std::map<text, perId_action_map> action_map;
    typedef std::map<Tree_p, ScopeAndCode>   page_action_map;
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
    QStringList           excludedFromSignature;

    // Rendering
    OpenGLState           gl;   // Must appear first (ctor and dtor order)
    QSharedPointer<TextureCache> textureCache;
    QGradient*            gradient;
    QColor                clearCol;
    SpaceLayout *         space;
    Layout *              layout;
    FrameInfo *           frameInfo;
    GraphicPath *         path;
    Table *               table;
    scale                 pageW, pageH, blurFactor, devicePixelRatio;
    text                  currentFlowName;
    flow_map              flows;
    text                  prevPageName, pageName, lastPageName;
    text                  gotoPageName, transitionPageName;
    page_map              pageLinks;
    page_list             pageNames, newPageNames;
    uint                  pageId, pageFound, prevPageShown, pageShown, pageTotal, pageToPrint;
    uint                  pageEntry, pageExit;
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
    uint                  stereoPlane, stereoPlanes;
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
    PreviewThread         savePreviewThread;
    PreviewThread         saveProofOfPlayThread;
    QString               screenShotPath;
    scale                 screenShotScale;
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
    text                  prevKeyPressText; // persists until next QKeyEvent
public:
    // Key event info accessed directly from .tbl. Valid only when current
    // refresh is caused by the QKeyEvent
    bool                  keyPressed;   // false if released
    text                  keyEventName; // e.g., "a", "A", "~A", "~Ctrl-A"
    text                  keyText;      // QKeyEvent::text()
    text                  keyName;      // QKeyEvent::key()
private:

    // Menus and widgets
    QMenu                *currentMenu;
    QMenuBar             *currentMenuBar;
    QToolBar             *currentToolBar;
    std::vector<MenuInfo*>menuItems;
    uint                  menuCount;
    Tree_p                colorAction, fontAction;
    Scope_p               colorScope, fontScope;
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

    // Processes
    QMap<QString,Process*>processMap;

private:
    Window *              taoWindow();
    StereoIdentTexture    newStereoIdentTexture(int i);
    void                  updateStereoIdentPatterns(int nb);
    template <typename Action>
    void                  runPurgeAction(Action &action);
    void                  updateFileDialog(Scope *scope, Tree *properties);
    Tree *                updateParentWithGroupInPlaceOfChild(Tree *parent,
                                                              Tree *child,
                                                              Tree *sel);
    bool                  updateParentWithChildrenInPlaceOfGroup(Tree *parent,
                                                                 Prefix *group);

    void                  commitPageChange(bool afterTransition);
    bool                  runPageExitHandlers();

public:
    static bool           refreshOnAPI(int event_type, double next_refresh);
    static double         currentTimeAPI();
    static double         currentPageTimeAPI();
    static double         DevicePixelRatioAPI();
    static bool           RenderingTransparencyAPI();
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
    bool                  inDraw, inRunPageExitHandlers, pageHasExitHandler;
    text                  changeReason;

    std::map<text, text>  xlTranslations;
    bool                  isInvalid;
#ifndef CFG_NO_DOC_SIGNATURE
    bool                  isDocumentSigned;
#endif
#ifdef CFG_UNLICENSED_MAX_PAGES
    bool                  pageLimitationDialogShown;
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
#define RTAO(x)         RESULT(Tao::Widget::Tao()->x)

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

struct DeleteSelectionAction
// ----------------------------------------------------------------------------
//    A specialized clone action that doesn't copy selected trees
// ----------------------------------------------------------------------------
{
    typedef Tree *value_type;

    DeleteSelectionAction(Widget *widget): widget(widget) {}
    Tree *Do(Integer *what)
    {
        if (widget->selected(what))
            return NULL;
        return updateRef(what, new Integer(what->value, what->Position()));
    }
    Tree *Do(Real *what)
    {
        if (widget->selected(what))
            return NULL;
        return updateRef(what, new Real(what->value, what->Position()));

    }
    Tree *Do(Text *what)
    {
        if (widget->selected(what))
            return NULL;
        return updateRef(what, new Text(what->value, what->opening, what->closing,
                        what->Position()));
    }
    Tree *Do(Name *what)
    {
        if (widget->selected(what))
            return NULL;
        return updateRef(what, new Name(what->value, what->Position()));
    }

    Tree *Do(Block *what)
    {
        if (widget->selected(what))
            return NULL;
        Tree *child = what->child->Do(this);
        if (!child)
            return NULL;
        return updateRef(what, new Block(child, what->opening, what->closing,
                                         what->Position()));
    }
    Tree *Do(XL::Infix *what)
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
    Tree *Do(Prefix *what)
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
    Tree *Do(Postfix *what)
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


struct SetAttributeAction
// ----------------------------------------------------------------------------
//    Copy the inserted item as attribute in all selected items
// ----------------------------------------------------------------------------
{
    typedef Tree *value_type;
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
Q_DECLARE_METATYPE(XL::Scope_p)

#endif // TAO_WIDGET_H
