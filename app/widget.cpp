// *****************************************************************************
// widget.cpp                                                      Tao3D project
// *****************************************************************************
//
// File description:
//
//     The main widget used to display some Tao stuff
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
// (C) 2010-2013, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2014, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
// (C) 2011-2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2010, Christophe de Dinechin <christophe@dinechin.org>
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

#include "widget.h"
#include "tao_main.h"
#include "main.h"
#include "runtime.h"
#include "opcodes.h"
#include "gl_keepers.h"
#include "opengl_state.h"
#include "frame.h"
#include "texture.h"
#include "svg.h"
#include "widget_surface.h"
#include "window.h"
#include "apply_changes.h"
#include "activity.h"
#include "selection.h"
#include "drag.h"
#include "manipulator.h"
#include "menuinfo.h"
#ifndef CFG_NOGIT
#include "repository.h"
#endif
#include "application.h"
#include "tao_utf8.h"
#include "tao_glu.h"
#include "layout.h"
#include "page_layout.h"
#include "space_layout.h"
#include "shapes.h"
#include "text_drawing.h"
#include "shapes3d.h"
#include "path3d.h"
#include "table.h"
#include "attributes.h"
#include "transforms.h"
#include "lighting.h"
#include "undo.h"
#include "serializer.h"
#include "binpack.h"
#include "normalize.h"
#include "error_message_dialog.h"
#include "group_layout.h"
#include "font.h"
#include "chooser.h"
#include "tree_cloning.h"
#include "documentation.h"
#include "formulas.h"
#include "html_converter.h"
#include "xl_source_edit.h"
#include "tool_window.h"
#include "tree-walk.h"
#include "raster_text.h"
#include "dir.h"
#include "display_driver.h"
#include "license.h"
#include "gc_thread.h"
#include "info_trash_can.h"
#include "tao_info.h"
#include "preferences_pages.h"
#include "texture_cache.h"
#include "tao_process.h"

#ifndef CFG_NO_DOC_SIGNATURE
#include "document_signature.h"
#endif
#ifdef CFG_UNLICENSED_MAX_PAGES
#include "nag_screen.h"
#endif
#include <recorder/recorder.h>

#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <queue>
#include <sys/time.h>
#include <string.h>
#include <ctype.h>

#include <QtGui>
#include <QDialog>
#include <QTextCursor>
#include <QApplication>
#include <QToolButton>
#include <QtGui/QImage>
#include <QFont>
#include <QVariant>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#ifndef CFG_NO_QTWEBKIT
#include <QtWebKit>
#endif
#include <QRunnable>
#include <QThreadPool>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#ifdef MACOSX_DISPLAYLINK
#include <CoreVideo/CoreVideo.h>

static int DisplayLink = -1;
#endif

#ifdef Q_OS_LINUX
#include "vsync_x11.h"
#endif

#define TAO_CLIPBOARD_MIME_TYPE "application/tao-clipboard"

#define CHECK_0_1_RANGE(var)                    \
    if (var < 0)                                \
        var = 0;                                \
    else if (var > 1)                           \
        var = 1;
#define CHECK_0_255_RANGE(var)                  \
    if (var < 0)                                \
        var = 0;                                \
    else if (var > 255)                         \
        var = 255;
#define CHECK_0_359_RANGE(var)                  \
    if (var < 0 || var >= 360)                  \
    {                                           \
        var = var % 360;                        \
        if (var < 0)                            \
            var += 360;                         \
    }
#define CHECK_0_1_RANGE_MODULO(var)             \
    if (var < 0 || var >= 1)                    \
    {                                           \
        var -= floor(var);                      \
    }

RECORDER(clipboard,             16, "Clipboard");
RECORDER(focus,                 16, "Application focus");
RECORDER(frames,                16, "Frame textures");
RECORDER(license_error,          4, "Errors reported relative to licensing");
RECORDER(pages,                 16, "Page management");
RECORDER(picture_pack_error,     4, "Errors from picture packer");
RECORDER(polycount,             16, "Polygon counts");
RECORDER(primitive,             32, "Tao primitives (C++ code)");
RECORDER(print,                 16, "Printing");
RECORDER(shaders,               16, "GLSL shaders");
RECORDER(widget,                32, "Tao main drawing widget");

RECORDER_DECLARE(menus);

namespace Tao {

extern const char *GITREV_;



// ============================================================================
//
//   Widget life management
//
// ============================================================================

static Point3 defaultCameraPosition(0, 0, 3000);


Widget * Widget::findTaoWidget()
// ----------------------------------------------------------------------------
//   Find the Widget on the main Window. Use when Tao() is not set.
// ----------------------------------------------------------------------------
{
    if (current) return current;
    return TaoApp->window()->taoWidget;
}


static inline QGLFormat TaoGLFormat()
// ----------------------------------------------------------------------------
//   Return the options we will use when creating the widget
// ----------------------------------------------------------------------------
//   This was made necessary by Bug #251
{
    // Note: check #2407 if you intend to add QGL::AccumBuffers here
    // (it is sometimes incompatible with QGL::SampleBuffers)
    QGL::FormatOptions options =
        (QGL::DoubleBuffer      |
         QGL::DepthBuffer       |
         QGL::StencilBuffer     |
         QGL::AlphaChannel);
    if (TaoApp->hasGLMultisample)
        options |= QGL::SampleBuffers;
    QGLFormat format(options);
    // VSync is set later in the Widget constructor
    format.setSwapInterval(0);
    return format;
}


Widget::Widget(QWidget *parent, SourceFile *sf)
// ----------------------------------------------------------------------------
//    Create the GL widget
// ----------------------------------------------------------------------------
    : QGLWidget(TaoGLFormat(), parent),
      xlProgram(sf), formulas(NULL), inError(false), mustUpdateDialogs(false),
      runOnNextDraw(true), contextFilesLoaded(false),
      srcFileMonitor("XL"),
      textureCache(NULL),
      gradient(NULL), clearCol(255, 255, 255, 255),
      space(NULL), layout(NULL), frameInfo(NULL), path(NULL), table(NULL),
      pageW(21), pageH(29.7), blurFactor(0.0), devicePixelRatio(1.0),
      currentFlowName(""), prevPageName(""), pageName(""), lastPageName(""),
      gotoPageName(""), transitionPageName(""),
      pageId(0), pageFound(0), prevPageShown(0), pageShown(1), pageTotal(0),
      pageEntry(0), pageExit(0),
      pageTree(NULL), transitionTree(NULL),
      transitionStartTime(0.0), transitionDurationValue(0.0),
      currentShape(NULL), currentGridLayout(NULL),
      currentShaderProgram(NULL), currentGroup(NULL),
      fontFileMgr(NULL),
      drawAllPages(false), animated(true),
      selectionRectangleEnabled(true),
      doMouseTracking(true), runningTransitionCode(false),
      stereoPlane(1), stereoPlanes(1),
      watermark(0), watermarkWidth(0), watermarkHeight(0),
      showingEvaluationWatermark(false),
#ifdef Q_OS_MACX
      bFrameBufferReady(false),
#endif
#ifdef Q_OS_LINUX
      vsyncState(false),
#endif
      activities(NULL),
      id(0), focusId(0), maxId(0), idDepth(0), maxIdDepth(0), handleId(0),
      selection(), selectionTrees(), selectNextTime(), actionMap(),
      hadSelection(false), selectionChanged(false),
      w_event(NULL), focusWidget(NULL), keyboardModifiers(0),
      prevKeyPressText(""), keyPressed(false), keyEventName(""), keyText(""),
      keyName(""),
      currentMenu(NULL), currentMenuBar(NULL),currentToolBar(NULL),
      menuItems(), menuCount(0),
      colorAction(NULL), fontAction(NULL),
      colorScope(NULL), fontScope(NULL),
      lastMouseX(0), lastMouseY(0), lastMouseButtons(0),
      mouseCoordinatesInfo(NULL),
#ifdef MACOSX_DISPLAYLINK
      displayLink(NULL), displayLinkStarted(false),
      pendingDisplayLinkEvent(false),
      stereoBuffersEnabled(TaoGLFormat().testOption(QGL::StereoBuffers)),
      stereoSkip(0), holdOff(false), droppedFrames(0),
#else
      timer(),
#endif
      dfltRefresh(0.0), idleTimer(this),
      pageStartTime(DBL_MAX), frozenTime(DBL_MAX), startTime(DBL_MAX),
      currentTime(DBL_MAX), stats(), frameCounter(0),
      nextSave(now()), nextSync(nextSave),
#ifndef CFG_NOGIT
      nextCommit(nextSave),
      nextPull(nextSave),
#endif
      pagePrintTime(0.0), printOverscaling(1), printer(NULL),
      currentFileDialog(NULL),
      dragging(false), bAutoHideCursor(false),
      savedCursorShape(Qt::ArrowCursor), mouseCursorHidden(false),
      renderFramesCanceled(0), inOfflineRendering(false), inDraw(false),
      inRunPageExitHandlers(false), pageHasExitHandler(false),
      isInvalid(false)
#ifndef CFG_NO_DOC_SIGNATURE
    , isDocumentSigned(false)
#endif
#ifdef CFG_UNLICENSED_MAX_PAGES
    , pageLimitationDialogShown(false)
#endif
{
    record(widget, "Widget constructor this=%p", this);

    if (Opt::transparent)
    {
        setStyleSheet("background:transparent;");
        QGLWidget::setAttribute(Qt::WA_TranslucentBackground);
    }

#ifdef MACOSX_DISPLAYLINK
    if (DisplayLink == -1)
    {
        DisplayLink = QEvent::registerEventType();
        record(widget,
               "Widget %p allocated %d as event for DisplayLink",
               this, DisplayLink);
        record(layout, "DisplayLink event is %d", DisplayLink);
    }
#endif

    setObjectName(QString("Widget"));
    memset(focusProjection, 0, sizeof focusProjection);
    memset(focusModel, 0, sizeof focusModel);
    memset(focusViewport, 0, sizeof focusViewport);
    memset(mouseTrackingViewport, 0, sizeof mouseTrackingViewport);

    // Make sure we don't fill background with crap
    setAutoFillBackground(false);

    // Make this the current context for OpenGL
    makeCurrent();
    gl.MakeCurrent();

    // Initialize the texture cache (GL must have been initialized first)
    textureCache = TextureCache::instance();
    textureCache->setSaveCompressed(Opt::compressTextures);

    // Sync to VBlank if configured to do so
    enableVSync(NULL, PerformancesPage::VSync());

    // Create the main page we draw on
    space = new SpaceLayout(this);
    layout = space;

    // Prepare the idle timer
    connect(&idleTimer, SIGNAL(timeout()), this, SLOT(dawdle()));
    idleTimer.start(100);

    // Receive notifications for focus
    connect(qApp, SIGNAL(focusChanged (QWidget *, QWidget *)),
            this,  SLOT(appFocusChanged(QWidget *, QWidget *)));
    setFocusPolicy(Qt::StrongFocus);

    // Prepare the menubar
    currentMenuBar = taoWindow()->menuBar();
    connect(taoWindow()->menuBar(), SIGNAL(triggered(QAction*)),
            this,                   SLOT(userMenu(QAction*)));

    toDialogLabel["LookIn"]   = (QFileDialog::DialogLabel)QFileDialog::LookIn;
    toDialogLabel["FileName"] = (QFileDialog::DialogLabel)QFileDialog::FileName;
    toDialogLabel["FileType"] = (QFileDialog::DialogLabel)QFileDialog::FileType;
    toDialogLabel["Accept"]   = (QFileDialog::DialogLabel)QFileDialog::Accept;
    toDialogLabel["Reject"]   = (QFileDialog::DialogLabel)QFileDialog::Reject;

    // Connect the symbol table for formulas
    // REVISIT : Temporarily disabled with new old compiler
    // formulas = new Context(NULL, NULL);
    // TaoFormulas::EnterFormulas(formulas);

    // Prepare activity to process mouse events even when no click is made
    // (but for performance reasons, mouse tracking is enabled only when program
    // execution asks for MouseMove events)
    mouseFocusTracker = new MouseFocusTracker(this);

    // Find which page overscaling to use
    unsigned printResolution = Opt::printResolution;
    while (printOverscaling < 8 &&
           printOverscaling * 72 < printResolution)
        printOverscaling <<= 1;

    // Initialize start time
    resetTimes();

    // Initialize view
    reset();

    // Create the object we will use to render frames
    current = this;
    displayDriver = new DisplayDriver;
    current = NULL; // #1180
    connect(this, SIGNAL(displayModeChanged(QString)),
            taoWindow(), SLOT(updateDisplayModeCheckMark(QString)));

    // Garbage collection is run by the GCThread object, either in the main
    // thread or in its own thread
    connect(this, SIGNAL(runGC()), TaoApp->gcThread, SLOT(collect()));

    // Be notified when source files change
    connect(&srcFileMonitor, SIGNAL(changed(QString,QString)),
            this, SLOT(addToReloadList(QString)));
    connect(&srcFileMonitor, SIGNAL(deleted(QString,QString)),
            this, SLOT(addToReloadList(QString)));

    // Prepare to record proof-of-play pictures
    text popf = Opt::proofOfPlayFile;
    if (popf != "")
    {
        uint popw = Opt::proofOfPlayWidth;
        uint poph = Opt::proofOfPlayHeight;
        saveProofOfPlayThread.setMaxSize(popw, poph);
        saveProofOfPlayThread.setInterval(Opt::proofOfPlayInterval);
        saveProofOfPlayThread.setPath(+popf);
    }
}


struct DisplayListInfo : XL::Info, InfoTrashCan
// ----------------------------------------------------------------------------
//    Store information about a display list
// ----------------------------------------------------------------------------
{
    DisplayListInfo(): displayListID(GL.GenLists(1)), version(0.0) {}
    ~DisplayListInfo() { GL.DeleteLists(displayListID, 1); }
    virtual void Delete() { trash.push_back(this); }
    GLuint      displayListID;
    double      version;
};


struct PurgeGLContextSensitiveInfo : XL::Action
// ----------------------------------------------------------------------------
//   Delete all Info structures that are invalid when the GL context is changed
// ----------------------------------------------------------------------------
{
    virtual Tree *Do (Tree *what)
    {
        what->Purge<AnimatedTextureInfo>();
        what->Purge<SvgRendererInfo>();
        what->Purge<TextureIdInfo>();
        what->Purge<ShaderProgramInfo>();
        what->Purge<DisplayListInfo>();
        return what;
    }
};


struct ExecOnceInfo : XL::Info
// ----------------------------------------------------------------------------
//    Mark node for single execution
// ----------------------------------------------------------------------------
{};


template <typename Action>
void Widget::runPurgeAction(Action &purge)
// ----------------------------------------------------------------------------
//   Recurse "purge info" action on whole program including imports
// ----------------------------------------------------------------------------
{
    if (!xlProgram || !xlProgram->tree)
        return;

    xlProgram->tree->Do(purge);

    // Do it also on imported files
    import_set iset;
    ScanImportedFiles(iset, false);
    {
        for (auto sf : iset)
            sf->tree->Do(purge);
    }
    // Drop the garbage
    InfoTrashCan::Empty();
}


Widget::Widget(Widget &o, const QGLFormat &format)
// ----------------------------------------------------------------------------
//   Create a copy of a widget [with a different QGLFormat]. INVALIDATES o.
// ----------------------------------------------------------------------------
    : QGLWidget(format, o.parentWidget()),
      xlProgram(o.xlProgram), formulas(o.formulas), inError(o.inError),
      mustUpdateDialogs(o.mustUpdateDialogs),
      runOnNextDraw(true), contextFilesLoaded(o.contextFilesLoaded),
      srcFileMonitor(o.srcFileMonitor),
      clearCol(o.clearCol),
      space(NULL), layout(NULL), frameInfo(NULL), path(o.path), table(o.table),
      pageW(o.pageW), pageH(o.pageH),
      blurFactor(o.blurFactor), devicePixelRatio(o.devicePixelRatio),
      currentFlowName(o.currentFlowName), flows(o.flows),
      prevPageName(o.prevPageName),
      pageName(o.pageName), lastPageName(o.lastPageName),
      gotoPageName(o.gotoPageName), transitionPageName(o.transitionPageName),
      pageLinks(o.pageLinks), pageNames(o.pageNames),
      newPageNames(o.newPageNames),
      pageId(o.pageId), pageFound(o.pageFound), prevPageShown(o.prevPageShown),
      pageShown(o.pageFound),
      pageTotal(o.pageTotal), pageToPrint(o.pageToPrint),
      pageEntry(o.pageEntry), pageExit(o.pageExit),
      pageTree(o.pageTree), transitionTree(o.transitionTree),
      transitionStartTime(o.transitionStartTime),
      transitionDurationValue(o.transitionDurationValue),
      currentShape(o.currentShape), currentGridLayout(o.currentGridLayout),
      currentShaderProgram(NULL),
      currentGroup(o.currentGroup),
      glyphCache(),
      fontFileMgr(o.fontFileMgr),
      drawAllPages(o.drawAllPages), animated(o.animated), blanked(o.blanked),
      stereoIdent(o.stereoIdent),
      doMouseTracking(o.doMouseTracking),
      runningTransitionCode(o.runningTransitionCode),
      stereoPlane(o.stereoPlane), stereoPlanes(o.stereoPlanes),
      displayDriver(o.displayDriver),
      watermark(0), watermarkText(""),
      watermarkWidth(0), watermarkHeight(0),
      showingEvaluationWatermark(false),
#ifdef Q_OS_MACX
      bFrameBufferReady(false),
#endif
      screenShotPath(o.screenShotPath), screenShotScale(1.0),
      screenShotWithAlpha(o.screenShotWithAlpha),
#ifdef Q_OS_LINUX
      vsyncState(false),
#endif
      activities(NULL),
      id(o.id), focusId(o.focusId), maxId(o.maxId),
      idDepth(o.idDepth), maxIdDepth(o.maxIdDepth), handleId(o.handleId),
      selection(o.selection), selectionTrees(o.selectionTrees),
      selectNextTime(o.selectNextTime), actionMap(o.actionMap),
      hadSelection(o.hadSelection), selectionChanged(o.selectionChanged),
      w_event(o.w_event), focusWidget(o.focusWidget),
      keyboardModifiers(o.keyboardModifiers),
      prevKeyPressText(o.prevKeyPressText), keyPressed(o.keyPressed),
      keyEventName(o.keyEventName), keyText(o.keyText), keyName(o.keyName),
      currentMenu(o.currentMenu), currentMenuBar(o.currentMenuBar),
      currentToolBar(o.currentToolBar),
      menuItems(o.menuItems), menuCount(o.menuCount),
      colorAction(o.colorAction), fontAction(o.fontAction),
      colorScope(o.colorScope), fontScope(o.fontScope),
      colorName(o.colorName), selectionColor(o.selectionColor),
      originalColor(o.originalColor),
      lastMouseX(o.lastMouseX), lastMouseY(o.lastMouseY),
      lastMouseButtons(o.lastMouseButtons),
      mouseCoordinatesInfo(o.mouseCoordinatesInfo),
#ifdef MACOSX_DISPLAYLINK
      displayLink(NULL), displayLinkStarted(false),
      pendingDisplayLinkEvent(false),
      stereoBuffersEnabled(format.testOption(QGL::StereoBuffers)),
      stereoSkip(o.stereoSkip), holdOff(o.holdOff),
      droppedFrames(o.droppedFrames),
#else
      timer(),
#endif
      dfltRefresh(o.dfltRefresh),
      pageStartTime(o.pageStartTime), frozenTime(o.frozenTime),
      startTime(o.startTime),
      currentTime(o.currentTime), stats(o.stats.isEnabled()),
      frameCounter(o.frameCounter),
      nextSave(o.nextSave), nextSync(o.nextSync),
#ifndef CFG_NOGIT
      nextCommit(o.nextCommit),
      nextPull(o.nextPull),
#endif
      pagePrintTime(o.pagePrintTime), printOverscaling(o.printOverscaling),
      printer(o.printer),
      currentFileDialog(o.currentFileDialog),
      zNear(o.zNear), zFar(o.zFar), scaling(o.scaling),
      zoom(o.zoom), eyeDistance(o.eyeDistance),
      cameraToScreen(o.cameraToScreen),
      cameraPosition(o.cameraPosition),
      cameraTarget(o.cameraTarget), cameraUpVector(o.cameraUpVector),
      panX(o.panX), panY(o.panY),
      dragging(o.dragging), bAutoHideCursor(o.bAutoHideCursor),
      savedCursorShape(o.savedCursorShape),
      mouseCursorHidden(o.mouseCursorHidden),
      renderFramesCanceled(o.renderFramesCanceled),
      inOfflineRendering(o.inOfflineRendering),
      offlineRenderingWidth(o.offlineRenderingWidth),
      offlineRenderingHeight(o.offlineRenderingHeight),
      toDialogLabel(o.toDialogLabel),
      pendingEvents(), // Nothing transferred from o's event queue
      inDraw(o.inDraw), inRunPageExitHandlers(o.inRunPageExitHandlers),
      pageHasExitHandler(o.pageHasExitHandler), changeReason(o.changeReason),
      isInvalid(false)
#ifndef CFG_NO_DOC_SIGNATURE
    , isDocumentSigned(o.isDocumentSigned)
#endif
#ifdef CFG_UNLICENSED_MAX_PAGES
    , pageLimitationDialogShown(o.pageLimitationDialogShown)
#endif
{
    record(widget, "Widget copy constructor copy %p from %p", this, &o);
    setObjectName(QString("Widget"));

    memcpy(focusProjection, o.focusProjection, sizeof(focusProjection));
    memcpy(focusModel, o.focusModel, sizeof(focusModel));
    memcpy(focusViewport, o.focusViewport, sizeof(focusViewport));
    memcpy(mouseTrackingViewport, o.mouseTrackingViewport,
           sizeof(mouseTrackingViewport));

    // Make sure we don't fill background with crap
    setAutoFillBackground(false);

    // Make this the current context for OpenGL
    makeCurrent();
    gl.MakeCurrent();

    // Do not change VSync mode
    enableVSync(NULL, o.VSyncEnabled());

    // Create new layout to draw into
    space = new SpaceLayout(this);
    layout = space;

    // Prepare the idle timer
    connect(&idleTimer, SIGNAL(timeout()), this, SLOT(dawdle()));
    idleTimer.start(100);

    // Receive notifications for focus
    connect(qApp, SIGNAL(focusChanged (QWidget *, QWidget *)),
            this,  SLOT(appFocusChanged(QWidget *, QWidget *)));
    setFocusPolicy(Qt::StrongFocus);

    // Prepare the menubar
    connect(currentMenuBar, SIGNAL(triggered(QAction*)),
            this,           SLOT(userMenu(QAction*)));

    // Prepare new mouse tracking activity
    mouseFocusTracker = new MouseFocusTracker(this);

    std::map<text, text>::iterator i;
    for (i = o.xlTranslations.begin(); i != o.xlTranslations.end(); i++)
        xlTranslations[(*i).first] = (*i).second;

    // Hide mouse cursor if it was hidden in previous widget
    if (o.cursor().shape() == Qt::BlankCursor)
        hideCursor();
    if (o.mouseCursorHidden)
        QWidget::setCursor(Qt::BlankCursor);

    // Invalidate any program info that depends on the old GL context
    PurgeGLContextSensitiveInfo purge;
    runPurgeAction(purge);

    // REVISIT:
    // Texture and glyph cache do not support multiple GL contexts. So,
    // clear them here so that textures or GL lists created with the
    // previous context are not re-used with the new.
    TextureCache::instance()->clear();
    if (o.watermark)
        GL.DeleteTextures(1, &o.watermark);

    o.glyphCache.Clear();
    o.updateStereoIdentPatterns(0);

    // Now, o has become invalid ; make sure it can't be redrawn before being
    // deleted (asynchronously, by deleteLater()).
    record(justify, "Widget %p clears layout %p", this, o.space);
    o.space->Clear();
    o.isInvalid = true;

    current = this;

    Window *win = taoWindow();
    connect(this, SIGNAL(displayModeChanged(QString)),
            win, SLOT(updateDisplayModeCheckMark(QString)));

    // Garbage collection is run by the GCThread object, either in the main
    // thread or in its own thread
    connect(this, SIGNAL(runGC()), TaoApp->gcThread, SLOT(collect()));

    // Be notified when source files change
    connect(&srcFileMonitor, SIGNAL(changed(QString,QString)),
            this, SLOT(addToReloadList(QString)));
    connect(&srcFileMonitor, SIGNAL(deleted(QString,QString)),
            this, SLOT(addToReloadList(QString)));

    runProgram();
}


Widget::~Widget()
// ----------------------------------------------------------------------------
//   Destroy the widget
// ----------------------------------------------------------------------------
{
    record(widget, "Widget destructor %p", this);
    xlProgram = NULL;           // Mark widget as invalid
    current = NULL;
    delete space;
    delete path;
    delete mouseFocusTracker;
    // REVISIT: delete activities?

#ifdef MACOSX_DISPLAYLINK
    if (displayLink)
    {
        if (displayLinkStarted)
            CVDisplayLinkStop(displayLink);
        CVDisplayLinkRelease(displayLink);
        displayLink = NULL;
    }
#endif

    RasterText::purge(QGLWidget::context());
    updateStereoIdentPatterns(0);
    // NB: if you're about to call glDeleteTextures here, think twice.
    // Or make sure you set the correct GL context. See #1686.

    TextureCache::instance()->clear();
}


template <class T> struct PurgeInfo : XL::Action
// ----------------------------------------------------------------------------
//   Delete all XL::Info structures of type T in a tree
// ----------------------------------------------------------------------------
{
    typedef Tree *value_type;
    Tree *Do (Tree *what)
    {
        what->Purge<T>();
        return what;
    }
};


void Widget::purgeTaoInfo()
// ----------------------------------------------------------------------------
//   Delete all TaoInfo associated with the current program
// ----------------------------------------------------------------------------
{
    PurgeInfo<Tao::Info> purge;
    runPurgeAction(purge);
}



// ============================================================================
//
//   Slots
//
// ============================================================================

void Widget::dawdle()
// ----------------------------------------------------------------------------
//   Operations to do when idle (in the background)
// ----------------------------------------------------------------------------
{
    if (!xlProgram)
        return;

    // Run all activities, which will get them a chance to update refresh
    for (Activity *a = activities; a; a = a->Idle()) ;

    // We will only auto-save and commit if we have a valid repository
    Repository *repo = repository();

    if (xlProgram->changed && xlProgram->readOnly)
    {
        if (!dragging)
            updateProgramSource();
        if (!repo)
            xlProgram->changed = false;
    }

    // Check if it's time to save
    ulonglong tick = now();
    longlong saveDelay = longlong(nextSave - tick);
    if (repo && saveDelay < 0 && repo->idle() && !dragging)
    {
        doSave(tick);
    }

#ifndef CFG_NOGIT
    if (GeneralPage::gitEnabled())
    {
        // Check if it's time to commit
        longlong commitDelay = longlong (nextCommit - tick);
        if (repo && commitDelay < 0 &&
            repo->state == Repository::RS_NotClean &&
            !dragging)
        {
            doCommit(tick);
        }

        // Check if it's time to merge from the remote repository
        // REVISIT: sync: what if several widgets share the same repository?
        longlong pullDelay = longlong (nextPull - tick);
        if (repo && pullDelay < 0 && repo->state == Repository::RS_Clean)
        {
            doPull(tick);
        }
    }
#endif

#ifndef CFG_NORELOAD
    // Check if it's time to reload
    refreshProgram();
#endif

    // Check if statistics status changed from the dialog box
    if (stats.isEnabled(Statistics::TO_CONSOLE) != XLTRACE(layout_fps))
        stats.enable(XLTRACE(layout_fps), Statistics::TO_CONSOLE);

    IFTRACE(layout_fps)
        printPerLayoutStatistics();
}


void Widget::setupPage()
// ----------------------------------------------------------------------------
//   Setup attributes before drawing or printing a page
// ----------------------------------------------------------------------------
{
    w_event = NULL;
    pageW = (21.0 / 2.54) * logicalDpiX(); // REVISIT
    pageH = (29.7 / 2.54) * logicalDpiY();
    pageId = 0;
    pageFound = 0;
    pageTree = NULL;
    lastPageName = "";
    newPageNames.clear();
}


void Widget::drawStereoIdent()
// ----------------------------------------------------------------------------
//   Draw a different pattern on each stereo plane to identify them
// ----------------------------------------------------------------------------
{
    if (stereoIdentPatterns.size() < (size_t)stereoPlanes)
        updateStereoIdentPatterns(stereoPlanes);
    StereoIdentTexture pattern = stereoIdentPatterns[stereoPlane];
    GLAllStateKeeper save;
    GL.Color(1.0, 1.0, 1.0, 1.0);
    drawFullScreenTexture(pattern.w, pattern.h, pattern.tex, true);
}


void Widget::drawScene()
// ----------------------------------------------------------------------------
//   Draw all objects in the scene
// ----------------------------------------------------------------------------
{
    if (Opt::gcThread)
    {
        // Start asynchronous garbage collection
        TaoApp->gcThread->clearCollectDone();
        emit runGC();
    }

    id = idDepth = 0;
    space->ClearPolygonOffset();
    space->ClearAttributes();
    if (blanked)
    {
        GL.ClearColor(0.0, 0.0, 0.0, 1.0);
        GL.Clear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    }
    else if (stereoIdent)
    {
        drawStereoIdent();
    }
    else
    {
        record(polycount, "Begin draw, polycount was %lu", GL.polycount);
        GL.polycount = 0;
        space->Draw(NULL);
        record(polycount, "End draw, polycount is %lu", GL.polycount);
    }

    if (showingEvaluationWatermark)
        drawWatermark();

    if (Opt::gcThread)
    {
        // Record wait time till asynchronous garbage collection completes
        stats.end(Statistics::DRAW);
        stats.begin(Statistics::GC_WAIT);
        TaoApp->gcThread->waitCollectDone();
        stats.end(Statistics::GC_WAIT);
        stats.begin(Statistics::DRAW);
    }

    // Delete objects that have GL resources, pushed by GC
    InfoTrashCan::Empty();
}


void Widget::drawSelection()
// ----------------------------------------------------------------------------
//   Draw selection items for all objects (selection boxes, manipulators)
// ----------------------------------------------------------------------------
{
    id = idDepth = 0;
    selectionTrees.clear();
    if (selection.size())
    {
        space->ClearPolygonOffset();
        space->ClearAttributes();
        GL.Disable(GL_DEPTH_TEST);
        space->DrawSelection(NULL);
        GL.Enable(GL_DEPTH_TEST);
    }
}


void Widget::drawActivities()
// ----------------------------------------------------------------------------
//   Draw chooser, selection rectangle, display statistics
// ----------------------------------------------------------------------------
{
    SpaceLayout selectionSpace(this);
    XL::Save<Layout *> saveLayout(layout, &selectionSpace);

    // Force "standard" view for drawing the activities
    setupGL();
    GL.DepthFunc(GL_ALWAYS);

    // Clear the topmost layout to avoid conflicts
    // with following taodyne ad. Fixed #2966.
    layout->Clear();

    for (Activity *a = activities; a; a = a->Display()) ;

    // Once we have recorded all the shapes in the selection space,
    // perform actual rendering
    selectionSpace.Draw(NULL);

    // Clear the topmost layout to avoid conflicts
    // with following taodyne ad. Fixed #2966.
    layout->Clear();

#ifndef CFG_NO_LICENSE
    // Check if something is unlicensed somewhere, if so, show Taodyne ad
    if (contextFilesLoaded &&
        (Licenses::UnlicensedCount() > 0
         || TaoApp->edition == Application::Player
         || TaoApp->edition == Application::Studio
#ifndef CFG_NO_DOC_SIGNATURE
         || (TaoApp->edition == Application::PlayerPro
             && !isDocumentSigned)
#endif // CFG_NO_DOC_SIGNATURE
            ))
    {
        GLStateKeeper save;
        SpaceLayout licenseOverlaySpace(this);
        XL::Save<Layout *> saveLayout2(layout, &licenseOverlaySpace);

        setupGL();
        GL.DepthFunc(GL_ALWAYS);
        GL.MatrixMode(GL_PROJECTION);
        GL.LoadIdentity();
        GLdouble w = width(), h = height();
        GL.Ortho(-w/2, w/2, -h/2, h/2, -1, 1);
        GL.MatrixMode(GL_MODELVIEW);
        GL.LoadIdentity();

        static Tree *adCode = XL::xl_parse_text(
#include "taodyne_ad.h"
            );
        if (xlProgram)
            saveAndEvaluate(xlProgram->scope, adCode);
        licenseOverlaySpace.Draw(NULL);
    }
#endif // CFG_NO_LICENSE

    GL.DepthFunc(GL_LEQUAL);

    // Show FPS as text overlay
    if (stats.isEnabled(Statistics::TO_SCREEN))
        printStatistics();

    if (stats.isEnabled(Statistics::TO_CONSOLE))
        logStatistics();
}


void Widget::setGlClearColor()
// ----------------------------------------------------------------------------
//   Clear color with the color specified in the widget
// ----------------------------------------------------------------------------
{
    qreal r, g, b, a;
    clearCol.getRgbF(&r, &g, &b, &a);
    GL.ClearColor (r, g, b, a);
}


void Widget::getCamera(Point3 *position, Point3 *target, Vector3 *upVector,
                       double *toScreen)
// ----------------------------------------------------------------------------
//   Get camera characteristics
// ----------------------------------------------------------------------------
{
    if (position) *position = cameraPosition;
    if (target)   *target   = cameraTarget;
    if (upVector) *upVector = cameraUpVector;
    if (toScreen) *toScreen = cameraToScreen;
}


void Widget::draw()
// ----------------------------------------------------------------------------
//    Redraw the widget
// ----------------------------------------------------------------------------
{
    record(widget, "Draw %p%+s%+s%+s",
           this,
           inOfflineRendering ? ", offline" : "",
           inDraw ? ", already drawing" : "",
           runOnNextDraw ? ", must evaluate program" : "");

    // The viewport used for mouse projection is (potentially) set by the
    // display function, clear it for current frame
    memset(mouseTrackingViewport, 0, sizeof(mouseTrackingViewport));

    // In offline rendering mode, just keep the widget clear
    if (inOfflineRendering)
    {
        GL.Clear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
        return;
    }

    // Recursive drawing may occur with video widgets, and it's bad
    if (inDraw)
        return;

    XL::Save<bool> drawing(inDraw, true);
    TaoSave saveCurrent(current, this);

    // Clean text selection
    TextSelect *sel = textSelection();
    if (sel)
    {
        sel->cursor.select(QTextCursor::Document);
        sel->cursor.removeSelectedText();
    }

    // Make sure program has been run at least once
    if (runOnNextDraw)
    {
        runOnNextDraw = false;
        runProgram();
    }

    if (!Opt::gcThread)
        emit runGC();

    // Run current display algorithm
    stats.begin(Statistics::FRAME);
    stats.begin(Statistics::DRAW);
#if QT_VERSION >= 0x050000
    devicePixelRatio = windowHandle()->devicePixelRatio();
#endif
    displayDriver->display();
    stats.end(Statistics::DRAW);
    stats.end(Statistics::FRAME);
    frameCounter++;

    // Remember number of elements drawn for GL selection buffer capacity
    if (maxId < id + 100 || maxId > 2 * (id + 100))
        maxId = id + 100;

    // Clipboard management
    checkCopyAvailable();

    // If we must update dialogs, do it now
    if (mustUpdateDialogs)
    {
        mustUpdateDialogs = false;
        if (colorDialog)
            updateColorDialog();
        if (fontDialog)
            updateFontDialog();
    }

    if (selectionChanged)
    {
        selectionChanged = false;

        // TODO: honoring isReadOnly involves more than just this
        if (!isReadOnly())
            updateProgramSource();
    }

}


bool Widget::refreshNow()
// ----------------------------------------------------------------------------
//    Redraw the widget by running program entirely
// ----------------------------------------------------------------------------
{
    return refreshNow(NULL);
}


void Widget::resetTimes()
// ----------------------------------------------------------------------------
//   Reset times to current time for all times
// ----------------------------------------------------------------------------
{
    frozenTime = CurrentTime();

    pageStartTime = frozenTime;
    startTime = frozenTime;
}


struct PurgeAnonymousFrameInfo : XL::Action
// ----------------------------------------------------------------------------
//   Delete anonymous FrameInfo structures (contained in MultiFrameInfo<uint>)
// ----------------------------------------------------------------------------
{
    Tree *Do (Tree *what)
    {
        // Do not delete named FrameInfos because:
        // - The purpose of the name is to make sure the texture id remains
        //   the same.
        // - Named FrameInfos are used to implement canvas which persist
        //   accross pages.
        what->Purge<MultiFrameInfo<uint> >();
        return what;
    }
};


void Widget::commitPageChange(bool afterTransition)
// ----------------------------------------------------------------------------
//   Commit a page change, e.g. as a result of 'goto_page' or transition end
// ----------------------------------------------------------------------------
{
    TaoSave saveCurrent(current, this);

    if (runPageExitHandlers())
        return;

    PurgeAnonymousFrameInfo purgemf;
    runPurgeAction(purgemf);

    prevPageName = pageName;
    pageName = gotoPageName;
    resetTimes();
    prevPageShown = pageShown;
    for (uint p = 0; p < pageNames.size(); p++)
        if (pageNames[p] == gotoPageName)
            pageShown = p + 1;
    gotoPageName = "";
    pageFound = 0;

    lastMouseButtons = 0;
    selection.clear();
    selectionTrees.clear();
    delete textSelection();
    delete drag();

    if (afterTransition)
    {
        pageStartTime = transitionStartTime;
        transitionStartTime = 0;
        transitionDurationValue = 0;
        transitionTree = NULL;
    }

    if (!inOfflineRendering && pageChangeActions.size())
    {
        for (auto &pc : pageChangeActions)
            xl_evaluate(pc.second.scope, pc.second.code);
    }

    record(pages, "New page number is %u", pageShown);
}


bool Widget::runPageExitHandlers()
// ----------------------------------------------------------------------------
//    If current page has exit handlers, run them
// ----------------------------------------------------------------------------
{
    // If current page has exit handlers, perform an additional evaluation of
    // the program to run them now
    if (pageHasExitHandler)
    {
        if (inRunPageExitHandlers)
            return true;
        record(pages, "Running exit handler(s) for page %u", pageShown);
        do
        {
            // Prevent recursion
            XL::Save<bool> saveInRPEH(inRunPageExitHandlers, true);
            // Mark pageExit so page exit handler(s) of current page will run
            XL::Save<uint> savePageExit(pageExit, pageShown);
            runProgramOnce();
        } while (0);
    }
    else
    {
        record(pages, "Page %u has no exit handler", pageShown);
    }
    return false;
}


bool Widget::refreshNow(QEvent *event)
// ----------------------------------------------------------------------------
//    Redraw the widget due to event or run program entirely
// ----------------------------------------------------------------------------
{
    record(widget, "Refresh %p now %+s%+s%+s",
           this,
           inDraw   ? " in draw"  : "",
           inError  ? " in error" : "",
           printer  ? " printing" : "");
    if (inDraw || inError || printer)
        return false;

    // Update times
    setCurrentTime();
    bool changed = false;
    double now = CurrentTime();

    // Check if we are replaying a transition, if so finish it
    if (transitionStartTime > 0)
    {
        if (frozenTime - transitionStartTime > transitionDurationValue)
        {
            commitPageChange(true);
            event = NULL;
        }
    }

    // Check if we need to change pages.
    else if (gotoPageName != "")
    {
        record(pages, "Goto page %s request from %s", gotoPageName, pageName);
        if (transitionTree == NULL)
        {
            commitPageChange(false);
        }
        else
        {
            // Activate transition start time when changing page
            transitionStartTime = now;
            transitionPageName = pageName;
        }
        event = NULL; // Force full refresh
        pageEntry = 0;
    }

#if defined(Q_OS_WIN)
    // #3017: after enabling or disabling quad buffer stereo on one of our
    // machines (AMD GPU and drivers), QGLContext::currentContext() returns
    // NULL after ~Widget has returned
    makeCurrent();
#endif

    // Redraw full scene if required
    if (!event || space->NeedRefresh(event, now))
    {
        record(layout, "Full refresh of widget %p", this);

        XL::Save<bool> drawing(inDraw, true);
        // runProgram needs current != NULL which is not always true here
        // for instance: from Window::loadFile()
        TaoSave saveCurrent(current, this);

        runProgram();

        changed = true;
    }
    else
    {
        int evt = event->type();
        record(layout, "Partial refresh of %p dur to event %s (%d)",
               this, LayoutState::ToText(evt), evt);

        // Layout::Refresh needs current != NULL
        TaoSave saveCurrent(current, this);
        stats.begin(Statistics::EXEC);
        changed = space->Refresh(event, now);
        if (changed)
            checkErrors();
        stats.end(Statistics::EXEC);
    }

    if (changed)
    {
        // Process explicit layout dependencies
        space->CheckRefreshDeps();
    }

    if (!inOfflineRendering)
    {
        // Redraw all
        TaoSave saveCurrent(current, NULL); // draw() assumes current == NULL
        updateGL();

        if (changed)
            processProgramEvents();
    }

    return changed;
}


static QDateTime fromSecsSinceEpoch(double when)
// ----------------------------------------------------------------------------
//    Convert the number of seconds passed since the epoch to QDateTime
// ----------------------------------------------------------------------------
{
    QDateTime d;
    d = d.fromMSecsSinceEpoch((qint64)(when * 1000));
    return d;
}


void Widget::refreshOn(int type, double nextRefresh)
// ----------------------------------------------------------------------------
//   Current layout (if any) should be updated on specified event
// ----------------------------------------------------------------------------
{
    if (!layout)
        return;

    if (type == QEvent::Timer)
    {
        double currentTime = CurrentTime();
        if (nextRefresh == DBL_MAX)
            nextRefresh = currentTime + dfltRefresh;
        record(layout,
               "Refresh %p at %f (in %fs)",
               this, nextRefresh, nextRefresh - currentTime);
    }

    layout->RefreshOn(type, nextRefresh);
}


bool Widget::refreshOnAPI(int event_type, double next_refresh)
// ----------------------------------------------------------------------------
//   Module interface to refreshOn
// ----------------------------------------------------------------------------
{
    if (next_refresh == -1.0)
        next_refresh = DBL_MAX;
    findTaoWidget()->refreshOn(event_type, next_refresh);
    return true;
}


double Widget::currentTimeAPI()
// ----------------------------------------------------------------------------
//   Module interface to currentTime()
// ----------------------------------------------------------------------------
{
    return findTaoWidget()->CurrentTime();
}


double Widget::currentPageTimeAPI()
// ----------------------------------------------------------------------------
//   Module interface to currentTime()
// ----------------------------------------------------------------------------
{
    Widget *widget = findTaoWidget();
    return widget->CurrentTime() - widget->pageStartTime;
}


bool Widget::offlineRenderingAPI()
// ----------------------------------------------------------------------------
//   Module interface to inOfflineRendering
// ----------------------------------------------------------------------------
{
    return findTaoWidget()->inOfflineRendering;
}


double Widget::DevicePixelRatioAPI()
// ----------------------------------------------------------------------------
//   Module interface to device pixel ratio
// ----------------------------------------------------------------------------
{
    Widget *widget = findTaoWidget();
    return widget->devicePixelRatio;
}


bool Widget::RenderingTransparencyAPI()
// ----------------------------------------------------------------------------
//   Module interface to checking if we are rendering with transparency
// ----------------------------------------------------------------------------
{
    Widget *widget = findTaoWidget();
    return widget->layout->transparency;
}


void Widget::makeGLContextCurrent()
// ----------------------------------------------------------------------------
//   Make GL context of the current Tao widget the current GL context
// ----------------------------------------------------------------------------
{
    Tao()->makeCurrent();
}


static QString errorHint(QString err)
// ----------------------------------------------------------------------------
//   Return a hint on how to fix error
// ----------------------------------------------------------------------------
{
    if (err.contains("No rewrite"))
    {
        ModuleManager * mmgr = ModuleManager::moduleManager();
        if (err.contains("movie") && mmgr->enabled("AudioVideo"))
            return "Hint: Try adding 'import AudioVideo'";
    }
    return "";
}


void Widget::runProgramOnce()
// ----------------------------------------------------------------------------
//   Run the  XL program once only
// ----------------------------------------------------------------------------
//   There are rare cases where we may need to evaluate the program twice
//   (and only twice to avoid infinite loops). For example, if the page
//   title is translated, it may not match on the next draw. See #2060.
{
    setupPage();
    setCurrentTime();

    // Don't run anything if we detected errors running previously
    if (inError)
        return;

    // Clean actionMap
    actionMap.clear();
    pageChangeActions.clear();
    dfltRefresh = optimalDefaultRefresh();

    // Clean text flow
    flows.clear();

    // Reset the selection id for the various elements being drawn
    id = idDepth = 0;
    selectionRectangleEnabled = true;

    clearCol.setRgb(255, 255, 255, 255);

    stats.begin(Statistics::EXEC);

    // Run the XL program associated with this widget
    XL::Save<Layout *> saveLayout(layout, space);
    record(justify, "Widget %p clear space %p to run program %t",
           this, space, (Tree *) xlProgram);
    space->Clear();

    // Evaluate the program
    if (xlProgram)
    {
        if (transitionTree && transitionStartTime > 0)
        {
            runTransition(xlProgram->scope);
        }
        else if (Tree *prog = xlProgram->tree)
        {
            saveAndEvaluate(xlProgram->scope, prog);
        }
    }
    stats.end(Statistics::EXEC);

    // If we have evaluation errors, show them (bug #498)
    checkErrors();

    // Clean the end of the old menu list, unless in a transition.
    if (!runningTransitionCode)
    {
        uint total = menuItems.size();
        for  (uint m = menuCount; m < total; m++)
        {
            delete menuItems[m];
            menuItems[m] = NULL;
        }

        // Reset the menuCount value.
        menuCount          = 0;
        currentMenu    = NULL;
        currentToolBar = NULL;
        currentMenuBar = taoWindow()->menuBar();
    }

    // Check pending events
    processProgramEvents();

    if (!dragging)
        finishChanges();
}



void Widget::runProgram()
// ----------------------------------------------------------------------------
//   Run the  XL program until we have found a page
// ----------------------------------------------------------------------------
//   There are rare cases where we may need to evaluate the program twice
//   (and only twice to avoid infinite loops). For example, if the page
//   title is translated, it may not match on the next draw. See #2060.
{
    record(widget, "Run program for widget %p", this);

#if defined(Q_OS_WIN)
    // #3017
    makeCurrent();
#endif

    runProgramOnce();

    record(pages, "Page found %u id %u shown %u", pageFound, pageId, pageShown);
    pageNames = newPageNames;
    pageTotal = pageId;
    if (pageFound)
    {
        pageShown = pageFound;
    }
    else
    {
        // If we had pages, but none matches, re-evaluate the program (#2060)
        pageName = pageShown > 0 && pageShown <= pageNames.size()
            ? pageNames[pageShown-1]
            : "";
        if (pageId > 0)
            runProgramOnce();
    }
}


Tree *Widget::saveAndEvaluate(Scope* scope, Tree *code)
// ----------------------------------------------------------------------------
//   Save GL states before evaluate a XL context
// ----------------------------------------------------------------------------
{
    IFTRACE(layout_fps)
        PerLayoutStatistics::beginExec(code);

    // Save GL state before evaluate
    GLAllStateKeeper save;

    // Evaluate context
    Tree *result = xl_evaluate(scope, code);

    IFTRACE(layout_fps)
        PerLayoutStatistics::endExec(code);

    return result;
}


void Widget::processProgramEvents()
// ----------------------------------------------------------------------------
//   Process registered program events
// ----------------------------------------------------------------------------
{
    LayoutState::qevent_ids refreshEvents = space->RefreshEvents();
    record(layout, "Program events: %s", Layout::ToText(refreshEvents));
    // Trigger mouse tracking only if needed
    bool mouseTracking = (refreshEvents.count(QEvent::MouseMove) != 0);
    if (mouseTracking)
        setMouseTracking(true);
    // Make sure refresh timer is restarted if needed
    startRefreshTimer();
}


void Widget::print(QPrinter *prt)
// ----------------------------------------------------------------------------
//   Print the pages on the given page range
// ----------------------------------------------------------------------------
{
    // Set the current printer while drawing
    TaoSave saveCurrent(current, this);
    XL::Save<QPrinter *> savePrinter(printer, prt);
    XL::Save<text> savePage(pageName, "");

    // Identify the page range
    uint firstPage = printer->fromPage();
    uint lastPage = printer->toPage();
    if (firstPage == 0)
        firstPage = 1;
    if (lastPage == 0)
        lastPage = pageTotal ? pageTotal : 1;

    // Get the printable area in the page and create a GL frame for it
    printer->setFullPage(true);
    QRect pageRect = printer->pageRect();
    QPainter painter(printer);
    uint w = pageRect.width(), h = pageRect.height();
    FrameInfo frame(w, h);
    record(print,
           "Page rectangle top %u left %u bottom %u right %u",
           pageRect.top(), pageRect.left(),
           pageRect.bottom(), pageRect.right());

    // Get the status bar
    QStatusBar *status = taoWindow()->statusBar();

    // Set the initial time we want to set and freeze animations
    XL::Save<bool> disableAnimations(animated, false);
    XL::Save<double> setPageTime(pageStartTime, 0);
    XL::Save<double> setFrozenTime(frozenTime, 0);
    XL::Save<double> saveStartTime(startTime, 0);
    XL::Save<double> savePixelRatio(devicePixelRatio, 1);

    // Save page information
    XL::Save<text> saveLastPage(lastPageName, pageName);
    XL::Save<text> savePageName(pageName, transitionPageName);
    XL::Save<page_map> saveLinks(pageLinks, pageLinks);
    XL::Save<page_list> saveList(pageNames, pageNames);
    XL::Save<uint> savePageFound(pageFound, 0);
    XL::Save<Tree_p> savePageTree(pageTree, pageTree);

    // Render the given page range
    for (pageToPrint = firstPage; pageToPrint <= lastPage; pageToPrint++)
    {
        int n = printOverscaling;
        QImage bigPicture(w * n, h * n, QImage::Format_RGB888);
        QPainter bigPainter(&bigPicture);
        bigPicture.fill(-1);
        record(print, "Printing page %u %u x %u overscaling %u",
               pageToPrint, w, h, printOverscaling);

        // Show crude progress information
        status->showMessage(tr("Printing page %1/%2...")
                            .arg(pageToPrint - firstPage + 1)
                            .arg(lastPage - firstPage + 1));
        QApplication::processEvents();

        // Center display on screen
        XL::Save<double> savePrintTime(pagePrintTime, 0);
        XL::Save<Point3> saveCenter(cameraTarget, Point3(0,0,0));
        XL::Save<Point3> saveEye(cameraPosition, defaultCameraPosition);
        XL::Save<Vector3> saveUp(cameraUpVector, Vector3(0,1,0));
        XL::Save<double> saveZoom(zoom, 1);
        XL::Save<double> saveScaling(scaling, scalingFactorFromCamera());

        // Evaluate twice time so that we correctly setup page info
        for (uint i = 0; i < 2; i++)
        {
            frozenTime = pagePrintTime;
            runProgram();
        }

        // We draw small fragments for overscaling
        int tile = 0, tiles = (2*n-1)*(2*n-1);
        for (int r = -n+1; r < n; r++)
        {
            for (int c = -n+1; c < n; c++)
            {
                double s = 1.0 / n;

                // Crude progress information
                // WARNING: do not run QApplication::processEvents here,
                // as it could mess upour carefully setup drawing environment
                tile++;
                uint pct = 100 * tile/tiles;
                status->showMessage(tr("Printing page %1/%2 (%3%)")
                                    .arg(pageToPrint - firstPage + 1)
                                    .arg(lastPage - firstPage + 1)
                                    .arg(pct));
                status->repaint(); // Does not work on MacOSX?

                record(print, "Page %u row %u column %u", pageToPrint, r, c);

                // Draw the layout in the frame context
                id = idDepth = 0;
                frame.begin();
                Box box(c * w * s, (n - 1 - r) * h * s, w, h);
                setup(w, h, &box);

                setGlClearColor();
                GL.Clear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

                space->Draw(NULL);
                frame.end();

                // Draw fragment
                QImage image(frame.toImage());
                // image = image.convertToFormat(QImage::Format_RGB888);
                QRect rect(c*w, r*h, w, h);
                bigPainter.drawImage(rect, image);
            }
        }

        // Draw the resulting big picture into the printer
        QRect printRect(0, 0, pageRect.width(), pageRect.height());
        painter.drawImage(printRect, bigPicture);

        if (pageToPrint < lastPage)
            printer->newPage();
    }

    // Finish the job
    painter.end();
    status->showMessage("");
    setup(width(), height());
}


struct SaveImage : QRunnable
// ----------------------------------------------------------------------------
//   A separate thread used to store rendered images to disk
// ----------------------------------------------------------------------------
{
    SaveImage(QString filename, QImage image, QSemaphore * semaphore):
        filename(filename), image(image), semaphore(semaphore)
    { semaphore->acquire(); }
    void run()    { image.save(filename); semaphore->release(); }
public:
    QString      filename;
    QImage       image;
    QSemaphore * semaphore;
};


static bool checkPrintfFormat(QString str)
// ----------------------------------------------------------------------------
//   true if str has 0 or 1 integer printf format, no other format or invalid %
// ----------------------------------------------------------------------------
{
    // I found a regular expressions on the net, and adapted it. Seems to work
    // well enough.

#define BEFORE_LETTER "%(?!%)(?:\\d+\\$)?[+-]?(?:[ 0]|'.{1})?-?\\d*(?:\\.\\d+)?"
    // Valid but unacceptable printf formats (like %s)
    QRegExp reject(BEFORE_LETTER "[bceEfFgGs]");
    int intfmt = 0;
    bool ok = reject.indexIn(str) == -1;
    if (ok)
    {
        // Want only one integer format
        QRegExp all(BEFORE_LETTER "[duoxX]");
        int pos = 0;
        while ((pos = all.indexIn((str), pos)) != -1)
        {
            if (intfmt == 1)
            {
                ok = false;
                break;
            }
            intfmt++;
            pos += all.matchedLength();
        }
    }
    if (ok)
    {
        // Want no single % other than the integer format string we may have
        // found above
        QString str2(str);
        str2.replace("%%", "");
        ok = str2.count(QChar('%')) == intfmt;
    }
#undef BEFORE_LETTER
    return ok;
}


void Widget::renderFrames(int w, int h, double start_time, double duration,
                          QString dir, double fps, int page,
                          double time_offset, QString disp, QString fileName,
                          int firstFrame)
// ----------------------------------------------------------------------------
//    Render frames to PNG files
// ----------------------------------------------------------------------------
{
    // Create output directory if needed
    if (!QFileInfo(dir).exists())
        QDir().mkdir(dir);
    if (!QFileInfo(dir).isDir() && dir != "/dev/null")
        return;

    if (!checkPrintfFormat(fileName))
    {
        QMessageBox::warning(NULL, tr("Error"),
                             tr("Invalid file name. Check any % format "
                                "specification and remember to use %% to "
                                "insert a percent character."));
        emit renderFramesDone();
        return;
    }

    TaoSave saveCurrent(current, this);

    // Select display. Requires current != NULL.
    QString prevDisplay;
    if (disp != "" && !displayDriver->isCurrentDisplayFunctionSameAs(disp))
    {
        // Select specific display function
        prevDisplay = displayDriver->getDisplayFunction();
        displayDriver->setDisplayFunction(disp);
    }
    if (displayDriver->isCurrentDisplayFunctionSameAs("legacy"))
    {
        // Compatibility
        std::cerr << "Legacy display function does not support offline "
                  << "rendering. Switching to \"2D\" for compatibility\n";
        if (prevDisplay == "")
            prevDisplay = displayDriver->getDisplayFunction();
        displayDriver->setDisplayFunction("2D");
    }

    XL::Save<uint> savePageEntry(pageEntry, pageEntry);

    // Set the initial time we want to set and freeze animations
    if (start_time == -1)
        start_time = trueCurrentTime();
    XL::Save<double> setPageTime(pageStartTime, start_time);
    XL::Save<double> setFrozenTime(frozenTime, start_time + time_offset);
    XL::Save<double> saveStartTime(startTime, start_time);
    XL::Save<page_list> savePageNames(pageNames, pageNames);
    XL::Save<double> savePixelRatio(devicePixelRatio, 1.0);

    GLAllStateKeeper saveGL;
    XL::Save<double> saveScaling(scaling, scaling);

    inOfflineRendering = true;
    offlineRenderingWidth = w;
    offlineRenderingHeight = h;

    // Select page, if not current
    if (page)
    {
        currentTime = start_time + time_offset;
        runProgram();
        gotoPage(NULL, pageNameAtIndex(NULL, page));
    }

    // Create a GL frame to render into
    FrameInfo frame(w, h);

    // Render frames for the whole time range
    int currentFrame = firstFrame, frameCount = duration * fps;
    int percent, prevPercent = 0;
    int digits = (int)log10(frameCount) + 1;

    if (fileName.contains("%0d"))
    {
        // Replace '%0d' with e.g., '%03d' if we need 3 digits.
        fileName.replace("%0d", "%0" + QString("%1d").arg(digits));
    }

    QThreadPool saveThreads(this); // # threads = # of CPU cores, by default
    // Semaphore limits the number of frames pending save
    QSemaphore semaphore(QThread::idealThreadCount() + 1);
    QTime fpsTimer;
    fpsTimer.start();

    for (double t = start_time; t < start_time + duration; t += 1.0/fps)
    {
#define CHECK_CANCELED()                                                \
        if (renderFramesCanceled == 2) { inOfflineRendering = false; return; } \
        else if (renderFramesCanceled == 1) { renderFramesCanceled = 0; break; }

        // Show progress information
        percent = 100*(currentFrame-firstFrame)/frameCount;
        if (percent != prevPercent)
        {
            prevPercent = percent;
            emit renderFramesProgress(percent);
        }

        // Set time and run program
        currentTime = t + time_offset;

        if (gotoPageName != "")
        {
            record(pages, "Goto page %s (renderframes) from %s",
                   gotoPageName, pageName);
            commitPageChange(false);
            if (time_offset && currentFrame == firstFrame)
            {
                frozenTime = start_time + time_offset;
                pageStartTime = startTime = start_time;
            }
            pageEntry = 0;
        }

        if (currentFrame == firstFrame)
        {
            runProgram();
        }
        else
        {
            QTimerEvent e(0);
            refreshNow(&e);
        }

        // Draw the layout in the frame context
        id = idDepth = 0;
        space->ClearPolygonOffset();
        frame.begin();
        displayDriver->display();
        frame.end();

        frameCounter++;

        QApplication::processEvents();
        CHECK_CANCELED();

        // Save frame to disk
        // QString::sprintf() expects (and outputs) Latin1 strings. We need UTF-8,
        // and this works.
        QString fp;
        fp.sprintf(QString("%1/%2").arg(dir).arg(fileName).toUtf8().data(),
                   currentFrame);
        QString filePath = QString::fromUtf8(fp.toLatin1().data());

        if (dir != "/dev/null")
        {
            SaveImage * task = new SaveImage(filePath, frame.toImage(),
                                             &semaphore);
            saveThreads.start(task);
        }

        currentFrame++;

        QApplication::processEvents();
        CHECK_CANCELED();
    }

    double elapsed = fpsTimer.elapsed();
    record(layout_fps,
           "Rendered %lu frames in %lu ms, approx %lu FPS",
           currentFrame, elapsed, 1000 * currentFrame / elapsed);
    saveThreads.waitForDone();
    elapsed = fpsTimer.elapsed();
    record(layout_fps,
           "Saved %lu frames in %lu ms, approx %lu FPS",
           currentFrame, elapsed, 1000 * currentFrame / elapsed);

    // Done with offline rendering
    inOfflineRendering = false;
    if (!prevDisplay.isEmpty())
        displayDriver->setDisplayFunction(prevDisplay);
    emit renderFramesDone();

    // Redraw
    refreshNow();
}


Name *Widget::saveThumbnail(int     w,
                            int     h,
                            int     page,
                            text    file,
                            double  pageTime)
// ----------------------------------------------------------------------------
//   Take a snapshot of a page (2D) and save it. Based on renderFrames().
// ----------------------------------------------------------------------------
{
    // Do not re-enter this function
    static bool inSaveThumbnail = false;
    if (inSaveThumbnail)
        return XL::xl_false;
    XL::Save<bool> noReenter(inSaveThumbnail, true);

    // Create output directory if needed
    QFileInfo inf(QDir(+currentDocumentFolder()), +file);
    QString dir = inf.absoluteDir().absolutePath();
    if (!QDir().mkpath(dir))
        return XL::xl_false;
    file = +inf.absoluteFilePath();

    bool ok = false;
    TaoSave saveCurrent(current, this);

    // Select 2D display. Requires current != NULL.
    QString disp = "2D";
    QString prevDisplay;
    if (!displayDriver->isCurrentDisplayFunctionSameAs(disp))
    {
        // Select 2D display function
        prevDisplay = displayDriver->getDisplayFunction();
        displayDriver->setDisplayFunction(disp);
    }

    XL::Save<uint> savePageEntry(pageEntry, pageEntry);

    // Set the initial time we want to set and freeze animations
    XL::Save<double> setPageTime(pageStartTime, 0);
    XL::Save<double> setFrozenTime(frozenTime, pageTime);
    XL::Save<double> saveStartTime(startTime, 0);
    XL::Save<page_list> savePageNames(pageNames, pageNames);
    XL::Save<text> savePageName(pageName, transitionPageName);

    GLAllStateKeeper saveGL;
    XL::Save<double> saveScaling(scaling, scaling);

    // Render with fullscreen size (then later scale)
    QDesktopWidget *dw = QApplication::desktop();
    QRect geom = dw->screenGeometry(this);
    int sw = geom.width();
    int sh = geom.height();
    offlineRenderingWidth = sw * displayDriver->windowWidthFactor();
    offlineRenderingHeight = sh * displayDriver->windowHeightFactor();
    inOfflineRendering = true;

    // Select page, if not current
    if (page != -1)
    {
        currentTime = pageTime;
        runProgram();
        gotoPage(NULL, pageNameAtIndex(NULL, page));
    }

    // Create a GL frame to render into
    FrameInfo frame(offlineRenderingWidth, offlineRenderingHeight);

    // Set time and run program
    currentTime = pageTime;

    if (gotoPageName != "")
    {
        record(pages, "Goto page %s (thumbnail) from %s",
               gotoPageName, pageName);
        commitPageChange(false);
        pageEntry = 0;
    }

    pageStartTime = startTime = 0; // REVISIT
    runProgram();

    // Draw the layout in the frame context
    id = idDepth = 0;
    space->ClearPolygonOffset();
    frame.begin();
    displayDriver->display();
    frame.end();

    // Save frame to disk
    QImage img = frame.toImage();
    QImage scaled = img.scaled(w, h, Qt::KeepAspectRatio,
                               Qt::SmoothTransformation);
    ok = scaled.save(+file);

    // Done with offline rendering
    inOfflineRendering = false;
    if (!prevDisplay.isEmpty())
        displayDriver->setDisplayFunction(prevDisplay);

    return ok ? XL::xl_true : XL::xl_false;
}


void Widget::identifySelection()
// ----------------------------------------------------------------------------
//   Draw the elements in global space for selection purpose
// ----------------------------------------------------------------------------
{
    id = idDepth = 0;
    space->offset.Set(0,0,0);
    space->Identify(NULL);
}


void Widget::updateSelection()
// ----------------------------------------------------------------------------
//   Redraw selection, perform associated operations
// ----------------------------------------------------------------------------
//   One important operation that happens while we draw the selection is
//   text editing. If a text span notes that there is a text selection,
//   and that the text selection has an associated operation, it will
//   update the Text* it points to (e.g. insert or delete characters)
{
    id = idDepth = 0;
    selectionTrees.clear();
    space->ClearPolygonOffset();
    space->ClearAttributes();
    space->DrawSelection(NULL);
}


#if 0
static std::ostream &operator<<(std::ostream &out, QWidget *w)
// ----------------------------------------------------------------------------
//   Print a widget for debugging purpose
// ----------------------------------------------------------------------------
{
    out << (void *) w;
    if (w)
        out << " (" << w->metaObject()->className() << ")";
    return out;
}
#endif



void Widget::appFocusChanged(QWidget *prev, QWidget *next)
// ----------------------------------------------------------------------------
//   Notifications when focus changes
// ----------------------------------------------------------------------------
{
    record(focus, "Focus from %p to %p", prev, next);
}



void Widget::checkCopyAvailable()
// ----------------------------------------------------------------------------
//   Emit a signal when clipboard can copy or cut something (or cannot anymore)
// ----------------------------------------------------------------------------
{
    bool sel = hasSelection();
    if (hadSelection != sel)
    {
        emit copyAvailableAndNotReadOnly(sel && !isReadOnly());
        emit copyAvailable(sel);
        hadSelection = sel;
    }
}


bool Widget::canPaste()
// ----------------------------------------------------------------------------
//   Is current clibpoard data in a suitable format to be pasted?
// ----------------------------------------------------------------------------
{
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    return (mimeData->hasFormat(TAO_CLIPBOARD_MIME_TYPE)
            || mimeData->hasHtml()
            || mimeData->hasText());
}


void Widget::cut()
// ----------------------------------------------------------------------------
//   Cut current selection into clipboard
// ----------------------------------------------------------------------------
{
    copy();
    record(clipboard, "Cut: Deleting selection");
    deleteSelection(xlProgram->scope, XL::xl_nil, "Delete");
}


void Widget::copy()
// ----------------------------------------------------------------------------
//   Copy current selection into clipboard
// ----------------------------------------------------------------------------
// If some text is selected it is copied as html and plain text into clipboard
// otherwise the selected tree is serialized and placed into the clipboard.
{
    if (!hasSelection())
    {
        //If no selection copy the Image
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setImage(grabFrameBuffer(true));

        return;
    }
    QMimeData *mimeData = new QMimeData;

    TextSelect *sel;
    if ((sel = textSelection())) // Text selected
    {
        QString copy = sel->cursor.document()->toHtml("utf-8");
        record(clipboard, "Copying %s", copy);

        // Encapsulate text as MIME data
        mimeData->setHtml(copy);
        mimeData->setText(sel->cursor.document()->toPlainText());
    }
    else  // Object selected
    {
        // Build a single tree from all the selected sub-trees
        Tree *tree = copySelection();

        if (!tree)
            return;

        record(clipboard, "Copying tree %t", tree);

        // Serialize the tree
        std::string ser;
        std::ostringstream ostr;
        XL::Serializer serializer(ostr);
        tree->Do(serializer);
        ser += ostr.str();

        // Encapsulate serialized tree as MIME data
        QByteArray binData;
        binData.append(ser.data(), ser.length());
        mimeData->setData(TAO_CLIPBOARD_MIME_TYPE, binData);
    }

    // Transfer into clipboard
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setMimeData(mimeData);
}


void Widget::paste()
// ----------------------------------------------------------------------------
//   Paste the clipboard content at the current selection
// ----------------------------------------------------------------------------
// Paste TAO MIME type or HTML MIME type or TEXT MIME type
{
    // Does clipboard contain Tao stuff?)
    if (!canPaste())
        return;

    const QMimeData *mimeData = QApplication::clipboard()->mimeData();

    if (mimeData->hasFormat(TAO_CLIPBOARD_MIME_TYPE))
    {
        // Extract serialized tree
        QByteArray binData = mimeData->data(TAO_CLIPBOARD_MIME_TYPE);
        std::string ser(binData.data(), binData.length());

        // De-serialize
        std::istringstream istr(ser);
        XL::Deserializer deserializer(istr);
        Tree *tree = deserializer.ReadTree();
        if (!deserializer.IsValid())
            return;

        record(clipboard, "Pasting tree %t", tree);

        // Insert tree at end of current page
        // TODO: paste with an offset to avoid exactly overlapping objects
        insert(xlProgram->scope, XL::xl_nil, tree);

        return;
    }

    TextSelect *sel  = textSelection();

    if (sel)
    {
        if (mimeData->hasHtml())
        {
            record(clipboard, "Pasting HTML %s", mimeData->html());
            HTMLConverter htmlCvt(xlProgram->tree, space);
            sel->replacement = "";
            sel->replace = true;
            sel->replacement_tree = htmlCvt.fromHTML(mimeData->html());
            record(clipboard, "Pasting HTML tree %t", sel->replacement_tree);
            updateGL();
            return;
        }
        if (mimeData->hasText())
        {
            record(clipboard, "Pasting text %s", mimeData->text());
            sel->replacement = +mimeData->text();
            sel->replace = true;
            updateGL();
            return;
        }
    }
    else
    {
        Tree * t = NULL;
        if (mimeData->hasHtml())
        {
            HTMLConverter htmlCvt(xlProgram->tree, space);
            t = htmlCvt.fromHTML(mimeData->html());
        }
        else if (mimeData->hasText())
        {
            text pasted = +mimeData->text();
            t = XL::XLCall("paragraph", pasted).Build();
        }
        else
        {
            return;
        }

        // Insert a text box with that content at the end of the doc/page.
        Block *block = new Block(t, "I+", "I-");
        Tree *box = XL::XLCall("text_box", 0, 0, 200, 200, block).Build();
        Block *shapeBlock = new Block(box, "I+", "I-");
        Tree *toPaste = XL::XLCall("shape", shapeBlock).Build();
        insert(xlProgram->scope, XL::xl_nil, toPaste, "paste from HTML");
    }
}


Name *Widget::bringToFront(Tree */*self*/)
// ----------------------------------------------------------------------------
//   Bring the selected shape to front
// ----------------------------------------------------------------------------
{
    Tree *select = copySelection();
    if (!select)
        return XL::xl_false;
    deleteSelection();

    insert(xlProgram->scope, XL::xl_nil, select, "Selection brought to front");
    return XL::xl_true;
}


Name *Widget::sendToBack(Tree */*self*/)
// ----------------------------------------------------------------------------
//   Send the selected shape to back
// ----------------------------------------------------------------------------
{
    if (!xlProgram || !markChange("Selection sent to back"))
        return XL::xl_false;    // Source code was edited

    Tree *select = copySelection();
    if (!select)
        return XL::xl_false;

    deleteSelection();
    // Make sure the new objects appear selected next time they're drawn
    selectStatements(select);

    // Start at the top of the program to find where we will insert
    Tree_p *top = &xlProgram->tree;
    Tree *page = pageTree;

    // If we have a current page, insert only in that context
    if (page)
    {
        // Restrict insertion to that page
        top = &pageTree;

        // The page instructions often runs a 'do' block
        if (Prefix *prefix = page->AsPrefix())
            if (Name *left = prefix->left->AsName())
                if (left->value == "do")
                    top = &prefix->right;

        // If the page code is a block, look inside
        if (Block *block = (*top)->AsBlock())
            top = &block->child;
    }
    if (*top)
    {
        *top = new Infix("\n", select, *top);
    }
    else
    {
        *top = select;
    }

    // Reload the program and mark the changes
    reloadProgram();

    return XL::xl_true;
}


Name *Widget::bringForward(Tree */*self*/)
// ----------------------------------------------------------------------------
//   Swap the selected shape and the one in front of it
// ----------------------------------------------------------------------------
{
    if (!xlProgram      ||
        !hasSelection() ||
        !markChange("Selection brought forward"))
        return XL::xl_false;

    tree_set::iterator sel = selectionTrees.begin();
    XL::FindParentAction getParent(*sel);
    Tree * parent = xlProgram->tree->Do(getParent);

    // Check if we are not the only one
    if (!parent)
        return XL::xl_false;
    Infix * current = parent->AsInfix();
    if (!current)
        return XL::xl_false;

    Tree * tmp =  NULL;
    Infix * next = current->right->AsInfix();
    if (!next)
    {
        // We are at the bottom of the tree
        //Check if we are already the latest
        if (current->right == *sel)
            return XL::xl_false;

        // Just swap left and right of parent.
        tmp = current->left;
        current->left = current->right;
        current->right = tmp;
    }
    else
    {
        tmp = current->left;
        current->left = next->left;
        next->left = tmp;
    }
    selectStatements(tmp);

    // Reload the program and mark the changes
    reloadProgram();

    return XL::xl_true;
}


Name *Widget::sendBackward(Tree */*self*/)
// ----------------------------------------------------------------------------
//   Swap the selected shape and the one just behind it
// ----------------------------------------------------------------------------
{
    if (!xlProgram || !hasSelection() || !markChange("Selection sent backward"))
        return XL::xl_false;

    tree_set::iterator sel = selectionTrees.begin();
    XL::FindParentAction getParent(*sel);
    Tree * parent = xlProgram->tree->Do(getParent);
    // Check if we are not the only one
    if (!parent)
        return XL::xl_false;
    Infix * current = parent->AsInfix();
    if (!current)
        return XL::xl_false;

    Tree * tmp = NULL;
    // check if we are at the bottom of the tree
    if (current->right == *sel)
    {
        tmp = current->right;
        current->right = current->left;
        current->left = tmp;
    }
    else
    {
        XL::FindParentAction getGrandParent(parent);
        Tree * grandParent = xlProgram->tree->Do(getGrandParent);
        // No grand parent means the shape is already to back
        if (!grandParent)
            return XL::xl_false;
        Infix * previous = grandParent->AsInfix();
        if (!previous)
            return XL::xl_false;

        tmp = current->left;
        current->left = previous->left;
        previous->left = tmp;
    }
    selectStatements(tmp);

    // Reload the program and mark the changes
    reloadProgram();

    return XL::xl_true;
}


QStringList Widget::fontFiles()
// ----------------------------------------------------------------------------
//   Return the paths of all font files used in the document
// ----------------------------------------------------------------------------
{
    struct FFM {
        FFM(FontFileManager *&m): m(m) { m = new FontFileManager(); }
        ~FFM()                          { delete m; m = NULL; }
        FontFileManager *&m;
    } ffm(fontFileMgr);

    TaoSave saveCurrent(current, this);
    drawAllPages = true;
    runProgram();
    drawAllPages = false;
    runProgram();
    if (!fontFileMgr->errors.empty())
    {
        // Some font files are not in a suitable format, so we won't try to
        // embed them (Qt can only load TrueType, TrueType Collection and
        // OpenType files).
        Window *window = taoWindow();
        foreach (QString m, fontFileMgr->errors)
            window->addError(m);
    }
    return fontFileMgr->fontFiles;
}


void Widget::enableAnimations(bool enable)
// ----------------------------------------------------------------------------
//   Enable or disable animations on the page
// ----------------------------------------------------------------------------
{
    if (enable)
        pageStartTime += (trueCurrentTime() - frozenTime);
    frozenTime = CurrentTime();
    startRefreshTimer(animated = enable);
}


void Widget::showHandCursor(bool enabled)
// ----------------------------------------------------------------------------
//   Switch panning mode on/off
// ----------------------------------------------------------------------------
{
    if (enabled)
        savedCursorShape = Qt::OpenHandCursor;
    else
        savedCursorShape = Qt::ArrowCursor;
    setCursor(savedCursorShape);
}


void Widget::hideCursor()
// ----------------------------------------------------------------------------
//   Hide the mouse cursor if in auto-hide mode
// ----------------------------------------------------------------------------
{
    if (bAutoHideCursor)
    {
        setCursor(Qt::BlankCursor);
        // Be notified when we need to restore cursor
        setMouseTracking(true);
    }
}


void Widget::setCursor(const QCursor &cursor)
// ----------------------------------------------------------------------------
//   Really set mouse cursor if cursor is not hidden, otherwise simulate.
// ----------------------------------------------------------------------------
{
    if (mouseCursorHidden)
    {
        cachedCursor = cursor;
        return;
    }
    QWidget::setCursor(cursor);
}


QCursor Widget::cursor() const
// ----------------------------------------------------------------------------
//   Return current mouse cursor if cursor is not hidden, otherwise simulate.
// ----------------------------------------------------------------------------
{
    if (mouseCursorHidden)
        return cachedCursor;
    return QWidget::cursor();
}


void Widget::resetView()
// ----------------------------------------------------------------------------
//   Restore default view parameters (zoom, position etc.)
// ----------------------------------------------------------------------------
{
    zNear = 1500.0;
    zFar  = 1e6;
    zoom  = 1.0;
    eyeDistance    = 25.0;
    cameraPosition = defaultCameraPosition;
    cameraTarget   = Point3(0.0, 0.0, 0.0);
    cameraUpVector = Vector3(0, 1, 0);
    cameraToScreen = Vector3(cameraTarget - cameraPosition).Length();
    scaling = scalingFactorFromCamera();
}


void Widget::resetViewAndRefresh()
// ----------------------------------------------------------------------------
//   Restore default view parameters and refresh display
// ----------------------------------------------------------------------------
{
    resetView();
    setup(renderWidth(), renderHeight());
    QEvent r(QEvent::Resize);
    refreshNow(&r);
}


void Widget::zoomIn()
// ----------------------------------------------------------------------------
//    Call zoom_in builtin
// ----------------------------------------------------------------------------
{
    if (!xlProgram) return;
    TaoSave saveCurrent(current, this);
    XL::XLCall("zoom_in")(xlProgram->scope);
    do
    {
        TaoSave saveCurrent(current, NULL);
        updateGL();
    } while (0);
}


void Widget::zoomOut()
// ----------------------------------------------------------------------------
//    Call zoom_out builtin
// ----------------------------------------------------------------------------
{
    if (!xlProgram) return;
    TaoSave saveCurrent(current, this);
    XL::XLCall("zoom_out")(xlProgram->scope);
    do
    {
        TaoSave saveCurrent(current, NULL);
        updateGL();
    } while (0);
}


void Widget::saveAndCommit()
// ----------------------------------------------------------------------------
//   Save files and commit to repository if needed
// ----------------------------------------------------------------------------
{
    ulonglong tick = now();
#ifdef CFG_NOGIT
    doSave(tick);
#else
    if (doSave(tick))
        if (GeneralPage::gitEnabled())
            doCommit(tick);
#endif
}


void Widget::userMenu(QAction *action)
// ----------------------------------------------------------------------------
//   User menu slot activation
// ----------------------------------------------------------------------------
{
    if (!action || !xlProgram)
        return;

    record(menus,
           "Action %s (%s) activated", action->objectName(), action->text());
    QVariant var =  action->data();
    if (!var.isValid())
        return;

    // Evaliuate the code for the action, typically inserting something
    TaoSave saveCurrent(current, this);
    Tree *actionCode = var.value<Tree_p>();
    if (actionCode)
        saveAndEvaluate(xlProgram->scope, actionCode);
}


void Widget::refresh(double delay)
// ----------------------------------------------------------------------------
//   Refresh current layout after the given number of seconds
// ----------------------------------------------------------------------------
{
    double end = CurrentTime() + delay;
    refreshOn(QEvent::Timer, end);
}



// ============================================================================
//
//   OpenGL setup
//
// ============================================================================

#ifdef Q_OS_MACX
bool Widget::frameBufferReady()
// ----------------------------------------------------------------------------
//    On MacOSX Lion we must check if GL frame buffer can be used. See #1658.
// ----------------------------------------------------------------------------
{
    if (bFrameBufferReady)
        return true;

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
    {
        bFrameBufferReady = true;
        return true;
    }

    return false;
}
#endif

void Widget::initializeGL()
// ----------------------------------------------------------------------------
//    Called once per rendering to setup the GL environment
// ----------------------------------------------------------------------------
{
}


void Widget::resizeGL(int width, int height)
// ----------------------------------------------------------------------------
//   Called when the size changes
// ----------------------------------------------------------------------------
{
    if (!frameBufferReady())
        return;

    // Use physical pixel coordinates for the viewport (#3254)
    space->space = Box3(-width/2, -height/2, 0, width, height, 0);
    stats.reset();
#ifdef MACOSX_DISPLAYLINK
    displayLinkMutex.lock();
    droppedFrames = 0;
    displayLinkMutex.unlock();
#endif

    TaoSave saveCurrent(current, this);
    QEvent r(QEvent::Resize);
    refreshNow(&r);
}


void Widget::paintGL()
// ----------------------------------------------------------------------------
//    Repaint the contents of the window
// ----------------------------------------------------------------------------
{
    if (!frameBufferReady())
        return;
    if (isInvalid)
        return;
    if (!printer)
    {
        draw();
        showGlErrors();
        if (screenShotPath != "" ||
            saveProofOfPlayThread.isReady() || savePreviewThread.isReady())
        {
            bool withAlpha = screenShotWithAlpha || screenShotPath == "";
            QImage grabbed = grabFrameBuffer(withAlpha);
            if (screenShotPath != "")
            {
                scale s = screenShotScale;
                if (s != 1.0)
                    QImage scaled = grabbed.scaled(s * grabbed.size());
                grabbed.save(screenShotPath);
                screenShotPath = "";
            }
            if (savePreviewThread.isReady())
                savePreviewThread.recordImage(grabbed);
            if (saveProofOfPlayThread.isReady())
                saveProofOfPlayThread.recordImage(grabbed);
        }
    }
}


double Widget::scalingFactorFromCamera()
// ----------------------------------------------------------------------------
//   Return the factor to use for zoom adjustments
// ----------------------------------------------------------------------------
{
    scale csf = cameraToScreen / zNear;
    return csf;
}


void Widget::setup(double w, double h, const Box *picking)
// ----------------------------------------------------------------------------
//   Setup an initial environment for drawing
// ----------------------------------------------------------------------------
{
#define ROUNDUP(x) (x+1 - fmod(x+1, 2))
    // #806, #3094 Rendering artifacts when window is resized (1 px)
    w = ROUNDUP(w);
    h = ROUNDUP(h);
#undef ROUNDUP

    // #2784 Make sure we don't pass invalid values to glFrustrum
    if (w == 0) w = 2;
    if (h == 0) h = 2;

    // Setup viewport
    scale s = printer && picking ? printOverscaling : 1;
    GLint vx = 0, vy = 0, vw = int(w * s), vh = int(h * s);
    GL.Viewport(vx, vy, vw, vh);

    // Setup the projection matrix
    GL.MatrixMode(GL_PROJECTION);
    GL.LoadIdentity();

    // Restrict the picking area if any is given as input
    if (picking)
    {
        // Use mouseTrackingViewport to fix #1465
        int pw = mouseTrackingViewport[2];
        int ph = mouseTrackingViewport[3];
        if (pw == 0 && ph == 0)
        {
            // mouseTrackingViewport not set (by display module), default to
            // current viewport
            pw = w;
            ph = h;
        }

        GLint viewport[4] = { 0, 0, pw, ph };
        Box b = *picking;
        b.Normalize();
        Vector size = b.upper - b.lower;
        Point center = b.lower + size / 2;
        GL.PickMatrix(center.x, center.y, size.x+1, size.y+1, viewport);
    }
    double zf = 0.5 / (zoom * scaling);
    GL.Frustum(-w*zf, w*zf, -h*zf, h*zf, zNear, zFar);
    GL.LoadMatrix();

    // Setup the model-view matrix
    GL.MatrixMode(GL_MODELVIEW);
    resetModelviewMatrix();

    // Reset default GL parameters
    setupGL();
}


void Widget::reset()
// ----------------------------------------------------------------------------
//   Reset view and other widget settings, ready to execute new document
// ----------------------------------------------------------------------------
{
    resetView();
    if (!animated)
        taoWindow()->toggleAnimations();
    blanked = false;
    stereoIdent = false;
    pageShown = 1;       // BUG #1986
    gotoPageName = "";   // BUG #2069
    pageName = "";       // BUG #2069
    pageEntry = pageExit = 0;
#ifdef CFG_UNLICENSED_MAX_PAGES
    pageLimitationDialogShown = false;
#endif
}


void Widget::resetModelviewMatrix()
// ----------------------------------------------------------------------------
//   Reset the model-view matrix, used by reset_transform and setup
// ----------------------------------------------------------------------------
{
    GL.LoadIdentity();

    // Position the camera
    Vector3 toTarget = Vector3(cameraTarget - cameraPosition).Normalize();
    toTarget *= cameraToScreen;
    Point3 target = cameraPosition + toTarget;
    GL.LookAt(cameraPosition, target, cameraUpVector);
}


void Widget::setupGL()
// ----------------------------------------------------------------------------
//   Setup default GL parameters
// ----------------------------------------------------------------------------
{
    // Setup other
    GL.Enable(GL_BLEND);
    GL.BlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
                         GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    GL.DepthMask(GL_TRUE);
    GL.DepthFunc(GL_LEQUAL);
    GL.Enable(GL_DEPTH_TEST);
    GL.Enable(GL_LINE_SMOOTH);
    GL.Enable(GL_POINT_SMOOTH);
    GL.Enable(GL_MULTISAMPLE);
    GL.Enable(GL_POLYGON_OFFSET_FILL);
    GL.Enable(GL_POLYGON_OFFSET_LINE);
    GL.Enable(GL_POLYGON_OFFSET_POINT);
    GL.Color(1.0f, 1.0f, 1.0f, 1.0f);
    GL.LineWidth(1);
    GL.LineStipple(1, -1);

    // Disable all texture units
    GL.ActivateTextureUnits(0);
    GL.ActiveTexture(GL_TEXTURE0); // Reset the default active texture
    GL.Disable(GL_TEXTURE_RECTANGLE_ARB);
    GL.Disable(GL_CULL_FACE);
    GL.CullFace(GL_BACK);
    GL.ShadeModel(GL_SMOOTH);
    GL.Disable(GL_LIGHTING);
    GL.Disable(GL_COLOR_MATERIAL);
    GL.UseProgram(0);
    GL.AlphaFunc(GL_GREATER, 0.01);
    GL.Enable(GL_ALPHA_TEST);

    // Turn on sphere map automatic texture coordinate generation
    GL.TexGen(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    GL.TexGen(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

    // Really nice perspective calculations
    GL.Hint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}


uint Widget::showGlErrors()
// ----------------------------------------------------------------------------
//   Display all OpenGL errors in the error window
// ----------------------------------------------------------------------------
{
    static GLenum last = GL_NO_ERROR;
    static unsigned int count = 0;
    GLenum err = glGetError();
    uint errorsFound = 0;
    while (err != GL_NO_ERROR)
    {
        QString msg;
        errorsFound++;
        if (!count)
        {
            char *p = (char *) gluErrorString(err);
            msg = tr("GL Error: %1 [error code: %2]")
                .arg(QString::fromLocal8Bit(p)).arg(err);
            last = err;
        }
        if (err != last || count == 100)
        {
            msg += tr("GL Error: error %1 repeated %2 times")
                .arg(last).arg(count);
            count = 0;
        }
        else
        {
            count++;
        }
        if (msg.length())
            taoWindow()->addError(msg);
        err = glGetError();
    }

    return errorsFound;
}


QFont &Widget::currentFont()
// ----------------------------------------------------------------------------
//   Return the font currently in use (the one in the current layout)
// ----------------------------------------------------------------------------
{
    return layout->font;
}






// ============================================================================
//
//   Widget basic events (painting, mause, ...)
//
// ============================================================================

bool Widget::forwardEvent(QEvent *event)
// ----------------------------------------------------------------------------
//   Forward event to the focus proxy if there is any
// ----------------------------------------------------------------------------
{
    refreshNow(event);
    if (QObject *focus = focusWidget)
    {
        record(widget,
               "Forward event %d to %s", event->type(), focus->objectName());
        return focus->event(event);
    }
    return false;
}


bool Widget::forwardEvent(QMouseEvent *event)
// ----------------------------------------------------------------------------
//   Forward event to the focus proxy if there is any, adjusting coordinates
// ----------------------------------------------------------------------------
{
    refreshNow(event);
    if (QObject *focus = focusWidget)
    {
        int x = event->x();
        int y = event->y();
        int w = focusWidget->width();
        int h = focusWidget->height();

        Point3 u = unproject(x, y, 0);
        int nx = u.x + w/2;
        int ny = h/2 - u.y;
        QMouseEvent local(event->type(), QPoint(nx, ny),
                          event->button(), event->buttons(),
                          event->modifiers());
        record(widget,
               "Forward event %d at (%d, %d) to %s",
               event->type(), nx, ny, focus->objectName());
        bool res = focus->event(&local);
        event->setAccepted(local.isAccepted());
        return res;
    }

    return false;
}


static text toKeyName(QKeyEvent *event)
// ----------------------------------------------------------------------------
//   Return text form of QKeyEvent::key()
// ----------------------------------------------------------------------------
{
    text ret =  "";

#define CHK(key) case Qt::key: ret = #key ; break

    uint key = (uint) event->key();
    switch(key)
    {
        CHK(Key_Space);
        CHK(Key_A); CHK(Key_B); CHK(Key_C); CHK(Key_D); CHK(Key_E); CHK(Key_F);
        CHK(Key_G); CHK(Key_H); CHK(Key_I); CHK(Key_J); CHK(Key_K); CHK(Key_L);
        CHK(Key_M); CHK(Key_N); CHK(Key_O); CHK(Key_P); CHK(Key_Q); CHK(Key_R);
        CHK(Key_S); CHK(Key_T); CHK(Key_U); CHK(Key_V); CHK(Key_W); CHK(Key_X);
        CHK(Key_Y); CHK(Key_Z);
        CHK(Key_0); CHK(Key_1); CHK(Key_2); CHK(Key_3); CHK(Key_4); CHK(Key_5);
        CHK(Key_6); CHK(Key_7); CHK(Key_8); CHK(Key_9);
        CHK(Key_Exclam);
        CHK(Key_QuoteDbl);
        CHK(Key_NumberSign);
        CHK(Key_Dollar);
        CHK(Key_Percent);
        CHK(Key_Ampersand);
        CHK(Key_Apostrophe);
        CHK(Key_ParenLeft);
        CHK(Key_ParenRight);
        CHK(Key_Asterisk);
        CHK(Key_Plus);
        CHK(Key_Comma);
        CHK(Key_Minus);
        CHK(Key_Period);
        CHK(Key_Slash);
        CHK(Key_Colon);
        CHK(Key_Semicolon);
        CHK(Key_Less);
        CHK(Key_Equal);
        CHK(Key_Greater);
        CHK(Key_Question);
        CHK(Key_At);
        CHK(Key_BracketLeft);
        CHK(Key_Backslash);
        CHK(Key_BracketRight);
        CHK(Key_AsciiCircum);
        CHK(Key_Underscore);
        CHK(Key_QuoteLeft);
        CHK(Key_BraceLeft);
        CHK(Key_Bar);
        CHK(Key_BraceRight);
        CHK(Key_AsciiTilde);
        CHK(Key_Escape);
        CHK(Key_Tab);
        CHK(Key_Backtab);
        CHK(Key_Backspace);
        CHK(Key_Return);
        CHK(Key_Enter);
        CHK(Key_Insert);
        CHK(Key_Delete);
        CHK(Key_Pause);
        CHK(Key_Print);
        CHK(Key_SysReq);
        CHK(Key_Clear);
        CHK(Key_Home);
        CHK(Key_End);
        CHK(Key_Left);
        CHK(Key_Up);
        CHK(Key_Right);
        CHK(Key_Down);
        CHK(Key_PageUp);
        CHK(Key_PageDown);
        CHK(Key_Shift);
        CHK(Key_Control);
        CHK(Key_Meta);
        CHK(Key_Alt);
        CHK(Key_AltGr);
        CHK(Key_CapsLock);
        CHK(Key_NumLock);
        CHK(Key_ScrollLock);
        CHK(Key_F1);  CHK(Key_F2);  CHK(Key_F3);  CHK(Key_F4);  CHK(Key_F5);
        CHK(Key_F6);  CHK(Key_F7);  CHK(Key_F8);  CHK(Key_F9);  CHK(Key_F10);
        CHK(Key_F11); CHK(Key_F12); CHK(Key_F13); CHK(Key_F14); CHK(Key_F15);
        CHK(Key_F16); CHK(Key_F17); CHK(Key_F18); CHK(Key_F19); CHK(Key_F20);
        CHK(Key_F21); CHK(Key_F22); CHK(Key_F23); CHK(Key_F24); CHK(Key_F25);
        CHK(Key_F26); CHK(Key_F27); CHK(Key_F28); CHK(Key_F29); CHK(Key_F30);
        CHK(Key_F31); CHK(Key_F32); CHK(Key_F33); CHK(Key_F34); CHK(Key_F35);
        CHK(Key_Menu);
        CHK(Key_Help);
        CHK(Key_Back);
        CHK(Key_Forward);
        CHK(Key_Stop);
        CHK(Key_Refresh);
        CHK(Key_VolumeDown);
        CHK(Key_VolumeMute);
        CHK(Key_VolumeUp);
        CHK(Key_BassBoost);
        CHK(Key_BassUp);
        CHK(Key_BassDown);
        CHK(Key_TrebleUp);
        CHK(Key_TrebleDown);
        CHK(Key_MediaPlay);
        CHK(Key_MediaStop);
        CHK(Key_MediaPrevious);
        CHK(Key_MediaNext);
        CHK(Key_MediaRecord);
        CHK(Key_HomePage);
        CHK(Key_Favorites);
        CHK(Key_Search);
        CHK(Key_Standby);
        CHK(Key_OpenUrl);
        CHK(Key_LaunchMail);
        CHK(Key_LaunchMedia);
        CHK(Key_Launch0); CHK(Key_Launch1); CHK(Key_Launch2); CHK(Key_Launch3);
        CHK(Key_Launch4); CHK(Key_Launch5); CHK(Key_Launch6); CHK(Key_Launch7);
        CHK(Key_Launch8); CHK(Key_Launch9); CHK(Key_LaunchA); CHK(Key_LaunchB);
        CHK(Key_LaunchC); CHK(Key_LaunchD); CHK(Key_LaunchE); CHK(Key_LaunchF);
        CHK(Key_MonBrightnessUp);
        CHK(Key_MonBrightnessDown);
        CHK(Key_KeyboardLightOnOff);
        CHK(Key_KeyboardBrightnessUp);
        CHK(Key_KeyboardBrightnessDown);
        CHK(Key_PowerOff);
        CHK(Key_WakeUp);
        CHK(Key_Eject);
        CHK(Key_ScreenSaver);
        CHK(Key_WWW);
        CHK(Key_Memo);
        CHK(Key_LightBulb);
        CHK(Key_Shop);
        CHK(Key_History);
        CHK(Key_AddFavorite);
        CHK(Key_HotLinks);
        CHK(Key_BrightnessAdjust);
        CHK(Key_Finance);
        CHK(Key_Community);
        CHK(Key_AudioRewind);
        CHK(Key_BackForward);
        CHK(Key_ApplicationLeft);
        CHK(Key_ApplicationRight);
        CHK(Key_Book);
        CHK(Key_CD);
        CHK(Key_Calculator);
        CHK(Key_ToDoList);
        CHK(Key_ClearGrab);
        CHK(Key_Close);
        CHK(Key_Copy);
        CHK(Key_Cut);
        CHK(Key_Display);
        CHK(Key_DOS);
        CHK(Key_Documents);
        CHK(Key_Excel);
        CHK(Key_Explorer);
        CHK(Key_Game);
        CHK(Key_Go);
        CHK(Key_iTouch);
        CHK(Key_LogOff);
        CHK(Key_Market);
        CHK(Key_Meeting);
        CHK(Key_MenuKB);
        CHK(Key_MenuPB);
        CHK(Key_MySites);
        CHK(Key_News);
        CHK(Key_OfficeHome);
        CHK(Key_Option);
        CHK(Key_Paste);
        CHK(Key_Phone);
        CHK(Key_Calendar);
        CHK(Key_Reply);
        CHK(Key_Reload);
        CHK(Key_RotateWindows);
        CHK(Key_RotationPB);
        CHK(Key_RotationKB);
        CHK(Key_Save);
        CHK(Key_Send);
        CHK(Key_Spell);
        CHK(Key_SplitScreen);
        CHK(Key_Support);
        CHK(Key_TaskPane);
        CHK(Key_Terminal);
        CHK(Key_Tools);
        CHK(Key_Travel);
        CHK(Key_Video);
        CHK(Key_Word);
        CHK(Key_Xfer);
        CHK(Key_ZoomIn);
        CHK(Key_ZoomOut);
        CHK(Key_Away);
        CHK(Key_Messenger);
        CHK(Key_WebCam);
        CHK(Key_MailForward);
        CHK(Key_Pictures);
        CHK(Key_Music);
        CHK(Key_Battery);
        CHK(Key_Bluetooth);
        CHK(Key_WLAN);
        CHK(Key_UWB);
        CHK(Key_AudioForward);
        CHK(Key_AudioRepeat);
        CHK(Key_AudioRandomPlay);
        CHK(Key_Subtitle);
        CHK(Key_AudioCycleTrack);
        CHK(Key_Time);
        CHK(Key_Hibernate);
        CHK(Key_View);
        CHK(Key_TopMenu);
        CHK(Key_PowerDown);
        CHK(Key_Suspend);
        CHK(Key_ContrastAdjust);
        CHK(Key_MediaLast);
        CHK(Key_Call);
        CHK(Key_Context1);
        CHK(Key_Context2);
        CHK(Key_Context3);
        CHK(Key_Context4);
        CHK(Key_Flip);
        CHK(Key_Hangup);
        CHK(Key_No);
        CHK(Key_Select);
        CHK(Key_Yes);
        CHK(Key_Execute);
        CHK(Key_Printer);
        CHK(Key_Play);
        CHK(Key_Sleep);
        CHK(Key_Zoom);
        CHK(Key_Cancel);
    }

#undef CHK

    return ret;
}


static text toKeyEventName(QKeyEvent *event, uint prevModifiers,
                           text prevKeyText)
// ----------------------------------------------------------------------------
//   Return the properly formatted key name for a key event
// ----------------------------------------------------------------------------
{
    text name = +event->text();
    text ctrl = "";             // Name for Control, Meta and Alt

    if (event->type() == QEvent::KeyRelease &&
        name == "" && prevKeyText != "")
    {
        return "~" + prevKeyText;
    }

    uint key = (uint) event->key();
    switch(key)
    {
    case Qt::Key_Space:                 name = "Space"; break;
    case Qt::Key_A:                     ctrl = "A"; break;
    case Qt::Key_B:                     ctrl = "B"; break;
    case Qt::Key_C:                     ctrl = "C"; break;
    case Qt::Key_D:                     ctrl = "D"; break;
    case Qt::Key_E:                     ctrl = "E"; break;
    case Qt::Key_F:                     ctrl = "F"; break;
    case Qt::Key_G:                     ctrl = "G"; break;
    case Qt::Key_H:                     ctrl = "H"; break;
    case Qt::Key_I:                     ctrl = "I"; break;
    case Qt::Key_J:                     ctrl = "J"; break;
    case Qt::Key_K:                     ctrl = "K"; break;
    case Qt::Key_L:                     ctrl = "L"; break;
    case Qt::Key_M:                     ctrl = "M"; break;
    case Qt::Key_N:                     ctrl = "N"; break;
    case Qt::Key_O:                     ctrl = "O"; break;
    case Qt::Key_P:                     ctrl = "P"; break;
    case Qt::Key_Q:                     ctrl = "Q"; break;
    case Qt::Key_R:                     ctrl = "R"; break;
    case Qt::Key_S:                     ctrl = "S"; break;
    case Qt::Key_T:                     ctrl = "T"; break;
    case Qt::Key_U:                     ctrl = "U"; break;
    case Qt::Key_V:                     ctrl = "V"; break;
    case Qt::Key_W:                     ctrl = "W"; break;
    case Qt::Key_X:                     ctrl = "X"; break;
    case Qt::Key_Y:                     ctrl = "Y"; break;
    case Qt::Key_Z:                     ctrl = "Z"; break;
    case Qt::Key_0:                     ctrl = "0"; break;
    case Qt::Key_1:                     ctrl = "1"; break;
    case Qt::Key_2:                     ctrl = "2"; break;
    case Qt::Key_3:                     ctrl = "3"; break;
    case Qt::Key_4:                     ctrl = "4"; break;
    case Qt::Key_5:                     ctrl = "5"; break;
    case Qt::Key_6:                     ctrl = "6"; break;
    case Qt::Key_7:                     ctrl = "7"; break;
    case Qt::Key_8:                     ctrl = "8"; break;
    case Qt::Key_9:                     ctrl = "9"; break;
    case Qt::Key_Exclam:                ctrl = "!"; break;
    case Qt::Key_QuoteDbl:              ctrl = "\""; break;
    case Qt::Key_NumberSign:            ctrl = "#"; break;
    case Qt::Key_Dollar:                ctrl = "$"; break;
    case Qt::Key_Percent:               ctrl = "%"; break;
    case Qt::Key_Ampersand:             ctrl = "&"; break;
    case Qt::Key_Apostrophe:            ctrl = "'"; break;
    case Qt::Key_ParenLeft:             ctrl = "("; break;
    case Qt::Key_ParenRight:            ctrl = ")"; break;
    case Qt::Key_Asterisk:              ctrl = "*"; break;
    case Qt::Key_Plus:                  ctrl = "+"; break;
    case Qt::Key_Comma:                 ctrl = ","; break;
    case Qt::Key_Minus:                 ctrl = "-"; break;
    case Qt::Key_Period:                ctrl = "."; break;
    case Qt::Key_Slash:                 ctrl = "/"; break;
    case Qt::Key_Colon:                 ctrl = ":"; break;
    case Qt::Key_Semicolon:             ctrl = ";"; break;
    case Qt::Key_Less:                  ctrl = "<"; break;
    case Qt::Key_Equal:                 ctrl = "="; break;
    case Qt::Key_Greater:               ctrl = ">"; break;
    case Qt::Key_Question:              ctrl = "?"; break;
    case Qt::Key_At:                    ctrl = "@"; break;
    case Qt::Key_BracketLeft:           ctrl = "["; break;
    case Qt::Key_Backslash:             ctrl = "\\"; break;
    case Qt::Key_BracketRight:          ctrl = "]"; break;
    case Qt::Key_AsciiCircum:           ctrl = "^"; break;
    case Qt::Key_Underscore:            ctrl = "_"; break;
    case Qt::Key_QuoteLeft:             ctrl = "`"; break;
    case Qt::Key_BraceLeft:             ctrl = "{"; break;
    case Qt::Key_Bar:                   ctrl = "|"; break;
    case Qt::Key_BraceRight:            ctrl = "}"; break;
    case Qt::Key_AsciiTilde:            ctrl = "~"; break;
    case Qt::Key_Escape:                name = "Escape"; break;
    case Qt::Key_Tab:                   name = "Tab"; break;
    case Qt::Key_Backtab:               name = "Backtab"; break;
    case Qt::Key_Backspace:             name = "Backspace"; break;
    case Qt::Key_Return:                name = "Return"; break;
    case Qt::Key_Enter:                 name = "Enter"; break;
    case Qt::Key_Insert:                name = "Insert"; break;
    case Qt::Key_Delete:                name = "Delete"; break;
    case Qt::Key_Pause:                 name = "Pause"; break;
    case Qt::Key_Print:                 name = "Print"; break;
    case Qt::Key_SysReq:                name = "SysReq"; break;
    case Qt::Key_Clear:                 name = "Clear"; break;
    case Qt::Key_Home:                  name = "Home"; break;
    case Qt::Key_End:                   name = "End"; break;
    case Qt::Key_Left:                  name = "Left"; break;
    case Qt::Key_Up:                    name = "Up"; break;
    case Qt::Key_Right:                 name = "Right"; break;
    case Qt::Key_Down:                  name = "Down"; break;
    case Qt::Key_PageUp:                name = "PageUp"; break;
    case Qt::Key_PageDown:              name = "PageDown"; break;
    case Qt::Key_Shift:                 name = ""; break;
    case Qt::Key_Control:               name = ""; break;
    case Qt::Key_Meta:                  name = ""; break;
    case Qt::Key_Alt:                   name = ""; break;
    case Qt::Key_AltGr:                 name = "AltGr"; break;
    case Qt::Key_CapsLock:              name = "CapsLock"; break;
    case Qt::Key_NumLock:               name = "NumLock"; break;
    case Qt::Key_ScrollLock:            name = "ScrollLock"; break;
    case Qt::Key_F1:                    name = "F1"; break;
    case Qt::Key_F2:                    name = "F2"; break;
    case Qt::Key_F3:                    name = "F3"; break;
    case Qt::Key_F4:                    name = "F4"; break;
    case Qt::Key_F5:                    name = "F5"; break;
    case Qt::Key_F6:                    name = "F6"; break;
    case Qt::Key_F7:                    name = "F7"; break;
    case Qt::Key_F8:                    name = "F8"; break;
    case Qt::Key_F9:                    name = "F9"; break;
    case Qt::Key_F10:                   name = "F10"; break;
    case Qt::Key_F11:                   name = "F11"; break;
    case Qt::Key_F12:                   name = "F12"; break;
    case Qt::Key_F13:                   name = "F13"; break;
    case Qt::Key_F14:                   name = "F14"; break;
    case Qt::Key_F15:                   name = "F15"; break;
    case Qt::Key_F16:                   name = "F16"; break;
    case Qt::Key_F17:                   name = "F17"; break;
    case Qt::Key_F18:                   name = "F18"; break;
    case Qt::Key_F19:                   name = "F19"; break;
    case Qt::Key_F20:                   name = "F20"; break;
    case Qt::Key_F21:                   name = "F21"; break;
    case Qt::Key_F22:                   name = "F22"; break;
    case Qt::Key_F23:                   name = "F23"; break;
    case Qt::Key_F24:                   name = "F24"; break;
    case Qt::Key_F25:                   name = "F25"; break;
    case Qt::Key_F26:                   name = "F26"; break;
    case Qt::Key_F27:                   name = "F27"; break;
    case Qt::Key_F28:                   name = "F28"; break;
    case Qt::Key_F29:                   name = "F29"; break;
    case Qt::Key_F30:                   name = "F30"; break;
    case Qt::Key_F31:                   name = "F31"; break;
    case Qt::Key_F32:                   name = "F32"; break;
    case Qt::Key_F33:                   name = "F33"; break;
    case Qt::Key_F34:                   name = "F34"; break;
    case Qt::Key_F35:                   name = "F35"; break;
    case Qt::Key_Menu:                  name = "Menu"; break;
    case Qt::Key_Help:                  name = "Help"; break;
    case Qt::Key_Back:                  name = "Back"; break;
    case Qt::Key_Forward:               name = "Forward"; break;
    case Qt::Key_Stop:                  name = "Stop"; break;
    case Qt::Key_Refresh:               name = "Refresh"; break;
    case Qt::Key_VolumeDown:            name = "VolumeDown"; break;
    case Qt::Key_VolumeMute:            name = "VolumeMute"; break;
    case Qt::Key_VolumeUp:              name = "VolumeUp"; break;
    case Qt::Key_BassBoost:             name = "BassBoost"; break;
    case Qt::Key_BassUp:                name = "BassUp"; break;
    case Qt::Key_BassDown:              name = "BassDown"; break;
    case Qt::Key_TrebleUp:              name = "TrebleUp"; break;
    case Qt::Key_TrebleDown:            name = "TrebleDown"; break;
    case Qt::Key_MediaPlay:             name = "MediaPlay"; break;
    case Qt::Key_MediaStop:             name = "MediaStop"; break;
    case Qt::Key_MediaPrevious:         name = "MediaPrevious"; break;
    case Qt::Key_MediaNext:             name = "MediaNext"; break;
    case Qt::Key_MediaRecord:           name = "MediaRecord"; break;
    case Qt::Key_HomePage:              name = "HomePage"; break;
    case Qt::Key_Favorites:             name = "Favorites"; break;
    case Qt::Key_Search:                name = "Search"; break;
    case Qt::Key_Standby:               name = "Standby"; break;
    case Qt::Key_OpenUrl:               name = "OpenUrl"; break;
    case Qt::Key_LaunchMail:            name = "LaunchMail"; break;
    case Qt::Key_LaunchMedia:           name = "LaunchMedia"; break;
    case Qt::Key_Launch0:               name = "Launch0"; break;
    case Qt::Key_Launch1:               name = "Launch1"; break;
    case Qt::Key_Launch2:               name = "Launch2"; break;
    case Qt::Key_Launch3:               name = "Launch3"; break;
    case Qt::Key_Launch4:               name = "Launch4"; break;
    case Qt::Key_Launch5:               name = "Launch5"; break;
    case Qt::Key_Launch6:               name = "Launch6"; break;
    case Qt::Key_Launch7:               name = "Launch7"; break;
    case Qt::Key_Launch8:               name = "Launch8"; break;
    case Qt::Key_Launch9:               name = "Launch9"; break;
    case Qt::Key_LaunchA:               name = "LaunchA"; break;
    case Qt::Key_LaunchB:               name = "LaunchB"; break;
    case Qt::Key_LaunchC:               name = "LaunchC"; break;
    case Qt::Key_LaunchD:               name = "LaunchD"; break;
    case Qt::Key_LaunchE:               name = "LaunchE"; break;
    case Qt::Key_LaunchF:               name = "LaunchF"; break;
    case Qt::Key_MonBrightnessUp:       name = "MonBrightnessUp"; break;
    case Qt::Key_MonBrightnessDown:     name = "MonBrightnessDown"; break;
    case Qt::Key_KeyboardLightOnOff:    name = "KeyboardLightOnOff"; break;
    case Qt::Key_KeyboardBrightnessUp:  name = "KeyboardBrightnessUp"; break;
    case Qt::Key_KeyboardBrightnessDown:name = "KeyboardBrightnessDown"; break;
    case Qt::Key_PowerOff:              name = "PowerOff"; break;
    case Qt::Key_WakeUp:                name = "WakeUp"; break;
    case Qt::Key_Eject:                 name = "Eject"; break;
    case Qt::Key_ScreenSaver:           name = "ScreenSaver"; break;
    case Qt::Key_WWW:                   name = "WWW"; break;
    case Qt::Key_Memo:                  name = "Memo"; break;
    case Qt::Key_LightBulb:             name = "LightBulb"; break;
    case Qt::Key_Shop:                  name = "Shop"; break;
    case Qt::Key_History:               name = "History"; break;
    case Qt::Key_AddFavorite:           name = "AddFavorite"; break;
    case Qt::Key_HotLinks:              name = "HotLinks"; break;
    case Qt::Key_BrightnessAdjust:      name = "BrightnessAdjust"; break;
    case Qt::Key_Finance:               name = "Finance"; break;
    case Qt::Key_Community:             name = "Community"; break;
    case Qt::Key_AudioRewind:           name = "AudioRewind"; break;
    case Qt::Key_BackForward:           name = "BackForward"; break;
    case Qt::Key_ApplicationLeft:       name = "ApplicationLeft"; break;
    case Qt::Key_ApplicationRight:      name = "ApplicationRight"; break;
    case Qt::Key_Book:                  name = "Book"; break;
    case Qt::Key_CD:                    name = "CD"; break;
    case Qt::Key_Calculator:            name = "Calculator"; break;
    case Qt::Key_ToDoList:              name = "ToDoList"; break;
    case Qt::Key_ClearGrab:             name = "ClearGrab"; break;
    case Qt::Key_Close:                 name = "Close"; break;
    case Qt::Key_Copy:                  name = "Copy"; break;
    case Qt::Key_Cut:                   name = "Cut"; break;
    case Qt::Key_Display:               name = "Display"; break;
    case Qt::Key_DOS:                   name = "DOS"; break;
    case Qt::Key_Documents:             name = "Documents"; break;
    case Qt::Key_Excel:                 name = "Excel"; break;
    case Qt::Key_Explorer:              name = "Explorer"; break;
    case Qt::Key_Game:                  name = "Game"; break;
    case Qt::Key_Go:                    name = "Go"; break;
    case Qt::Key_iTouch:                name = "iTouch"; break;
    case Qt::Key_LogOff:                name = "LogOff"; break;
    case Qt::Key_Market:                name = "Market"; break;
    case Qt::Key_Meeting:               name = "Meeting"; break;
    case Qt::Key_MenuKB:                name = "MenuKB"; break;
    case Qt::Key_MenuPB:                name = "MenuPB"; break;
    case Qt::Key_MySites:               name = "MySites"; break;
    case Qt::Key_News:                  name = "News"; break;
    case Qt::Key_OfficeHome:            name = "OfficeHome"; break;
    case Qt::Key_Option:                name = "Option"; break;
    case Qt::Key_Paste:                 name = "Paste"; break;
    case Qt::Key_Phone:                 name = "Phone"; break;
    case Qt::Key_Calendar:              name = "Calendar"; break;
    case Qt::Key_Reply:                 name = "Reply"; break;
    case Qt::Key_Reload:                name = "Reload"; break;
    case Qt::Key_RotateWindows:         name = "RotateWindows"; break;
    case Qt::Key_RotationPB:            name = "RotationPB"; break;
    case Qt::Key_RotationKB:            name = "RotationKB"; break;
    case Qt::Key_Save:                  name = "Save"; break;
    case Qt::Key_Send:                  name = "Send"; break;
    case Qt::Key_Spell:                 name = "Spell"; break;
    case Qt::Key_SplitScreen:           name = "SplitScreen"; break;
    case Qt::Key_Support:               name = "Support"; break;
    case Qt::Key_TaskPane:              name = "TaskPane"; break;
    case Qt::Key_Terminal:              name = "Terminal"; break;
    case Qt::Key_Tools:                 name = "Tools"; break;
    case Qt::Key_Travel:                name = "Travel"; break;
    case Qt::Key_Video:                 name = "Video"; break;
    case Qt::Key_Word:                  name = "Word"; break;
    case Qt::Key_Xfer:                  name = "Xfer"; break;
    case Qt::Key_ZoomIn:                name = "ZoomIn"; break;
    case Qt::Key_ZoomOut:               name = "ZoomOut"; break;
    case Qt::Key_Away:                  name = "Away"; break;
    case Qt::Key_Messenger:             name = "Messenger"; break;
    case Qt::Key_WebCam:                name = "WebCam"; break;
    case Qt::Key_MailForward:           name = "MailForward"; break;
    case Qt::Key_Pictures:              name = "Pictures"; break;
    case Qt::Key_Music:                 name = "Music"; break;
    case Qt::Key_Battery:               name = "Battery"; break;
    case Qt::Key_Bluetooth:             name = "Bluetooth"; break;
    case Qt::Key_WLAN:                  name = "WLAN"; break;
    case Qt::Key_UWB:                   name = "UWB"; break;
    case Qt::Key_AudioForward:          name = "AudioForward"; break;
    case Qt::Key_AudioRepeat:           name = "AudioRepeat"; break;
    case Qt::Key_AudioRandomPlay:       name = "AudioRandomPlay"; break;
    case Qt::Key_Subtitle:              name = "Subtitle"; break;
    case Qt::Key_AudioCycleTrack:       name = "AudioCycleTrack"; break;
    case Qt::Key_Time:                  name = "Time"; break;
    case Qt::Key_Hibernate:             name = "Hibernate"; break;
    case Qt::Key_View:                  name = "View"; break;
    case Qt::Key_TopMenu:               name = "TopMenu"; break;
    case Qt::Key_PowerDown:             name = "PowerDown"; break;
    case Qt::Key_Suspend:               name = "Suspend"; break;
    case Qt::Key_ContrastAdjust:        name = "ContrastAdjust"; break;
    case Qt::Key_MediaLast:             name = "MediaLast"; break;
    case Qt::Key_Call:                  name = "Call"; break;
    case Qt::Key_Context1:              name = "Context1"; break;
    case Qt::Key_Context2:              name = "Context2"; break;
    case Qt::Key_Context3:              name = "Context3"; break;
    case Qt::Key_Context4:              name = "Context4"; break;
    case Qt::Key_Flip:                  name = "Flip"; break;
    case Qt::Key_Hangup:                name = "Hangup"; break;
    case Qt::Key_No:                    name = "No"; break;
    case Qt::Key_Select:                name = "Select"; break;
    case Qt::Key_Yes:                   name = "Yes"; break;
    case Qt::Key_Execute:               name = "Execute"; break;
    case Qt::Key_Printer:               name = "Printer"; break;
    case Qt::Key_Play:                  name = "Play"; break;
    case Qt::Key_Sleep:                 name = "Sleep"; break;
    case Qt::Key_Zoom:                  name = "Zoom"; break;
    case Qt::Key_Cancel:                name = "Cancel"; break;
    }

    // Add modifiers to the name if we have them
    Qt::KeyboardModifiers modifiers = event->modifiers();
    Qt::KeyboardModifiers mod = 0;
    if (event->type() == QEvent::KeyPress)
    {
        mod = modifiers;
    }
    else if (event->type() == QEvent::KeyRelease)
    {
        mod = (Qt::KeyboardModifiers)(modifiers ^ prevModifiers);
    }
    if (ctrl == "")
    {
        if (name.length() != 1 && (mod & Qt::ShiftModifier))
            name = "Shift-" + name;
        ctrl = name;
    }
    else
    {
        Qt::KeyboardModifiers shift = mod & Qt::ShiftModifier;
        if (shift && shift != mod)
            name = ctrl = "Shift-" + ctrl;
    }
    if (mod & Qt::ControlModifier)
        name = ctrl = "Control-" + ctrl;
    if (mod & Qt::AltModifier)
        name = ctrl = "Alt-" + ctrl;
    if (mod & Qt::MetaModifier)
        name = ctrl = "Meta-" + ctrl;

    if (event->type() == QEvent::KeyRelease)
        name = "~" + name;

    // Make sure name does not end with "-" (modifiers only)
    size_t len = name.length();
    if (len > 1 && name[len - 1] == '-')
        name = name.substr(0, len - 1);

    return name;
}


void Widget::keyPressEvent(QKeyEvent *event)
// ----------------------------------------------------------------------------
//   A key is pressed
// ----------------------------------------------------------------------------
{
    handleKeyEvent(event, true);
}


void Widget::keyReleaseEvent(QKeyEvent *event)
// ----------------------------------------------------------------------------
//   A key is released
// ----------------------------------------------------------------------------
{
    handleKeyEvent(event, false);
}


void Widget::handleKeyEvent(QKeyEvent *event, bool keypress)
// ----------------------------------------------------------------------------
//   Save info about keypress/keyreleased event, and forward.
// ----------------------------------------------------------------------------
{
#ifdef CFG_TIMED_FULLSCREEN
    emit userActivity();
#endif

    TaoSave saveCurrent(current, this);
    EventSave save(this->w_event, event);

    // Record event info
    uint prevKeyboardModifiers = keyboardModifiers;
    keyboardModifiers = event->modifiers();
    XL::Save<bool> saveKeyPressed(keyPressed, keypress);
    XL::Save<text> saveKeyName(keyName, toKeyName(event));
    XL::Save<text> saveKeyText(keyText, +event->text());
    XL::Save<text> saveKeyEvent(keyEventName,
                                toKeyEventName(event,
                                               prevKeyboardModifiers,
                                               prevKeyPressText));
    // Remember keypress event->text() until next event
    prevKeyPressText = keypress ? keyText : "";

    // Forward event down the regular event chain
    if (forwardEvent(event))
        return;

    bool handled = false;
    if (keypress)
    {
        // Check if one of the activities handled the key
        Activity *next;
        for (Activity *a = activities; a; a = next)
        {
            Activity * n = a->next;
            next = a->Key(keyEventName);
            handled |= next != n;
        }
    }

    // If the key was not handled by any activity, forward to document
    if (!handled && xlProgram)
        XL::XLCall ("key", keyEventName) (xlProgram->scope);
    if (keypress)
        updateGL();
}


void Widget::mousePressEvent(QMouseEvent *event)
// ----------------------------------------------------------------------------
//   Mouse button click
// ----------------------------------------------------------------------------
{
#ifdef CFG_TIMED_FULLSCREEN
    emit userActivity();
#endif

    if (cursor().shape() == Qt::OpenHandCursor)
        return startPanning(event);

    TaoSave saveCurrent(current, this);
    EventSave save(this->w_event, event);
    keyboardModifiers = event->modifiers();

    QMenu * contextMenu = NULL;
    uint    button      = (uint) event->button();
    int     x           = event->x();
    int     y           = event->y();

    // Save location
    lastMouseX = x;
    lastMouseY = y;
    lastMouseButtons = event->buttons();

    // Create a selection if left click and nothing going on right now
    if (button == Qt::LeftButton && selectionRectangleEnabled)
        new Selection(this);
    else if ((event->modifiers() & Qt::ShiftModifier) == 0)
        if (uint id = Identify("Cl", this).ObjectAtPoint(x, height() - y))
            shapeAction("click", id, x, y);

    // Send the click to all activities
    for (Activity *a = activities; a; a = a->Click(button, 1, x, y)) ;

    // Check if some widget is selected and wants that event
    if (forwardEvent(event))
        return;

    // Otherwise create our local contextual menu
    if (button ==  Qt::RightButton)
    {
        switch (event->modifiers())
        {
        default :
        case Qt::NoModifier :
            contextMenu = taoWindow()->findChild<QMenu*>(CONTEXT_MENU);
            break;
        case Qt::ShiftModifier :
            contextMenu = taoWindow()->findChild<QMenu*>(SHIFT_CONTEXT_MENU);
            break;
        case Qt::ControlModifier :
            contextMenu = taoWindow()->findChild<QMenu*>(CONTROL_CONTEXT_MENU);
            break;
        case Qt::AltModifier :
            contextMenu = taoWindow()->findChild<QMenu*>(ALT_CONTEXT_MENU);
            break;
        case Qt::MetaModifier :
            contextMenu = taoWindow()->findChild<QMenu*>(META_CONTEXT_MENU);
            break;
        }

        if (contextMenu)
            contextMenu->exec(event->globalPos());
    }
}


void Widget::mouseReleaseEvent(QMouseEvent *event)
// ----------------------------------------------------------------------------
//   Mouse button is released
// ----------------------------------------------------------------------------
{
    if (cursor().shape() == Qt::ClosedHandCursor)
        return endPanning(event);

    TaoSave saveCurrent(current, this);
    EventSave save(this->w_event, event);
    keyboardModifiers = event->modifiers();

    uint button = (uint) event->button();
    int x = event->x();
    int y = event->y();

    // Save location
    lastMouseX = x;
    lastMouseY = y;
    lastMouseButtons = event->buttons();

    // Check if there is an activity that deals with it
    for (Activity *a = activities; a; a = a->Click(button, 0, x, y)) ;

    // Pass the event down the event chain
    forwardEvent(event);
}


void Widget::mouseMoveEvent(QMouseEvent *event)
// ----------------------------------------------------------------------------
//    Mouse move
// ----------------------------------------------------------------------------
{
#ifdef CFG_TIMED_FULLSCREEN
    emit userActivity();
#endif

    if (cursor().shape() == Qt::BlankCursor)
    {
        setCursor(savedCursorShape);
        LayoutState::qevent_ids refreshEvents = space->RefreshEvents();
        bool mouseTracking = (refreshEvents.count(QEvent::MouseMove) != 0);
        if (!mouseTracking)
            setMouseTracking(false);
        if (bAutoHideCursor)
            QTimer::singleShot(2000, this, SLOT(hideCursor()));
    }

    if (cursor().shape() == Qt::ClosedHandCursor)
        return doPanning(event);

    TaoSave saveCurrent(current, this);
    EventSave save(this->w_event, event);
    keyboardModifiers = event->modifiers();
    int buttons = event->buttons();
    bool active = buttons != Qt::NoButton;
    int x = event->x();
    int y = event->y();

    // Save location
    lastMouseX = x;
    lastMouseY = y;
    lastMouseButtons = buttons;

    // Check if there is an activity that deals with it
    for (Activity *a = activities; a; a = a->MouseMove(x, y, active)) ;

    // Pass the event down the event chain
    forwardEvent(event);
}


void Widget::mouseDoubleClickEvent(QMouseEvent *event)
// ----------------------------------------------------------------------------
//   Mouse double click
// ----------------------------------------------------------------------------
{
    TaoSave saveCurrent(current, this);
    EventSave save(this->w_event, event);
    keyboardModifiers = event->modifiers();

    // Create a selection if left click and nothing going on right now
    uint    button      = (uint) event->button();
    int     x           = event->x();
    int     y           = event->y();
    if (selectionRectangleEnabled)
        if (button == Qt::LeftButton && (!activities || !activities->next))
            new Selection(this);

    // Save location
    lastMouseX = x;
    lastMouseY = y;
    lastMouseButtons = button;

    // Send the click to all activities
    for (Activity *a = activities; a; a = a->Click(button, 2, x, y)) ;

    forwardEvent(event);
}


void Widget::wheelEvent(QWheelEvent *event)
// ----------------------------------------------------------------------------
//   Mouse wheel: zoom in/out
// ----------------------------------------------------------------------------
{
    TaoSave saveCurrent(current, this);
    EventSave save(this->w_event, event);
    keyboardModifiers = event->modifiers();
    int     x           = event->x();
    int     y           = event->y();

    // Save location
    lastMouseX = x;
    lastMouseY = y;

    if (forwardEvent(event))
        return;

    // Propagate the wheel event
    if (!xlProgram) return;
    int d = event->delta();
    Qt::Orientation orientation = event->orientation();
    longlong dx = orientation == Qt::Horizontal ? d : 0;
    longlong dy = orientation == Qt::Vertical   ? d : 0;
    XL::XLCall("wheel_event", dx, dy)(xlProgram->scope);
    do
    {
        TaoSave saveCurrent(current, NULL);
        updateGL();
    } while (0);
}


#ifdef MACOSX_DISPLAYLINK
void Widget::sendTimerEvent()
// ----------------------------------------------------------------------------
//    Slot that sends a timer event to self
// ----------------------------------------------------------------------------
{
    QTimerEvent e(0);
    timerEvent(&e);
}
#endif


bool Widget::event(QEvent *event)
// ----------------------------------------------------------------------------
//    Process event
// ----------------------------------------------------------------------------
{
    int type = event->type();

#ifdef MACOSX_DISPLAYLINK
    if (type == DisplayLink)
    {
        displayLinkMutex.lock();
        pendingDisplayLinkEvent = false;
        displayLinkMutex.unlock();
        if (holdOff)
        {
            holdOff = false;
            return true;
        }
        // Do not call timerEvent() directly. Doing so may cause some events
        // to remain stalled in the main event loop for a long time.
        // This 1ms timer is the solution I found.
        // See commit comment for details.
        QTimer::singleShot(1, this, SLOT(sendTimerEvent()));
        return true;
    }
    else if (type == QEvent::MouseMove ||
             type == QEvent::KeyPress  ||
             type == QEvent::KeyRelease)
    {
        // Skip next frame, to keep some processing power for subsequent
        // user events. This effectively gives a higher priority to them
        // than to timer events.
        holdOff = true;
    }
#endif

    // Process user events after the Mac code because DisplayLink is a user
    // event
    if (type >= QEvent::User)
    {
        // Unblock postEventOnceAPI for this event type
        pendingEvents.erase(type);

        refreshNow(event);
        return true;
    }

    return QGLWidget::event(event);
}


#ifdef MACOSX_DISPLAYLINK

static CVReturn displayLinkCallback(CVDisplayLinkRef displayLink,
                                    const CVTimeStamp *now,
                                    const CVTimeStamp *outputTime,
                                    CVOptionFlags flagsIn,
                                    CVOptionFlags *flagsOut,
                                    void *displayLinkContext)
// ----------------------------------------------------------------------------
//    MacOSX Core Video display link callback.
// ----------------------------------------------------------------------------
//    Using a display link is a way to implement precise timers, synchronized
//    with screen refresh.
//    See http://developer.apple.com/library/mac/#qa/qa1385/_index.html.
//    This method is called from a thread that is NOT the GUI thread, so we
//    can't do much here. We will just post an event to be processed ASAP by
//    the main thread, to reevaluate any part of the document, and then redraw
//    the scene as needed.
{
    Q_UNUSED(displayLink);
    Q_UNUSED(now);
    Q_UNUSED(outputTime);
    Q_UNUSED(flagsIn);
    Q_UNUSED(flagsOut);

    Widget *w = (Widget*)displayLinkContext;
    w->displayLinkEvent();

    return kCVReturnSuccess;
}


void Widget::displayLinkEvent()
// ----------------------------------------------------------------------------
//    Post a user event to the GUI thread (this is our OpenGL refresh timer)
// ----------------------------------------------------------------------------
{
    // Stereoscopy with quad buffers: scene refresh rate is half of screen
    // frequency
    if (stereoBuffersEnabled && (++stereoSkip % 2 == 0))
        return;

    // Post event to main thread (have no more than one event pending,
    // otherwise it means we can't keep up => dropped frame)
    displayLinkMutex.lock();
    bool pending = pendingDisplayLinkEvent;
    if (pending)
        droppedFrames++;
    pendingDisplayLinkEvent = true;
    displayLinkMutex.unlock();
    if (!pending)
        qApp->postEvent(this, new QEvent((QEvent::Type)DisplayLink));
}

static CGDirectDisplayID getCurrentDisplayID(const  QWidget *widget)
// ----------------------------------------------------------------------------
//    Return the display ID where the largest part of widget is
// ----------------------------------------------------------------------------
{
    CGDirectDisplayID id = kCGDirectMainDisplay;
    QDesktopWidget *dw = qApp->desktop();
    if (dw)
    {
        int index = dw->screenNumber(widget);
        if (index != -1)
        {
            QRect rect = dw->screenGeometry(index);
            QPoint c = rect.center();

            const int max = 64;   // Should be enough for any system
            CGDisplayErr st;
            CGDisplayCount count;
            CGDirectDisplayID displayIDs[max];
            CGPoint point;

            point.x = c.x();
            point.y = c.y();
            st = CGGetDisplaysWithPoint(point, max, displayIDs, &count);
            if (st == kCGErrorSuccess && count == 1)
                id = displayIDs[0];
        }
    }
    return id;
}

unsigned int Widget::droppedFramesLocked()
// ----------------------------------------------------------------------------
//    Access value of droppedFrames under lock
// ----------------------------------------------------------------------------
{
    unsigned int dropped;
    displayLinkMutex.lock();
    dropped = droppedFrames;
    displayLinkMutex.unlock();
    return dropped;
}

#endif


void Widget::startRefreshTimer(bool on)
// ----------------------------------------------------------------------------
//    Make sure refresh timer runs if it has to
// ----------------------------------------------------------------------------
{
    if (inOfflineRendering)
        return;

    if (!on)
    {
#ifdef MACOSX_DISPLAYLINK
        if (displayLink && displayLinkStarted)
        {
            CVDisplayLinkStop(displayLink);
            displayLinkStarted = false;
            pendingDisplayLinkEvent = false;
            QCoreApplication::removePostedEvents(this, DisplayLink);
        }
#else
        timer.stop();
#endif
        QCoreApplication::removePostedEvents(this, QEvent::Timer);
        return;
    }

    if (inError || !animated)
        return;

#ifdef MACOSX_DISPLAYLINK
    if (!displayLink)
    {
        // Create display link
        CVDisplayLinkCreateWithCGDisplay(getCurrentDisplayID(this),
                                         &displayLink);
        // Set render callback
        if (displayLink)
            CVDisplayLinkSetOutputCallback(displayLink, &displayLinkCallback,
                                           this);
    }
    if (displayLink && !displayLinkStarted)
    {
        CVDisplayLinkStart(displayLink);
        displayLinkStarted = true;
    }
#else
    assert(space || !"No space layout");
    double next = space->NextRefresh();
    if (next != DBL_MAX)
    {
        double now = trueCurrentTime();
        double remaining = next - now;
        if (remaining <= 0)
            remaining = 0.001;
        int ms = remaining * 1000;
        IFTRACE(layoutevents)
            std::cerr << "Starting refresh timer: " << ms << " ms (current "
            "time " << now << " next refresh " << next
                      << ")\n";
        if (timer.isActive())
            timer.stop();
        timer.start(ms, this);
    }
#endif
}


void Widget::timerEvent(QTimerEvent *event)
// ----------------------------------------------------------------------------
//    Timer expired
// ----------------------------------------------------------------------------
{
    TaoSave saveCurrent(current, this);
    EventSave save(this->w_event, event);

#ifdef MACOSX_DISPLAYLINK
    setCurrentTime();
    if (CurrentTime() < space->NextRefresh())
        return;
#else
    bool is_refresh_timer = (event->timerId() == timer.timerId());
    if (is_refresh_timer)
    {
        timer.stop();
        setCurrentTime();
        IFTRACE(layoutevents)
            std::cerr << "Refresh timer expired, now = " << trueCurrentTime()
                      << "\n";

        if (CurrentTime() < space->NextRefresh())
        {
            // Timer expired early. Unfortunately, this occurs sometime.
            return startRefreshTimer();
        }
    }
#endif
    refreshNow(event);
}


static double toSecsSinceEpoch(const QDateTime &d)
// ----------------------------------------------------------------------------
//    Convert QDateTime to the number of seconds passed since the epoch
// ----------------------------------------------------------------------------
{
    return (double)d.toMSecsSinceEpoch() / 1000;
}


static double nextDay(const QDateTime &d)
// ----------------------------------------------------------------------------
//    Return next day
// ----------------------------------------------------------------------------
{
    QDateTime next(d);
    next.setTime(QTime(0, 0, 0, 0));
    next = next.addDays(1);
    return toSecsSinceEpoch(next);
}


double Widget::CurrentTime()
// ----------------------------------------------------------------------------
//    Cached current time (refreshed on timer event)
// ----------------------------------------------------------------------------
{
    if (currentTime == DBL_MAX)
        setCurrentTime();
    return currentTime;
}


double Widget::trueCurrentTime()
// ----------------------------------------------------------------------------
//    Query and return the current time (seconds since epoch, ~ms resolution)
// ----------------------------------------------------------------------------
{
    QDateTime dt = QDateTime::currentDateTime();
    return toSecsSinceEpoch(dt);
}


void Widget::setCurrentTime()
// ----------------------------------------------------------------------------
//    Update cached current time value
// ----------------------------------------------------------------------------
{
    if (inOfflineRendering)
        return;
    currentTime = trueCurrentTime();
}



void Widget::startPanning(QMouseEvent *event)
// ----------------------------------------------------------------------------
//    Enter view panning mode
// ----------------------------------------------------------------------------
{
    setCursor(savedCursorShape = Qt::ClosedHandCursor);
    panX = event->x();
    panY = event->y();
}


void Widget::doPanning(QMouseEvent *event)
// ----------------------------------------------------------------------------
//    Move view to follow mouse (panning mode)
// ----------------------------------------------------------------------------
{
    int x, y, dx, dy;

    x = event->x();
    y = event->y();
    dx = x - panX;
    dy = y - panY;

    cameraPosition.x -= dx/zoom;
    cameraPosition.y += dy/zoom;
    cameraTarget.x -= dx/zoom;
    cameraTarget.y += dy/zoom;

    panX = x;
    panY = y;

    updateGL();
}


void Widget::endPanning(QMouseEvent *)
// ----------------------------------------------------------------------------
//    Leave view panning mode
// ----------------------------------------------------------------------------
{
    setCursor(savedCursorShape = Qt::OpenHandCursor);
}



// ============================================================================
//
//    XL program management
//
// ============================================================================

bool Widget::sourceChanged()
// ----------------------------------------------------------------------------
//   Return 'true' if the source window was modified
// ----------------------------------------------------------------------------
{
    // Protect against derivatives of #933 or #929 (e.g. QT Bug 2616)
    if (isBeingDestroyed())
        return true;

#ifndef CFG_NOSRCEDIT
    if (taoWindow()->srcEdit->document()->isModified())
        return true;
#endif
    return false;
}


void Widget::normalizeProgram()
// ----------------------------------------------------------------------------
//   Put the program in normalized form
// ----------------------------------------------------------------------------
{
    if (sourceChanged())
        return;
    if (Tree *prog = xlProgram->tree)
    {
        Renormalize renorm(this);
        xlProgram->tree = prog->Do(renorm);
    }
}


void Widget::updateProgram(XL::SourceFile *source)
// ----------------------------------------------------------------------------
//   Change the XL program, clean up stuff along the way
// ----------------------------------------------------------------------------
{
    if (xlProgram != NULL && sourceChanged())
        return;
    record(justify, "Widget %p clears layout %p for new program %t",
           this, space, (Tree *) xlProgram);
    space->Clear();
    dfltRefresh = optimalDefaultRefresh();
    clearCol.setRgb(255, 255, 255, 255);
    xlProgram = source;
    transitionTree = NULL;
    setObjectName(QString("Widget:").append(+xlProgram->name));
    normalizeProgram();
    refreshProgram(); // REVISIT not needed?
    clearErrors();
}


int Widget::loadFile(text name)
// ----------------------------------------------------------------------------
//   Load regular source file in current widget
// ----------------------------------------------------------------------------
{
    purgeTaoInfo();
    TaoSave saveCurrent(current, this);
    int st = XL::MAIN->LoadFile(name);
#if !defined(CFG_NO_DOC_SIGNATURE)
    checkDocumentSigned();
#endif
    return st;
}


void Widget::loadContextFiles(XL::source_names &files XL_UNUSED)
// ----------------------------------------------------------------------------
//   Load context files in current context
// ----------------------------------------------------------------------------
{
    TaoSave saveCurrent(current, this);
    XL_ASSERT(!"No longer implemented, repair it");
    xlProgram = NULL;    // Destroyed by LoadContextFiles
    contextFilesLoaded = true;
}


void Widget::reloadProgram(Tree *newProg)
// ----------------------------------------------------------------------------
//   Set the program to reload
// ----------------------------------------------------------------------------
{
    if (sourceChanged())
        return;

    if (!newProg)
    {
        // We want to force a clone so that we recompile everything
        normalizeProgram();
    }

    // Check if we can simply change some parameters in the tree
    else if (Tree *prog = xlProgram->tree)
    {
        ApplyChanges changes(newProg);
        if (!prog->Do(changes))
        {
            // Need a big hammer, i.e. reload the complete program
            xlProgram->tree = newProg;
        }
    }

    // Now update the window
    updateProgramSource();
    refreshNow();
    clearErrors();
}


void Widget::updateProgramSource(bool notWhenHidden)
// ----------------------------------------------------------------------------
//   Update the contents of the program source window
// ----------------------------------------------------------------------------
{
#ifndef CFG_NOSRCEDIT
    Window *window = taoWindow();
    if ((window->src->isHidden() && notWhenHidden) ||
        !xlProgram || sourceChanged())
        return;
    window->srcEdit->render(xlProgram->tree, &selectionTrees);
#else
    Q_UNUSED(notWhenHidden);
#endif
}


void Widget::listNames(XLSourceEdit *editor)
// ----------------------------------------------------------------------------
//   Return list of names in current program
// ----------------------------------------------------------------------------
{
#ifndef CFG_NOSRCEDIT
    XL::Context         context(xlProgram->scope);
    XL::RewriteList     rewrites;
    std::set<text>      names, infixen, prefixen, postfixen;

    context.ListNames("", rewrites, true, true);
    for (XL::Rewrite *rw : rewrites)
    {
        Tree *defined = XL::RewriteDefined(rw->left);
        switch(defined->Kind())
        {
        case XL::NAME:
        {
            Name *name = (Name *) defined;
            names.insert(name->value);
            break;
        }
        case XL::INFIX:
        {
            Infix *infix = (Infix *) defined;
            if (Name *name = infix->left->AsName())
                infixen.insert(name->value);
            break;
        }
        case XL::PREFIX:
        {
            Prefix *prefix = (Prefix *) defined;
            if (Name *name = prefix->left->AsName())
                prefixen.insert(name->value);
            break;
        }
        case XL::POSTFIX:
        {
            Postfix *infix = (Postfix *) defined;
            if (Name *name = infix->left->AsName())
                postfixen.insert(name->value);
            break;
        }
        default:
        {
            break;
        }
        }
    }

    infixen.erase("\n");
    prefixen.insert("if");
    prefixen.insert("while");
    prefixen.insert("until");
    prefixen.insert("for");
    editor->highlightNames(0, infixen);
    editor->highlightNames(1, postfixen);
    editor->highlightNames(2, prefixen);
    editor->highlightNames(3, names);
#else
    Q_UNUSED(editor);
#endif
}


#ifndef CFG_NO_DOC_SIGNATURE
#ifndef TAO_PLAYER
static bool isUserFile(QString path)
// ----------------------------------------------------------------------------
//   True if file is not a Tao file (tao.xl, etc.) or module
// ----------------------------------------------------------------------------
{
    path = QFileInfo(path).absoluteFilePath();
    ModuleManager *mmgr = ModuleManager::moduleManager();
    QList<QDir> exclude;
    exclude << mmgr->userModuleDir() << mmgr->systemModuleDir()
            << QDir(Application::applicationDirPath());
    foreach (QDir dir, exclude)
        if (path.startsWith(dir.absolutePath()))
            return false;
    return true;
}


static QString attachSigatureInfoAndSign(XL::SourceFile &sf)
// ----------------------------------------------------------------------------
//   Attach SignatureInfo object to tree and sign file
// ----------------------------------------------------------------------------
{
    SignatureInfo *si = sf.GetInfo<SignatureInfo>();
    if (!si)
    {
        si = new SignatureInfo(sf.name);
        sf.SetInfo<SignatureInfo>(si);
    }
    return si->signFileWithDocKey();
}


QString Widget::signDocument(text path)
// ----------------------------------------------------------------------------
//   Create .sig for path or all file in a document (except modules/Tao files)
// ----------------------------------------------------------------------------
{
    QString err;
    using namespace XL;
    if (path != "")
    {
        if (!excludedFromSignature.contains(+path))
            err = attachSigatureInfoAndSign(MAIN->files[path]);
    }
    else
    {
        source_files &files = MAIN->files;
        source_files::iterator it;
        for (it = files.begin(); it != files.end(); it++)
        {
            SourceFile &sf = (*it).second;
            IFTRACE(lic)
                std::cerr << "[SignatureInfo] " << sf.name
                          << (excludedFromSignature.contains(+sf.name)
                              ? " excluded\n" : " not excluded\n");
            if (excludedFromSignature.contains(+sf.name))
                continue;
            if (!isUserFile(+sf.name))
                continue;
            err = attachSigatureInfoAndSign(sf);
            if (!err.isEmpty())
                break;
        }

        // Generate the publisher information
        QDir    outDir(taoWindow()->currentProjectFolderPath());
        QString outFileName(outDir.filePath("publisher.info"));
        QFile   outFile(outFileName);
        if (outFile.open(QFile::WriteOnly))
        {
            QTextStream out(&outFile);
            out.setCodec("UTF-8");
            out << +Licenses::Name() << "\n"
                << +Licenses::Company() << "\n"
                << +Licenses::Mail() << "\n";
            outFile.close();

            // Sign the license file
            SignatureInfo si(+outFileName);
            si.signFileWithDocKey();
        }
        else
        {
            err = "publisher.info: "  + outFile.errorString();
        }

    }
    checkDocumentSigned();
    return err;
}
#endif // TAO_PLAYER


bool Widget::checkDocumentSigned()
// ----------------------------------------------------------------------------
//   Scan all files used by the document, check if they have a SignatureInfo
// ----------------------------------------------------------------------------
{
    bool sig = false;
    using namespace XL;

    // Checking the signature of the .XL and .DDD files
    source_files &files = MAIN->files;
    source_files::iterator it;
    for (it = files.begin(); it != files.end(); it++)
    {
        SourceFile &sf = (*it).second;
        if (!excludedFromSignature.contains(+sf.name))
        {
            SignatureInfo *si = sf.GetInfo<SignatureInfo>();
            if (!si || si->loadAndCheckSignature() != SignatureInfo::SI_VALID)
            {
                sig = false;
                IFTRACE(lic)
                    std::cerr << "Bad signature for " << sf.name << "\n";
                break;
            }
        }
        sig = true;
    }

    // Checking the signature of the publisher
    if (sig)
    {
        QDir          pubDir(taoWindow()->currentProjectFolderPath());
        QString       pubFileName(pubDir.filePath("publisher.info"));
        SignatureInfo si(+pubFileName);

        IFTRACE(lic)
            std::cerr << "Player publisher info is " << +pubFileName << "\n";

        if (si.loadAndCheckSignature() == SignatureInfo::SI_VALID)
        {
            QFile   pubFile(pubFileName);
            if (pubFile.open(QFile::ReadOnly))
            {
                QTextStream in(&pubFile);
                in.setCodec("UTF-8");
                QString name = in.readLine();
                QString company = in.readLine();
                QString mail = in.readLine();
                pubFile.close();

                sig = Licenses::PublisherMatches(name) ||
                    Licenses::PublisherMatches(company) ||
                    Licenses::PublisherMatches(mail);
            }
        }
    }

    IFTRACE2(lic, fileload)
        std::cerr << "Document is " << (sig ? "" : "not ") << "signed\n";

    return (isDocumentSigned = sig);
}


void Widget::excludeFromSignature(text path)
// ----------------------------------------------------------------------------
//    Exclude a given path from the signature
// ----------------------------------------------------------------------------
{
    QString qpath = +path;
    if (!excludedFromSignature.contains(qpath))
    {
        IFTRACE(lic)
            std::cerr << "[SignatureInfo] Excluding " << +qpath << "\n";
        excludedFromSignature << qpath;
    }
}
#endif // CFG_NO_DOC_SIGNATURE


QString Widget::documentPath()
// ----------------------------------------------------------------------------
//   Return the document path
// ----------------------------------------------------------------------------
{
    return taoWindow()->currentProjectFolderPath();
}


void Widget::addSearchPath(Scope *scope, text prefix, text path)
// ----------------------------------------------------------------------------
//   Add a search path for resources
// ----------------------------------------------------------------------------
{
    Context context(scope);
    context.SetPrefixPath(prefix, path);
}


text Widget::resolvePrefixedPath(Scope *scope, text filename)
// ----------------------------------------------------------------------------
//   Search in the given prefix path
// ----------------------------------------------------------------------------
{
    Context context(scope);
    filename = context.ResolvePrefixedPath(filename);
    return filename;
}


void Widget::refreshProgram()
// ----------------------------------------------------------------------------
//   Check if any of the source files we depend on changed
// ----------------------------------------------------------------------------
{
    if (toReload.isEmpty() || !xlProgram || xlProgram->readOnly)
        return;

    do
    {
        TaoSave saveCurrent(current, this);
        runPageExitHandlers();
        pageEntry = 0; // will run page entry handlers of displayed page
    } while (0);

    Repository *repo = repository();
    bool needBigHammer = false;
    bool needRefresh   = false;
    foreach (QString path, toReload)
    {
        text fname = +path;
        IFTRACE(filesync)
            std::cerr << "Must reload " << fname << "\n";

        Tree *replacement = NULL;
        if (repo)
        {
            replacement = repo->read(fname);
        }
        else
        {
            XL::Syntax syntax(XL::MAIN->syntax);
            XL::Positions &positions = XL::MAIN->positions;
            XL::Errors errors;
            XL::Parser parser(fname.c_str(), syntax, positions, errors);
            replacement = parser.Parse();
        }

        if (!replacement)
        {
            // Uh oh, file went away?
            needBigHammer = true;
            break;
        }
        else
        {
            XL_ASSERT(XL::MAIN->files.count(fname));
            XL::SourceFile &sf = XL::MAIN->files[fname];
            if (!sf.tree)
            {
                // Reached when the main document is initially empty,
                // then modified
                TaoSave saveCurrent(current, this);
                XL::MAIN->LoadFile(sf.name);
                needBigHammer = true;
                break;
            }

            // Make sure we normalize the replacement
            Renormalize renorm(this);
            replacement = replacement->Do(renorm);

            // Check if we can simply change some parameters in file
            ApplyChanges changes(replacement);
            if (!sf.tree->Do(changes))
            {
                needBigHammer = true;
                break;
            }
            else if (xlProgram && (fname == xlProgram->name))
            {
                updateProgramSource();
            }

            needRefresh = true;

        } // Replacement checked
    } // foreach file

    IFTRACE(filesync)
    {
        if (needBigHammer)
            std::cerr << "Need to reload everything.\n";
        else
            std::cerr << "Surgical replacement worked\n";
    }

    // Clear errors and indicate a reload
    clearErrors();

    // If we were not successful with simple changes, reload everything...
    if (needBigHammer)
    {
        TaoSave saveCurrent(current, this);
        purgeTaoInfo();
        import_set iset;
        import_set::iterator it;
        ScanImportedFiles(iset, false);
        for (it = iset.begin(); it != iset.end(); it++)
        {
            XL::SourceFile &sf = **it;
            XL::MAIN->LoadFile(sf.name);
        }
        updateProgramSource();
        needRefresh = true;
    }
    if (needRefresh)
    {
        if (xlProgram->tree)
        {
            PurgeInfo<ExecOnceInfo> purgeOnceInfo;
            xlProgram->tree->Do(purgeOnceInfo);
        }

        // If a file was modified, we need to refresh the screen
        transitionStartTime = 0;
        transitionDurationValue = 0;
        transitionTree = NULL;
        TaoSave saveCurrent(current, this);
        refreshNow();
    }
    toReload.clear();

    // Check if we have an info file generated by the editor
    checkEditorInstructionsFile();

#if !defined(CFG_NO_DOC_SIGNATURE)
    // Update signature status
    checkDocumentSigned();
#endif
}


void Widget::checkEditorInstructionsFile()
// ----------------------------------------------------------------------------
//   Check if the Tao editor left instructions for us
// ----------------------------------------------------------------------------
{
    QString instrsFile = +xlProgram->name + ".tao-instrs";
    QFileInfo instrsInfo(instrsFile);
    if (instrsInfo.exists())
    {
        QFile file(instrsInfo.absoluteFilePath());
        file.open(QIODevice::ReadOnly);
        QTextStream textStream(&file);
        textStream.setAutoDetectUnicode(true);
        text data = +textStream.readAll();
        file.close();
        file.remove();

        unsigned pnum = 0, pid = 0;
        if (sscanf(data.c_str(), "page %u id %u", &pnum, &pid) == 2)
        {
            if (pnum)
            {
                pnum--;
                if (pnum < pageNames.size())
                {
                    gotoPageName = pageNames[pnum];

                    QDir dir = instrsInfo.dir();
                    dir.mkdir(".thumbnails");

                    QFileInfo sourceInfo(+xlProgram->name);
                    QString base(sourceInfo.fileName());
                    QString pattern(".thumbnails/%1-%2.png");
                    QString preview = pattern.arg(base).arg(pid);
                    preview = dir.absoluteFilePath(preview);

                    savePreviewThread.setMaxSize(800, 600);
                    savePreviewThread.setInterval(533);
                    savePreviewThread.setPath(preview);

                    refresh(0);
                }
            }
        }
    }
}


void Widget::preloadSelectionCode()
// ----------------------------------------------------------------------------
//   Make sure that we compile the selection code before running
// ----------------------------------------------------------------------------
//   This is mostly to avoid a pause the first time the user clicks
{
    // Make sure we compile the selection the first time
    static bool first = true;
    if (first)
    {
        // REVISIT: Save the various xlcall (and related code)
        double x = 0;
        Scope *scope = xlProgram->scope;
        XL::XLCall("draw_selection", x,x,x,x).Analyze(scope);
        XL::XLCall("draw_widget_selection", x,x,x,x).Analyze(scope);
        XL::XLCall("draw_3D_selection", x,x,x,x,x,x).Analyze(scope);
        XL::XLCall("draw_handle", x, x, x).Analyze(scope);
        XL::XLCall("draw_control_point_handle", x, x, x).Analyze(scope);
        first = false;
    }
}


bool Widget::markChange(text reason)
// ----------------------------------------------------------------------------
//    Record that we're about to change program
// ----------------------------------------------------------------------------
{
    if (sourceChanged())
        return false;

    changeReason = reason;

    // Caller is allowed to modify the source code
    return true;
}


void Widget::finishChanges()
// ----------------------------------------------------------------------------
//    Check if program has changed and save+commit+update src view if needed
// ----------------------------------------------------------------------------
{
    if (changeReason == "")
        return;

    bool changed = false;

    if (xlProgram && xlProgram->tree)
    {
        import_set done;
        ScanImportedFiles(done, true);

        import_set::iterator f;
        for (f = done.begin(); f != done.end(); f++)
        {
            XL::SourceFile &sf = **f;
            if (&sf != xlProgram && sf.changed)
            {
                changed = true;
                writeIfChanged(sf);
            }
        }
    }

    if (changed)
    {
        Repository *repo = repository();
        if (repo && changeReason != "")
            repo->markChanged(changeReason);

        // Record change to repository
        saveAndCommit();

        // Now update the window
        updateProgramSource();
    }

    changeReason = "";
}


void Widget::selectStatements(Tree *tree)
// ----------------------------------------------------------------------------
//   Put all statements in the given selection in the next selection
// ----------------------------------------------------------------------------
{
    // Deselect previous selection
    selection.clear();
    selectionTrees.clear();

    // Fill the selection for next time
    selectNextTime.clear();
    Tree *t = tree;
    while (Infix *i = t->AsInfix())
    {
        if (i->name != "\n" && i->name != ";")
            break;
        selectNextTime.insert(i->left);
        t = i->right;
    }
    selectNextTime.insert(t);
    selectionChanged = true;
}


bool Widget::writeIfChanged(XL::SourceFile &sf)
// ----------------------------------------------------------------------------
//   Write file to repository if marked 'changed' and reset change attributes
// ----------------------------------------------------------------------------
{
    text fname = sf.name;
    if (sf.changed)
    {
        Repository *repo = repository();

        if (!repo)
            return false;

        if (repo->write(fname, sf.tree))
        {
            // Mark the tree as no longer changed
            sf.changed = false;

            // Record that we need to commit it sometime soon
            repo->change(fname);
            IFTRACE(filesync)
                std::cerr << "Changed " << fname << "\n";

            // Record time when file was changed
            QDateTime modified = QFileInfo(+fname).lastModified();
            sf.modified = modified.toTime_t();

            return true;
        }

        IFTRACE(filesync)
            std::cerr << "Could not write " << fname << " to repository\n";
    }
    return false;
}


#ifndef CFG_NOGIT
void Widget::commitSuccess(QString id, QString msg)
// ----------------------------------------------------------------------------
//   Document was succesfully committed to repository (see doCommit())
// ----------------------------------------------------------------------------
{
    taoWindow()->undoStack->push(new UndoCommand(repository(), id, msg));
}


bool Widget::doCommit(ulonglong tick)
// ----------------------------------------------------------------------------
//   Commit files previously written to repository and reset next commit time
// ----------------------------------------------------------------------------
{
    Repository * repo = repository();
    if (!repo)
        return false;
    if (repo->state == Repository::RS_Clean)
        return false;

    IFTRACE(filesync)
        std::cerr << "Commit\n";
    bool done;
    done = repo->commit();
    if (done)
    {
        nextCommit = tick + Opt::commitInterval * 1000;
        taoWindow()->markChanged(false);
        return true;
    }
    return false;
}


bool Widget::doPull(ulonglong tick)
// ----------------------------------------------------------------------------
//   Pull from remote repository and reset next pull time
// ----------------------------------------------------------------------------
{
    Repository *repo = repository();
    bool ok = repo->pull();
    nextPull = tick + repo->pullInterval * 1000;
    return ok;
}

#endif

bool Widget::setDragging(bool on)
// ----------------------------------------------------------------------------
//   A drag operation starts or ends
// ----------------------------------------------------------------------------
{
    bool old = dragging;
    dragging = on;
    updateProgramSource();
    if (!dragging)
        saveAndCommit();
    return old;
}


bool Widget::doSave(ulonglong tick)
// ----------------------------------------------------------------------------
//   Save source files that have changed and reset next save time
// ----------------------------------------------------------------------------
{
    bool changed = false;
    XL::Main *xlr = XL::MAIN;
    XL::source_files::iterator it;
    for (it = xlr->files.begin(); it != xlr->files.end(); it++)
    {
        XL::SourceFile &sf = (*it).second;
        if (writeIfChanged(sf))
            changed = true;
    }

    // Record when we will save file again
    nextSave = tick + Opt::saveInterval * 1000;
    return changed;
}


Repository * Widget::repository()
// ----------------------------------------------------------------------------
//   Return the repository associated with the current document (may be NULL)
// ----------------------------------------------------------------------------
{
    return taoWindow()->repository();
}


Tree *Widget::get(Tree *shape, text name, text topNameList)
// ----------------------------------------------------------------------------
//   Find an attribute in the current shape, group or returns NULL
// ----------------------------------------------------------------------------
{
    // Can't get attributes without a current shape
    if (!shape)
        return NULL;

    // The current shape has to be a 'shape' prefix
    Prefix *shapePrefix = shape->AsPrefix();
    if (!shapePrefix)
        return NULL;
    Name *shapeName = shapePrefix->left->AsName();
    if (!shapeName || topNameList.find(shapeName->value) == std::string::npos)
        return NULL;

    // Take the right child. If it's a block, extract the block
    Tree *child = shapePrefix->right;
    if (Block *block = child->AsBlock())
        child = block->child;

    // Now loop on all statements, looking for the given name
    while (child)
    {
        Tree *what = child;

        // Check if we have \n or ; infix
        Infix *infix = child->AsInfix();
        if (infix && (infix->name == "\n" || infix->name == ";"))
        {
            what = infix->left;
            child = infix->right;
        }
        else
        {
            child = NULL;
        }

        // Analyze what we got here: is it in the form 'funcname args' ?
        if (Prefix *prefix = what->AsPrefix())
            if (Name *prefixName = prefix->left->AsName())
                if (prefixName->value == name)
                    return prefix;

        // Is it a name
        if (Name *singleName = what->AsName())
            if (singleName->value == name)
                return singleName;
    }

    return NULL;
}


bool Widget::set(Tree *shape, text name, Tree *value, text topNameList)
// ----------------------------------------------------------------------------
//   Set an attribute in the current shape or group, return true if successful
// ----------------------------------------------------------------------------
{
    // Can't get attributes without a current shape
    if (!shape)
        return false;

    // The current shape has to be a 'shape' prefix
    Prefix *shapePrefix = shape->AsPrefix();
    if (!shapePrefix)
        return false;
    Name *shapeName = shapePrefix->left->AsName();
    if (!shapeName || topNameList.find(shapeName->value) == std::string::npos)
        return false;

    // Take the right child. If it's a block, extract the block
    Tree_p *addr = &shapePrefix->right;
    Tree *child = *addr;
    if (Block *block = child->AsBlock())
    {
        addr = &block->child;
        child = *addr;
    }
    Tree_p *topAddr = addr;

    // Now loop on all statements, looking for the given name
    while (child)
    {
        Tree *what = child;

        // Check if we have \n or ; infix
        Infix *infix = child->AsInfix();
        if (infix && (infix->name == "\n" || infix->name == ";"))
        {
            addr = &infix->left;
            what = *addr;
            child = infix->right;
        }
        else
        {
            child = NULL;
        }

        // Analyze what we got here: is it in the form 'funcname args' ?
        if (value->AsPrefix())
        {
            if (Prefix *prefix = what->AsPrefix())
            {
                if (Name *prefixName = prefix->left->AsName())
                {
                    if (prefixName->value == name)
                    {
                        ApplyChanges changes(value);
                        if (!(*addr)->Do(changes))
                        {
                            // Need big hammer here, reload everything
                            *addr = value;
                            reloadProgram();
                        }
                        else
                            refreshNow();

                        return true;
                    }
                }
            }
        }
        else if (Name *newName = value->AsName())
        {
            if (Name *stmtName = what->AsName())
            {
                if (stmtName->value == name)
                {
                    // If the name is different, need to update
                    if (newName->value != name)
                    {
                        *addr = value;
                        reloadProgram();
                    }
                    return true;
                }
            }
        }

    } // Loop on all items

    // We didn't find the name: set the top level item
    *topAddr = new Infix("\n", value, *topAddr);
    reloadProgram();
    return true;
}


bool Widget::get(Tree *shape, text name, TreeList &args, text topName)
// ----------------------------------------------------------------------------
//   Get the arguments, decomposing args in a comma-separated list
// ----------------------------------------------------------------------------
{
    // Check if we can get the tree
    Tree *attrib = get(shape, name, topName);
    if (!attrib)
        return false;

    // Check if we expect a single name or a prefix
    args.clear();
    if (attrib->AsName())
        return true;

    // Check that we have a prefix
    Prefix *prefix = attrib->AsPrefix();
    if (!prefix)
        return false;           // ??? This shouldn't happen

    // Get attribute arguments and decompose them into 'args'
    Tree *argsTree = prefix->right;
    while (Infix *infix = argsTree->AsInfix())
    {
        if (infix->name != ",")
            break;
        args.push_back(infix->right);
        argsTree = infix->left;
    }
    args.push_back(argsTree);
    std::reverse(args.begin(), args.end());

    // Success
    return true;
}


bool Widget::set(Tree *shape, text name, TreeList &args, text topName)
// ----------------------------------------------------------------------------
//   Set the arguments, building the comma-separated list
// ----------------------------------------------------------------------------
{
    Tree *call = XL::XLCall(name, args).Build();
    return set(shape, name, call, topName);
}


bool Widget::get(Tree *shape, text name, attribute_args &args, text topName)
// ----------------------------------------------------------------------------
//   Get the arguments, decomposing args in a comma-separated list
// ----------------------------------------------------------------------------
{
    if (!xlProgram)
        return false;

    // Get the trees
    TreeList treeArgs;
    if (!get(shape, name, treeArgs, topName))
        return false;

    // Convert from integer or tree values
    Scope *scope = xlProgram->scope;
    for (Tree *arg : treeArgs)
    {
        if (!arg->IsConstant())
            arg = xl_evaluate(scope, arg);
        if (Real *asReal = arg->AsReal())
            args.push_back(asReal->value);
        else if (Integer *asInteger = arg->AsInteger())
            args.push_back(asInteger->value);
        else
            return false;
    }

    return true;
}


bool Widget::isReadOnly()
// ----------------------------------------------------------------------------
//   Is document currently flagged read-only?
// ----------------------------------------------------------------------------
{
    return taoWindow()->isReadOnly;
}



// ============================================================================
//
//    Performance timing
//
// ============================================================================

ulonglong Widget::now()
// ----------------------------------------------------------------------------
//    Return the current time in microseconds
// ----------------------------------------------------------------------------
{
    // Timing
    struct timeval tv;
    gettimeofday(&tv, NULL);
    ulonglong t = tv.tv_sec * 1000000ULL + tv.tv_usec;
    return t;
}


void Widget::printStatistics()
// ----------------------------------------------------------------------------
//    Print current rendering statistics as text overlay
// ----------------------------------------------------------------------------
{
    char fps[6] = "---.-";
    double n = stats.fps();
    if (n >= 0)
        snprintf(fps, sizeof(fps), "%5.1f", n);
    char dropped[20] = "";
#ifdef MACOSX_DISPLAYLINK
    snprintf(dropped, sizeof(dropped), " (dropped %d)", droppedFramesLocked());
#endif
    GLint vp[4] = {0,0,0,0};
    GLint vx, vy, vw, vh;

    glGetIntegerv(GL_VIEWPORT, vp);
    vx = vp[0]; vy = vp[1]; vw = vp[2]; vh = vp[3];
    RasterText::moveTo(vx + 20, vy + vh - 20 - 10);
    RasterText::printf("%dx%dx%d %s fps%s", vw, vh, stereoPlanes, fps,
                       dropped);

    RasterText::moveTo(vx + 20, vy + vh - 20 - 10 - 17);
    if (n >= 0)
    {
        int xa, xm, da, dm, ga, gm, wa, wm, sa, sm;
        xa = stats.averageTimePerFrame(Statistics::EXEC);
        xm = stats.maxTime(Statistics::EXEC);
        da = stats.averageTimePerFrame(Statistics::DRAW);
        dm = stats.maxTime(Statistics::DRAW);
        ga = stats.averageTimePerFrame(Statistics::GC);
        gm = stats.maxTime(Statistics::GC);
        sa = stats.averageTimePerFrame(Statistics::SELECT);
        sm = stats.maxTime(Statistics::SELECT);
        if (Opt::gcThread)
        {
            wa = stats.averageTimePerFrame(Statistics::GC_WAIT);
            wm = stats.maxTime(Statistics::GC_WAIT);
            RasterText::printf("Avg/peak ms: Exec %3d/%3d Draw %3d/%3d "
                               "Select %3d/%3d "
                               "GCw %3d/%3d (GC %3d/%3d)",
                               xa, xm, da, dm,
                               sa, sm,
                               wa, wm, ga, gm);
        }
        else
        {
            RasterText::printf("Avg/peak ms: Exec %3d/%3d Draw %3d/%3d "
                               "Select %3d/%3d "
                               "GC %3d/%3d", xa, xm, da, dm, sa, sm, ga, gm);
        }
    }
    else
    {
        if (Opt::gcThread)
        {
            RasterText::printf("Avg/peak ms: Exec ---/--- Draw ---/--- "
                               "Select ---/--- "
                               "GCw ---/--- (GC ---/---)");
        }
        else
        {
            RasterText::printf("Avg/peak ms: Exec ---/--- Draw ---/--- "
                               "Select ---/--- "
                               "GC ---/---");
        }
    }

    // Display garbage collection statistics
    XL::GarbageCollector *gc = XL::GarbageCollector::GC();
    uint tot  = 0, alloc = 0, avail = 0, freed = 0, scanned = 0, collected = 0;
    gc->Statistics(tot, alloc, avail, freed, scanned, collected);

    RasterText::moveTo(vx + 20, vy + vh - 20 - 10 - 17 - 17);
    RasterText::printf("Program memory: "
                       "%5dK reserved, "
                       "%5dK available, "
                       "%5dK used, "
                       "%5dK freed",
                       tot>>10, avail>>10, alloc>>10, freed>>10);
}


struct PerLayoutStatisticsWrapper
// ----------------------------------------------------------------------------
//   Used to provider the right operator less-than
// ----------------------------------------------------------------------------
{
    PerLayoutStatisticsWrapper(Tree *lb, PerLayoutStatistics *stats)
        : layoutBody(lb), stats(stats) {}
    bool operator< (const PerLayoutStatisticsWrapper &o) const
    {
        return stats->totalTime() < o.stats->totalTime();
    }
    Tree                *layoutBody;
    PerLayoutStatistics *stats;
};
typedef std::priority_queue<PerLayoutStatisticsWrapper> PerLayoutStatisticsQ;


static void insertPerLayoutStatistics(Tree *tree,
                                      PerLayoutStatisticsQ &queue)
// ----------------------------------------------------------------------------
//    Insert the per-layout statistics in the queue
// ----------------------------------------------------------------------------
{
    PerLayoutStatistics *info = tree->GetInfo<PerLayoutStatistics>();
    if (info)
        queue.push(PerLayoutStatisticsWrapper(tree, info));
    if (Infix *infix = tree->AsInfix())
    {
        insertPerLayoutStatistics(infix->left, queue);
        insertPerLayoutStatistics(infix->right, queue);
    }
    else if (Prefix *prefix = tree->AsPrefix())
    {
        insertPerLayoutStatistics(prefix->left, queue);
        insertPerLayoutStatistics(prefix->right, queue);
    }
    else if (Postfix *postfix = tree->AsPostfix())
    {
        insertPerLayoutStatistics(postfix->left, queue);
        insertPerLayoutStatistics(postfix->right, queue);
    }
    else if (Block *block = tree->AsBlock())
    {
        insertPerLayoutStatistics(block->child, queue);
    }
}


void Widget::printPerLayoutStatistics()
// ----------------------------------------------------------------------------
//    Print the rendering statistics for all the layouts
// ----------------------------------------------------------------------------
{
    static int refresh = 0;

    if (refresh++ < 100)
        return;
    refresh = 0;

    PerLayoutStatisticsQ queue;
    XL::Main *xlr = XL::MAIN;
    XL::source_files::iterator it;
    for (it = xlr->files.begin(); it != xlr->files.end(); it++)
    {
        XL::SourceFile &sf = (*it).second;
        if (sf.tree)
            insertPerLayoutStatistics(sf.tree, queue);
    }

    fprintf(stderr,
            "%s,%s,%s,%s,%s,%s,%s,%s\n",
            "FILE", "LINE",
            "EXEC (ms)", "XCNT", "XAVG",
            "DRAW (ms)", "DCNT", "DAVG");
    while (!queue.empty())
    {
        const PerLayoutStatisticsWrapper &top = queue.top();
        PerLayoutStatistics *stats = top.stats;

        if (stats->sourceLine == 0)
            XL::MAIN->positions.GetInfo(top.layoutBody->Position(),
                                        &stats->sourceFile,
                                        &stats->sourceLine,
                                        NULL, NULL);
        if (stats->execCount || stats->drawCount)
            fprintf(stderr, "%s,%lu,%lu,%lu,%lu,%lu,%lu,%lu\n",
                    stats->sourceFile.c_str(), stats->sourceLine,
                    stats->totalExecTime, stats->execCount,
                    stats->totalExecTime/(stats->execCount|!stats->execCount),
                    stats->totalDrawTime, stats->drawCount,
                    stats->totalDrawTime/(stats->drawCount|!stats->drawCount));
        stats->reset();
        queue.pop();
    }
}


void Widget::logStatistics()
// ----------------------------------------------------------------------------
//    Print current rendering statistics as CSV to stdout
// ----------------------------------------------------------------------------
{
    static bool printHeader = true;
    static QTime tm;

    double n = stats.fps();
    if (n >= 0)
    {
        if (tm.isValid() && tm.elapsed() < 1000 /* ms */)
            return;
        tm.restart();
        if (printHeader)
        {
            std::cout << "Time;PageNum;FPS;Exec;MaxExec;Draw;MaxDraw;GC;MaxGC;"
                "Select;MaxSelect";
            if (Opt::gcThread)
                std::cout << ";GCWait;MaxGCWait";
#ifdef MACOSX_DISPLAYLINK
            std::cout << ";Dropped";
#endif
            std::cout << "\n";
            printHeader = false;
        }
        std::cout << CurrentTime() << ";"
                  << pageShown << ";"
                  << n << ";"
                  << stats.averageTimePerFrame(Statistics::EXEC) << ";"
                  << stats.maxTime(Statistics::EXEC) << ";"
                  << stats.averageTimePerFrame(Statistics::DRAW) << ";"
                  << stats.maxTime(Statistics::DRAW) << ";"
                  << stats.averageTimePerFrame(Statistics::GC) << ";"
                  << stats.maxTime(Statistics::GC) << ";"
                  << stats.averageTimePerFrame(Statistics::SELECT) << ";"
                  << stats.maxTime(Statistics::SELECT);

        if (Opt::gcThread)
        {
            std::cout << ";"
                      << stats.averageTimePerFrame(Statistics::GC_WAIT) << ";"
                      << stats.maxTime(Statistics::GC_WAIT);
        }
#ifdef MACOSX_DISPLAYLINK
        std::cout << ";" << droppedFramesLocked();
#endif
        std::cout << "\n" << std::flush;
    }
}



// ============================================================================
//
//    Selection management
//
// ============================================================================

uint Widget::selected(uint i)
// ----------------------------------------------------------------------------
//   Test if the given shape ID is in the selection map, return selection value
// ----------------------------------------------------------------------------
{
    selection_map::iterator found = selection.find(i);
    if (found != selection.end())
        return (*found).second;
    return 0;
}


uint Widget::selected(Layout *layout)
// ----------------------------------------------------------------------------
//   Test if the current shape (as identified by layout ID) is selected
// ----------------------------------------------------------------------------
{
    if (layout->groupDrag)
        return Widget::CONTAINER_SELECTED;
    return selected(layout->id);
}


void Widget::select(uint id, uint count)
// ----------------------------------------------------------------------------
//    Change the current shape selection state
// ----------------------------------------------------------------------------
//    We use the following special selection 'counts':
//    - CHAR_SELECTED means we selected characters, see TextSelect
//    - CONTAINER_OPENED means that we are opening a hierarchy, e.g.
//      a text box or a group.
{
    selection[id] = count;
}


void Widget::reselect(Tree *from, Tree *to)
// ----------------------------------------------------------------------------
//   If 'from' is in any selection map, add 'to' to this selection
// ----------------------------------------------------------------------------
{
    // Check if we are possibly changing the selection
    if (selectionTrees.count(from))
        selectionTrees.insert(to);

    // Check if we are possibly changing the next selection
    if (selectNextTime.count(from))
        selectNextTime.insert(to);

    // Check if we are possibly changing the page tree reference
    if (pageTree == from)
        pageTree = to;

    // Copy relevant tree data
    if (XL::CommentsInfo *cinfo = from->GetInfo<XL::CommentsInfo>())
        to->SetInfo<XL::CommentsInfo> (new XL::CommentsInfo(*cinfo));
}


void Widget::selectionContainerPush()
// ----------------------------------------------------------------------------
//    Push current item, which becomes a parent in the selection hierarchy
// ----------------------------------------------------------------------------
{
    idDepth++;
    if (maxIdDepth < idDepth)
        maxIdDepth = idDepth;
}


void Widget::selectionContainerPop()
// ----------------------------------------------------------------------------
//   Pop current child in selection hierarchy
// ----------------------------------------------------------------------------
{
    idDepth--;
}


void Widget::saveSelectionColorAndFont(Layout *where)
// ----------------------------------------------------------------------------
//   Save the color and font for the selection
// ----------------------------------------------------------------------------
{
    if (where && where->id)
    {
        selectionColor["line_color"] = where->lineColor;
        selectionColor["color"] = where->fillColor;
        selectionFont = where->font;
        IFTRACE (fonts)
        {
            std::cerr << "Widget::saveSelectionColorAndFont(" << where->id
                      << ") font : " << +where->font.toString() << std::endl;
        }
    }
}


bool Widget::focused(Layout *layout)
// ----------------------------------------------------------------------------
//   Test if the current shape is selected
// ----------------------------------------------------------------------------
{
    return layout->id == focusId;
}


void Widget::deleteFocus(QWidget *widget)
// ----------------------------------------------------------------------------
//   Make sure we don't keep a focus on a widget that was deleted
// ----------------------------------------------------------------------------
{
    if (focusWidget == widget)
        focusWidget = NULL;
}


bool Widget::requestFocus(QWidget *widget, coord x, coord y)
// ----------------------------------------------------------------------------
//   Some other widget request the focus
// ----------------------------------------------------------------------------
{
    if (focusWidget != widget)
    {
        record(widget, "Widget focus from %p to %p", focusWidget, widget);
        record(focus,  "Widget focus from %p to %p", focusWidget, widget);
        if (focusWidget)
        {
            QFocusEvent focusOut(QEvent::FocusOut, Qt::ActiveWindowFocusReason);
            QObject *fout = focusWidget;
            record(focus, "Focus %p out", fout);
            fout->event(&focusOut);
            focusWidget->clearFocus();
        }
        focusWidget = widget;
        if (widget)
        {
            GLMatrixKeeper saveGL;
            Vector3 v = layout->Offset() + Vector3(x, y, 0);
            GL.Translate(v.x, v.y, v.z);
            recordProjection();
            QFocusEvent focusIn(QEvent::FocusIn, Qt::ActiveWindowFocusReason);
            QObject *fin = focusWidget;
            QApplication::focusWidget()->clearFocus();
            record(focus, "Focus %p in", widget);
            fin->event(&focusIn);
            focusWidget->setFocus();
            record(focus,
                   "Focus widget %p %+s",
                   QApplication::focusWidget(),
                   focusWidget->hasFocus() ? "has focus" : "has no focus");
        }
        record(focus, "Focus widget %+s focus",
               focusWidget && focusWidget->hasFocus() ? "has" : "doesn't have");
    }

    if (focusWidget)
    {
        if (QApplication::focusWidget() &&
            QApplication::focusWidget() != focusWidget)
            QApplication::focusWidget()->clearFocus();
        focusWidget->setFocus();
    }

    record(focus,
           "Focus widget %p app %p", focusWidget, QApplication::focusWidget());
    record(focus, "Second attempt focus widget %+s focus",
           focusWidget && focusWidget->hasFocus() ? "has" : "doesn't have");

    return focusWidget == widget;
}


void Widget::recordProjection(GLdouble *proj, GLdouble *model, GLint *viewport)
// ----------------------------------------------------------------------------
//   Record the transformation matrix for the current projection
// ----------------------------------------------------------------------------
{
    memcpy(proj, GL.ProjectionMatrix(), sizeof focusProjection);
    memcpy(model, GL.ModelViewMatrix(), sizeof focusModel);
    if (mouseTrackingViewport[2] == 0 && mouseTrackingViewport[3] == 0)
    {
        // mouseTrackingViewport not set (by display module), default to
        // current viewport
        GLint *glvp = GL.Viewport();
        viewport[0] = glvp[0];
        viewport[1] = glvp[1];
        viewport[2] = glvp[2];
        viewport[3] = glvp[3];
    }
    else
    {
        viewport[0] = mouseTrackingViewport[0];
        viewport[1] = mouseTrackingViewport[1];
        viewport[2] = mouseTrackingViewport[2];
        viewport[3] = mouseTrackingViewport[3];
    }
}


void Widget::recordProjection()
// ----------------------------------------------------------------------------
//   Record focus projection
// ----------------------------------------------------------------------------
{
    recordProjection(focusProjection, focusModel, focusViewport);
}


Point3 Widget::unproject (coord x, coord y, coord z,
                          GLdouble *proj, GLdouble *model, GLint *viewport)
// ----------------------------------------------------------------------------
//   Convert mouse clicks into 3D planar coordinates for the focus object
// ----------------------------------------------------------------------------
{
    // Adjust between mouse and OpenGL coordinate systems
    y = height() - y;

    // On Retina display, we need to convert to physical pixels (#3254)
    x *= devicePixelRatio;
    y *= devicePixelRatio;

    // Get 3D coordinates for the near plane based on window coordinates
    GLdouble x3dn = 0, y3dn = 0, z3dn = 0;
    GL.UnProject(x, y, 0.0,
                 model, proj, viewport,
                 &x3dn, &y3dn, &z3dn);

    // Same with far-plane 3D coordinates
    GLdouble x3df = 0, y3df = 0, z3df = 0;
    GL.UnProject(x, y, 1.0,
                 model, proj, viewport,
                 &x3df, &y3df, &z3df);

    GLfloat zDistance = z3dn - z3df;
    if (zDistance == 0.0)
        zDistance = 1.0;
    GLfloat ratio = (z3dn - z) / zDistance;
    GLfloat x3d = x3dn + ratio * (x3df - x3dn);
    GLfloat y3d = y3dn + ratio * (y3df - y3dn);

    return Point3(x3d, y3d, z);
}


Point3 Widget::unproject (coord x, coord y, coord z)
// ----------------------------------------------------------------------------
//   Unproject in widget's focus transform
// ----------------------------------------------------------------------------
{
    return unproject(x, y, z, focusProjection, focusModel, focusViewport);
}


Point3 Widget::unprojectLastMouse(GLdouble *proj, GLdouble *model, GLint *view)
// ----------------------------------------------------------------------------
//   Unproject last mouse coordinates in given context
// ----------------------------------------------------------------------------
{
    return unproject(lastMouseX, lastMouseY, 0.0, proj, model, view);
}


Point3 Widget::unprojectLastMouse()
// ----------------------------------------------------------------------------
//    Unproject last mouse coordinates in current focus transform
// ----------------------------------------------------------------------------
{
    return unproject(lastMouseX, lastMouseY, 0.0,
                     focusProjection, focusModel, focusViewport);
}


Point3 Widget::project (coord x, coord y, coord z)
// ----------------------------------------------------------------------------
//   project widget's focus transform
// ----------------------------------------------------------------------------
{
    return project(x, y, z, focusProjection, focusModel, focusViewport);
}


Point3 Widget::project (coord x, coord y, coord z,
                        GLdouble *proj, GLdouble *model, GLint *viewport)
// ----------------------------------------------------------------------------
//   Convert mouse clicks into 3D planar coordinates for the focus object
// ----------------------------------------------------------------------------
{
    GLdouble wx, wy, wz;
    GL.Project(x, y, z,
               model, proj, viewport,
               &wx, &wy, &wz);

    return Point3(wx, wy, wz);
}


Point3 Widget::objectToWorld(coord x, coord y,
                             GLdouble *proj, GLdouble *model, GLint *viewport)
// ----------------------------------------------------------------------------
//    Convert object coordinates to world coordinates
// ----------------------------------------------------------------------------
{
    Point3 pos, win;

    // Map object coordinates to window coordinates
    GL.Project(x, y, 0,
               model, proj, viewport,
               &win.x, &win.y, &win.z);

    pos = windowToWorld(win.x, win.y, proj, model, viewport);

    return pos;
}


Point3 Widget::windowToWorld(coord x, coord y,
                             GLdouble *proj, GLdouble *model, GLint *viewport)
// ----------------------------------------------------------------------------
//    Convert window coordinates to world coordinates
// ----------------------------------------------------------------------------
{
    Point3 pos;
    GLfloat pixelDepth;

    // Read depth buffer
    glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT,
                 &pixelDepth);

    // Map window coordinates to object coordinates
    GL.UnProject(x, y, pixelDepth,
                 model, proj, viewport,
                 &pos.x,
                 &pos.y,
                 &pos.z);

    return pos;
}



Drag *Widget::drag()
// ----------------------------------------------------------------------------
//   Return the drag activity that we can use to unproject
// ----------------------------------------------------------------------------
{
    Drag *result = active<Drag>();
    if (result)
        recordProjection();
    return result;
}


TextSelect *Widget::textSelection()
// ----------------------------------------------------------------------------
//   Return text selection if appropriate, possibly creating it from a Drag
// ----------------------------------------------------------------------------
{
    TextSelect *result = active<TextSelect>();
    if (result)
        recordProjection();
    return result;
}


static inline void resetLayout(Layout *where)
// ----------------------------------------------------------------------------
//   Put layout back into a state appropriate for drawing a selection
// ----------------------------------------------------------------------------
{
    if (where)
    {
        where->lineWidth = 1;
        where->lineColor = Color(0,0,0,0);
        where->fillColor = Color(0,1,0,0.8);
        where->programId = 0;
    }
}


static inline uint layoutId(Layout *where)
// ----------------------------------------------------------------------------
//   Return the layout ID for the given layout, or 0
// ----------------------------------------------------------------------------
{
    return where ? where->id : 0;
}


void Widget::drawSelection(const Box3 &bnds,
                           text selName,
                           Layout *where)
// ----------------------------------------------------------------------------
//    Draw a 2D or 3D selection with the given coordinates
// ----------------------------------------------------------------------------
{
    if (!xlProgram)
        return;

    Box3 bounds(bnds);
    bounds.Normalize();

    coord w = bounds.Width();
    coord h = bounds.Height();
    coord d = bounds.Depth();
    Point3 c  = bounds.Center();

    SpaceLayout selectionSpace(this);

    if (where && where->groupDrag)
        selName = "group_item";

    XL::Save<Layout *> saveLayout(layout, &selectionSpace);
    GLAttribKeeper     saveGL;
    resetLayout(layout);
    selectionSpace.id = layoutId(where);
    saveSelectionColorAndFont(where);
    Scope *scope = xlProgram->scope;
    if (bounds.Depth() > 0)
        XL::XLCall("draw_" + selName, c.x, c.y, c.z, w, h, d) (scope);
    else
        XL::XLCall("draw_" + selName, c.x, c.y, w, h) (scope);

    selectionSpace.Draw(NULL);
}


void Widget::drawHandle(const Point3 &p,
                        text handleName,
                        Layout *where XL_UNUSED,
                        uint id)
// ----------------------------------------------------------------------------
//    Draw the handle of a 2D or 3D selection
// ----------------------------------------------------------------------------
{
    if (!xlProgram)
        return;

    SpaceLayout selectionSpace(this);

    XL::Save<Layout *> saveLayout(layout, &selectionSpace);
    GLAttribKeeper     saveGL;
    resetLayout(layout);
    selectionSpace.id = id | HANDLE_SELECTED;
    XL::XLCall("draw_" + handleName, p.x, p.y, p.z) (xlProgram->scope);

    selectionSpace.Draw(NULL);
}


Layout *Widget::drawTree(Scope *scope,
                         Tree *code,
                         Layout *where)
// ----------------------------------------------------------------------------
//    Draw some tree, e.g. cell fill and border, return new layout
// ----------------------------------------------------------------------------
{
    Layout *result = where->NewChild();
    XL::Save<Layout *> saveLayout(layout, result);
    saveAndEvaluate(scope, code);
    return result;
}


void Widget::drawCall(XL::XLCall &call,
                      Layout *where)
// ----------------------------------------------------------------------------
//   Draw the given call in a selection context
// ----------------------------------------------------------------------------
{
    if (!xlProgram)
        return;

    // Symbols where we will find the selection code
    SpaceLayout selectionSpace(this);

    XL::Save<Layout *> saveLayout(layout, &selectionSpace);
    GLAttribKeeper     saveGL;
    resetLayout(layout);
    selectionSpace.id = layoutId(where);
    call(xlProgram->scope);
    selectionSpace.Draw(where);
}


Tree * Widget::shapeAction(text n, GLuint id, int x, int y)
// ----------------------------------------------------------------------------
//   Return the shape action for the given name and GL id
// ----------------------------------------------------------------------------
{
    action_map::iterator foundName = actionMap.find(n);
    if (foundName != actionMap.end())
    {
        perId_action_map::iterator foundAction = (*foundName).second.find(id);
        if (foundAction != (*foundName).second.end())
        {
            Tree *action = (*foundAction).second;
            Scope_p scope = xlProgram->scope;

            // Set event mouse coordinates (bug #937, #1013)
            CoordinatesInfo *m = action->GetInfo<CoordinatesInfo>();
            XL::Save<CoordinatesInfo *> s(mouseCoordinatesInfo, m);

            // Adjust coordinates with latest event information
            m->coordinates = unproject(x, y, 0,
                                       m->projection, m->model, m->viewport);

            return xl_evaluate(scope, action);
        }
    }
    return NULL;
}



// ============================================================================
//
//   XLR runtime entry points
//
// ============================================================================

#pragma GCC diagnostic ignored "-Wunused-parameter"

Widget *Widget::current = NULL;
typedef Tree Tree;

Text *Widget::page(Scope *scope, Text *namePtr, Tree *body)
// ----------------------------------------------------------------------------
//   Start a new page, returns the previously named page
// ----------------------------------------------------------------------------
{
    text name = *namePtr;
    record(pages,
           "Displaying page %u id %u / %u target %s name %s",
           pageShown, pageId, pageTotal, pageName, name);

    // Evaluation mode check
#ifdef CFG_UNLICENSED_MAX_PAGES
    if (pageId >= CFG_UNLICENSED_MAX_PAGES
        && (TaoApp->edition == Application::Player ||
            TaoApp->edition == Application::Studio)
#ifndef CFG_NO_DOC_SIGNATURE
        && !isDocSigned()
#endif
        )
    {
        if (!pageLimitationDialogShown)
        {
            pageLimitationDialogShown = true;
            record(license_error,
                   "Reached page %u, not licensed to exceed %u",
                   pageId, CFG_UNLICENSED_MAX_PAGES);
            QString info;
            info = tr("<p>This unlicensed version of Tao3D "
                      "has a limit of 5 pages per document.</p>"
                      "<p>You may suppress this limitation by purchasing a "
                      "Pro license.</p><p>Thank you.</p>");
            NagScreen nag;
            nag.setText(tr("<h3>Warning</h3>"));
            nag.setInformativeText(info);
            nag.exec();
        }
        return new Text(pageName);
    }
#endif

    // We start with first page if we had no page set
    if (pageName == "")
        pageName = name;

    // Increment pageId and build page list
    pageId++;
    newPageNames.push_back(name);

    // If the page is set, then we display it
    if (printer && pageToPrint == pageId)
        pageName = name;
    if (drawAllPages || pageName == name)
    {
        // Check if we already displayed a page with that name
        if (pageFound && !(drawAllPages || printer))
        {
            Ooops("Page name $1 is already used", namePtr);
        }
        else
        {
            // Initialize back-link
            pageFound = pageId;
            pageLinks.clear();
            if (pageId > 1)
                pageLinks["Up"] = lastPageName;
            pageTree = body;
            pageHasExitHandler = false;
            bool isPageEntry = (pageEntry == 0);
            if (isPageEntry)
                pageEntry = pageId;
            saveAndEvaluate(scope, body);
            if (isPageEntry)
                pageEntry = (uint)-1; // assuming < 4 billion pages ;-)
        }
    }
    else if (pageName == lastPageName)
    {
        // We are executing the page following the current one:
        // Check if Down is set, otherwise set current page as default
        if (pageLinks.count("Down") == 0)
            pageLinks["Down"] = name;
    }

    lastPageName = name;
    return new Text(pageName, namePtr->Position());
}


Text *Widget::pageUniqueName(Tree *self, text name)
// ----------------------------------------------------------------------------
//   Find a page name that is unique
// ----------------------------------------------------------------------------
{
    text unique = +QString("%1 #%2").arg(+name).arg(pageId);
    return new Text(unique, "\"", "\"", self->Position());
}


Text *Widget::pageLink(Tree *self, text key, text name)
// ----------------------------------------------------------------------------
//   Indicate the chaining of pages, returns previous information
// ----------------------------------------------------------------------------
{
    text old = pageLinks[key];
    pageLinks[key] = name;
    return new Text(old, self->Position());
}


Real *Widget::pageSetPrintTime(Tree *self, double t)
// ----------------------------------------------------------------------------
//   Set the time at which animations should be set when printing pages
// ----------------------------------------------------------------------------
{
    double old = pagePrintTime;
    pagePrintTime = t;
    return new Real(old, self->Position());
}


Text *Widget::gotoPage(Tree *self, Text * page, bool abortTransitions)
// ----------------------------------------------------------------------------
//   Convert the value
// ----------------------------------------------------------------------------
{
    return gotoPage(self, page->value, abortTransitions);
}


Text *Widget::gotoPage(Tree *self, text page, bool abortTransitions)
// ----------------------------------------------------------------------------
//   Directly go to the given page
// ----------------------------------------------------------------------------
{
    text old = pageName;
    IFTRACE(pages)
        std::cerr << "Goto page '" << page
                  << "' from '" << pageName << "'\n";

    if (transitionStartTime > 0)
    {
        if (abortTransitions)
        {
            gotoPageName = page;
            commitPageChange(true);
            refresh(0);
        }
    }
    else
    {
        gotoPageName = page;
        refresh(0); // Not refreshNow(), see #1074
    }
    return new Text(old);
}


Text *Widget::pageLabel(Tree *self)
// ----------------------------------------------------------------------------
//   Return the label of the current page
// ----------------------------------------------------------------------------
{
    return new Text(pageName, self->Position());
}


Integer *Widget::pageNumber(Tree *self)
// ----------------------------------------------------------------------------
//   Return the number of the current page
// ----------------------------------------------------------------------------
{
    return new Integer(pageShown, self->Position());
}


Integer *Widget::pageCount(Tree *self)
// ----------------------------------------------------------------------------
//   Return the number of pages in the current document
// ----------------------------------------------------------------------------
{
    uint pages = 0;
    if (pageTotal)
        pages = pageTotal;
    else if (pageId)
        pages = pageId;
    return new Integer(pages, self->Position());
}


Text *Widget::pageNameAtIndex(Tree *self, uint index)
// ----------------------------------------------------------------------------
//   Return the nth page
// ----------------------------------------------------------------------------
{
    index--;
    text name = index < pageNames.size() ? pageNames[index] : "";
    return new Text(name, self->Position());
}


Real *Widget::pageWidth(Tree *self)
// ----------------------------------------------------------------------------
//   Return the width of the page
// ----------------------------------------------------------------------------
{
    return new Real(pageW, self->Position());
}


Real *Widget::pageHeight(Tree *self)
// ----------------------------------------------------------------------------
//   Return the height of the page
// ----------------------------------------------------------------------------
{
    return new Real(pageH, self->Position());
}


Integer *Widget::prevPageNumber(Tree *self)
// ----------------------------------------------------------------------------
//   Return the number of the previous page (the one displayed before current)
// ----------------------------------------------------------------------------
{
    return new Integer(prevPageShown, self->Position());
}


Text *Widget::prevPageLabel(Tree *self)
// ----------------------------------------------------------------------------
//   Return the label of the previous page (the one displayed before current)
// ----------------------------------------------------------------------------
{
    return new Text(prevPageName, self->Position());
}



// ============================================================================
//
//    Transitions
//
// ============================================================================

Tree *Widget::transition(Scope *scope XL_UNUSED,
                         Tree *self,
                         double dur,
                         Tree *body)
// ----------------------------------------------------------------------------
//    Define a transition
// ----------------------------------------------------------------------------
{
    if (transitionTree || inOfflineRendering)
        // Already running in a transition
        return XL::xl_false;

    transitionDurationValue = dur;
    transitionTree = body;
    return XL::xl_true;
}


Real *Widget::transitionTime(Tree *self)
// ----------------------------------------------------------------------------
//   Return the transition time
// ----------------------------------------------------------------------------
{
    double ttime = 0.0;
    if (transitionStartTime > 0)
    {
        refreshOn(QEvent::Timer);
        if (animated)
            frozenTime = CurrentTime();
        ttime = frozenTime - transitionStartTime;
        if (ttime > transitionDurationValue)
            ttime = transitionDurationValue;
    }
    return new Real(ttime, self->Position());
}


Real *Widget::transitionDuration(Tree *self)
// ----------------------------------------------------------------------------
//   Return the transition duration
// ----------------------------------------------------------------------------
{
    return new Real(transitionDurationValue, self->Position());
}


Real *Widget::transitionRatio(Tree *self)
// ----------------------------------------------------------------------------
//   Return the transition ratio
// ----------------------------------------------------------------------------
{
    double ratio = 0.0;
    if (transitionStartTime > 0 && transitionDurationValue > 0.0)
    {
        refreshOn(QEvent::Timer);
        if (animated)
            frozenTime = CurrentTime();
        ratio = (frozenTime - transitionStartTime) / transitionDurationValue;
        if (ratio > 1.0)
            ratio = 1.0;
    }
    return new Real(ratio, self->Position());
}


Tree *Widget::transitionCurrentPage(Scope *scope, Tree *self)
// ----------------------------------------------------------------------------
//   Evaluate the current page during a transition
// ----------------------------------------------------------------------------
{
    if (xlProgram)
    {
        XL::Save<text> saveLastPage(lastPageName, pageName);
        XL::Save<text> savePageName(pageName, transitionPageName);
        XL::Save<page_map> saveLinks(pageLinks, pageLinks);
        XL::Save<page_list> saveList(pageNames, pageNames);
        XL::Save<uint> savePageFound(pageFound, 0);
        XL::Save<Tree_p> savePageTree(pageTree, pageTree);

        for (uint p = 0; p < pageNames.size(); p++)
        {
            if (pageNames[p] == pageName)
            {
                pageShown = p + 1;
                break;
            }
        }
        clearColor(self, QColor::fromRgbF(1, 1, 1, 1));
        return xl_evaluate(xlProgram->scope, xlProgram->tree);
    }
    return XL::xl_false;
}


Tree *Widget::transitionNextPage(Scope *scope, Tree *self)
// ----------------------------------------------------------------------------
//   Evaluate the next page during a transition
// ----------------------------------------------------------------------------
{
    if (xlProgram)
    {
        XL::Save<text> saveLastPage(lastPageName, pageName);
        XL::Save<text> savePageName(pageName, gotoPageName);
        XL::Save<page_map> saveLinks(pageLinks, pageLinks);
        XL::Save<page_list> saveList(pageNames, pageNames);
        XL::Save<page_list> saveNewList(newPageNames, newPageNames);
        XL::Save<uint> savePageId(pageId, 0);
        XL::Save<uint> savePageFound(pageFound, 0);
        XL::Save<uint> savePageShown(pageShown, pageShown);
        XL::Save<uint> savePageTotal(pageTotal, pageTotal);
        XL::Save<Tree_p> savePageTree(pageTree, pageTree);
        XL::Save<double> savePageTime(pageStartTime, transitionStartTime);

        for (uint p = 0; p < pageNames.size(); p++)
        {
            if (pageNames[p] == pageName)
            {
                pageShown = p + 1;
                break;
            }
        }
        clearColor(self, QColor::fromRgbF(1, 1, 1, 1));
        return xl_evaluate(xlProgram->scope, xlProgram->tree);
    }
    return XL::xl_false;
}


Tree *Widget::runTransition(Scope *scope)
// ----------------------------------------------------------------------------
//   Evaluate the transition code until we are done
// ----------------------------------------------------------------------------
{
    Tree *result;
    IFTRACE(pages)
        std::cerr << "Transition duration " << transitionDurationValue
                  << " remaining " << frozenTime - transitionStartTime
                  << "\n";
    if (frozenTime - transitionStartTime > transitionDurationValue)
    {
        commitPageChange(true);
        result = xl_evaluate(scope, xlProgram->tree);
    }
    else
    {
        XL::Save<bool> saveInTransition(runningTransitionCode, true);
        result = xl_evaluate(scope, xlProgram->tree);
    }
    return result;
}



// ============================================================================
//
//    Frames
//
// ============================================================================

Real *Widget::frameWidth(Tree *self)
// ----------------------------------------------------------------------------
//   Return the width of the current layout frame
// ----------------------------------------------------------------------------
{
    return new Real(layout->Bounds(layout).Width(), self->Position());
}


Real *Widget::frameHeight(Tree *self)
// ----------------------------------------------------------------------------
//   Return the height of the current layout frame
// ----------------------------------------------------------------------------
{
    return new Real(layout->Bounds(layout).Height(), self->Position());
}


Real *Widget::frameDepth(Tree *self)
// ----------------------------------------------------------------------------
//   Return the depth of the current layout frame
// ----------------------------------------------------------------------------
{
    return new Real(layout->Bounds(layout).Depth(), self->Position());
}


int Widget::width()
// ----------------------------------------------------------------------------
//   Return the width of the drawing widget in px
// ----------------------------------------------------------------------------
{
    if (inOfflineRendering)
        return offlineRenderingWidth;
    return QWidget::width();
}


int Widget::height()
// ----------------------------------------------------------------------------
//   Return the height of the drawing widget in px
// ----------------------------------------------------------------------------
{
    if (inOfflineRendering)
        return offlineRenderingHeight;
    return QWidget::height();
}


Real *Widget::windowWidth(Tree *self)
// ----------------------------------------------------------------------------
//   Return the width of the window in which we display
// ----------------------------------------------------------------------------
{
    refreshOn(QEvent::Resize);
    double w = printer ? printer->paperRect().width() : width();
    w *= displayDriver->windowWidthFactor();
    return new Real(w, self->Position());
}


Real *Widget::windowHeight(Tree *self)
// ----------------------------------------------------------------------------
//   Return the height of window in which we display
// ----------------------------------------------------------------------------
{
    refreshOn(QEvent::Resize);
    double h = printer ? printer->paperRect().height() : height();
    h *= displayDriver->windowHeightFactor();
    return new Real(h, self->Position());
}


Integer *Widget::seconds(Tree *self, double t)
// ----------------------------------------------------------------------------
//    Return the second for time t or the current time if t < 0
// ----------------------------------------------------------------------------
{
    double tm = (t < 0) ? CurrentTime() : t;
    int second = fmod(tm, 60);
    if (t < 0)
    {
        double refresh = (double)(int)tm + 1.0;
        refreshOn(QEvent::Timer, refresh);
    }
    return new Integer(second, self->Position());
}


Integer *Widget::minutes(Tree *self, double t)
// ----------------------------------------------------------------------------
//    Return the minute for time t or the current time if t < 0
// ----------------------------------------------------------------------------
{
    double tm = (t < 0) ? CurrentTime() : t;
    QDateTime now = fromSecsSinceEpoch(tm);
    int minute = now.time().minute();
    if (t < 0)
    {
        QTime t = now.time();
        t = t.addMSecs(-t.msec());
        t.setHMS(t.hour(), t.minute(), 0);
        QDateTime next(now);
        next.setTime(t);
        next = next.addSecs(60);
        double refresh = toSecsSinceEpoch(next);
        refreshOn(QEvent::Timer, refresh);
    }
    return new Integer(minute, self->Position());
}


Integer *Widget::hours(Tree *self, double t)
// ----------------------------------------------------------------------------
//    Return the localtime hour for time t or the current time if t < 0
// ----------------------------------------------------------------------------
{
    double tm = (t < 0) ? CurrentTime() : t;
    QDateTime now = fromSecsSinceEpoch(tm);
    int hour = now.time().hour();
    if (t < 0)
    {
        QTime t = now.time();
        t = t.addMSecs(-t.msec());
        t.setHMS(t.hour(), 0, 0);
        QDateTime next(now);
        next.setTime(t);
        next = next.addSecs(3600);
        double refresh = toSecsSinceEpoch(next);
        refreshOn(QEvent::Timer, refresh);
    }
    return new Integer(hour, self->Position());
}


Integer *Widget::day(Tree *self, double t)
// ----------------------------------------------------------------------------
//    Return the day (1-31) for time t or the current time if t < 0
// ----------------------------------------------------------------------------
{
    double tm = (t < 0) ? CurrentTime() : t;
    QDateTime now = fromSecsSinceEpoch(tm);
    int day = now.date().day();
    if (t < 0)
    {
        double refresh = nextDay(now);
        refreshOn(QEvent::Timer, refresh);
    }
    return new Integer(day, self->Position());
}


Integer *Widget::weekDay(Tree *self, double t)
// ----------------------------------------------------------------------------
//    Return the week day (1-7) for time t or the current time if t < 0
// ----------------------------------------------------------------------------
{
    double tm = (t < 0) ? CurrentTime() : t;
    QDateTime now = fromSecsSinceEpoch(tm);
    int day = now.date().dayOfWeek();
    if (t < 0)
    {
        double refresh = nextDay(now);
        refreshOn(QEvent::Timer, refresh);
    }
    return new Integer(day, self->Position());
}


Integer *Widget::yearDay(Tree *self, double t)
// ----------------------------------------------------------------------------
//    Return the year day (1-365) for time t or the current time if t < 0
// ----------------------------------------------------------------------------
{
    double tm = (t < 0) ? CurrentTime() : t;
    QDateTime now = fromSecsSinceEpoch(tm);
    int day = now.date().dayOfYear();
    if (t < 0)
    {
        double refresh = nextDay(now);
        refreshOn(QEvent::Timer, refresh);
    }
    return new Integer(day, self->Position());
}


Integer *Widget::month(Tree *self, double t)
// ----------------------------------------------------------------------------
//    Return the month for time t or the current time if t < 0
// ----------------------------------------------------------------------------
{
    double tm = (t < 0) ? CurrentTime() : t;
    QDateTime now = fromSecsSinceEpoch(tm);
    int month = now.date().month();
    if (t < 0)
    {
        double refresh = nextDay(now);
        refreshOn(QEvent::Timer, refresh);
    }
    return new Integer(month, self->Position());
}


Integer *Widget::year(Tree *self, double t)
// ----------------------------------------------------------------------------
//    Return the year for time t or the current time if t < 0
// ----------------------------------------------------------------------------
{
    double tm = (t < 0) ? CurrentTime() : t;
    QDateTime now = fromSecsSinceEpoch(tm);
    int year = now.date().year();
    if (t < 0)
    {
        double refresh = nextDay(now);
        refreshOn(QEvent::Timer, refresh);
    }
    return new Integer(year, self->Position());
}


Real *Widget::time(Tree *self)
// ----------------------------------------------------------------------------
//   Return a fractional time, including milliseconds
// ----------------------------------------------------------------------------
{
    refreshOn(QEvent::Timer);
    if (animated)
        frozenTime = CurrentTime();
    return new Real(frozenTime, self->Position());
}


Real *Widget::pageTime(Tree *self)
// ----------------------------------------------------------------------------
//   Return a fractional time, including milliseconds
// ----------------------------------------------------------------------------
{
    refreshOn(QEvent::Timer);
    if (animated)
        frozenTime = CurrentTime();
    return new Real(frozenTime - pageStartTime, self->Position());
}


Integer *Widget::pageSeconds(Tree *self)
// ----------------------------------------------------------------------------
//   Return integral number of seconds
// ----------------------------------------------------------------------------
{
    double now = CurrentTime();
    if (animated)
        frozenTime = CurrentTime();
    refreshOn(QEvent::Timer, now + 1);
    return new Integer((longlong) (frozenTime - pageStartTime),
                       self->Position());
}


Real *Widget::after(Scope *scope, double delay, Tree *code)
// ----------------------------------------------------------------------------
//   Execute the given code only after the specified amount of time
// ----------------------------------------------------------------------------
{
    if (animated)
        frozenTime = CurrentTime();

    double now = frozenTime;
    double elapsed = now - pageStartTime;

    if (elapsed < delay)
    {
        refreshOn(QEvent::Timer, startTime + delay);
    }
    else
    {
        XL::Save<double> saveTime(startTime, startTime + delay);
        xl_evaluate(scope, code);
    }

    return new Real(elapsed);
}


Real *Widget::every(Scope *scope,
                    double interval, double duty,
                    Tree *code)
// ----------------------------------------------------------------------------
//   Execute the given code only after the specified amount of time
// ----------------------------------------------------------------------------
{
    if (animated)
        frozenTime = CurrentTime();

    double now = frozenTime;
    double elapsed = now - startTime;
    double active = fmod(elapsed, interval);
    double start = now - active;
    double delay = duty * interval;

    if (active > delay)
    {
        refreshOn(QEvent::Timer, start + interval);
    }
    else
    {
        XL::Save<double> saveTime(startTime, start);
        xl_evaluate(scope, code);
        refreshOn(QEvent::Timer, start + delay);
    }
    return new Real(elapsed);
}



Name *Widget::once(Scope *scope, Tree *self, Tree *prog)
// ----------------------------------------------------------------------------
//   Execute prog once (or after a full reload)
// ----------------------------------------------------------------------------
{
    // REVISIT: Move to XL runtime
    if (!self->GetInfo<ExecOnceInfo>())
    {
        self->SetInfo<ExecOnceInfo>(new ExecOnceInfo);
        xl_evaluate(scope, prog);
    }
    return XL::xl_false;
}


Real *Widget::mouseX(Tree *self)
// ----------------------------------------------------------------------------
//    Return the position of the mouse
// ----------------------------------------------------------------------------
{
    // Get on_mouseover / on_click coordinates if they are set (bug #937, #1013)
    if (mouseCoordinatesInfo)
        return new Real(mouseCoordinatesInfo->coordinates.x);
    refreshOn(QEvent::MouseMove);
    layout->Add(new RecordMouseCoordinates(self));
    if (CoordinatesInfo *info = self->GetInfo<CoordinatesInfo>())
        return new Real(info->coordinates.x);
    return new Real(0.0);
}


Real *Widget::mouseY(Tree *self)
// ----------------------------------------------------------------------------
//   Return the position of the mouse
// ----------------------------------------------------------------------------
{
    // Get on_mouseover / on_click coordinates if they are set (bug #937, #1013)
    if (mouseCoordinatesInfo)
        return new Real(mouseCoordinatesInfo->coordinates.y);
    refreshOn(QEvent::MouseMove);
    layout->Add(new RecordMouseCoordinates(self));
    if (CoordinatesInfo *info = self->GetInfo<CoordinatesInfo>())
        return new Real(info->coordinates.y);
    return new Real(0.0);
}


Integer *Widget::screenMouseX(Tree *self)
// ----------------------------------------------------------------------------
//    Return the X position of the mouse in screen (pixel) coordinates
// ----------------------------------------------------------------------------
{
    refreshOn(QEvent::MouseMove);
    QPoint m = QWidget::mapFromGlobal(QCursor::pos());
    lastMouseX = m.x();
    lastMouseY = m.y();
    return new Integer(lastMouseX);
}


Integer *Widget::screenMouseY(Tree *self)
// ----------------------------------------------------------------------------
//    Return the Y position of the mouse in screen (pixel) coordinates
// ----------------------------------------------------------------------------
{
    refreshOn(QEvent::MouseMove);
    QPoint m = QWidget::mapFromGlobal(QCursor::pos());
    lastMouseX = m.x();
    lastMouseY = m.y();
    return new Integer(lastMouseY);
}


Integer *Widget::mouseButtons(Tree *self)
// ----------------------------------------------------------------------------
//    Return the buttons of the last mouse event
// ----------------------------------------------------------------------------
{
    refreshOn(QEvent::MouseButtonPress);
    refreshOn(QEvent::MouseButtonRelease);
    return new Integer(lastMouseButtons);
}


Tree *Widget::shapeAction(Scope *scope,
                          Tree  *self,
                          text   name,
                          Tree  *action)
// ----------------------------------------------------------------------------
//   Set the action associated with a click or other on the object
// ----------------------------------------------------------------------------
{
    // First handle actions not related to a shape
    QString qname(+name);
    if (name == "pagechange")
    {
        if (!pageChangeActions.count(self))
            pageChangeActions[self] = ScopeAndCode(scope, action);
        return XL::xl_true;
    }
    else if ((+name).startsWith("keydown"))
    {
        refreshOn(QEvent::KeyPress);

        if (keyPressed)
        {
            QRegExp re(".*");
            if (qname.startsWith("keydown:"))
                re = QRegExp(qname.mid(qname.indexOf(':') + 1));

            QString qkeyName = +keyName;
            QString qkeyText = +keyText;
            QString qkeyEvent = +keyEventName;
            bool eval = (!qkeyEvent.isEmpty() && re.exactMatch(qkeyEvent)) ||
                (!qkeyName.isEmpty()  && re.exactMatch(qkeyName))  ||
                (!qkeyText.isEmpty()  && re.exactMatch(qkeyText));
            if (eval)
                return xl_evaluate(scope, action);
        }
    }
    else if ((+name).startsWith("keyup"))
    {
        refreshOn(QEvent::KeyRelease);

        if (!keyPressed)
        {
            QRegExp re(".*");
            if (qname.startsWith("keyup:"))
                re = QRegExp(qname.mid(qname.indexOf(':') + 1));

            QString qkeyName = +keyName;
            QString qkeyText = +keyText;
            QString qkeyEvent = (+keyEventName).mid(1); // Remove leading ~
            bool eval = (!qkeyEvent.isEmpty() && re.exactMatch(qkeyEvent)) ||
                (!qkeyName.isEmpty()  && re.exactMatch(qkeyName))  ||
                (!qkeyText.isEmpty()  && re.exactMatch(qkeyText));
            if (eval)
                return xl_evaluate(scope, action);
        }
    }
    else if ((+name).startsWith("key"))
    {
        refreshOn(QEvent::KeyPress);
        refreshOn(QEvent::KeyRelease);

        QRegExp re(".*");
        QString pattern = "";
        if (qname.startsWith("key:"))
        {
            pattern = qname.mid(qname.indexOf(':')+1);
            re = QRegExp(pattern);
        }

        QString qkeyName = +keyName;
        QString qkeyText = +keyText;
        if (!keyPressed)
        {
            if (!qkeyName.isEmpty())
                qkeyName = "~" + qkeyName;
            if (!qkeyText.isEmpty())
                qkeyText = "~" + qkeyText;
            // keyEvent already has the ~
        }
        bool eval =
            (!(+keyEventName).isEmpty() &&
             (pattern == +keyEventName || re.exactMatch(+keyEventName)))  ||
            (!qkeyName.isEmpty()    &&
             (pattern == qkeyName      || re.exactMatch(qkeyName)))       ||
            (!qkeyText.isEmpty()    &&
             (pattern == qkeyText      || re.exactMatch(qkeyText)));

        if (eval)
            return xl_evaluate(scope, action);
    }
    else if (name == "pageentry" && pageEntry == pageId)
    {
        return xl_evaluate(scope, action);
    }
    else if (name == "pageexit")
    {
        if (pageExit == pageId)
            return xl_evaluate(scope, action);
        pageHasExitHandler = true;
    }

    record(layout,
           "Register ation %s on layout %p id %u", name, layout, layout->id);
    if (name == "mouseover")
        refreshOn(QEvent::MouseMove);

    // Make sure we record proper mouse coordinates (bugs #1013 and #937)
    layout->Add (new RecordMouseCoordinates(action));
    actionMap[name][layout->id] = action;
    return XL::xl_true;
}


Tree *Widget::locally(Scope *scope, Tree *self, Tree *body)
// ----------------------------------------------------------------------------
//   Evaluate the body tree while preserving the current state
// ----------------------------------------------------------------------------
{
    Layout *childLayout = layout->AddChild(shapeId(), body, scope);
    XL::Save<Layout *> save(layout, childLayout);
    Tree *result = saveAndEvaluate(scope, body);
    return result;
}


Tree *Widget::shape(Scope *scope, Tree *self, Tree *body)
// ----------------------------------------------------------------------------
//   Evaluate the body and mark the current shape
// ----------------------------------------------------------------------------
{
    Layout *childLayout = layout->AddChild(selectionId(), body, scope);
    XL::Save<Layout *> saveLayout(layout, childLayout);
    XL::Save<Tree_p>   saveShape (currentShape, self);
    if (selectNextTime.count(self))
    {
        selection[id]++;
        selectNextTime.erase(self);
    }
    Tree *result = saveAndEvaluate(scope, body);
    return result;
}


Tree *Widget::activeWidget(Scope *scope, Tree *self, Tree *child)
// ----------------------------------------------------------------------------
//   Create a context for active widgets, e.g. buttons
// ----------------------------------------------------------------------------
//   We set currentShape to NULL, which means that we won't create manipulator
//   so the widget is active (it can be selected) but won't budge
{
    Layout *childLayout = layout->AddChild(selectionId(), child, scope);
    XL::Save<Layout *> saveLayout(layout, childLayout);
    XL::Save<Tree_p>   saveShape (currentShape, NULL);
    if (selectNextTime.count(self))
    {
        selection[id]++;
        selectNextTime.erase(self);
    }
    Tree *result = saveAndEvaluate(scope, child);
    return result;
}


Tree *Widget::anchor(Scope *scope, Tree *self, Tree *child, bool abs)
// ----------------------------------------------------------------------------
//   Anchor a set of shapes to the current position
// ----------------------------------------------------------------------------
{
    AnchorLayout *anchor = new AnchorLayout(this, abs);
    layout->AddChild(shapeId(), child, scope, anchor);
    record(layout, "Anchor %p id %d", anchor, anchor->id);
    XL::Save<Layout *> saveLayout(layout, anchor);
    if (selectNextTime.count(self))
    {
        selection[id]++;
        selectNextTime.erase(self);
    }
    Tree *result = saveAndEvaluate(scope, child);
    return result;
}


Tree *Widget::stereoViewpoints(Scope *scope, Tree *self,
                               Integer *viewpoints,Tree *child)
// ----------------------------------------------------------------------------
//   Create a layout that is only active for a given viewpoint
// ----------------------------------------------------------------------------
{
    ulong vpts = *viewpoints;
    Layout *childLayout = new StereoLayout(*layout, vpts);
    childLayout = layout->AddChild(shapeId(), child, scope, childLayout);
    XL::Save<Layout *> save(layout, childLayout);
    Tree *result = saveAndEvaluate(scope, child);
    return result;
}


Integer *Widget::stereoViewpoints()
// ----------------------------------------------------------------------------
//   The number of viewpoints for the current display
// ----------------------------------------------------------------------------
{
    refreshOn(displayDriver->viewPointsChangedEvent());
    return new Integer(Tao()->stereoPlanes);
}


Tree *Widget::resetTransform(Tree *self)
// ----------------------------------------------------------------------------
//   Reset transform to original projection state
// ----------------------------------------------------------------------------
{
    layout->Add(new ResetTransform());
    return XL::xl_false;
}


static inline Real *r(double x)
// ----------------------------------------------------------------------------
//   Utility shortcut to create a constant real value
// ----------------------------------------------------------------------------
{
    return new Real(x);
}


Tree *Widget::rotatex(Tree *self, Real *rx)
// ----------------------------------------------------------------------------
//   Rotate around X
// ----------------------------------------------------------------------------
{
    return rotate(self, rx, r(1), r(0), r(0));
}


Tree *Widget::rotatey(Tree *self, Real *ry)
// ----------------------------------------------------------------------------
//   Rotate around Y
// ----------------------------------------------------------------------------
{
    return rotate(self, ry, r(0), r(1), r(0));
}


Tree *Widget::rotatez(Tree *self, Real *rz)
// ----------------------------------------------------------------------------
//   Rotate around Z
// ----------------------------------------------------------------------------
{
    return rotate(self, rz, r(0), r(0), r(1));
}


Tree *Widget::rotate(Tree *self, Real *ra, Real *rx, Real *ry, Real *rz)
// ----------------------------------------------------------------------------
//    Rotation along an arbitrary axis
// ----------------------------------------------------------------------------
{
    // Check that matrix mode is modelview
    if (GL.matrixMode == GL_MODELVIEW)
        layout->model.Rotate(*ra, *rx, *ry, *rz);

    layout->Add(new Rotation(*ra, *rx, *ry, *rz));
    return XL::xl_true;
}


Tree *Widget::translatex(Tree *self, Real *x)
// ----------------------------------------------------------------------------
//   Translate along X
// ----------------------------------------------------------------------------
{
    return translate(self, x, r(0), r(0));
}


Tree *Widget::translatey(Tree *self, Real *y)
// ----------------------------------------------------------------------------
//   Translate along Y
// ----------------------------------------------------------------------------
{
    return translate(self, r(0), y, r(0));
}


Tree *Widget::translatez(Tree *self, Real *z)
// ----------------------------------------------------------------------------
//   Translate along Z
// ----------------------------------------------------------------------------
{
    return translate(self, r(0), r(0), z);
}


Tree *Widget::translate(Tree *self, Real *tx, Real *ty, Real *tz)
// ----------------------------------------------------------------------------
//     Translation along three axes
// ----------------------------------------------------------------------------
{
    // Check that matrix mode is modelview
    if (GL.matrixMode == GL_MODELVIEW)
        layout->model.Translate(*tx, *ty, *tz);

    layout->Add(new Translation(*tx, *ty, *tz));
    return XL::xl_true;
}


Tree *Widget::rescalex(Tree *self, Real *x)
// ----------------------------------------------------------------------------
//   Rescale along X
// ----------------------------------------------------------------------------
{
    return rescale(self, x, r(1), r(1));
}


Tree *Widget::rescaley(Tree *self, Real *y)
// ----------------------------------------------------------------------------
//   Rescale along Y
// ----------------------------------------------------------------------------
{
    return rescale(self, r(1), y, r(1));
}


Tree *Widget::rescalez(Tree *self, Real *z)
// ----------------------------------------------------------------------------
//   Rescale along Z
// ----------------------------------------------------------------------------
{
    return rescale(self, r(1), r(1), z);
}


Tree *Widget::rescale(Tree *self, Real *sx, Real *sy, Real *sz)
// ----------------------------------------------------------------------------
//     Scaling along three axes
// ----------------------------------------------------------------------------
{
    // Check that matrix mode is modelview
    if (GL.matrixMode == GL_MODELVIEW)
        layout->model.Scale(*sx, *sy, *sz);

    layout->Add(new Scale(*sx, *sy, *sz));
    return XL::xl_true;
}


Tree *Widget::clipPlane(Tree *self, int plane,
                        double a, double b, double c, double d)
// ----------------------------------------------------------------------------
//    Defining a clip plane
// ----------------------------------------------------------------------------
{
    layout->Add(new ClipPlane(plane, a, b, c, d));
    return XL::xl_true;
}


Tree *Widget::windowSize(Tree *self, Integer *width, Integer *height)
// ----------------------------------------------------------------------------
//   Resize the main widget to the specified size.
// ----------------------------------------------------------------------------
{
    QSize delta = QSize(width->value, height->value) - geometry().size();
    QWidget *win = taoWindow();
    win->resize(win->size() + delta);
    win->updateGeometry();
    return XL::xl_true;
}


static bool fuzzy_equal(kstring ref, kstring test)
// ----------------------------------------------------------------------------
//   Check if 'test' matches 'ref'
// ----------------------------------------------------------------------------
{
    for(;;)
    {
        char r = *ref++;
        char t = *test++;
        if (!r && !t)
            return true;
        if (!r || !t)
            return false;
        if (toupper(r) != toupper(t))
        {
            if (r == '_')
            {
                if (t == ' ' || t == '-')
                    continue;
                test--;
                continue;
            }
            return false;
        }
    }
    return false;               // Impossible
}


static GLenum TextToGLEnum(text t, GLenum e)
// ----------------------------------------------------------------------------
//   Compute the GL enum corresponding to the input text
// ----------------------------------------------------------------------------
{
    kstring s = t.c_str();

#define TEST_GLENUM(E) if (fuzzy_equal(#E, s)) e = GL_##E
    TEST_GLENUM(ZERO);
    TEST_GLENUM(ONE);
    TEST_GLENUM(DST_COLOR);
    TEST_GLENUM(ONE_MINUS_DST_COLOR);
    TEST_GLENUM(SRC_ALPHA);
    TEST_GLENUM(ONE_MINUS_SRC_ALPHA);
    TEST_GLENUM(DST_ALPHA);
    TEST_GLENUM(ONE_MINUS_DST_ALPHA);
    TEST_GLENUM(SRC_ALPHA_SATURATE);
    TEST_GLENUM(CONSTANT_COLOR);
    TEST_GLENUM(ONE_MINUS_CONSTANT_COLOR);
    TEST_GLENUM(CONSTANT_ALPHA);
    TEST_GLENUM(ONE_MINUS_CONSTANT_ALPHA);
    TEST_GLENUM(FUNC_ADD);
    TEST_GLENUM(FUNC_SUBTRACT);
    TEST_GLENUM(FUNC_REVERSE_SUBTRACT);
    TEST_GLENUM(MIN);
    TEST_GLENUM(MAX);
    TEST_GLENUM(MODULATE);
    TEST_GLENUM(REPLACE);
    TEST_GLENUM(DECAL);
    TEST_GLENUM(ADD);
    TEST_GLENUM(ALWAYS);
    TEST_GLENUM(NEVER);
    TEST_GLENUM(LESS);
    TEST_GLENUM(LEQUAL);
    TEST_GLENUM(EQUAL);
    TEST_GLENUM(GREATER);
    TEST_GLENUM(NOTEQUAL);
    TEST_GLENUM(GEQUAL);
    TEST_GLENUM(NEAREST);
    TEST_GLENUM(LINEAR);
    TEST_GLENUM(NEAREST_MIPMAP_NEAREST);
    TEST_GLENUM(LINEAR_MIPMAP_NEAREST);
    TEST_GLENUM(NEAREST_MIPMAP_LINEAR);
    TEST_GLENUM(LINEAR_MIPMAP_LINEAR);
#undef TEST_GLENUM

    return e;
}


Name *Widget::depthTest(Tree *self, bool enable)
// ----------------------------------------------------------------------------
//   Enable or disable OpenGL depth test
// ----------------------------------------------------------------------------
{
    layout->Add(new DepthTest(enable));
    return XL::xl_true;
}


Name *Widget::depthMask(Tree *self, bool enable)
// ----------------------------------------------------------------------------
//   Enable or disable OpenGL depth mask
// ----------------------------------------------------------------------------
{
    layout->Add(new DepthMask(enable));
    return XL::xl_true;
}


Name *Widget::depthFunction(Tree *self, text func)
// ----------------------------------------------------------------------------
//   Specifies the depth comparison function
// ----------------------------------------------------------------------------
{
    GLenum funcEnum = TextToGLEnum(func, GL_LESS);
    layout->Add(new DepthFunc(funcEnum));
    return XL::xl_true;
}


Name *Widget::blendFunction(Tree *self, text src, text dst)
// ----------------------------------------------------------------------------
//   Change the blend function
// ----------------------------------------------------------------------------
{
    GLenum srcEnum = TextToGLEnum(src, GL_SRC_ALPHA);
    GLenum dstEnum = TextToGLEnum(dst, GL_ONE_MINUS_SRC_ALPHA);
    layout->Add(new BlendFunc(srcEnum, dstEnum));
    return XL::xl_true;
}


Name *Widget::blendFunctionSeparate(Tree *self,
                                    text src, text dst,
                                    text srca, text dsta)
// ----------------------------------------------------------------------------
//   Change the blend function separately for color and alpha
// ----------------------------------------------------------------------------
{
    GLenum srcE  = TextToGLEnum(src, GL_SRC_ALPHA);
    GLenum dstE  = TextToGLEnum(dst, GL_ONE_MINUS_SRC_ALPHA);
    GLenum srcaE = TextToGLEnum(srca, GL_SRC_ALPHA);
    GLenum dstaE = TextToGLEnum(dsta, GL_ONE_MINUS_SRC_ALPHA);
    layout->Add(new BlendFuncSeparate(srcE, dstE, srcaE, dstaE));
    return XL::xl_true;
}


Name *Widget::blendEquation(Tree *self, text eq)
// ----------------------------------------------------------------------------
//   Change the blend equation
// ----------------------------------------------------------------------------
{
    GLenum eqE = TextToGLEnum(eq, GL_FUNC_ADD);
    layout->Add(new BlendEquation(eqE));
    return XL::xl_true;
}


Tree *Widget::refresh(Tree *self, double delay)
// ----------------------------------------------------------------------------
//    Refresh current layout after the given number of seconds
// ----------------------------------------------------------------------------
{
    refresh (delay);
    return XL::xl_true;
}


Tree *Widget::refreshOn(Tree *self, int eventType)
// ----------------------------------------------------------------------------
//    Refresh current layout on event
// ----------------------------------------------------------------------------
{
    refreshOn(eventType);
    return XL::xl_true;
}


Tree *Widget::noRefreshOn(Tree *self, int eventType)
// ----------------------------------------------------------------------------
//    Do NOT refresh current layout on event
// ----------------------------------------------------------------------------
{
    if (layout)
    {
        QEvent::Type type = (QEvent::Type) eventType;
        record(layout, "Request not to refresh %p id %d on event %+s",
               layout, layout->id, LayoutState::ToText(type));
        layout->NoRefreshOn(type);
    }

    return XL::xl_true;
}


Tree *Widget::defaultRefresh(Tree *self, double delay)
// ----------------------------------------------------------------------------
//    Get/set the default refresh time for layouts that depend on QEvent::Timer
// ----------------------------------------------------------------------------
{
    double prev = dfltRefresh;
    if (delay >= 0.0)
        dfltRefresh = delay;
    return new Real(prev);
}


struct LastRefreshInfo : XL::Info
// ----------------------------------------------------------------------------
//    Refresh information for refresh_time
// ----------------------------------------------------------------------------
{
    LastRefreshInfo(double last): last(last), refresh(0) {}
    double last, refresh;
};


Real *Widget::refreshTime(Tree *self)
// ----------------------------------------------------------------------------
//    Return time since last execution of the current layout, or 0
// ----------------------------------------------------------------------------
{
    double current = CurrentTime();
    LastRefreshInfo *info = self->GetInfo<LastRefreshInfo>();
    if (!info)
    {
        info = new LastRefreshInfo(current);
        self->SetInfo<LastRefreshInfo>(info);
    }

    double refresh = current - info->last;
    if (refresh > 0)
    {
        info->refresh = refresh;
        info->last = current;
    }
    else
    {
        refresh = info->refresh;
    }
    return new Real(refresh, self->Position());
}


Real *Widget::defaultCurveSteps(Tree *self, scale min, scale incr, scale max)
// ----------------------------------------------------------------------------
//   Set the drawing steps, returns previous one
// ----------------------------------------------------------------------------
{
    scale prev = GraphicPath::steps_min;
    if (min >= 0.0 && min < 20)
        GraphicPath::steps_min = min;
    if (incr >= 0.0 && incr < 20)
        GraphicPath::steps_increase = incr;
    if (max >= 2.0 && max < 30)
        GraphicPath::steps_max = max;
    return new Real(prev, self->Position());
}


double Widget::optimalDefaultRefresh()
// ----------------------------------------------------------------------------
//    Set default refresh for best results
// ----------------------------------------------------------------------------
{
#ifdef MACOSX_DISPLAYLINK
    // Refresh as fast as possible, but never more frequently than the current
    // display's refresh rate (due to the DisplayLink timer)
    return 0.0;
#endif
    if (VSyncSupported())
    {
        // Refresh as fast as possible. That is:
        // - If VSync is enabled: refresh rate will be limited by the display
        //   rate.
        // - If Vsync is supported but currently disabled: refresh will occur
        //   as fast as possible, not limited by the display rate.
        //   May be used for performance testing (how many fps can we achieve
        //   with this CPU/GPU?)
        return 0.0;
    }
    // Limit refresh rate to ~60 Hz when VSync is not supported, to avoid
    // uselessy taxing the CPU
    return 0.016;
}


Tree *Widget::postEvent(int eventType, bool once)
// ----------------------------------------------------------------------------
//    Post user event to this widget
// ----------------------------------------------------------------------------
{
    if (eventType < QEvent::User || eventType > QEvent::MaxUser)
        return XL::xl_false;
    if (once)
    {
        if (pendingEvents.count(eventType))
            return XL::xl_false;
        else
            pendingEvents.insert(eventType);
    }
    QEvent::Type type = (QEvent::Type) eventType;
    record(layout, "Post %+s in widget %p", LayoutState::ToText(type), this);
    qApp->postEvent(this, new QEvent(type));
    return XL::xl_true;
}


void Widget::postEventAPI(int eventType)
// ----------------------------------------------------------------------------
//    Export postEvent to the module API
// ----------------------------------------------------------------------------
{
    findTaoWidget()->postEvent(eventType);
}


bool Widget::postEventOnceAPI(int eventType)
// ----------------------------------------------------------------------------
//    Export postEvent to the module API. Return true if event was accepted.
// ----------------------------------------------------------------------------
{
    Tree *posted = findTaoWidget()->postEvent(eventType, true);
    return (posted == XL::xl_true);
}


Integer *Widget::registerUserEvent(text name)
// ----------------------------------------------------------------------------
//    Return an available Qt user event type for each name
// ----------------------------------------------------------------------------
{
    static std::map<text, int> user_events;

    if (!user_events.count(name))
    {
        int eventType = QEvent::registerEventType();
        user_events[name] = eventType;
        record(layout,
               "Registered Qt event type %d for user event %s",
               eventType, name);
    }
    return new Integer(user_events[name]);
}


Name *Widget::addLayoutName(text name)
// ----------------------------------------------------------------------------
//   Give a name to current layout
// ----------------------------------------------------------------------------
{
    layout->AddName(name);
    return XL::xl_true;
}


Name *Widget::refreshAlso(text name)
// ----------------------------------------------------------------------------
//   Layout 'name' shall be refreshed when current layout is refreshed
// ----------------------------------------------------------------------------
{
    layout->AddDep(name);
    return XL::xl_true;
}


#ifndef CFG_NOSRCEDIT

Name *Widget::showSource(Tree *self, bool show)
// ----------------------------------------------------------------------------
//   Show or hide source code
// ----------------------------------------------------------------------------
{
    bool old = taoWindow()->showSourceView(show);
    return old ? XL::xl_true : XL::xl_false;
}

#endif

Name *Widget::fullScreen(Tree *self, bool fs)
// ----------------------------------------------------------------------------
//   Switch to full screen
// ----------------------------------------------------------------------------
{
#ifdef Q_OS_MACX
    bFrameBufferReady = false;
#endif
    bool oldFs = isFullScreen();
    taoWindow()->switchToFullScreen(fs);
#ifdef MACOSX_DISPLAYLINK
    CVDisplayLinkSetCurrentCGDisplay(displayLink, getCurrentDisplayID(this));
#endif

    return oldFs ? XL::xl_true : XL::xl_false;
}


Name *Widget::toggleFullScreen(Tree *self)
// ----------------------------------------------------------------------------
//   Switch to full screen
// ----------------------------------------------------------------------------
{
    return fullScreen(self, !isFullScreen());
}


Name *Widget::toggleHandCursor(Tree *self)
// ----------------------------------------------------------------------------
//   Switch between hand and arrow cursor
// ----------------------------------------------------------------------------
{
    bool isArrow = (cursor().shape() == Qt::ArrowCursor);
    showHandCursor(isArrow);
    return (!isArrow) ? XL::xl_true : XL::xl_false;
}


Name *Widget::toggleAutoHideCursor(Tree *self)
// ----------------------------------------------------------------------------
//   Toggle auto-hide cursor mode
// ----------------------------------------------------------------------------
{
    return autoHideCursor(self, !bAutoHideCursor);
}


Name *Widget::autoHideCursor(Tree *self, bool ah)
// ----------------------------------------------------------------------------
//   Enable or disable auto-hiding of mouse cursor
// ----------------------------------------------------------------------------
{
    bool oldAutoHide = bAutoHideCursor;
    bAutoHideCursor = ah;
    if (ah)
    {
        QTimer::singleShot(2000, this, SLOT(hideCursor()));
    }
    else
    {
        setCursor(savedCursorShape);
        LayoutState::qevent_ids refreshEvents = space->RefreshEvents();
        bool mouseTracking = (refreshEvents.count(QEvent::MouseMove) != 0);
        if (!mouseTracking)
            setMouseTracking(false);
    }
    return oldAutoHide ? XL::xl_true : XL::xl_false;
}


Name *Widget::enableMouseCursor(Tree *self, bool on)
// ----------------------------------------------------------------------------
//   Enable or disable visibility of mouse cursor
// ----------------------------------------------------------------------------
{
    bool old = !mouseCursorHidden;
    if (on != old)
    {
        mouseCursorHidden = !on;
        if (mouseCursorHidden)
        {
            cachedCursor = QWidget::cursor();
            QWidget::setCursor(Qt::BlankCursor);
        }
        else
        {
            QWidget::setCursor(cachedCursor);
            cachedCursor = Qt::BlankCursor;
        }
    }
    return old ? XL::xl_true : XL::xl_false;
}


Name *Widget::showStatistics(Tree *self, bool ss)
// ----------------------------------------------------------------------------
//   Display or hide performance statistics (frames per second)
// ----------------------------------------------------------------------------
{
    bool prev = stats.enable(ss, Statistics::TO_SCREEN);
    return prev ? XL::xl_true : XL::xl_false;
}


Name *Widget::toggleShowStatistics(Tree *self)
// ----------------------------------------------------------------------------
//   Toggle display of statistics
// ----------------------------------------------------------------------------
{
    return showStatistics(self, !stats.isEnabled(Statistics::TO_SCREEN));
}


Name *Widget::logStatistics(Tree *self, bool ss)
// ----------------------------------------------------------------------------
//   Enable or disable logging of performance statistics (frames per second)
// ----------------------------------------------------------------------------
{
    if (ss)
    {
        std::cout.setf(std::ios::fixed, std::ios::floatfield);
        std::cout.setf(std::ios::showpoint);
        std::cout.precision(3);
    }
    bool prev = stats.enable(ss, Statistics::TO_CONSOLE);
    return prev ? XL::xl_true : XL::xl_false;
}


Name *Widget::toggleLogStatistics(Tree *self)
// ----------------------------------------------------------------------------
//   Toggle logging of performance statistics
// ----------------------------------------------------------------------------
{
    return logStatistics(self, !stats.isEnabled(Statistics::TO_CONSOLE));
}


Integer *Widget::frameCount(Tree *self)
// ----------------------------------------------------------------------------
//   The number of frames displayed so far for the current document
// ----------------------------------------------------------------------------
{
    return new Integer(frameCounter);
}


Name *Widget::slideShow(Tree *self, bool ss)
// ----------------------------------------------------------------------------
//   Switch to slide show mode
// ----------------------------------------------------------------------------
{
#ifdef Q_OS_MACX
    bFrameBufferReady = false;
#endif
    bool oldMode = taoWindow()->switchToSlideShow(ss);
    return oldMode ? XL::xl_true : XL::xl_false;
}


Name *Widget::toggleSlideShow(Tree *self)
// ----------------------------------------------------------------------------
//   Toggle slide show mode
// ----------------------------------------------------------------------------
{
#ifdef Q_OS_MACX
    bFrameBufferReady = false;
#endif
    bool oldMode = taoWindow()->toggleSlideShow();
    return oldMode ? XL::xl_true : XL::xl_false;
}


Name *Widget::blankScreen(Tree *self, bool bs)
// ----------------------------------------------------------------------------
//   Blank screen or restore normal display
// ----------------------------------------------------------------------------
{
    bool oldMode = blanked;
    blanked = bs;
    return oldMode ? XL::xl_true : XL::xl_false;
}


Name *Widget::toggleBlankScreen(Tree *self)
// ----------------------------------------------------------------------------
//   Toggle blank screen
// ----------------------------------------------------------------------------
{
    return blankScreen(self, !blanked);
}


Window * Widget::taoWindow()
// ----------------------------------------------------------------------------
//   Return a pointer to the Tao main window this widget belongs to
// ----------------------------------------------------------------------------
{
    QWidget * p = parentWidget();
    XL_ASSERT(p || !"Widget has no parent widget");
    QWidget *pp = p->parentWidget();
    XL_ASSERT(pp || !"Widget has no grandparent widget");
    Window * w = dynamic_cast<Window *>(pp);
    XL_ASSERT(w || !"Widget grandparent is not QMainWindow");
    return w;
}


Widget::StereoIdentTexture Widget::newStereoIdentTexture(int i)
// ----------------------------------------------------------------------------
//   Create texture to identify viewpoint: big number centered on texture
// ----------------------------------------------------------------------------
{
    int w = 400, h = 400;
    QImage image(w, h, QImage::Format_ARGB32);
    enum { Red = 0xFF770000, Green = 0xFF007700 };
    // Background color:

    // Left eye/odd viewpoint: red, right eye/even viewpoint: green.
    if (i % 2)
        image.fill(Red);
    else
        image.fill(Green);
    QPainter painter;
    painter.begin(&image);
    QPainterPath path;
    QString t = QString("%1").arg(i);
    path.addText(0, 0, QFont("Ubuntu", 200), t);
    QRectF brect = path.boundingRect();
    path.translate((w - brect.width())/2, (h + brect.height())/2);
    painter.setBrush(QBrush(Qt::white));
    QPen pen(Qt::black);
    pen.setWidth(1);
    painter.setPen(pen);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawPath(path);
    painter.end();

    // Generate the GL texture
    QImage texture = QGLWidget::convertToGLFormat(image);
    GLuint tex;
    GL.GenTextures(1, &tex);
    GL.BindTexture(GL_TEXTURE_2D, tex);
    GL.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                  w, h, 0, GL_RGBA,
                  GL_UNSIGNED_BYTE, texture.bits());

    IFTRACE(fileload)
        std::cerr << "Created stereo identification texture: viewpoint #"
                  << i << ", texture id " << tex << "\n";
    return StereoIdentTexture(w, h, tex);
}


void Widget::updateStereoIdentPatterns(int nb)
// ----------------------------------------------------------------------------
//   Create or delete textures used for stereoscopic identification
// ----------------------------------------------------------------------------
{
    int size = stereoIdentPatterns.size();
    if (nb == 0)
    {
        for (int i = 0; i < size; i++)
        {
            GLuint tex = stereoIdentPatterns[i].tex;
            GL.DeleteTextures(1, &tex);
            IFTRACE(fileload)
                std::cerr << "Deleted texture #" << tex
                          << " (stereo identification for viewpoint #"
                          << i+1 << ")\n";
        }
        stereoIdentPatterns.clear();
        return;
    }
    else
    {
        if (nb > size)
            for (int i = size ; i < nb; i++)
                stereoIdentPatterns.push_back(newStereoIdentTexture(i+1));
    }
}


Name *Widget::stereoIdentify(Tree *self, bool on)
// ----------------------------------------------------------------------------
//   Enable or disable stereoscopic test pattern
// ----------------------------------------------------------------------------
{
    bool oldMode = stereoIdent;

    if (oldMode != on)
    {
        stereoIdent = on;
        if (!stereoIdent)
            updateStereoIdentPatterns(0);
    }

    return oldMode ? XL::xl_true : XL::xl_false;
}


Name *Widget::toggleStereoIdentify(Tree *self)
// ----------------------------------------------------------------------------
//   Toggle between stereoscopic test pattern and normal output
// ----------------------------------------------------------------------------
{
    return stereoIdentify(self, !stereoIdent);
}


Name *Widget::resetViewAndRefresh(Tree *self)
// ----------------------------------------------------------------------------
//   Restore default view parameters (zoom, position etc.)
// ----------------------------------------------------------------------------
{
    resetViewAndRefresh();
    return XL::xl_true;
}


Name *Widget::panView(Tree *self, coord dx, coord dy)
// ----------------------------------------------------------------------------
//   Pan the current view by the current amount
// ----------------------------------------------------------------------------
{
    cameraPosition.x += dx;
    cameraPosition.y += dy;
    cameraTarget.x += dx;
    cameraTarget.y += dy;
    return XL::xl_true;
}


Real *Widget::currentZoom(Tree *self)
// ----------------------------------------------------------------------------
//   Return the current zoom level
// ----------------------------------------------------------------------------
{
    return new Real(zoom);
}


Name *Widget::setZoom(Tree *self, scale z)
// ----------------------------------------------------------------------------
//   Decrease zoom level
// ----------------------------------------------------------------------------
{
    if (z > 0)
    {
        zoom = z;
        return XL::xl_true;
    }
    return XL::xl_false;
}


Real *Widget::currentScaling(Tree *self)
// ----------------------------------------------------------------------------
//   Return the current scaling level
// ----------------------------------------------------------------------------
{
    return new Real(scaling);
}


Name *Widget::setScaling(Tree *self, scale s)
// ----------------------------------------------------------------------------
//   Decrease scaling level
// ----------------------------------------------------------------------------
{
    if (s > 0)
    {
        scaling = s;
        return XL::xl_true;
    }
    return XL::xl_false;
}


template<typename TreeType,
         typename ValueType = typename TreeType::value_type>
static Tree *xl_item_list(Tree *self, uint n, ValueType *values)
// ----------------------------------------------------------------------------
//   Build an infix from a list of values
// ----------------------------------------------------------------------------
{
    struct ListCache : Info
    {
        ListCache(Tree *list, uint size): list(list), size(size) {}
        Tree_p      list;
        uint        size;
    };

    ListCache *saved = self->GetInfo<ListCache>();
    if (saved && saved->size == n)
    {
        // We already generated a list, just update it in place
        Tree *list = saved->list;
        while (Infix *infix = list->AsInfix())
        {
            TreeType *left = infix->left->As<TreeType>();
            list = infix->right;
            left->value = *values++;
        }
        TreeType *last = list->As<TreeType>();
        last->value = *values++;
        return saved->list;
    }

    Tree *result = nullptr;
    uint size = n;
    while (n)
    {
        Tree *left = new TreeType(values[--n], self->Position());
        if (result)
            result = new Infix(",", left, result, self->Position());
        else
            result = left;
    }
    if (!saved)
        self->SetInfo(new ListCache(result, size));
    else
        saved->list = result, saved->size = size;
    return result;
}


static inline Tree *xl_real_list(Tree *self, uint n, double *values)
// ----------------------------------------------------------------------------
//   The case for real numbers
// ----------------------------------------------------------------------------
{
    return xl_item_list<Real>(self, n, values);
}


static inline Tree *xl_integer_list(Tree *self, uint n, longlong *values)
// ----------------------------------------------------------------------------
//   The case for integer values
// ----------------------------------------------------------------------------
{
    return xl_item_list<Integer>(self, n, values);
}


Infix *Widget::currentCameraPosition(Tree *self)
// ----------------------------------------------------------------------------
//   Return the current camera position
// ----------------------------------------------------------------------------
{
    Tree *result = xl_real_list(self, 3, &cameraPosition.x);
    return result->AsInfix();
}


Name *Widget::setCameraPosition(Tree *self, coord x, coord y, coord z)
// ----------------------------------------------------------------------------
//   Set the cameraPosition position and update view
// ----------------------------------------------------------------------------
{
    if (cameraPosition.x!=x || cameraPosition.y!=y || cameraPosition.z!=z)
    {
        cameraPosition.x = x;
        cameraPosition.y = y;
        cameraPosition.z = z;
    }
    return XL::xl_true;
}


Infix *Widget::currentCameraTarget(Tree *self)
// ----------------------------------------------------------------------------
//   Return the current center position
// ----------------------------------------------------------------------------
{
    Tree *result = xl_real_list(self, 3, &cameraTarget.x);
    return result->AsInfix();
}


Name *Widget::setCameraTarget(Tree *self, coord x, coord y, coord z)
// ----------------------------------------------------------------------------
//   Set the center position and update view
// ----------------------------------------------------------------------------
{
    if (x != cameraTarget.x || y != cameraTarget.y || z != cameraTarget.z)
    {
        cameraTarget.x = x;
        cameraTarget.y = y;
        cameraTarget.z = z;
        refresh(0);
    }
    return XL::xl_true;
}


Infix *Widget::currentCameraUpVector(Tree *self)
// ----------------------------------------------------------------------------
//   Return the current up vector
// ----------------------------------------------------------------------------
{
    Tree *result = xl_real_list(self, 3, &cameraUpVector.x);
    return result->AsInfix();
}


Name *Widget::setCameraUpVector(Tree *self, coord x, coord y, coord z)
// ----------------------------------------------------------------------------
//   Set the up vector
// ----------------------------------------------------------------------------
{
    if (cameraUpVector.x!=x || cameraUpVector.y!=y || cameraUpVector.z!=z)
    {
        cameraUpVector.x = x;
        cameraUpVector.y = y;
        cameraUpVector.z = z;
        refresh(0);
    }
    return XL::xl_true;
}


Name *Widget::setEyeDistance(Tree *self, double eyeD)
// ----------------------------------------------------------------------------
//   Set the distance between the eyes for stereoscopy
// ----------------------------------------------------------------------------
{
    eyeDistance = eyeD;
    return XL::xl_true;
}


Real *Widget::getEyeDistance(Tree *self)
// ----------------------------------------------------------------------------
//    Get the distance between the eyse for stereoscopy
// ----------------------------------------------------------------------------
{
    return new Real(eyeDistance);
}


Name *Widget::setZNear(Tree *self, double zn)
// ----------------------------------------------------------------------------
//   Set the nearest position for OpenGL
// ----------------------------------------------------------------------------
{
    zNear = zn;
    return XL::xl_true;
}


Real *Widget::getZNear(Tree *self)
// ----------------------------------------------------------------------------
//   Get the nearest position for OpenGL
// ----------------------------------------------------------------------------
{
    return new Real(zNear);
}


Name *Widget::setZFar(Tree *self, double zf)
// ----------------------------------------------------------------------------
//   Set the farthest position for OpenGL
// ----------------------------------------------------------------------------
{
    zFar = zf;
    return XL::xl_true;
}


Real *Widget::getZFar(Tree *self)
// ----------------------------------------------------------------------------
//   Get the nearest position for OpenGL
// ----------------------------------------------------------------------------
{
    return new Real(zFar);
}


Name *Widget::setCameraToScreen(Tree *self, double d)
// ----------------------------------------------------------------------------
//   Set the distance between camera and screen
// ----------------------------------------------------------------------------
{
    cameraToScreen = d;
    return XL::xl_true;
}


Real *Widget::getCameraToScreen(Tree *self)
// ----------------------------------------------------------------------------
//   Get the distance between camera and screen
// ----------------------------------------------------------------------------
{
    return new Real(cameraToScreen);
}


Infix *Widget::currentModelMatrix(Tree *self)
// ----------------------------------------------------------------------------
//   Return the current model matrix converting from object to world space
// ----------------------------------------------------------------------------
{
    Tree *result = xl_real_list(self, 16, layout->model.Data(false));
    return result->AsInfix();
}

Integer *Widget::lastModifiers(Tree *self)
// ----------------------------------------------------------------------------
//   Return the current modifiers
// ----------------------------------------------------------------------------
{
    refreshOn(QEvent::KeyPress);
    refreshOn(QEvent::KeyRelease);
    return new Integer(keyboardModifiers);
}


Name *Widget::enableAnimations(Tree *self, bool fs)
// ----------------------------------------------------------------------------
//   Enable or disable animations
// ----------------------------------------------------------------------------
{
    bool oldFs = hasAnimations();
    if (oldFs != fs)
        taoWindow()->toggleAnimations();
    return oldFs ? XL::xl_true : XL::xl_false;
}


Name *Widget::enableSelectionRectangle(Tree *self, bool sre)
// ----------------------------------------------------------------------------
//   Enable or disable selection rectangle
// ----------------------------------------------------------------------------
{
    bool old = selectionRectangleEnabled;
    selectionRectangleEnabled = sre;
    return old ? XL::xl_true : XL::xl_false;
}


Name *Widget::setDisplayMode(Tree *self, text name)
// ----------------------------------------------------------------------------
//   Select a display function
// ----------------------------------------------------------------------------
{
    TaoSave saveCurrent(current, this); // #1223
    QString newName = +name;
    if (displayDriver->isCurrentDisplayFunctionSameAs(newName))
        return XL::xl_true;
    bool ok = displayDriver->setDisplayFunction(newName);
    if (ok)
    {
        emit displayModeChanged(newName);
        updateGL();
        return XL::xl_true;
    }
    return XL::xl_false;
}


Name *Widget::addDisplayModeToMenu(Tree *self, text mode, text label)
// ----------------------------------------------------------------------------
//   Add a display mode entry to the view menu
// ----------------------------------------------------------------------------
{
    taoWindow()->addDisplayModeMenu(+mode, +label);
    return XL::xl_true;
}


Name *Widget::enableStereoscopy(Tree *self, Name *name)
// ----------------------------------------------------------------------------
//   Enable or disable stereoscopic mode
// ----------------------------------------------------------------------------
{
    text n = name->value;
    if (name ==  XL::xl_false)
        n = "false";
    else if (name ==  XL::xl_true)
        n = "true";
    return enableStereoscopyText(self, n);
}


Name *Widget::enableStereoscopyText(Tree *self, text name)
// ----------------------------------------------------------------------------
//   Enable or disable stereoscopic mode
// ----------------------------------------------------------------------------
{
    bool oldState = !displayDriver->isCurrentDisplayFunctionSameAs("2D");
    text newState = name;

    // Testing on/off values and legacy names
    if (name == "false" || name == "no" || name == "none")
    {
        newState = "2D";
    }
    else if (name == "true" || name == "hardware")
    {
        newState = "quadstereo";
    }
    else if (name == "hsplit")
    {
        newState = "hsplitstereo";
    }
    else if (name == "vsplit")
    {
        newState = "vsplitstereo";
    }
    else if (name == "interlace" || name == "interlaced" ||
             name == "interleave" || name == "interleaved")
    {
        newState = "hintstereo";
    }

    if (hasDisplayModeText(NULL, newState))
        setDisplayMode(NULL, newState);
    else
        std::cerr << "Stereoscopy mode " << name << " is unknown\n";
    return oldState ? XL::xl_true : XL::xl_false;
}


Integer * Widget::polygonOffset(Tree *self,
                                double f0, double f1,
                                double u0, double u1)
// ----------------------------------------------------------------------------
//   Set the polygon offset factors
// ----------------------------------------------------------------------------
{
    Layout::factorBase = f0;
    Layout::factorIncrement = f1;
    Layout::unitBase = u0;
    Layout::unitIncrement = u1;
    return new Integer(Layout::polygonOffset);
}


Name *Widget::enableVSync(Tree *self, bool enable)
// ----------------------------------------------------------------------------
//   Enable or disable vertical sync (prevent tearing), return previous state
// ----------------------------------------------------------------------------
{
    bool prev = VSyncEnabled();

#if defined(Q_OS_MACX)
    if (enable != prev)
    {
        const GLint swapInterval = enable ? 1 : 0;
        CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &swapInterval);
    }
#elif defined(Q_OS_WIN)
    typedef BOOL (*set_fn_t) (int interval);
    static
        set_fn_t set_fn = (set_fn_t) wglGetProcAddress("wglSwapIntervalEXT");
    if (set_fn) {
        if (enable != prev)
        {
            int swapInterval = enable ? 1 : 0;
            set_fn(swapInterval);
        }
    }
#elif defined(Q_OS_LINUX)
    if (enableVSyncX11(this, enable))
        vsyncState = enable;
#else
    // Command not supported, but do it silently
#endif

    return prev ? XL::xl_true : XL::xl_false;
}


bool Widget::VSyncEnabled()
// ----------------------------------------------------------------------------
//   Return true if vsync is enabled, false otherwise
// ----------------------------------------------------------------------------
{
    bool enabled = false;

#if defined(Q_OS_MACX)
    GLint old = 0;
    CGLGetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &old);
    enabled = (old != 0);
#elif defined(Q_OS_WIN)
    typedef int  (*get_fn_t) (void);
    static
        get_fn_t get_fn = (get_fn_t) wglGetProcAddress("wglGetSwapIntervalEXT");
    int old = 0;
    if (get_fn)
        old = get_fn();
    enabled = (old != 0);
#elif defined(Q_OS_LINUX)
    enabled = vsyncState;
#endif

    return enabled;
}


bool Widget::VSyncSupported()
// ----------------------------------------------------------------------------
//   Return true if vsync can be set (enableVSync() works), false otherwise
// ----------------------------------------------------------------------------
{
    static bool supported = false, tested = false;
    if (!tested)
    {
        bool prev = (enableVSync(NULL, false) == XL::xl_true);
        bool f = (enableVSync(NULL, true) == XL::xl_true);
        bool t = (enableVSync(NULL, prev) == XL::xl_true);
        supported = (!f && t);
        tested = true;
        IFTRACE(displaymode)
            std::cerr << "Setting VSync is " << (supported ? "" : "NOT ")
                      << "supported\n";
    }
    return supported;
}


QColor Widget::colorByName(text name, double alpha)
// ----------------------------------------------------------------------------
//    Return a color by name, or black if the color is invalid
// ----------------------------------------------------------------------------
{
    if (QColor::isValidColor(+name))
    {
        QColor result(+name);
        alpha *= result.alphaF();
        CHECK_0_1_RANGE(alpha);
        result.setAlphaF(alpha);
        return result;
    }
    kstring cname = name.c_str();

    // RGB model
    int r, g, b, a;
    if (sscanf(cname, "rgb(%d,%d,%d)", &r, &g, &b) == 3)
    {
        CHECK_0_255_RANGE(r);
        CHECK_0_255_RANGE(g);
        CHECK_0_255_RANGE(b);
        return QColor::fromRgb(r, g, b, int(255 * alpha));
    }
    if (sscanf(cname, "rgba(%d,%d,%d,%d)", &r, &g, &b, &a) == 4)
    {
        CHECK_0_255_RANGE(r);
        CHECK_0_255_RANGE(g);
        CHECK_0_255_RANGE(b);
        CHECK_0_255_RANGE(a);
        return QColor::fromRgb(r, g, b, int(a * alpha));
    }

    // HSV model
    int h, s, l, v;
    if (sscanf(cname, "hsv(%d,%d,%d)", &h, &s, &v) == 3)
    {
        CHECK_0_359_RANGE(h);
        CHECK_0_255_RANGE(s);
        CHECK_0_255_RANGE(v);
        return QColor::fromHsv(h, s, v, int(255 * alpha));
    }
    if (sscanf(cname, "hsva(%d,%d,%d,%d)", &h, &s, &v, &a) == 4)
    {
        CHECK_0_359_RANGE(h);
        CHECK_0_255_RANGE(s);
        CHECK_0_255_RANGE(v);
        CHECK_0_255_RANGE(a);
        return QColor::fromHsv(h, s, v, int (a * alpha));
    }

    // HSL model
    if (sscanf(cname, "hsl(%d,%d,%d)", &h, &s, &l) == 3)
    {
        CHECK_0_359_RANGE(h);
        CHECK_0_255_RANGE(s);
        CHECK_0_255_RANGE(l);
        return QColor::fromHsl(h, s, l, int(255 * alpha));
    }
    if (sscanf(cname, "hsla(%d,%d,%d,%d)", &h, &s, &l, &a) == 4)
    {
        CHECK_0_359_RANGE(h);
        CHECK_0_255_RANGE(s);
        CHECK_0_255_RANGE(l);
        CHECK_0_255_RANGE(a);
        return QColor::fromHsl(h, s, l, int (a * alpha));
    }

    return QColor(0.0, 0.0, 0.0, alpha);
}


QColor Widget::colorRGB(scale r, scale g, scale b, scale a)
// ----------------------------------------------------------------------------
//   Compute an RGB color
// ----------------------------------------------------------------------------
{
    CHECK_0_1_RANGE(r);
    CHECK_0_1_RANGE(g);
    CHECK_0_1_RANGE(b);
    CHECK_0_1_RANGE(a);
    return QColor::fromRgbF(r, g, b, a);
}


QColor Widget::colorHSV(scale h, scale s, scale v, scale a)
// ----------------------------------------------------------------------------
//   Compute an HSV color
// ----------------------------------------------------------------------------
{
    h /= 360;
    CHECK_0_1_RANGE_MODULO(h);
    CHECK_0_1_RANGE(s);
    CHECK_0_1_RANGE(v);
    CHECK_0_1_RANGE(a);
    return QColor::fromHsvF(h, s, v, a);
}


QColor Widget::colorHSL(scale h, scale s, scale l, scale a)
// ----------------------------------------------------------------------------
//   Compute an HSL color
// ----------------------------------------------------------------------------
{
    h /= 360;
    CHECK_0_1_RANGE_MODULO(h);
    CHECK_0_1_RANGE(s);
    CHECK_0_1_RANGE(l);
    CHECK_0_1_RANGE(a);
    return QColor::fromHslF(h, s, l, a);
}


QColor Widget::colorCMYK(scale c, scale m, scale y, scale k, scale a)
// ----------------------------------------------------------------------------
//   Compute a CMYK color
// ----------------------------------------------------------------------------
{
    CHECK_0_1_RANGE(c);
    CHECK_0_1_RANGE(m);
    CHECK_0_1_RANGE(y);
    CHECK_0_1_RANGE(k);
    CHECK_0_1_RANGE(a);
    return QColor::fromCmykF(c, m, y, k, a);
}


Tree *Widget::clearColor(Tree *self, QColor c)
// ----------------------------------------------------------------------------
//    Set the clear (background) color for current FrameInfo or for the Widget
// ----------------------------------------------------------------------------
{
    if (frameInfo)
    {
        frameInfo->clearColor.Set(c.redF(), c.greenF(), c.blueF(), c.alphaF());
    }
    else
    {
        clearCol = c;
        if (c.alphaF() < 1.0)
            taoWindow()->makeWindowTransparent();
    }

    return XL::xl_true;
}


Tree *Widget::fillColor(Tree *self, QColor c)
// ----------------------------------------------------------------------------
//    Set the fill color
// ----------------------------------------------------------------------------
{
    layout->Add(new FillColor(c.redF(), c.greenF(), c.blueF(), c.alphaF()));
    return XL::xl_true;
}


Tree *Widget::lineColor(Tree *self, QColor c)
// ----------------------------------------------------------------------------
//    Set the named color for lines
// ----------------------------------------------------------------------------
{
    layout->Add(new LineColor(c.redF(), c.greenF(), c.blueF(), c.alphaF()));
    return XL::xl_true;
}


Tree * Widget::gradientColor(Tree *self, double pos, QColor c)
// ----------------------------------------------------------------------------
//   Define a step of a color gradient
// ----------------------------------------------------------------------------
{
    if (!gradient)
    {
        Ooops("No gradient defined for $1", self);
        return XL::xl_false;
    }

    gradient->setColorAt(pos, c);
    return XL::xl_true;
}


Tree *Widget::motionBlur(Tree *self, double f)
// ----------------------------------------------------------------------------
//    Set the motion blur factor
// ----------------------------------------------------------------------------
{
    CHECK_0_1_RANGE(f);
    blurFactor = f;
    return XL::xl_true;
}


Tree *Widget::visibility(Tree *self, double v)
// ----------------------------------------------------------------------------
//    Select the visibility amount
// ----------------------------------------------------------------------------
{
    layout->Add(new Visibility(v));
    return XL::xl_true;
}


Tree *Widget::lineWidth(Tree *self, double lw)
// ----------------------------------------------------------------------------
//    Select the line width for OpenGL
// ----------------------------------------------------------------------------
{
    layout->Add(new LineWidth(lw));
    return XL::xl_true;
}


Tree *Widget::lineStipple(Tree *self, uint16 pattern, uint16 scale)
// ----------------------------------------------------------------------------
//    Select the line stipple pattern for OpenGL
// ----------------------------------------------------------------------------
{
    layout->Add(new LineStipple(pattern, scale));
    return XL::xl_true;
}


Integer* Widget::fillTextureUnit(Tree *self, GLuint texUnit)
// ----------------------------------------------------------------------------
//     Build a GL texture out of an id
// ----------------------------------------------------------------------------
{
    if (texUnit > GL.MaxTextureUnits())
    {
        Ooops("Invalid texture unit for $1", self);
        return new Integer(0, self->Position());
    }

    layout->Add(new TextureUnit(texUnit));
    return new Integer(texUnit);
}


Integer* Widget::fillTextureId(Tree *self, GLuint texId)
// ----------------------------------------------------------------------------
//     Build a GL texture out of an id
// ----------------------------------------------------------------------------
{
    if (!GL.currentTextures.textures.count(texId))
        return new Integer(-1, self->Position());

    // Get corresponding texture type
    GLenum type = GL.currentTextures.textures[texId].type;
    layout->Add(new FillTexture(texId, type));
    return new Integer(texId, self->Position());
}


Integer* Widget::fillTexture(Scope *scope, Tree *self, text img)
// ----------------------------------------------------------------------------
//     Build a GL texture out of an image file
// ----------------------------------------------------------------------------
{
    uint texId = 0;

    refreshOn(TextureCache::instance()->textureChangedEvent());

    if (img != "")
    {
        img  = resolvePrefixedPath(scope, img);

        text docPath = +taoWindow()->currentProjectFolderPath();
        CachedTexture *t = TextureCache::instance()->load(+img, +docPath);
        layout->Add(new FillTexture(t->id, GL_TEXTURE_2D));
        GL.TextureSize(t->width, t->height);
        texId = t->id;
    }
    else
    {
        layout->Add(new FillTexture(texId, GL_TEXTURE_2D));
        GL.TextureSize(0, 0);
    }

    return new Integer(texId, self->Position());
}


Integer* Widget::fillAnimatedTexture(Scope *scope, Tree *self, text img)
// ----------------------------------------------------------------------------
//     Build a GL texture out of a movie file
// ----------------------------------------------------------------------------
{
    uint texId = 0;

    refreshOn(QEvent::Timer);

    if (img != "")
    {
        img = resolvePrefixedPath(scope, img);

        AnimatedTextureInfo *rinfo = self->GetInfo<AnimatedTextureInfo>();
        if (!rinfo)
        {
            rinfo = new AnimatedTextureInfo();
            self->SetInfo<AnimatedTextureInfo>(rinfo);
        }
        layout->Add(new FillTexture(rinfo->bind(img), GL_TEXTURE_2D));
        GL.TextureSize(rinfo->width, rinfo->height);
    }
    else
    {
        layout->Add(new FillTexture(texId));
        GL.TextureSize(0, 0);
    }

    return new Integer(texId, self->Position());
}


Integer* Widget::fillTextureFromSVG(Scope *scope, Tree *self, text img)
// ----------------------------------------------------------------------------
//    Draw an image in SVG format
// ----------------------------------------------------------------------------
//    The image may be animated, in which case we will get repaintNeeded()
//    signals that we send to our 'draw()' so that we redraw as needed.
{
    uint texId = 0;

    refreshOn(QEvent::Timer);

    if (img != "")
    {
        img = resolvePrefixedPath(scope, img);

        SvgRendererInfo *rinfo = self->GetInfo<SvgRendererInfo>();
        if (!rinfo)
        {
            rinfo = new SvgRendererInfo(this);
            self->SetInfo<SvgRendererInfo>(rinfo);
        }

        layout->Add(new FillTexture(rinfo->bind(img), GL_TEXTURE_2D));
        GL.TextureSize(rinfo->w, rinfo->h);
    }
    else
    {
        layout->Add(new FillTexture(texId));
        GL.TextureSize(0, 0);
    }

    return new Integer(texId, self->Position());
}


Integer* Widget::image(Scope *scope,
                       Tree *self, Real *x, Real *y, Real *sxp, Real *syp,
                       text filename)
//----------------------------------------------------------------------------
//  Make an image
//----------------------------------------------------------------------------
//  If w or h is 0 then the image width or height is used and assigned to it.
{
    refreshOn(TextureCache::instance()->textureChangedEvent());

    filename = resolvePrefixedPath(scope, filename);

    XL::Save<Layout *> saveLayout(layout, layout->AddChild(shapeId()));
    double sx = sxp->value;
    double sy = syp->value;

    text docPath = +taoWindow()->currentProjectFolderPath();
    CachedTexture *t = TextureCache::instance()->load(+filename, +docPath);
    layout->Add(new FillTexture(t->id, GL_TEXTURE_2D));
    GL.TextureSize(t->width, t->height);

    double w0 = t->width;
    double h0 = t->height;
    double w = w0 * sx;
    double h = h0 * sy;

    Rectangle shape(Box(*x-w/2, *y-h/2, w, h));
    layout->Add(new Rectangle(shape));
    if (sxp && syp && currentShape)
        layout->Add(new ImageManipulator(currentShape, x, y, sxp,syp, w0,h0));

    return new Integer(t->id, self->Position());
}


Infix *Widget::imageSize(Scope *scope,
                         Tree *self, text filename)
//----------------------------------------------------------------------------
//  Return the width and height of an image
//----------------------------------------------------------------------------
{
    refreshOn(TextureCache::instance()->textureChangedEvent());

    filename = resolvePrefixedPath(scope, filename);

    text docPath = +taoWindow()->currentProjectFolderPath();
    CachedTexture *t = TextureCache::instance()->load(+filename, +docPath);
    longlong v[2] = { t->width, t->height };
    Tree *result = xl_integer_list(self, 2, v);
    return result->AsInfix();
}


static void list_files(Scope *scope, Dir &current,
                       Tree *self, Tree *patterns, Tree_p *&parent)
// ----------------------------------------------------------------------------
//   Append files matching patterns (relative to directory current) to parent
// ----------------------------------------------------------------------------
{
    if (Block *block = patterns->AsBlock())
    {
        list_files(scope, current, self, block->child, parent);
        return;
    }
    if (Infix *infix = patterns->AsInfix())
    {
        if (infix->name == "," || infix->name == ";" || infix->name == "\n")
        {
            list_files(scope, current, self, infix->left, parent);
            list_files(scope, current, self, infix->right, parent);
            return;
        }
    }

    patterns = xl_evaluate(scope, patterns);
    if (Text *glob = patterns->AsText())
    {
        QString filter = +glob->value;
        QFileInfoList files = current.entryInfoGlobList(filter);
        foreach (QFileInfo file, files)
        {
            Text *listed = new Text(+file.absoluteFilePath());
            if (*parent)
            {
                Infix *added = new Infix(",", *parent, listed);
                *parent = added;
                parent = &added->right;
            }
            else
            {
                *parent = listed;
            }
        }
        return;
    }
    Ooops("Malformed files list for $1: $2", self, patterns);
}


Tree *Widget::listFiles(Scope *scope, Tree *self, Tree *pattern)
// ----------------------------------------------------------------------------
//   List files, current directory is the document directory
// ----------------------------------------------------------------------------
{
    Tree_p result = NULL;
    Tree_p *parent = &result;
    Dir current(taoWindow()->currentProjectFolderPath());
    list_files(scope, current, self, pattern, parent);
    if (!result)
        result = XL::xl_nil;
    return result;
}


Tree *Widget::textureWrap(Tree *self, bool s, bool t)
// ----------------------------------------------------------------------------
//   Record if we want to wrap textures or clamp them
// ----------------------------------------------------------------------------
{
    layout->Add(new TextureWrap(s, t));
    return XL::xl_true;
}


Tree *Widget::textureMode(Tree *self, text mode)
// ----------------------------------------------------------------------------
//   Record the mode of blending of the current texture
// ----------------------------------------------------------------------------
{
    GLenum glMode = TextToGLEnum(mode, GL_MODULATE);
    layout->Add(new TextureMode(glMode));
    return XL::xl_true;
}


Tree *Widget::textureMinFilter(Tree *self, text filter)
// ----------------------------------------------------------------------------
//   Set the minifying filter function for textures
// ----------------------------------------------------------------------------
{
    GLenum glFilt = TextToGLEnum(filter, GL_LINEAR);
    layout->Add(new TextureMinFilter(glFilt));
    return XL::xl_true;
}


Tree *Widget::textureMagFilter(Tree *self, text filter)
// ----------------------------------------------------------------------------
//   Set the minifying filter function for textures
// ----------------------------------------------------------------------------
{
    GLenum glFilt = TextToGLEnum(filter, GL_LINEAR);
    layout->Add(new TextureMagFilter(glFilt));
    return XL::xl_true;
}


Tree *Widget::textureTransform(Scope *scope, Tree *self, Tree *code)
// ----------------------------------------------------------------------------
//   Apply a texture transformation
// ----------------------------------------------------------------------------
{
    uint texUnit = GL.ActiveTextureUnitIndex() - GL_TEXTURE0;

    //Check if we can use this texture unit for transform according
    //to the maximum of texture coordinates (maximum of texture transformation)
    if (texUnit >= GL.MaxTextureCoords())
    {
        Ooops("Invalid texture unit $2 (max $3) for $1", self)
            .Arg(texUnit).Arg(GL.MaxTextureCoords());
        return XL::xl_false;
    }

    layout->Add(new TextureTransform(true));
    Tree *result = xl_evaluate(scope, code);
    layout->Add(new TextureTransform(false));
    return result;
}


Integer* Widget::textureWidth(Tree *self)
// ----------------------------------------------------------------------------
//   Return the current texture width
// ----------------------------------------------------------------------------
{
    return new Integer(GL.TextureWidth());
}


Integer* Widget::textureHeight(Tree *self)
// ----------------------------------------------------------------------------
//   Return the current texture height
// ----------------------------------------------------------------------------
{
    return new Integer(GL.TextureHeight());
}


Integer* Widget::textureType(Tree *self)
// ----------------------------------------------------------------------------
//   Return the current texture type
// ----------------------------------------------------------------------------
{
    return new Integer(GL.TextureType());
}


Text *Widget::textureMode(Tree *self)
// ----------------------------------------------------------------------------
//   Return the current texture mode
// ----------------------------------------------------------------------------
{
    text mode;
    uint m = GL.TextureMode();
    switch(m)
    {
    case GL_REPLACE: mode = "replace";  break;
    case GL_ADD    : mode = "add";      break;
    case GL_DECAL  : mode = "decal";    break;
    default        : mode = "modulate"; break;
    }

    return new Text(mode);
}


Integer* Widget::textureId(Tree *self)
// ----------------------------------------------------------------------------
//   Return the current texture id
// ----------------------------------------------------------------------------
{
    return new Integer(GL.TextureID());
}



Tree *Widget::hasTexture(Tree *self, GLuint unit)
// ----------------------------------------------------------------------------
//   Return the texture id set at the specified unit
// ----------------------------------------------------------------------------
{
    if (unit > GL.MaxTextureUnits())
    {
        Ooops("Invalid texture unit $2 for $1, maximum is $3", self)
            .Arg(unit).Arg(GL.MaxTextureUnits());
        return XL::xl_false;
    }

    uint hasTexture = GL.ActiveTextureUnits() & (1ULL << unit);
    return hasTexture ? XL::xl_true : XL::xl_false;
}


Integer* Widget::textureUnit(Tree *self)
// ----------------------------------------------------------------------------
//   Return the current texture unit
// ----------------------------------------------------------------------------
{
    return new Integer(GL.ActiveTextureUnitIndex() - GL_TEXTURE0);
}


Integer *Widget::framePixelCount(Tree *self, float alphaMin)
// ----------------------------------------------------------------------------
//    Return number of non-transparent pixels in the current frame
// ----------------------------------------------------------------------------
{
    ulonglong result = 0;
    if (frameInfo)
    {
        QImage image = frameInfo->toImage();
        int width = image.width();
        int height = image.height();
        for (int r = 0; r < height; r++)
            for (int c = 0; c < width; c++)
                if (qAlpha(image.pixel(c, r)) > alphaMin * 255)
                    result++;
    }
    return new Integer(result, self->Position());
}


Integer *Widget::framePixel(Tree *self, float x, float y,
                            Real *r, Real *g, Real *b, Real *a)
// ----------------------------------------------------------------------------
//    Return the pixel at the given coordinates
// ----------------------------------------------------------------------------
{
    longlong result = -1;
    if (frameInfo)
    {
        QImage image = frameInfo->toImage();
        uint width  = image.width();
        uint height = image.height();
        if (x >= 0 && x < width && y >= 0 && y < height)
        {
            QRgb rgb = image.pixel((int) x, (int) y);
            uint rr = qRed(rgb);
            uint gg = qGreen(rgb);
            uint bb = qBlue(rgb);
            uint aa = qAlpha(rgb);
            r->value = rr / 255.0;
            g->value = gg / 255.0;
            b->value = bb  / 255.0;
            a->value = aa / 255.0;
            result = (rr << 24) | (gg << 16) | (bb << 8) | (aa << 0);
        }
    }
    return new Integer(result, self->Position());
}


Tree *Widget::extrudeDepth(Tree *self, float depth)
// ----------------------------------------------------------------------------
//    Set the extrude depth
// ----------------------------------------------------------------------------
{
    layout->Add (new ExtrudeDepth(depth));
    return XL::xl_true;
}


Tree *Widget::extrudeRadius(Tree *self, float radius)
// ----------------------------------------------------------------------------
//   Set the extrude radius
// ----------------------------------------------------------------------------
{
    layout->Add (new ExtrudeRadius(radius));
    return XL::xl_true;
}


Tree *Widget::extrudeCount(Tree *self, int count)
// ----------------------------------------------------------------------------
//   Set the extrude count
// ----------------------------------------------------------------------------
{
    if (count < -10)
        count = -10;
    else if (count > 10)
        count = 10;
    layout->Add (new ExtrudeCount(count));
    return XL::xl_true;
}


Integer *Widget::lightsMask(Tree *self XL_UNUSED)
// ----------------------------------------------------------------------------
//  Return a bitmask of all current activated lights
// ----------------------------------------------------------------------------
{
    return new Integer(GL.LightsMask());
}


Tree *Widget::perPixelLighting(Tree *self,  bool enable)
// ----------------------------------------------------------------------------
//  Enable or Disable per pixel lighting
// ----------------------------------------------------------------------------
{
    layout->Add(new PerPixelLighting(enable));
    return XL::xl_true;
}


Tree *Widget::lightId(Tree *self, GLuint id, bool enable)
// ----------------------------------------------------------------------------
//   Select and enable or disable a light
// ----------------------------------------------------------------------------
{
    layout->Add(new LightId(id, enable));
    return XL::xl_true;
}

Tree *Widget::light(Tree *self, GLuint function, GLfloat value)
// ----------------------------------------------------------------------------
//   Set a light parameter with a single float value
// ----------------------------------------------------------------------------
{
    layout->Add(new Light(function, value));
    return XL::xl_true;
}


Tree *Widget::light(Tree *self, GLuint function,
                    GLfloat a, GLfloat b, GLfloat c)
// ----------------------------------------------------------------------------
//   Set a light parameter with four float values (direction)
// ----------------------------------------------------------------------------
{
    layout->Add(new Light(function, a, b, c));
    return XL::xl_true;
}


Tree *Widget::light(Tree *self, GLuint function,
                    GLfloat a, GLfloat b, GLfloat c, GLfloat d)
// ----------------------------------------------------------------------------
//   Set a light parameter with four float values (position, color)
// ----------------------------------------------------------------------------
{
    layout->Add(new Light(function, a, b, c, d));
    return XL::xl_true;
}


Tree *Widget::material(Tree *self,
                       GLenum face, GLenum function,
                       GLfloat value)
// ----------------------------------------------------------------------------
//   Set a material parameter with a single float value
// ----------------------------------------------------------------------------
{
    layout->Add(new Material(face, function, value));
    return XL::xl_true;
}


Tree *Widget::material(Tree *self,
                       GLenum face, GLenum function,
                       GLfloat a, GLfloat b, GLfloat c, GLfloat d)
// ----------------------------------------------------------------------------
//   Set a light parameter with four float values (position, color)
// ----------------------------------------------------------------------------
{
    layout->Add(new Material(face, function, a, b, c, d));
    return XL::xl_true;
}


Tree *Widget::shaderProgram(Scope *scope, Tree *self, Tree *code)
// ----------------------------------------------------------------------------
//    Creates a new shader program in which we will evaluate shaders
// ----------------------------------------------------------------------------
//    Note that we compile and evaluate the shader only once
{
    Tree *result = XL::xl_true;

    Name* name = code->AsName();
    if(name && (name->value == "false"))
    {
        // Disable shader program
        layout->Add(new ShaderProgram());
        return result;
    }

    if (currentShaderProgram)
    {
        Ooops("Nested shader program $1", self);
        return XL::xl_false;
    }

    ShaderProgramInfo *info = self->GetInfo<ShaderProgramInfo>();
    QGLShaderProgram *program = NULL;

    if (!info)
    {
        program = new QGLShaderProgram;
        info = new ShaderProgramInfo(program);
        self->SetInfo<ShaderProgramInfo> (info);
    }

    XL::Save<ShaderProgramInfo *> saveInfo(currentShaderProgram, info);
    result = xl_evaluate(scope, code);
    program = info->program;

    if (!program->isLinked())
        if (!program->link())
            taoWindow()->addError(program->log());

    layout->Add(new ShaderProgram(program));
    return result;
}


static inline QGLShader::ShaderType ShaderType(Widget::ShaderKind kind)
// ----------------------------------------------------------------------------
//   Convert our shader kind into Qt shader type.
// ----------------------------------------------------------------------------
{
    switch (kind)
    {
    case Widget::VERTEX:        return QGLShader::Vertex;
    case Widget::FRAGMENT:      return QGLShader::Fragment;
    case Widget::GEOMETRY:      return QGLShader::Geometry;
    }
    XL::Ooops("Internal error: Shader type $1 not implemented")
        .Arg((int) kind);
    return QGLShader::ShaderType(0);
}


Tree *Widget::shaderFromSource(Tree *self, ShaderKind kind, text source)
// ----------------------------------------------------------------------------
//   Load a shader from shader source
// ----------------------------------------------------------------------------
{
    if (!currentShaderProgram)
    {
        Ooops("No shader program for $1", self);
        return XL::xl_false;
    }

    QGLShader::ShaderType shaderType = ShaderType(kind);
    XL_ASSERT(shaderType < ShaderProgramInfo::SHADER_TYPES);
    if (source != currentShaderProgram->shaderSource[shaderType])
    {
        QGLShaderProgram *prog = currentShaderProgram->program;

        // Remove old shader of that kind
        QList<QGLShader *> shaders = prog->shaders();
        foreach(QGLShader *shader, shaders)
            if (shader->shaderType() == shaderType)
                prog->removeShader(shader);

        // Add new shader
        bool ok = prog->addShaderFromSourceCode(shaderType, +source);
        currentShaderProgram->shaderSource[shaderType] = source;
        currentShaderProgram->inError = !ok;
        if (!ok)
            inError = true;
        return ok ? XL::xl_true : XL::xl_false;
    }
    if (currentShaderProgram->inError)
        inError = true;

    return XL::xl_true;
}


Tree *Widget::shaderFromFile(Scope *scope,
                             Tree *self,
                             ShaderKind kind,
                             text file)
// ----------------------------------------------------------------------------
//   Load a shader from shader source
// ----------------------------------------------------------------------------
{
    if (!currentShaderProgram)
    {
        Ooops("No shader program for $1", self);
        return XL::xl_false;
    }

    record(shaders, "Loading shader from %s", file);
    Text *source = loadText(scope, self, file, "");
    record(shaders, "Source for %s is %s", file, source->value);
    return shaderFromSource(self, kind, source->value);
}


static inline GLenum getTypeOfActiveUniform(GLuint progId, kstring name,
                                            int hint = 0)
// ----------------------------------------------------------------------------
//   Return type of active uniform 'name' in shader program 'progId', or 0
// ----------------------------------------------------------------------------
{
    // Setting hint to the uniform location improves performance because most
    // of the time the index has the same value as the location (tested on
    // MacOSX 10.6.8).
    // This is probably true on many other platforms, since we could live for
    // so long without commit efc31e990.

    int uniforms = 0;
    GL.GetProgram(progId, GL_ACTIVE_UNIFORMS, &uniforms);
    if (hint >= uniforms)
        hint = 0;

    GLsizei size = -1;
    GLsizei length = 0;
    GLenum type = 0;
    char uniformName[80];
    // If uniform variable is an array, we want the type of its elements.
    // glActiveUniform will return 'varname[0]' so we'll just test if this
    // name begins with 'varname['.
    char nameArr[80];
    snprintf(nameArr, sizeof(nameArr), "%s[", name);
    for (int i = hint; i < hint + uniforms; i++)
    {
        uint index = i % uniforms;
        GL.GetActiveUniform(progId, index, sizeof(uniformName) - 1, &length,
                            &size, &type, uniformName);

        if (!strcmp(uniformName, name) ||
            !strncmp(uniformName, nameArr, strlen(nameArr)))
            break;
        else
            type = 0;
    }

    return type;
}


Tree *Widget::shaderSet(Scope *scope, Tree *self, Tree *code)
// ----------------------------------------------------------------------------
//   Evaluate the code argument as an assignment for the current shader
// ----------------------------------------------------------------------------
{
    GLuint programId = layout->programId;
    if (!programId)
    {
        Ooops("No shader program for $1", self);
        return XL::xl_false;
    }

    if (Infix *infix = code->AsInfix())
    {
        if (infix->name == ":=")
        {
            kstring cname = "";
            Name *name = infix->left->AsName();
            if (name)
            {
                cname = name->value.c_str();
            }
            else
            {
                Text *tname = infix->left->AsText();
                if (tname)
                {
                    cname = tname->value.c_str();
                }
                else
                {
                    Ooops("Shader name $2 must be a name or text in $1",
                          self, infix->left);
                    cname = "unknown";
                }
            }

            Tree *arg = infix->right;
            if (Block *block = arg->AsBlock())
                arg = block->child;

            // Transform the input arguments into a list of values
            ShaderValue::Values values;
            while (arg)
            {
                Tree *value = arg;
                Infix *iarg = arg->AsInfix();
                if (iarg &&
                    (iarg->name == "," ||
                     iarg->name == "\n" ||
                     iarg->name == ";"))
                {
                    value = iarg->left;
                    arg = iarg->right;
                }
                else
                {
                    arg = NULL;
                }

                // Evaluate the argument in the proper context
                value = xl_evaluate(scope, value);

                if (Integer *it = value->AsInteger())
                    values.push_back(it->value);
                else if (Real *rt = value->AsReal())
                    values.push_back(rt->value);
                else
                    Ooops("Shader value $2 is not a number in $1", self, value);
            }


            // Get location for the given uniform
            GLint uniformLoc = GL.GetUniformLocation(programId, cname);

            if (uniformLoc >= 0)
            {
                GLenum type = getTypeOfActiveUniform(programId, cname,
                                                     uniformLoc);

                // Register that we want to set a shader value
                layout->Add(new ShaderValue(uniformLoc, type, values));
            }
            else
            {
                GLint attributeLoc = GL.GetAttribLocation(programId, cname);
                if (attributeLoc >= 0)
                {
                    layout->Add(new ShaderAttribute(attributeLoc,
                                                    GL_FLOAT, values));
                }
                else
                {
                    Ooops("Unkown shader variable name $2 in $1",
                          self, infix->left);
                    return XL::xl_false;
                }
            }

            return XL::xl_true;
        }
    }
    Ooops("Malformed shader_set statement $1", self);
    return XL::xl_false;
}


Text *Widget::shaderLog(Tree *self)
// ----------------------------------------------------------------------------
//   Return the log for the shader
// ----------------------------------------------------------------------------
{
    if (!currentShaderProgram)
    {
        Ooops("No shader program for $1", self);
        return new Text("");
    }

    text message = +currentShaderProgram->program->log();
    return new Text(message);
}


Name *Widget::setGeometryInputType(Tree *self, uint inputType)
// ----------------------------------------------------------------------------
//   Specify input type for geometry shader
// ----------------------------------------------------------------------------
{
    if (!currentShaderProgram)
    {
        Ooops("No shader program for $1", self);
        return XL::xl_false;
    }

    currentShaderProgram->program->setGeometryInputType(inputType);
    return XL::xl_true;
}


Integer* Widget::geometryInputType(Tree *self)
// ----------------------------------------------------------------------------
//   return input type of geometry shader
// ----------------------------------------------------------------------------
{
    if (!currentShaderProgram)
    {
        Ooops("No shader program for $1", self);
        return new Integer(0, self->Position());
    }
    return new Integer(currentShaderProgram->program->geometryInputType());
}


Name *Widget::setGeometryOutputType(Tree *self, uint outputType)
// ----------------------------------------------------------------------------
//   Specify output type for geometry shader
// ----------------------------------------------------------------------------
{
    if (!currentShaderProgram)
    {
        Ooops("No shader program for $1", self);
        return XL::xl_false;
    }

    QGLShaderProgram *prog = currentShaderProgram->program;
    switch(outputType)
    {
    case GL_LINE_STRIP:
        prog->setGeometryOutputType(GL_LINE_STRIP); break;
    case GL_TRIANGLE_STRIP:
        prog->setGeometryOutputType(GL_TRIANGLE_STRIP); break;
    default:
        prog->setGeometryOutputType(GL_POINTS); break;
    }
    return XL::xl_true;
}

Integer* Widget::geometryOutputType(Tree *self)
// ----------------------------------------------------------------------------
//   return output type of geometry shader
// ----------------------------------------------------------------------------
{
    if (!currentShaderProgram)
    {
        Ooops("No shader program for $1", self);
        return new Integer((longlong)0);
    }
    return new Integer(currentShaderProgram->program->geometryOutputType());
}

Name *Widget::setGeometryOutputCount(Tree *self, uint outputCount)
// ----------------------------------------------------------------------------
//   Specify output vertices count for geometry shader
// ----------------------------------------------------------------------------
{
    if (!currentShaderProgram)

    {
        Ooops("No shader program for $1", self);
        return XL::xl_false;
    }

    QGLShaderProgram *prog = currentShaderProgram->program;
    uint maxVertices = prog->maxGeometryOutputVertices();
    if (outputCount < maxVertices)
        prog->setGeometryOutputVertexCount(outputCount);
    else
        prog->setGeometryOutputVertexCount(maxVertices);

    return XL::xl_true;
}

Integer* Widget::geometryOutputCount(Tree *self)
// ----------------------------------------------------------------------------
//   Specify output vertices count for geometry shader
// ----------------------------------------------------------------------------
{
    if (!currentShaderProgram)
    {
        Ooops("No shader program for $1", self);
        return new Integer(0, self->Position());
    }

    QGLShaderProgram *prog = currentShaderProgram->program;
    return new Integer(prog->geometryOutputVertexCount());
}



// ============================================================================
//
//    Path management
//
// ============================================================================

Tree *Widget::newPath(Scope *scope, Tree *self, Tree *child)
// ----------------------------------------------------------------------------
//   Evaluate the child tree within a polygon
// ----------------------------------------------------------------------------
{
    if (path)
    {
        Ooops("Nest path $1", self);
        return XL::xl_false;
    }

    TesselatedPath *localPath = new TesselatedPath(GLU_TESS_WINDING_ODD);
    XL::Save<GraphicPath *> save(path, localPath);
    layout->Add(localPath);
    if (currentShape)
        layout->Add(new GraphicPathManipulator(currentShape, localPath, self));
    Tree *result = xl_evaluate(scope, child);
    return result;
}


Tree *Widget::moveTo(Tree *self, Real *x, Real *y, Real *z)
// ----------------------------------------------------------------------------
//    Add a 'moveTo' to the current path
// ----------------------------------------------------------------------------
{
    if (path)
    {
        path->moveTo(Point3(*x,*y,*z));
        path->AddControl(self, true, x, y, z);
    }
    else
    {
        layout->Add(new MoveTo(*x, *y, *z));
    }
    return XL::xl_true;
}


Tree *Widget::lineTo(Tree *self, Real *x, Real *y, Real *z)
// ----------------------------------------------------------------------------
//    Add a 'lineTo' to the current path
// ----------------------------------------------------------------------------
{
    if (!path)
    {
        Ooops("No path for $1", self);
        return XL::xl_false;
    }
    path->lineTo(Point3(*x,*y,*z));
    path->AddControl(self, true, x, y, z);
    return XL::xl_true;
}


Tree *Widget::curveTo(Tree *self,
                      Real *cx, Real *cy, Real *cz,
                      Real *x, Real *y, Real *z)
// ----------------------------------------------------------------------------
//    Add a quadric curveTo to the current path
// ----------------------------------------------------------------------------
{
    if (!path)
    {
        Ooops("No path for $1", self);
        return XL::xl_false;
    }
    path->curveTo(Point3(*cx, *cy, *cz), Point3(*x,*y,*z));
    path->AddControl(self, true, x, y, z);
    path->AddControl(self, false, cx, cy, cz);
    return XL::xl_true;
}


Tree *Widget::curveTo(Tree *self,
                      Real *c1x, Real *c1y, Real *c1z,
                      Real *c2x, Real *c2y, Real *c2z,
                      Real *x, Real *y, Real *z)
// ----------------------------------------------------------------------------
//    Add a cubic curveTo to the current path
// ----------------------------------------------------------------------------
{
    if (!path)
    {
        Ooops("No path for $1", self);
        return XL::xl_false;
    }
    path->curveTo(Point3(*c1x, *c1y, *c1z),
                  Point3(*c2x, *c2y, *c2z),
                  Point3(*x,   *y,   *z));
    path->AddControl(self, true, x, y, z);
    path->AddControl(self, false, c1x, c1y, c1z);
    path->AddControl(self, false, c2x, c2y, c2z);
    return XL::xl_true;
}


Tree *Widget::moveToRel(Tree *self, Real *x, Real *y, Real *z)
// ----------------------------------------------------------------------------
//    Add a relative moveTo
// ----------------------------------------------------------------------------
{
    if (path)
    {
        path->moveTo(Vector3(*x, *y, *z));
        path->AddControl(self, true, x, y, z);
    }
    else
    {
        layout->Add(new MoveToRel(*x, *y, *z));
    }
    return XL::xl_true;
}


Tree *Widget::lineToRel(Tree *self, Real *x, Real *y, Real *z)
// ----------------------------------------------------------------------------
//    Add a 'lineTo' to the current path
// ----------------------------------------------------------------------------
{
    if (!path)
    {
        Ooops("No path for $1", self);
        return XL::xl_false;
    }
    path->lineTo(Vector3(*x, *y, *z));
    path->AddControl(self, true, x, y, z);
    return XL::xl_true;
}


Tree *Widget::pathTextureCoord(Tree *self, Real *x, Real *y, Real *r)
// ----------------------------------------------------------------------------
//    Add a texture coordinate to the path
// ----------------------------------------------------------------------------
{
    XL::Ooops ("Path texture coordinate $1 not supported yet", self);
    return XL::xl_false;
}


Tree *Widget::closePath(Tree *self)
// ----------------------------------------------------------------------------
//    Close the path back to its origin
// ----------------------------------------------------------------------------
{
    if (!path)
    {
        Ooops("No path for $1", self);
        return XL::xl_false;
    }
    path->close();
    return XL::xl_true;
}


static GraphicPath::EndpointStyle endpointStyle(text name)
// ----------------------------------------------------------------------------
//   Translates XL name into endpoint style enum
// ----------------------------------------------------------------------------
{
    std::transform(name.begin(), name.end(), name.begin(), ::toupper);

    if (name == "NONE")
    {
        return GraphicPath::NONE;
    }
    else if (name == "ARROWHEAD")
    {
        return GraphicPath::ARROWHEAD;
    }
    else if (name == "TRIANGLE")
    {
        return GraphicPath::TRIANGLE;
    }
    else if (name == "POINTER")
    {
        return GraphicPath::POINTER;
    }
    else if (name == "DIAMOND")
    {
        return GraphicPath::DIAMOND;
    }
    else if (name == "CIRCLE")
    {
        return GraphicPath::CIRCLE;
    }
    else if (name == "SQUARE")
    {
        return GraphicPath::SQUARE;
    }
    else if (name == "BAR")
    {
        return GraphicPath::BAR;
    }
    else if (name == "CUP")
    {
        return GraphicPath::CUP;
    }
    else if (name == "FLETCHING")
    {
        return GraphicPath::FLETCHING;
    }
    else if (name == "HOLLOW_CIRCLE")
    {
        return GraphicPath::HOLLOW_CIRCLE;
    }
    else if (name == "HOLLOW_SQUARE")
    {
        return GraphicPath::HOLLOW_SQUARE;
    }
    else if (name == "ROUNDED")
    {
        return GraphicPath::ROUNDED;
    }
    else
    {
        // Others...
        return GraphicPath::NONE;
    }
}


Tree *Widget::endpointsStyle(Tree *self, Name *s, Name *e)
// ----------------------------------------------------------------------------
//   Specify the style of the path endpoints
// ----------------------------------------------------------------------------
{
    if (!path)
    {
        Ooops("No path for $1", self);
        return XL::xl_false;
    }

    path->startStyle = endpointStyle(s->value);
    path->endStyle   = endpointStyle(e->value);

    return XL::xl_true;
}


Tree *Widget::endpointsStyle(Tree *self, text s, text e)
// ----------------------------------------------------------------------------
//   Specify the style of the path endpoints
// ----------------------------------------------------------------------------
{
    if (!path)
    {
        Ooops("No path for $1", self);
        return XL::xl_false;
    }

    path->startStyle = endpointStyle(s);
    path->endStyle   = endpointStyle(e);

    return XL::xl_true;
}


static Qt::PenStyle lineStyle(text name)
// ----------------------------------------------------------------------------
//   Translates name into line style enum
// ----------------------------------------------------------------------------
{
    if (name == "Solid")
    {
        return Qt::SolidLine;
    }
    else if (name == "Dash")
    {
        return Qt::DashLine;
    }
    else if (name == "Dot")
    {
        return Qt::DotLine;
    }
    else if (name == "DashDot")
    {
        return Qt::DashDotLine;
    }
    else if (name == "DashDotDot")
    {
        return Qt::DashDotDotLine;
    }
    else
    {
        // Others...
        return Qt::SolidLine;
    }
}

Tree *Widget::lineStipple(Tree *self, text name)
// ----------------------------------------------------------------------------
//   Specify the style of the path endpoints
// ----------------------------------------------------------------------------
{
    if (!path)
    {
        Ooops("No path for $1", self);
        return XL::xl_false;
    }

    path->lineStyle = lineStyle(name);

    return XL::xl_true;
}



// ============================================================================
//
//    2D primitives that can be in a path or standalone
//
// ============================================================================

Tree *Widget::fixedSizePoint(Tree *self, coord x,coord y,coord z, coord s)
// ----------------------------------------------------------------------------
//   Draw a point with the given size
// ----------------------------------------------------------------------------
{
    layout->Add(new FixedSizePoint(Point3(x, y, z), s));
    return XL::xl_true;
}


Tree *Widget::rectangle(Tree *self, Real *x, Real *y, Real *w, Real *h)
// ----------------------------------------------------------------------------
//    Draw a nice rectangle
// ----------------------------------------------------------------------------
{
    Rectangle shape(Box(*x-*w/2, *y-*h/2, *w, *h));
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new Rectangle(shape));

    if (currentShape)
        layout->Add(new ControlRectangle(currentShape, x, y, w, h));

    return XL::xl_true;
}


Tree *Widget::plane(Tree *tree, Real *x, Real *y, Real *w,
                    Real *h, Integer *lines_nb, Integer *columns_nb)
// ----------------------------------------------------------------------------
//    Draw a subdivided plane
// ----------------------------------------------------------------------------
{
    layout->Add(new Plane(*x, *y, *w, *h, *lines_nb, *columns_nb));
    if (currentShape)
        layout->Add(new ControlRectangle(currentShape, x, y, w, h));

    return XL::xl_true;
}


Tree *Widget::isoscelesTriangle(Tree *self,
                                Real *x, Real *y, Real *w, Real *h)
// ----------------------------------------------------------------------------
//    Draw an isosceles triangle
// ----------------------------------------------------------------------------
{
    IsoscelesTriangle shape(Box(*x-*w/2, *y-*h/2, *w, *h));
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new IsoscelesTriangle(shape));

    if (currentShape)
        layout->Add(new ControlRectangle(currentShape, x, y, w, h));

    return XL::xl_true;
}


Tree *Widget::rightTriangle(Tree *self,
                            Real *x, Real *y, Real *w, Real *h)
// ----------------------------------------------------------------------------
//    Draw a right triangle
// ----------------------------------------------------------------------------
{
    RightTriangle shape(Box(*x-*w/2, *y-*h/2, *w, *h));
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new RightTriangle(shape));

    if (currentShape)
        layout->Add(new ControlRectangle(currentShape, x, y, w, h));

    return XL::xl_true;
}


Tree *Widget::ellipse(Tree *self, Real *cx, Real *cy,
                      Real *w, Real *h)
// ----------------------------------------------------------------------------
//   Circle centered around (cx,cy), size w * h
// ----------------------------------------------------------------------------
{
    Ellipse shape(Box(*cx-*w/2, *cy-*h/2, *w, *h));
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new Ellipse(shape));

    if (currentShape)
        layout->Add(new ControlRectangle(currentShape, cx, cy, w, h));

    return XL::xl_true;
}


Tree *Widget::ellipseArc(Tree *self,
                         Real *cx, Real *cy, Real *w, Real *h,
                         Real *start, Real *sweep)
// ----------------------------------------------------------------------------
//   Elliptical arc centered around (cx,cy)
// ----------------------------------------------------------------------------
{
    // Start and sweep must be provided upsidedown because of y flip.
    // See Bug#787
    EllipseArc shape(Box(*cx-*w/2, *cy-*h/2, *w, *h), -*start, -*sweep);
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new EllipseArc(shape));

    if (currentShape)
        layout->Add(new ControlRectangle(currentShape, cx, cy, w, h));

    return XL::xl_true;
}


Tree *Widget::ellipseSector(Tree *self,
                            Real *cx, Real *cy, Real *w, Real *h,
                            Real *start, Real *sweep)
// ----------------------------------------------------------------------------
//   Elliptical sector centered around (cx,cy)
// ----------------------------------------------------------------------------
{
    EllipseArc shape(Box(*cx-*w/2, *cy-*h/2, *w, *h), -*start, -*sweep, true);
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new EllipseArc(shape));

    if (currentShape)
        layout->Add(new ControlRectangle(currentShape, cx, cy, w, h));

    return XL::xl_true;
}


Tree *Widget::roundedRectangle(Tree *self,
                               Real *cx, Real *cy,
                               Real *w, Real *h, Real *r)
// ----------------------------------------------------------------------------
//   Rounded rectangle with radius r for the rounded corners
// ----------------------------------------------------------------------------
{
    RoundedRectangle shape(Box(*cx-*w/2, *cy-*h/2, *w, *h), *r);
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new RoundedRectangle(shape));

    if (currentShape)
        layout->Add(new ControlRoundedRectangle(currentShape, cx,cy,w,h, r));


    return XL::xl_true;
}


Tree *Widget::ellipticalRectangle(Tree *self,
                                  Real *cx, Real *cy,
                                  Real *w, Real *h, Real *r)
// ----------------------------------------------------------------------------
//   Elliptical rectangle with ratio r for the elliptic sides
// ----------------------------------------------------------------------------
{
    EllipticalRectangle shape(Box(*cx-*w/2, *cy-*h/2, *w, *h), *r);
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new EllipticalRectangle(shape));

    if (currentShape)
        layout->Add(new ControlRoundedRectangle(currentShape,
                                                cx, cy, w, h, r));

    return XL::xl_true;
}


Tree *Widget::arrow(Tree *self,
                    Real *cx, Real *cy, Real *w, Real *h,
                    Real *ax, Real *ary)
// ----------------------------------------------------------------------------
//   Arrow
// ----------------------------------------------------------------------------
{
    Arrow shape(Box(*cx-*w/2, *cy-*h/2, *w, *h), *ax, *ary);
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new Arrow(shape));

    if (currentShape)
        layout->Add(new ControlArrow(currentShape, cx, cy, w, h, ax, ary));

    return XL::xl_true;
}


Tree *Widget::doubleArrow(Tree *self,
                          Real *cx, Real *cy, Real *w, Real *h,
                          Real *ax, Real *ary)
// ----------------------------------------------------------------------------
//   Double arrow
// ----------------------------------------------------------------------------
{
    DoubleArrow shape(Box(*cx-*w/2, *cy-*h/2, *w, *h), *ax, *ary);
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new DoubleArrow(shape));

    if (currentShape)
        layout->Add(new ControlArrow(currentShape, cx,cy,w,h, ax,ary, true));

    return XL::xl_true;
}


Tree *Widget::starPolygon(Tree *self,
                          Real *cx, Real *cy, Real *w, Real *h,
                          Integer *p, Integer *q)
// ----------------------------------------------------------------------------
//     GL regular p-side star polygon {p/q} centered around (cx,cy)
// ----------------------------------------------------------------------------
{
    StarPolygon shape(Box(*cx-*w/2, *cy-*h/2, *w, *h), *p, *q);
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new StarPolygon(shape));

    if (currentShape)
        layout->Add(new ControlPolygon(currentShape, cx, cy, w, h, p));

    return XL::xl_true;
}


Tree *Widget::star(Tree *self,
                   Real *cx, Real *cy, Real *w, Real *h,
                   Integer *p, Real *r)
// ----------------------------------------------------------------------------
//     GL regular p-side star centered around (cx,cy), inner radius ratio r
// ----------------------------------------------------------------------------
{
    Star shape(Box(*cx-*w/2, *cy-*h/2, *w, *h), *p, *r);
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new Star(shape));

    if (currentShape)
        layout->Add(new ControlStar(currentShape, cx, cy, w, h, p, r));

    return XL::xl_true;
}


Tree *Widget::speechBalloon(Tree *self,
                            Real *cx, Real *cy, Real *w, Real *h,
                            Real *r, Real *ax, Real *ay)
// ----------------------------------------------------------------------------
//   Speech balloon with radius r for rounded corners, and point a for the tail
// ----------------------------------------------------------------------------
{
    SpeechBalloon shape(Box(*cx-*w/2, *cy-*h/2, *w, *h), *r, *ax, *ay);
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new SpeechBalloon(shape));

    if (currentShape)
        layout->Add(new ControlBalloon(currentShape, cx, cy, w, h, r, ax, ay));

    return XL::xl_true;
}


Tree *Widget::callout(Tree *self,
                      Real *cx, Real *cy, Real *w, Real *h,
                      Real *r, Real *ax, Real *ay, Real *d)
// ----------------------------------------------------------------------------
//   Callout with radius r for corners, and point a, width b for the tail
// ----------------------------------------------------------------------------
{
    Callout shape(Box(*cx-*w/2, *cy-*h/2, *w, *h), *r, *ax, *ay, *d);
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new Callout(shape));

    if (currentShape)
        layout->Add(new ControlCallout(currentShape,
                                       cx, cy, w, h,
                                       r, ax, ay, d));

    return XL::xl_true;
}


struct ImagePacker : XL::Action
// ----------------------------------------------------------------------------
//   Pack images into a bigger texture
// ----------------------------------------------------------------------------
{
    ImagePacker(BinPacker &bp, uint w, uint h, uint pw, uint ph):
        bp(bp), maxWidth(w), maxHeight(h), padWidth(pw), padHeight(ph),
        composite(bp.Width(), bp.Height(), QImage::Format_ARGB32)
    {
        composite.fill(0);
    }

    void AddImage(QString file)
    {
        QFileInfo fi(file);
        if (fi.isDir())
        {
            QDir dir(file);
            QStringList subdirs = dir.entryList(QDir::Dirs | QDir::Files |
                                                QDir::Readable |
                                                QDir::NoDotAndDotDot);
            foreach (QString d, subdirs)
                AddImage(dir.cleanPath(dir.absoluteFilePath(d)));
        }
        else
        {
            QImage image(file);
            if (image.isNull())
            {
                QString qualified = "texture:" + file;
                image.load(qualified);
            }
            if (!image.isNull())
            {
                uint w = image.width();
                uint h = image.height();

                if (w > maxWidth)
                {
                    image = image.scaledToWidth(maxWidth,
                                                Qt::SmoothTransformation);
                    w = image.width();
                    h = image.height();
                }
                if (h > maxHeight)
                {
                    image = image.scaledToHeight(maxHeight,
                                                 Qt::SmoothTransformation);
                    w = image.width();
                    h = image.height();
                }

                BinPacker::Rect rect;
                if (bp.Allocate(w + padWidth, h + padHeight, rect))
                {
                    QPainter painter(&composite);
                    painter.setBrush(Qt::transparent);
                    painter.setPen(Qt::white);
                    painter.drawImage(rect.x1, rect.y1, image);
                    painter.end();
                }
                else
                {
                    record(picture_pack_error,
                           "No room for image %s (%u x %u pixels)",
                           file, w, h);
                }
            }
            else
            {
                record(picture_pack_error,
                       "Unable to load image %s", file);
            }
        }
    }
    Tree *Do (Tree *what) { return what; }
    Tree *Do(Text *what)
    {
        AddImage(+what->value);
        return what;
    }
    BinPacker   &bp;
    uint        maxWidth, maxHeight;
    uint        padWidth, padHeight;
    QImage      composite;
};


Integer* Widget::picturePacker(Tree *self,
                               uint tw, uint th,
                               uint iw, uint ih,
                               uint pw, uint ph,
                               Tree *t)
// ----------------------------------------------------------------------------
//   Debug the bin packer
// ----------------------------------------------------------------------------
{
    TextureIdInfo *tinfo = self->GetInfo<TextureIdInfo>();
    if (!tinfo)
    {
        // Put all the images in the packer
        BinPacker binpack(tw, th);
        ImagePacker imagePacker(binpack, iw, ih, pw, ph);
        t->Do(imagePacker);

        // Attach the texture ID to the 'self' tree to avoid re-creating
        tinfo = new TextureIdInfo();
        self->SetInfo<TextureIdInfo> (tinfo);

        // Convert the resulting image into a texture
        QImage texImg = QGLWidget::convertToGLFormat(imagePacker.composite);
        GL.BindTexture(GL_TEXTURE_2D, tinfo->textureId);
        GL.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                      texImg.width(), texImg.height(), 0, GL_RGBA,
                      GL_UNSIGNED_BYTE, texImg.bits());
    }

    uint texId   = tinfo->bind();
    layout->Add(new FillTexture(texId));
    GL.TextureSize(iw, ih);
    return new Integer(texId, self->Position());
}



// ============================================================================
//
//    3D primitives
//
// ============================================================================

Tree *Widget::sphere(Tree *self,
                     Real *x, Real *y, Real *z,
                     Real *w, Real *h, Real *d,
                     Integer *slices, Integer *stacks)
// ----------------------------------------------------------------------------
//     A simple sphere
// ----------------------------------------------------------------------------
{
    layout->Add(new Sphere(Box3(*x-*w/2, *y-*h/2, *z-*d/2, *w,*h,*d),
                           *slices, *stacks));
    if (currentShape)
        layout->Add (new ControlBox(currentShape, x, y, z, w, h, d));
    return XL::xl_true;
}


Tree *Widget::cube(Tree *self,
                   Real *x, Real *y, Real *z,
                   Real *w, Real *h, Real *d)
// ----------------------------------------------------------------------------
//    A simple cubic box
// ----------------------------------------------------------------------------
{
    layout->Add(new Cube(Box3(*x-*w/2, *y-*h/2, *z-*d/2, *w,*h,*d)));
    if (currentShape)
        layout->Add(new ControlBox(currentShape, x, y, z, w, h, d));
    return XL::xl_true;
}


Tree *Widget::torus(Tree *self,
                    Real *x, Real *y, Real *z,
                    Real *w, Real *h, Real *d,
                    Integer *slices, Integer *stacks, double ratio)
// ----------------------------------------------------------------------------
//    A simple torus
// ----------------------------------------------------------------------------
{
    layout->Add(new Torus(Box3(*x-*w/2, *y-*h/2, *z-*d/2, *w,*h,*d),
                          *slices, *stacks, ratio));
    if (currentShape)
        layout->Add(new ControlBox(currentShape, x, y, z, w, h, d));
    return XL::xl_true;
}


Tree *Widget::cone(Tree *self,
                   Real *x, Real *y, Real *z,
                   Real *w, Real *h, Real *d,
                   double ratio)
// ----------------------------------------------------------------------------
//    A simple cone
// ----------------------------------------------------------------------------
{
    layout->Add(new Cone(Box3(*x-*w/2, *y-*h/2, *z-*d/2, *w,*h,*d), ratio));
    if (currentShape)
        layout->Add(new ControlBox(currentShape, x, y, z, w, h, d));
    return XL::xl_true;
}


bool Widget::addControlBox(Real *x, Real *y, Real *z,
                           Real *w, Real *h, Real *d)
// ----------------------------------------------------------------------------
//   Module interface to add a control box to the current layout
// ----------------------------------------------------------------------------
{
    Widget *widget = Tao();
    Tree *currentShape = widget->currentShape;
    Layout *layout = widget->layout;
    if (currentShape)
        layout->Add(new ControlBox(currentShape, x, y, z, w, h, d));
    return true;
}


// ============================================================================
//
//    Text and font
//
// ============================================================================

Tree * Widget::textEdit(Scope *scope, Tree *self,
                        Real *x, Real *y, Real *w, Real *h,
                        Text *html)
// ----------------------------------------------------------------------------
//   Create a new text edit widget and render text in it
// ----------------------------------------------------------------------------
{
    XL::Save<Layout *> saveLayout(layout, layout->AddChild(selectionId()));
    Tree * result = textEditTexture(scope, self, *w, *h, html);
    TextEditSurface *surface = html->GetInfo<TextEditSurface>();
    layout->Add(new ClickThroughRectangle(Box(*x-*w/2, *y-*h/2, *w, *h),
                                          surface));
    if (currentShape)
        layout->Add(new WidgetManipulator(currentShape, x, y, w, h, surface));
    return result;
}


Tree * Widget::textEditTexture(Scope *scope, Tree *self,
                               double w, double h, Text *html)
// ----------------------------------------------------------------------------
//   Create a new text edit widget and render text in it
// ----------------------------------------------------------------------------
{
    if (w < 16) w = 16;
    if (h < 16) h = 16;

    // Get or build the current frame if we don't have one
    TextEditSurface *surface = html->GetInfo<TextEditSurface>();
    if (!surface)
    {
        surface = new TextEditSurface(scope, html, this);
        html->SetInfo<TextEditSurface> (surface);
    }

    // Resize to requested size, bind texture and save current infos
    surface->resize(w,h);
    uint texId = surface->bindHTML(html);
    layout->Add(new FillTexture(texId));
    GL.TextureSize(w, h);

    return new Integer(texId, self->Position());
}


Tree * Widget::textBox(Scope *scope, Tree *self,
                       Real *x, Real *y, Real *w, Real *h,
                       Tree *body)
// ----------------------------------------------------------------------------
//   Create a new text box and render the given flow into it
// ----------------------------------------------------------------------------
{
    PageLayout *tbox = new PageLayout(this);
    tbox->space = Box(*x - *w/2, *y-*h/2, *w, *h);
    tbox->id = shapeId();
    tbox->body = body;
    tbox->scope = scope;
    layout->Add(tbox);
    if (currentShape)
    {
        tbox->selectId = layout->id;
        layout->Add(new ControlRectangle(currentShape, x, y, w, h));
    }

    XL::Save<Layout *> save(layout, tbox);
    Tree *result = saveAndEvaluate(scope, body);
    return result;
}


Tree *Widget::textFlow(Scope *scope, Tree *self,
                       Text *flowName, Tree *prog)
// ----------------------------------------------------------------------------
//   Define the contents of a text flow, or add to an existing one
// ----------------------------------------------------------------------------
{
    currentFlowName = *flowName;
    TextFlow *existing = flows[currentFlowName];
    Layout *parent = existing ? existing : layout;
    record(justify, "Widget %p text flow named %t is %p",
           this, (Tree *) flowName, existing);

    TextFlow *flow = new TextFlow(parent, currentFlowName);
    flow->id = shapeId();
    flow->body = prog;
    flow->scope = scope;
    if (!existing)
        flows[currentFlowName] = flow;

    layout->Add(flow);
    XL::Save<Layout *> save(layout, flow);
    Tree *result = saveAndEvaluate(scope, prog);

    return result;
}


Tree *Widget::textFlow(Scope *scope, Tree *self, Text *name)
// ----------------------------------------------------------------------------
//   Replay the reminder of an existing text flow in the current layout
// ----------------------------------------------------------------------------
{
    if (flows.count(*name) == 0)
        return XL::xl_false;

    TextFlowReplay *replay = new TextFlowReplay(*name);
    layout->Add(replay);
    return XL::xl_true;
}


Text *Widget::textFlow(Scope *scope, Tree *self)
// ----------------------------------------------------------------------------
//   Return the name of the current text flow
// ----------------------------------------------------------------------------
{
    return new Text(currentFlowName, "\"", "\"", self->Position());
}


Name *Widget:: textFlowExists(Scope *scope, Tree *self, Text *name)
// ----------------------------------------------------------------------------
//   Return true if the given text flow has been created
// ----------------------------------------------------------------------------
{
    if (flows.count(name->value))
        return XL::xl_true;
    return XL::xl_false;
}


Tree *Widget::textSpan(Scope *scope, Tree *self, Tree *child)
// ----------------------------------------------------------------------------
//   Evaluate the child tree while preserving the current text format state
// ----------------------------------------------------------------------------
{
    // To be preserved:
    // Font, color, line_color, texture, alignement, linewidth, rotation, scale
    TextSpan *childLayout = new TextSpan(layout);
    childLayout->body = child;
    childLayout->scope = scope;
    layout->Add(childLayout);

    XL::Save<Layout *> saveLayout(layout, childLayout);
    Tree *result = saveAndEvaluate(scope, child);

    return result;
}


Tree *Widget::textUnit(Tree *self, Text *contents)
// ----------------------------------------------------------------------------
//   Insert a block of text with the current definition of font, color, ...
// ----------------------------------------------------------------------------
{
    if (path)
        TextUnit(contents).Draw(*path, layout);
    else
        layout->Add(new TextUnit(contents));

    return XL::xl_true;
}


struct HTMLTextInfo : Info
// ----------------------------------------------------------------------------
//  Records the HTML text tree at the current position
// ----------------------------------------------------------------------------
{
    text        html;
    text        css;
    Tree_p      code;
};


Tree *Widget::htmlTextUnit(Scope *scope, Tree *self, text html, text css)
// ----------------------------------------------------------------------------
//   Insert a block of text written in HTML
// ----------------------------------------------------------------------------
{
    // Cache the converted code
    HTMLTextInfo *info = self->GetInfo<HTMLTextInfo>();
    if (!info)
    {
        info = new HTMLTextInfo;
        self->SetInfo<HTMLTextInfo>(info);
    }
    if (info->html != html || info->css != css)
    {
        info->code = HTMLConverter(self, layout).fromHTML(+html, +css);
        info->html = html;
        info->css = css;
    }

    // Evaluate the generated code
    return saveAndEvaluate(scope, info->code);
}


Box3 Widget::textSize(Tree *self, Text *content)
// ----------------------------------------------------------------------------
//   Return the dimensions of a given text
// ----------------------------------------------------------------------------
{
    // Saving offset to avoid changes of curor position. Refs #3125.
    XL::Save<Point3>      saveOffset(layout->offset, Point3(0, 0, 0));

    TextUnit u(content);
    Box3 bbox(u.Bounds(layout));
    return bbox;
}


Tree *Widget::textFormula(Scope *scope, Tree *self, Tree *value)
// ----------------------------------------------------------------------------
//   Insert a block of text corresponding to the given formula
// ----------------------------------------------------------------------------
{
    Prefix *prefix = self->AsPrefix();
    assert(prefix);

    if (path)
        TextFormula(scope, prefix, this).Draw(*path, layout);
    else
        layout->Add(new TextFormula(scope, prefix, this));
    return value;
}


Tree *Widget::textValue(Scope *scope, Tree *self, Tree *value)
// ----------------------------------------------------------------------------
//   Insert a block of text corresponding to the given formula
// ----------------------------------------------------------------------------
{
    XL::kind k = value->Kind();
    if (k > XL::KIND_LEAF_LAST)
    {
        value = xl_evaluate(scope, value);
        k = value->Kind();
    }

    if (k <= XL::KIND_LEAF_LAST)
    {
        if (path)
            TextValue(value, this).Draw(*path, layout);
        else
            layout->Add(new TextValue(value, this));
    }
    else
    {
        Prefix *prefix = self->AsPrefix();
        if (path)
            TextFormula(scope, prefix, this).Draw(*path, layout);
        else
            layout->Add(new TextFormula(scope, prefix, this));
    }
    return value;
}


Tree *Widget::font(Scope *scope, Tree *self,
                   Tree *descr1, Tree *descr2)
// ----------------------------------------------------------------------------
//   Select a font family
// ----------------------------------------------------------------------------
{
    QFont &font = layout->font;
    FontParsingAction parseFont(scope, font);
    descr1->Do(parseFont);
    if (descr2)
        descr2->Do(parseFont);
    font = parseFont.font;
    layout->Add(new FontChange(font));
    if (fontFileMgr)
        fontFileMgr->AddFontFiles(font);
    return XL::xl_true;
}


Tree *Widget::fontFamily(Scope *scope, Tree *self, Text *family)
// ----------------------------------------------------------------------------
//   Select a font family
// ----------------------------------------------------------------------------
{
    return font(scope, self, family);
}


Tree *Widget::fontFamily(Scope *scope, Tree *self,
                         Text *family, Real *size)
// ----------------------------------------------------------------------------
//   Select a font family
// ----------------------------------------------------------------------------
{
    return font(scope, self, family, size);
}


Tree *Widget::fontSize(Tree *self, double size)
// ----------------------------------------------------------------------------
//   Select a font size
// ----------------------------------------------------------------------------
{
    layout->font.setPointSizeF(fontSizeAdjust(size));
    layout->Add(new FontChange(layout->font));
    return XL::xl_true;
}


Tree *Widget::fontPlain(Tree *self)
// ----------------------------------------------------------------------------
//   Select whether this is italic or not
// ----------------------------------------------------------------------------
{
    QFont &font = layout->font;
    font.setStyle(QFont::StyleNormal);
    font.setWeight(QFont::Normal);
    font.setStretch(QFont::Unstretched);
    font.setUnderline(false);
    font.setStrikeOut(false);
    font.setOverline(false);
    layout->Add(new FontChange(font));
    return self;
}


static inline scale clamp(scale value, scale min, scale max)
// ----------------------------------------------------------------------------
//   Clamp the input value between the min and max given
// ----------------------------------------------------------------------------
{
    if (value < min)    value = min;
    if (value > max)    value = max;
    return value;
}


Tree *Widget::fontItalic(Tree *self, scale amount)
// ----------------------------------------------------------------------------
//   Select whether this is italic or not
// ----------------------------------------------------------------------------
//   Qt italic values range from 0 (Normal) to 2 (Oblique)
{
    amount = clamp(amount, 0, 2);
    layout->font.setStyle(QFont::Style(amount));
    layout->Add(new FontChange(layout->font));
    return self;
}


Tree *Widget::fontBold(Tree *self, scale amount)
// ----------------------------------------------------------------------------
//   Select whether the font is bold or not
// ----------------------------------------------------------------------------
//   Qt weight values range from 0 to 99 with 50 = regular
{
    amount = clamp(amount, 0, 99);
    layout->font.setWeight(/*QFont::Weight*/(amount));
    layout->Add(new FontChange(layout->font));
    return self;
}


Tree *Widget::fontUnderline(Tree *self, scale amount)
// ----------------------------------------------------------------------------
//    Select whether we underline a font
// ----------------------------------------------------------------------------
//    Qt doesn't support setting the size of the underline, it's on or off
{
    layout->font.setUnderline(bool(amount));
    layout->Add(new FontChange(layout->font));
    return self;
}


Tree *Widget::fontOverline(Tree *self, scale amount)
// ----------------------------------------------------------------------------
//    Select whether we draw an overline
// ----------------------------------------------------------------------------
//    Qt doesn't support setting the size of the overline, it's on or off
{
    layout->font.setOverline(bool(amount));
    layout->Add(new FontChange(layout->font));
    return self;
}


Tree *Widget::fontStrikeout(Tree *self, scale amount)
// ----------------------------------------------------------------------------
//    Select whether we strikeout a font
// ----------------------------------------------------------------------------
//    Qt doesn't support setting the size of the strikeout, it's on or off
{
    layout->font.setStrikeOut(bool(amount));
    layout->Add(new FontChange(layout->font));
    return self;
}


Tree *Widget::fontStretch(Tree *self, scale amount)
// ----------------------------------------------------------------------------
//    Set font stretching factor
// ----------------------------------------------------------------------------
//    Qt font stretch ranges from 0 to 4000, with 100 = 100%.
{
    amount = clamp(amount, 0, 40);
    layout->font.setStretch(int(amount * 100));
    layout->Add(new FontChange(layout->font));
    return self;
}


static inline JustificationChange::Axis jaxis(uint a)
// ----------------------------------------------------------------------------
//   Return the right justification axis
// ----------------------------------------------------------------------------
{
    switch(a)
    {
    default:
    case 0: return JustificationChange::AlongX;
    case 1: return JustificationChange::AlongY;
    case 2: return JustificationChange::AlongZ;
    }
}


Tree *Widget::align(Tree *self, scale center, scale justify, scale spread,
                    scale fullJustify, uint axis)
// ----------------------------------------------------------------------------
//   Change the text alignment along the given axis
// ----------------------------------------------------------------------------
{
    layout->Add(new JustificationChange(justify, jaxis(axis)));
    layout->Add(new PartialJustificationChange(fullJustify, jaxis(axis)));
    layout->Add(new CenteringChange(center, jaxis(axis)));
    layout->Add(new SpreadChange(spread, jaxis(axis)));
    return XL::xl_true;
}


Tree *Widget::spacing(Tree *self, scale amount, uint axis)
// ----------------------------------------------------------------------------
//   Change the text spacing along the given axis
// ----------------------------------------------------------------------------
{
    layout->Add(new SpacingChange(amount, jaxis(axis)));
    return XL::xl_true;
}


Tree *Widget::minimumSpace(Tree *self, coord before, coord after, uint axis)
// ----------------------------------------------------------------------------
//   Define the paragraph or word space
// ----------------------------------------------------------------------------
{
    layout->Add(new MinimumSpacingChange(before, after, jaxis(axis)));
    return XL::xl_true;
}


Tree *Widget::horizontalMargins(Tree *self, coord left, coord right)
// ----------------------------------------------------------------------------
//   Set the horizontal margin for text
// ----------------------------------------------------------------------------
{
    layout->Add(new HorizontalMarginChange(left, right));
    return XL::xl_true;
}


Tree *Widget::verticalMargins(Tree *self, coord top, coord bottom)
// ----------------------------------------------------------------------------
//   Set the vertical margin for text
// ----------------------------------------------------------------------------
{
    layout->Add(new VerticalMarginChange(top, bottom));
    return XL::xl_true;
}


Tree *Widget::drawingBreak(Tree *self, BreakOrder order)
// ----------------------------------------------------------------------------
//   Change the spacing along the given axis
// ----------------------------------------------------------------------------
{
    layout->Add(new DrawingBreak(order));
    return XL::xl_true;
}


static inline Name *to_name_p(Tree *t)
// ----------------------------------------------------------------------------
//   Cast from Tree *to Name_p
// ----------------------------------------------------------------------------
{
    if (Name *n = t->AsName())
        return n;
    return XL::xl_false;
}


Name *Widget::textEditKey(Scope *scope, Tree *self, text key)
// ----------------------------------------------------------------------------
//   Send a key to the text editing activities
// ----------------------------------------------------------------------------
{
    for (Activity *a = activities; a; a = a->next)
    {
        if (TextSelect *tsel = dynamic_cast<TextSelect *> (a))
        {
            tsel->Edit(key);
            return XL::xl_true;
        }
    }

    // Check if we are changing pages here...
    if (pageLinks.count(key))
    {
        gotoPage(self, pageLinks[key], true);
        return XL::xl_true;
    }

    if (!xlProgram)
        return XL::xl_false;

    // PageUp/PageDown do the same as Up/Down by default (even when Up/Down
    // have been redefined)
    if (key == "PageUp")
        return to_name_p(XL::XLCall ("key", "Up") (scope));
    if (key == "PageDown")
        return to_name_p(XL::XLCall ("key", "Down") (scope));

    return XL::xl_false;
}


struct LoadTextInfo : Info
// ----------------------------------------------------------------------------
//  Records text values loaded by a given load_text
// ----------------------------------------------------------------------------
{
    struct PerFile
    {
        QFileInfo   fileInfo;
        Text_p      loaded;
    };
    std::map<text, PerFile> file;
};


Text *Widget::loadText(Scope *scope,
                       Tree *self, text file, text encoding)
// ----------------------------------------------------------------------------
//    Load a text file from disk
// ----------------------------------------------------------------------------
{
    bool doLoad = false;

    file  = resolvePrefixedPath(scope, file);

    QFileInfo fileInfo(+file);
    if (!fileInfo.isAbsolute())
    {
        file = "doc:" + file;
        fileInfo.setFile(+file);
    }

    LoadTextInfo *info = self->GetInfo<LoadTextInfo>();
    LoadTextInfo::PerFile *pf = NULL;
    if (info)
    {
        pf = &info->file[file];
        if (!pf->loaded)
        {
            pf->loaded = new Text("", "\"", "\"", self->Position());
            doLoad = true;
        }
        else if (fileInfo.lastModified() > pf->fileInfo.lastModified())
        {
            doLoad = true;
        }
    }
    else
    {
        info = new LoadTextInfo;
        pf = &info->file[file];
        self->SetInfo<LoadTextInfo>(info);
        pf->loaded = new Text("", "\"", "\"", self->Position());
        doLoad = true;
    }

    if (doLoad)
    {
        if (fileInfo.exists())
        {
            text &value = pf->loaded->value;

            QFile file(fileInfo.absoluteFilePath());
            file.open(QIODevice::ReadOnly);
            QTextStream textStream(&file);
            textStream.setAutoDetectUnicode(true);
            textStream.setCodec(encoding.c_str());
            QString data = textStream.readAll();
            value = +data;
        }
        pf->fileInfo = fileInfo;
    }
    return pf->loaded;
}


Text *Widget::taoLanguage(Tree *self)
// ----------------------------------------------------------------------------
//    Return the current language code of the Tao GUI ("en", "fr")
// ----------------------------------------------------------------------------
{
    return new Text(+TaoApp->lang);
}



Text *Widget::xlTrAdd(Tree *self, text from, text to)
// ----------------------------------------------------------------------------
//    Add a new translation
// ----------------------------------------------------------------------------
{
    xlTranslations[from] = to;
    return new Text(to);
}


Text *Widget::xlTr(Tree *self, text t)
// ----------------------------------------------------------------------------
//    Translate a string
// ----------------------------------------------------------------------------
{
    text translated = t;
    if (xlTranslations.count(t))
        translated = xlTranslations[t];
    return new Text(translated);
}


Text *Widget::taoVersion(Tree *self)
// ----------------------------------------------------------------------------
//    Return the version of the Tao program
// ----------------------------------------------------------------------------
{
    static text v = "";
    if (v == "")
        v = GITREV_;
    return new Text(v);
}


Text *Widget::taoEdition(Tree *self)
// ----------------------------------------------------------------------------
//    Return the edition string
// ----------------------------------------------------------------------------
{
    return new Text(+Application::editionStr());
}


Text *Widget::docVersion(Tree *self)
// ----------------------------------------------------------------------------
//    Return the version of the current document (if known)
// ----------------------------------------------------------------------------
{
    text        version        = "?";
    Repository *repo           = repository();

    if (repo)
        version = repo->version();

    return new Text(version);
}


Name *Widget::enableGlyphCache(Tree *self, bool enable)
// ----------------------------------------------------------------------------
//   Enable or disable glyph cache
// ----------------------------------------------------------------------------
{
    bool old = TextUnit::cacheEnabled;
    TextUnit::cacheEnabled = enable;
    return old ? XL::xl_true : XL::xl_false;
}


Tree *Widget::glyphCacheSizeRange(Tree *self, double min, double max)
// ----------------------------------------------------------------------------
//   Change the font scaling in the glyph cache
// ----------------------------------------------------------------------------
{
    glyphCache.minFontSize = min;
    glyphCache.maxFontSize = max;
    return XL::xl_true;
}


Tree *Widget::glyphCacheScaling(Tree *self, double scaling, double minAASize)
// ----------------------------------------------------------------------------
//   Change the font scaling in the glyph cache
// ----------------------------------------------------------------------------
{
    if (glyphCache.fontScaling != scaling ||
        glyphCache.minFontSizeForAntialiasing != minAASize)
    {
        glyphCache.Clear();
        glyphCache.fontScaling = scaling;
        glyphCache.minFontSizeForAntialiasing = minAASize;
        return XL::xl_true;
    }
    return XL::xl_false;
}


Integer *Widget::glyphCacheTexture(Tree *self)
// ----------------------------------------------------------------------------
//   Return a texture corresponding to the glyph cache contents
// ----------------------------------------------------------------------------
{
    uint textureID = glyphCache.Texture();
    layout->Add(new FillTexture(textureID, GL_TEXTURE_RECTANGLE_ARB));
    return new Integer(textureID, self->Position());
}


Text *Widget::unicodeChar(Tree *self, int code)
// ----------------------------------------------------------------------------
//    Return the character with the specified Unicode code point
// ----------------------------------------------------------------------------
{
    return new Text(+QString(QChar(code)));
}


Text *Widget::unicodeCharText(Tree *self, text code)
// ----------------------------------------------------------------------------
//    Return the character with the specified Unicode code point
// ----------------------------------------------------------------------------
{
    // Example: unicodeChar("65") == unicodeChar("x41") == "A"

    bool ok = false;
    int icode = 0;
    QString qcode(+code);

    if (qcode.startsWith('x'))
        icode = qcode.mid(1).toInt(&ok, 16);
    else
        icode = qcode.toInt(&ok);

    if (ok)
        return unicodeChar(self, icode);
    else
        return new Text("");
}


// ============================================================================
//
//   Tables
//
// ============================================================================

Tree *Widget::newTable(Scope *scope, Tree *self,
                       Integer *rows, Integer *columns,
                       Tree *body)
// ----------------------------------------------------------------------------
//   Case of a new table without a position
// ----------------------------------------------------------------------------
{
    return newTable(scope, self, r(0), r(0), rows, columns, body);
}


Tree *Widget::newTable(Scope *scope, Tree *self,
                       Real *x, Real *y,
                       Integer *r, Integer *c, Tree *body)
// ----------------------------------------------------------------------------
//   Create a new table
// ----------------------------------------------------------------------------
{
    Table *tbl = new Table(this, scope, *x, *y, *r, *c);
    XL::Save<Table *> saveTable(table, tbl);

    // Patch the symbol table with short versions of table_xyz functions
    if (Prefix *prefix = self->AsPrefix())
    {
        NameToNameReplacement replacer;
        replacer["cell"]    = "table_cell";
        replacer["margins"] = "table_cell_margins";
        replacer["fill"]    = "table_cell_fill";
        replacer["border"]  = "table_cell_border";
        replacer["x"]       = "table_cell_x";
        replacer["y"]       = "table_cell_y";
        replacer["w"]       = "table_cell_w";
        replacer["h"]       = "table_cell_h";
        replacer["row"]     = "table_cell_row";
        replacer["column"]  = "table_cell_column";
        replacer["rows"]    = "table_rows";
        replacer["columns"] = "table_columns";
        Tree *tablified = replacer.Replace(prefix->right);
        if (replacer.replaced)
        {
            prefix->right = tablified;
            reloadProgram();
            return XL::xl_false;
        }
    }

    Tree *result = xl_evaluate(scope, body);

    // After we evaluated the body, add element to the layout
    layout->Add(tbl);
    if (currentShape)
        layout->Add(new TableManipulator(currentShape, x, y, tbl));

    return result;
}


Tree *Widget::tableCell(Scope *scope, Tree *self,
                        Real *w, Real *h, Tree *body)
// ----------------------------------------------------------------------------
//   Define a sized cell in the table
// ----------------------------------------------------------------------------
{
    if (!table)
    {
        Ooops("Table cell $1 outside of any table", self);
        return XL::xl_false;
    }

    // Define a new text layout
    PageLayout *tbox = new PageLayout(this);
    tbox->space = Box(0, 0, *w, *h);
    table->Add(tbox);

    tbox->body = body;
    tbox->scope = scope;

    XL::Save<Layout *> save(layout, tbox);
    Tree *result = saveAndEvaluate(scope, body);

    table->NextCell();
    return result;
}


Tree *Widget::tableCell(Scope *scope, Tree *self, Tree *body)
// ----------------------------------------------------------------------------
//   Define a free-size cell in the table
// ----------------------------------------------------------------------------
{
    if (!table)
    {
        Ooops("Table cell $1 outside of any table", self);
        return XL::xl_false;
    }

    // Define a new text layout
    Layout *tbox = new Layout(this);
    table->Add(tbox);

    XL::Save<Layout *> save(layout, tbox);
    Tree *result = saveAndEvaluate(scope, body);
    table->NextCell();
    return result;
}


Tree *Widget::tableMargins(Tree *self,
                           Real *x, Real *y, Real *w, Real *h)
// ----------------------------------------------------------------------------
//   Set the margin rectangle for the table
// ----------------------------------------------------------------------------
{
    if (!table)
    {
        Ooops("Table margins $1 outside of any table", self);
        return XL::xl_false;
    }
    table->margins = Box(*x-*w/2, *y-*h/2, *w, *h);
    return XL::xl_true;
}


Tree *Widget::tableMargins(Tree *self, Real *w, Real *h)
// ----------------------------------------------------------------------------
//   Set the margin rectangle for the table
// ----------------------------------------------------------------------------
{
    if (!table)
    {
        Ooops("Table cell $1 outside of any table", self);
        return XL::xl_false;
    }
    table->margins = Box(-*w/2, -*h/2, *w, *h);
    return XL::xl_true;
}


Tree *Widget::tableFill(Tree *self, Tree *code)
// ----------------------------------------------------------------------------
//   Define the fill code for cells
// ----------------------------------------------------------------------------
{
    if (!table)
    {
        Ooops("Table fill $1 outside of any table", self);
        return XL::xl_false;
    }
    table->fill = code;
    return XL::xl_true;
}


Tree *Widget::tableBorder(Tree *self, Tree *code)
// ----------------------------------------------------------------------------
//   Define the border code for cells
// ----------------------------------------------------------------------------
{
    if (!table)
    {
        Ooops("Table border $1 outside of any table", self);
        return XL::xl_false;
    }
    table->border = code;
    return XL::xl_true;
}


Real *Widget::tableCellX(Tree *self)
// ----------------------------------------------------------------------------
//   Get the horizontal center of the current table cell
// ----------------------------------------------------------------------------
{
    if (!table)
    {
        Ooops("Table cell position $1 without a table", self);
        return new Real(0.0, self->Position());
    }
    return new Real(table->cellBox.Center().x, self->Position());
}


Real *Widget::tableCellY(Tree *self)
// ----------------------------------------------------------------------------
//   Get the vertical center of the current table cell
// ----------------------------------------------------------------------------
{
    if (!table)
    {
        Ooops("Table cell position $1 without a table", self);
        return new Real(0.0, self->Position());
    }
    return new Real(table->cellBox.Center().y, self->Position());
}


Real *Widget::tableCellW(Tree *self)
// ----------------------------------------------------------------------------
//   Get the horizontal size of the current table cell
// ----------------------------------------------------------------------------
{
    if (!table)
    {
        Ooops("Table cell size $1 without a table", self);
        return new Real(1.0, self->Position());
    }
    return new Real(table->cellBox.Width(), self->Position());
}


Real *Widget::tableCellH(Tree *self)
// ----------------------------------------------------------------------------
//   Get the vertical size of the current table cell
// ----------------------------------------------------------------------------
{
    if (!table)
    {
        Ooops("Table cell size $1 without a table", self);
        return new Real(1.0, self->Position());
    }
    return new Real(table->cellBox.Height(), self->Position());
}


Integer *Widget::tableRow(Tree *self)
// ----------------------------------------------------------------------------
//   Return the current row
// ----------------------------------------------------------------------------
{
    if (!table)
    {
        Ooops("Table cell attribute $1 without a table", self);
        return new Integer(0, self->Position());
    }
    return new Integer(table->row, self->Position());
}


Integer *Widget::tableColumn(Tree *self)
// ----------------------------------------------------------------------------
//   Return the current column
// ----------------------------------------------------------------------------
{
    if (!table)
    {
        Ooops("Table cell attribute $1 without a table", self);
        return new Integer(0, self->Position());
    }
    return new Integer(table->column, self->Position());
}


Integer *Widget::tableRows(Tree *self)
// ----------------------------------------------------------------------------
//   Return the number of rows in the current table
// ----------------------------------------------------------------------------
{
    if (!table)
    {
        Ooops("Table attribute $1 without a table", self);
        return new Integer(0, self->Position());
    }
    return new Integer(table->rows, self->Position());
}


Integer *Widget::tableColumns(Tree *self)
// ----------------------------------------------------------------------------
//   Return the number of columns in the current table
// ----------------------------------------------------------------------------
{
    if (!table)
    {
        Ooops("Table attribute $1 without a table", self);
        return new Integer(0, self->Position());
    }
    return new Integer(table->columns, self->Position());
}



// ============================================================================
//
//   Frames and widgets
//
// ============================================================================

Tree *Widget::status(Tree *self, text caption, float timeout)
// ----------------------------------------------------------------------------
//   Set the status line of the window
// ----------------------------------------------------------------------------
{
    taoWindow()->statusBar()->showMessage(+caption, timeout*1000);
    return XL::xl_true;
}


Integer* Widget::framePaint(Scope *scope, Tree *self,
                            Real *x, Real *y, Real *w, Real *h,
                            Tree *prog)
// ----------------------------------------------------------------------------
//   Draw a frame with the current text flow
// ----------------------------------------------------------------------------
{
    Layout *childLayout = layout->AddChild(shapeId(), prog, scope);
    XL::Save<Layout *> saveLayout(layout, childLayout);
    Integer *tex = frameTexture(scope, self, *w, *h, prog);

    // Draw a rectangle with the resulting texture
    layout->Add(new Rectangle(Box(*x-*w/2, *y-*h/2, *w, *h)));
    if (currentShape)
        layout->Add(new FrameManipulator(currentShape, x, y, w, h));
    return tex;
}


Integer* Widget::frameTexture(Scope *scope, Tree *self,
                              double w, double h, Tree *prog, text name,
                              Integer *withDepth, bool canvas)
// ----------------------------------------------------------------------------
//   Make a texture out of the current text layout
// ----------------------------------------------------------------------------
{
    if (canvas && name == "")
    {
        // A canvas must have a name or it would not persist accross pages
        // (all anonymous FrameInfos are destroyed on page change, #3090).
        QString qname = QString("canvas%1").arg(shapeId());
        name = +qname;
    }

    // Need to synchronize GL states
    // just before glPushAttrib.
    GL.Sync();

    // We need to use original glPushAttrib/glPopAttrib
    // as QT fbo functions modify implicitly GL states.
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    GLuint maxTextureSize = GL.MaxTextureSize();
    if (w > maxTextureSize) w = maxTextureSize;
    if (h > maxTextureSize) h = maxTextureSize;
    if (w < 1) w = 2;
    if (h < 1) h = 2;

    FrameInfo *pFrame;
    if (name != "")
    {
        // Get or build the current frame if we don't have one
        MultiFrameInfo<text> *mf = self->GetInfo< MultiFrameInfo<text> >();
        if (!mf)
        {
            mf = new MultiFrameInfo<text>();
            self->SetInfo< MultiFrameInfo<text> > (mf);
            record(frames, "Created text frame info for %t", self);
        }

        pFrame = &mf->frame(name);
        record(frames, "Named frame texture %s is %p", name, pFrame);
    }
    else
    {
        // Get or build the current frame if we don't have one
        MultiFrameInfo<uint> *mf = self->GetInfo< MultiFrameInfo<uint> >();
        if (!mf)
        {
            mf = new MultiFrameInfo<uint>();
            self->SetInfo< MultiFrameInfo<uint> > (mf);
            record(frames, "Created numeric frame info for %t", self);
        }

        uint id = shapeId();
        pFrame = &mf->frame(id);
        record(frames, "Frame texture id %u is %p", id, pFrame);
    }

    FrameInfo &frame = *pFrame;
    bool forceEval = !w_event;
    bool draw = false;
    if (!frame.layout)
    {
        frame.layout = new SpaceLayout(this);
        forceEval = true;
        record(frames, "Created new layout %p", frame.layout);
    }
    if (frame.layout->body != prog || frame.layout->scope != scope)
    {
        record(frames, "Body changed from %t to %t", frame.layout->body, prog);
        frame.layout->body = prog;
        frame.layout->scope = scope;
        forceEval = true;
    }

    do
    {
        GLAllStateKeeper      saveGL;
        XL::Save<Layout *>    saveLayout(layout, frame.layout);
        XL::Save<FrameInfo *> saveFrameInfo(frameInfo, pFrame);
        XL::Save<Point3>      saveCenter(cameraTarget, Point3(0,0,0));
        XL::Save<Point3>      saveEye(cameraPosition, defaultCameraPosition);
        XL::Save<Vector3>     saveUp(cameraUpVector, Vector3(0,1,0));
        XL::Save<double> saveCamToScr(cameraToScreen,
                                      (cameraTarget-cameraPosition).Length());
        XL::Save<double>      saveZoom(zoom, 1);
        XL::Save<double>      saveScaling(scaling, scalingFactorFromCamera());

        // Clear the background and setup initial state
        frame.resize(w,h);
        setup(w, h);

        // Evaluate the program
        if (forceEval)
        {
            record(frames, "Evaluating program %t", prog);
            layout->Clear();
            layout->InheritState(saveLayout.saved);
            saveAndEvaluate(scope, prog);
            draw = true;
        }
        else if (layout->Refresh(w_event, CurrentTime()))
        {
            record(frames, "Refresh event for layout %p", layout);
            draw = true;
        }

        // Draw the layout in the frame context
        if (draw)
        {
            record(frames, "Drawing layout %p", layout);

            stats.end(Statistics::EXEC);
            stats.begin(Statistics::DRAW);

            frame.begin(canvas == false);
            layout->Draw(NULL);
            frame.end();

            stats.end(Statistics::DRAW);
            stats.begin(Statistics::EXEC);
        }

        // Parent layout should refresh when layout would need to
        saveLayout.saved->RefreshOn(layout);
    } while(0);

    glPopAttrib();

    // As we have use our own GL functions, we need
    // to resync after glPopAttrib to assure that our
    //GL state is consistent with the real GL state. Refs #2970
    GL.Sync();

    // Bind the resulting texture and save current infos
    uint texId = frame.bind();
    layout->Add(new FillTexture(texId));
    GL.TextureSize(w, h);

    record(frames, "Texture ID %u size %u x %u pixels", texId, w, h);
    if (withDepth)
    {
        uint depthTexId = frame.depthTexture();
        withDepth->value = depthTexId;
    }

    return new Integer(texId, self->Position());
}


Tree* Widget::drawingCache(Scope *scope, Tree *self,
                           double version, Tree *prog)
// ----------------------------------------------------------------------------
//   Create a compiled display list out of the program's result
// ----------------------------------------------------------------------------
{
    Tree *result = XL::xl_false;

    // Get or build the current frame if we don't have one
    DisplayListInfo *info = self->GetInfo<DisplayListInfo>();
    if (!info)
    {
        // First drawing: draw the hard way
        info = new DisplayListInfo();
        self->SetInfo<DisplayListInfo>(info);
        info->version = version - 1;
    }
    if (info->version != version)
    {
        info->version = version;

        Layout *parent = layout;
        GLAllStateKeeper saveGL;
        XL::Save<Layout *> saveLayout(layout, layout->NewChild());

        result = saveAndEvaluate(scope, prog);

        stats.end(Statistics::EXEC);
        stats.begin(Statistics::DRAW);

        GL.NewList(info->displayListID, GL_COMPILE);
        layout->Draw(NULL);
        GL.EndList();

        stats.end(Statistics::DRAW);
        stats.begin(Statistics::EXEC);

        // Parent layout should refresh when layout would need to
        parent->RefreshOn(layout);

        // Delete the layout (it's not a child of the outer layout)
        delete layout;
        layout = NULL;
    }

    layout->Add(new CachedDrawing(info->displayListID));
    return result;
}


Integer* Widget::thumbnail(Scope *scope,
                           Tree *self, scale s, double interval, text page)
// ----------------------------------------------------------------------------
//   Generate a texture with a page thumbnail of the given page
// ----------------------------------------------------------------------------
{
    // Need to synchronize GL states
    // just before glPushAttrib.
    GL.Sync();

    // We need to use original glPushAttrib/glPopAttrib
    // as QT fbo functions modify implicitly GL states.
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // Prohibit recursion on thumbnails
    if (page == pageName || !xlProgram)
        return new Integer((longlong)0);

    Tree *prog = xlProgram->tree;
    double w = width() * s;
    double h = height() * s;

    // Get or build the current frame if we don't have one
    MultiFrameInfo<text> *multiframe = self->GetInfo< MultiFrameInfo<text> >();
    if (!multiframe)
    {
        multiframe = new MultiFrameInfo<text>();
        self->SetInfo< MultiFrameInfo<text> > (multiframe);
    }
    FrameInfo &frame = multiframe->frame(page);
    bool forceEval = !w_event;
    if (!frame.layout)
    {
        frame.layout = new SpaceLayout(this);
        forceEval = true;
    }
    if (frame.layout->body != prog || frame.layout->scope != scope)
    {
        frame.layout->body = prog;
        frame.layout->scope = scope;
        forceEval = true;
    }

    do
    {
        GLAllStateKeeper saveGL;
        XL::Save<Layout *> saveLayout(layout,frame.layout);
        XL::Save<Point3> saveCenter(cameraTarget, cameraTarget);
        XL::Save<Point3> saveEye(cameraPosition, cameraPosition);
        XL::Save<Vector3> saveUp(cameraUpVector, cameraUpVector);
        XL::Save<double> saveZoom(zoom, zoom);
        XL::Save<double> saveScaling(scaling, scaling * s);
        XL::Save<text> savePage(pageName, page);
        XL::Save<text> saveLastPage(lastPageName, page);
        XL::Save<page_map> saveLinks(pageLinks, pageLinks);
        XL::Save<page_list> saveList(pageNames, pageNames);
        XL::Save<uint> savePageId(pageId, 0);
        XL::Save<uint> savePageFound(pageFound, 0);
        XL::Save<uint> savePageShown(pageShown, pageShown);
        XL::Save<uint> savePageTotal(pageShown, pageTotal);
        XL::Save<Tree_p> savePageTree(pageTree, pageTree);
        XL::Save<bool> saveDAP(drawAllPages, false);
        XL::Save<QPrinter *> savePrinter(printer, NULL);

        // Clear the background and setup initial state
        frame.resize(w,h);
        setup(w, h);

        // Evaluate the program, not the context files (bug #1054)
        bool draw = false;
        if (forceEval)
        {
            layout->Clear();
            if (prog)
                saveAndEvaluate(scope, prog);
            draw = true;
        }
        else if (layout->Refresh(w_event, CurrentTime()))
        {
            draw = true;
        }

        // Draw the layout in the frame context
        if (draw)
        {
            stats.end(Statistics::EXEC);
            stats.begin(Statistics::DRAW);
            frame.begin();
            layout->Draw(NULL);
            frame.end();
            stats.end(Statistics::DRAW);
            stats.begin(Statistics::EXEC);
        }

        // Update refresh time
        layout->RefreshOn(QEvent::Timer, CurrentTime() + interval);
    } while(0);

    glPopAttrib();

    // As we have use our own GL functions, we need
    // to resync after glPopAttrib to assure that our
    //GL state is consistent with the real GL state. Refs #2970
    GL.Sync();

    // Bind the resulting texture and save current infos
    uint texId = frame.bind();
    layout->Add(new FillTexture(texId));
    GL.TextureSize (w, h);

    return new Integer(texId, self->Position());
}


Integer* Widget::linearGradient(Scope *scope, Tree *self,
                                Real *start_x, Real *start_y,
                                Real *end_x, Real *end_y,
                                double w, double h, Tree *prog)
// ----------------------------------------------------------------------------
//   Generate a texture to draw a linear gradient
// ----------------------------------------------------------------------------
{
    // Need to synchronize GL states
    // just before glPushAttrib.
    GL.Sync();

    // We need to use glPushAttrib/glPopAttrib
    // as QT fbo functions modify implicitly some GL states.
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    Tree *result = XL::xl_false;
    if (w < 16) w = 16;
    if (h < 16) h = 16;

    // Get or build the current frame if we don't have one
    MultiFrameInfo<uint> *multiframe = self->GetInfo< MultiFrameInfo<uint> >();
    if (!multiframe)
    {
        multiframe = new MultiFrameInfo<uint>();
        self->SetInfo< MultiFrameInfo<uint> > (multiframe);
    }
    uint id = shapeId();
    FrameInfo &frame = multiframe->frame(id);

    Layout *parent = layout;
    do
    {
        GLAllStateKeeper saveGL;
        XL::Save<Layout *> saveLayout(layout, layout->NewChild());
        XL::Save<Point3> saveCenter(cameraTarget, Point3(0,0,0));
        XL::Save<Point3> saveEye(cameraPosition, defaultCameraPosition);
        XL::Save<Vector3> saveUp(cameraUpVector, Vector3(0,1,0));
        XL::Save<double> saveZoom(zoom, 1);
        XL::Save<double> saveScaling(scaling, scalingFactorFromCamera());

        // Clear the background and setup initial state
        frame.resize(w,h);
        setup(w, h);

        // Define a painter to draw in current frame
        FramePainter painter(&frame);
        painter.setRenderHint(QPainter::Antialiasing, true);

        // Define our gradient type
        gradient = new QLinearGradient(*start_x, *start_y, *end_x, *end_y);

        // Evaluate the program
        result = saveAndEvaluate(scope, prog);

        // Draw gradient in a rectangle
        painter.fillRect(0, 0, w, h, (*gradient));

        delete gradient;
        gradient = NULL;

        // Parent layout should refresh when layout would need to
        parent->RefreshOn(layout);
        // Delete the layout (it's not a child of the outer layout)
        delete layout;
        layout = NULL;
    } while (0); // State keeper and layout

    glPopAttrib();

    // As we have use our own GL functions, we need
    // to resync after glPopAttrib to assure that our
    //GL state is consistent with the real GL state. Refs #2970
    GL.Sync();

    // Bind the resulting texture and save current infos
    uint texId = frame.bind();
    layout->Add(new FillTexture(texId));
    GL.TextureSize (w, h);

    return new Integer(texId, self->Position());
}


Integer* Widget::radialGradient(Scope *scope, Tree *self,
                                Real *center_x, Real *center_y,
                                Real *radius,
                                double w, double h, Tree *prog)
// ----------------------------------------------------------------------------
//   Generate a texture to draw a radial gradient
// ----------------------------------------------------------------------------
{
    // Need to synchronize GL states
    // just before glPushAttrib.
    GL.Sync();

    // We need to use original glPushAttrib/glPopAttrib
    // as QT fbo functions modify implicitly GL states.
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    Tree *result = XL::xl_false;
    if (w < 16) w = 16;
    if (h < 16) h = 16;

    // Get or build the current frame if we don't have one
    MultiFrameInfo<uint> *multiframe = self->GetInfo< MultiFrameInfo<uint> >();
    if (!multiframe)
    {
        multiframe = new MultiFrameInfo<uint>();
        self->SetInfo< MultiFrameInfo<uint> > (multiframe);
    }
    uint id = shapeId();
    FrameInfo &frame = multiframe->frame(id);

    Layout *parent = layout;
    do
    {
        GLAllStateKeeper saveGL;
        XL::Save<Layout *> saveLayout(layout, layout->NewChild());
        XL::Save<Point3> saveCenter(cameraTarget, Point3(0,0,0));
        XL::Save<Point3> saveEye(cameraPosition, defaultCameraPosition);
        XL::Save<Vector3> saveUp(cameraUpVector, Vector3(0,1,0));
        XL::Save<double> saveZoom(zoom, 1);
        XL::Save<double> saveScaling(scaling, scalingFactorFromCamera());

        // Clear the background and setup initial state
        frame.resize(w,h);
        setup(w, h);

        // Define a painter to draw in current frame
        FramePainter painter(&frame);
        painter.setRenderHint(QPainter::Antialiasing, true);

        // Define our gradient type
        gradient = new QRadialGradient(*center_x, *center_y, *radius);

        // Evaluate the program
        result = saveAndEvaluate(scope, prog);

        // Draw gradient in a rectangle
        painter.fillRect(0, 0, w, h, (*gradient));

        delete gradient;
        gradient = NULL;

        // Parent layout should refresh when layout would need to
        parent->RefreshOn(layout);
        // Delete the layout (it's not a child of the outer layout)
        delete layout;
        layout = NULL;
    } while (0); // State keeper and layout

    glPopAttrib();

    // As we have use our own GL functions, we need
    // to resync after glPopAttrib to assure that our
    //GL state is consistent with the real GL state. Refs #2970
    GL.Sync();

    // Bind the resulting texture and save current infos
    uint texId = frame.bind();
    layout->Add(new FillTexture(texId));
    GL.TextureSize (w, h);

    return new Integer(texId, self->Position());
}


Integer* Widget::conicalGradient(Scope *scope, Tree *self,
                                 Real *center_x, Real *center_y,
                                 Real *angle,
                                 double w, double h, Tree *prog)
// ----------------------------------------------------------------------------
//   Generate a texture to draw a conical gradient
// ----------------------------------------------------------------------------
{
    // Need to synchronize GL states
    // just before glPushAttrib.
    GL.Sync();

    // We need to use original glPushAttrib/glPopAttrib
    // as QT fbo functions modify implicitly GL states.
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    Tree *result = XL::xl_false;
    if (w < 16) w = 16;
    if (h < 16) h = 16;

    // Get or build the current frame if we don't have one
    MultiFrameInfo<uint> *multiframe = self->GetInfo< MultiFrameInfo<uint> >();
    if (!multiframe)
    {
        multiframe = new MultiFrameInfo<uint>();
        self->SetInfo< MultiFrameInfo<uint> > (multiframe);
    }
    uint id = shapeId();
    FrameInfo &frame = multiframe->frame(id);

    Layout *parent = layout;
    do
    {
        GLAllStateKeeper saveGL;
        XL::Save<Layout *> saveLayout(layout, layout->NewChild());
        XL::Save<Point3> saveCenter(cameraTarget, Point3(0,0,0));
        XL::Save<Point3> saveEye(cameraPosition, defaultCameraPosition);
        XL::Save<Vector3> saveUp(cameraUpVector, Vector3(0,1,0));
        XL::Save<double> saveZoom(zoom, 1);
        XL::Save<double> saveScaling(scaling, scalingFactorFromCamera());

        // Clear the background and setup initial state
        frame.resize(w,h);
        setup(w, h);

        // Define a painter to draw in current frame
        FramePainter painter(&frame);
        painter.setRenderHint(QPainter::Antialiasing, true);

        // Define our gradient type
        gradient = new QConicalGradient(*center_x, *center_y, *angle);

        // Evaluate the program
        result = saveAndEvaluate(scope, prog);

        // Draw gradient in a rectangle
        painter.fillRect(0, 0, w, h, (*gradient));

        delete gradient;
        gradient = NULL;

        // Parent layout should refresh when layout would need to
        parent->RefreshOn(layout);
        // Delete the layout (it's not a child of the outer layout)
        delete layout;
        layout = NULL;
    } while (0); // State keeper and layout

    glPopAttrib();

    // As we have use our own GL functions, we need
    // to resync after glPopAttrib to assure that our
    //GL state is consistent with the real GL state. Refs #2970
    GL.Sync();

    // Bind the resulting texture and save current infos
    uint texId = frame.bind();
    layout->Add(new FillTexture(texId));
    GL.TextureSize (w, h);

    return new Integer(texId, self->Position());
}


Name *Widget::offlineRendering(Tree *self)
// ----------------------------------------------------------------------------
//   Return true if we are currently rendering offline
// ----------------------------------------------------------------------------
{
    return inOfflineRendering ? XL::xl_true : XL::xl_false;
}


Tree *Widget::urlPaint(Tree *self,
                       Real *x, Real *y, Real *w, Real *h,
                       Text *url, Integer *progress)
// ----------------------------------------------------------------------------
//   Draw a URL in the curent frame
// ----------------------------------------------------------------------------
{
#ifdef CFG_NO_QTWEBKIT
    return XL::xl_false;
#else
    XL::Save<Layout *> saveLayout(layout, layout->AddChild(selectionId()));
    if (!urlTexture(self, *w, *h, url, progress))
        return XL::xl_false;

    WebViewSurface *surface = self->GetInfo<WebViewSurface>();
    layout->Add(new ClickThroughRectangle(Box(x-w/2, y-h/2, w, h), surface));
    if (currentShape)
        layout->Add(new WidgetManipulator(currentShape, x, y, w, h, surface));
    return XL::xl_true;
#endif
}


Integer* Widget::urlTexture(Tree *self, double w, double h,
                            Text *url, Integer *progress)
// ----------------------------------------------------------------------------
//   Make a texture out of a given URL
// ----------------------------------------------------------------------------
{
#ifdef CFG_NO_QTWEBKIT
    return new Integer(0, self->Position());
#else
    if (w < 16) w = 16;
    if (h < 16) h = 16;

    // Get or build the current frame if we don't have one
    WebViewSurface *surface = self->GetInfo<WebViewSurface>();
    if (!surface)
    {
        surface = new WebViewSurface(self, this);
        self->SetInfo<WebViewSurface> (surface);
    }

    // Resize to requested size, bind texture and save current infos
    surface->resize (w,h);
    uint texId = surface->bindURL(url, progress);
    layout->Add(new FillTexture(texId));
    GL.TextureSize (w, h);

    return new Integer(texId, self->Position());
#endif
}


Tree *Widget::lineEdit(Scope *scope, Tree *self,
                       Real *x, Real *y, Real *w, Real *h,
                       Text *txt)
// ----------------------------------------------------------------------------
//   Draw a line editor in the current frame
// ----------------------------------------------------------------------------
{
    XL::Save<Layout *> saveLayout(layout, layout->AddChild(selectionId()));
    lineEditTexture(scope, self, *w, *h, txt);
    LineEditSurface *surface = txt->GetInfo<LineEditSurface>();
    layout->Add(new ClickThroughRectangle(Box(*x-*w/2, *y-*h/2, *w, *h),
                                          surface));
    if (currentShape)
        layout->Add(new WidgetManipulator(currentShape, x, y, w, h, surface));
    return XL::xl_true;
}


Integer* Widget::lineEditTexture(Scope *scope, Tree *self,
                                 double w, double h, Text *txt)
// ----------------------------------------------------------------------------
//   Make a texture out of a given line editor
// ----------------------------------------------------------------------------
{
    if (w < 16) w = 16;
    if (h < 16) h = 16;

    // Get or build the current frame if we don't have one
    LineEditSurface *surface = txt->GetInfo<LineEditSurface>();
    if (!surface)
    {
        surface = new LineEditSurface(scope, txt, this);
        txt->SetInfo<LineEditSurface> (surface);
    }


    // Resize to requested size, bind texture and save current infos
    surface->resize (w, h);
    uint texId = surface->bindText(txt);
    layout->Add(new FillTexture(texId));
    GL.TextureSize (w, h);

    return new Integer(texId, self->Position());
}


Tree *Widget::radioButton(Scope *scope, Tree *self,
                          Real *x,Real *y, Real *w,Real *h,
                          Text *name, Text *label,
                          Text *sel, Tree *act)
// ----------------------------------------------------------------------------
//   Draw a radio button in the curent frame
// ----------------------------------------------------------------------------
{
    XL::Save<Layout *> saveLayout(layout, layout->AddChild(selectionId()));
    radioButtonTexture(scope, self, *w, *h, name, label, sel, act);
    return abstractButton(scope, self, name, x, y, w, h);
}


Integer* Widget::radioButtonTexture(Scope *scope, Tree *self,
                                    double w, double h,
                                    Text *name, Text *label,
                                    Text * sel, Tree *act)
// ----------------------------------------------------------------------------
//   Make a texture out of a given radio button
// ----------------------------------------------------------------------------
{
    if (w < 16) w = 16;
    if (h < 16) h = 16;

    // Get or build the current frame if we don't have one
    AbstractButtonSurface *surface = name->GetInfo<AbstractButtonSurface>();
    if (!surface)
    {
        surface = new RadioButtonSurface(scope, name, this, +name->value);
        name->SetInfo<AbstractButtonSurface> (surface);
    }

    // Resize to requested size, bind texture and save current infos
    surface->resize (w, h);
    uint texId = surface->bindButton(label, act, sel);
    layout->Add(new FillTexture(texId));
    GL.TextureSize (w, h);

    return new Integer(texId, self->Position());
}


Tree *Widget::checkBoxButton(Scope *scope, Tree *self,
                             Real *x,Real *y, Real *w, Real *h,
                             Text *name, Text *label, Text * sel, Tree *act)
// ----------------------------------------------------------------------------
//   Draw a check button in the curent frame
// ----------------------------------------------------------------------------
{
    XL::Save<Layout *> saveLayout(layout, layout->AddChild(selectionId()));
    checkBoxButtonTexture(scope, self, *w, *h, name, label, sel, act);
    return abstractButton(scope, self, name, x, y, w, h);
}


Integer* Widget::checkBoxButtonTexture(Scope *scope, Tree *self,
                                       double w, double h, Text *name,
                                       Text *label, Text * sel, Tree *act)
// ----------------------------------------------------------------------------
//   Make a texture out of a given checkbox button
// ----------------------------------------------------------------------------
{
    if (w < 16) w = 16;
    if (h < 16) h = 16;

    // Get or build the current frame if we don't have one
    AbstractButtonSurface *surface = name->GetInfo<AbstractButtonSurface>();
    if (!surface)
    {
        surface = new CheckBoxSurface(scope, name, this, +name->value);
        name->SetInfo<AbstractButtonSurface> (surface);
    }

    // Resize to requested size, bind texture and save current infos
    surface->resize (w, h);
    uint texId = surface->bindButton(label, act, sel);
    layout->Add(new FillTexture(texId));
    GL.TextureSize (w, h);

    return new Integer(texId, self->Position());
}


Tree *Widget::pushButton(Scope *scope, Tree *self,
                         Real *x, Real *y, Real *w, Real *h,
                         Text *name, Text *label, Tree * act)
// ----------------------------------------------------------------------------
//   Draw a push button in the curent frame
// ----------------------------------------------------------------------------
{
    XL::Save<Layout *> saveLayout(layout, layout->AddChild(selectionId()));
    pushButtonTexture(scope, self, *w, *h, name, label, act);
    return abstractButton(scope, self, name, x, y, w, h);
}


Integer* Widget::pushButtonTexture(Scope *scope, Tree *self,
                                   double w, double h, Text *name,
                                   Text *label, Tree *act)
// ----------------------------------------------------------------------------
//   Make a texture out of a given push button
// ----------------------------------------------------------------------------
{
    if (w < 16) w = 16;
    if (h < 16) h = 16;

    // Get or build the current frame if we don't have one
    AbstractButtonSurface *surface = name->GetInfo<AbstractButtonSurface>();
    if (!surface)
    {
        surface = new PushButtonSurface(scope, name, this, +name->value);
        name->SetInfo<AbstractButtonSurface> (surface);
    }

    // Resize to requested size, bind texture and save current infos
    surface->resize (w, h);
    uint texId = surface->bindButton(label, act, NULL);
    layout->Add(new FillTexture(texId));
    GL.TextureSize (w, h);

    return new Integer(texId, self->Position());
}


Tree *Widget::abstractButton(Scope *scope, Tree *self, Text *name,
                             Real *x, Real *y, Real *w, Real *h)
// ----------------------------------------------------------------------------
//   Draw any button in the curent frame
// ----------------------------------------------------------------------------
{
    AbstractButtonSurface *surface = name->GetInfo<AbstractButtonSurface>();

    if (currentGroup &&
        !currentGroup->buttons().contains((QAbstractButton*)surface->widget))
    {
        currentGroup->addButton((QAbstractButton*)surface->widget);
    }

    if (currentGridLayout)
    {
        currentGridLayout->addWidget(surface->widget, *x, *y);
        surface->widget->setVisible(true);
        return XL::xl_true;
    }
    layout->Add (new FillColor(1.0, 1.0, 1.0, 1.0));
    layout->Add(new ClickThroughRectangle(Box(*x-*w/2, *y-*h/2, *w, *h),
                                          surface));
    if (currentShape)
        layout->Add(new WidgetManipulator(currentShape, x, y, w, h, surface));

    return XL::xl_true;
}


QColorDialog *Widget::colorDialog = NULL;
Tree *Widget::colorChooser(Scope *scope,
                           Tree *self,
                           text treeName,
                           Tree *action)
// ----------------------------------------------------------------------------
//   Draw a color chooser
// ----------------------------------------------------------------------------
{
    if (colorDialog)
    {
        delete colorDialog;
        colorDialog = NULL;
    }

    colorAction = action;
    colorScope = scope;
    colorName = treeName;

    // Setup the color dialog
    colorDialog = new QColorDialog(this);
    colorDialog->setObjectName("colorDialog");
    colorDialog->setOption(QColorDialog::ShowAlphaChannel, true);
    colorDialog->setOption(QColorDialog::DontUseNativeDialog, false);
    colorDialog->setModal(false);
    updateColorDialog();

    // Connect the dialog and sh'ow it
#ifdef Q_WS_MAC
    // To make the color dialog look Mac-like, we don't show OK and Cancel
    colorDialog->setOption(QColorDialog::NoButtons, true);
#else
    // On other platforms, it's expected fro OK and Cancel to show up
    connect(colorDialog, SIGNAL(colorSelected (const QColor&)),
            this, SLOT(colorChosen(const QColor &)));
    connect(colorDialog, SIGNAL(rejected()), this, SLOT(colorRejected()));
#endif
    connect(colorDialog, SIGNAL(currentColorChanged (const QColor&)),
            this, SLOT(colorChanged(const QColor &)));
    colorDialog->show();

    return XL::xl_true;
}


void Widget::colorRejected()
// ----------------------------------------------------------------------------
//   Slot called by the color widget's "cancel" button.
// ----------------------------------------------------------------------------
{
    colorChanged(originalColor);
}


void Widget::colorChosen(const QColor & col)
// ----------------------------------------------------------------------------
//   Slot called by the color widget a color is chosen and dialog is closed
// ----------------------------------------------------------------------------
{
    colorChanged(col);
}


void Widget::colorChanged(const QColor & col)
// ----------------------------------------------------------------------------
//   Slot called by the color widget when a color is selected
// ----------------------------------------------------------------------------
{
    if (!colorAction)
        return;

    record(widget, "Color %s was chosen for %t", col.name(), colorAction);

    // The tree to be evaluated needs its own symbol table before evaluation
    ColorTreeClone replacer(col);
    Tree *toBeEvaluated = colorAction;
    toBeEvaluated = toBeEvaluated->Do(replacer);

    // Evaluate the input tree
    assert(!current); // Recursive evaluation due to events is bad...
    TaoSave saveCurrent(current, this);
    xl_evaluate(colorScope, toBeEvaluated);
}


void Widget::updateColorDialog()
// ----------------------------------------------------------------------------
//   Pick colors from the selection
// ----------------------------------------------------------------------------
{
    if (!colorDialog)
        return;

    TaoSave saveCurrent(current, this);

    // Make sure we don't update the trees, only get their colors
    XL::Save<Tree_p> action(colorAction, NULL);
    Color c = selectionColor[colorName];
    originalColor.setRgbF(c.red, c.green, c.blue, c.alpha);

    // Get the default color from the first selected shape
    for (tree_set::iterator i = selectionTrees.begin();
         i != selectionTrees.end();
         i++)
    {
        attribute_args color;
        if (get(*i, colorName, color) && color.size() == 4)
        {
            originalColor.setRgbF(color[0], color[1], color[2], color[3]);
            break;
        }
    }
    colorDialog->setCurrentColor(originalColor);
}


QFontDialog *Widget::fontDialog = NULL;
Tree *Widget::fontChooser(Scope *scope,
                          Tree *self,
                          text name,
                          Tree *action)
// ----------------------------------------------------------------------------
//   Draw a font chooser
// ----------------------------------------------------------------------------
{
    if (fontDialog)
    {
        delete fontDialog;
        fontDialog = NULL;
    }

    fontDialog = new QFontDialog(this);
    fontDialog->setObjectName("fontDialog");
    connect(fontDialog, SIGNAL(fontSelected (const QFont&)),
            this, SLOT(fontChosen(const QFont &)));

    fontDialog->setOption(QFontDialog::NoButtons, true);
    fontDialog->setOption(QFontDialog::DontUseNativeDialog, false);
    fontDialog->setModal(false);
    if (!name.empty())
        selectionFont = QFont(+name);
    updateFontDialog();

    fontDialog->show();
    fontAction = action;
    fontScope = scope;

    return XL::xl_true;
}


void Widget::fontChosen(const QFont& ft)
// ----------------------------------------------------------------------------
//    A font was selected. Evaluate the action.
// ----------------------------------------------------------------------------
{
    if (!fontAction)
        return;

    record(widget, "Font %s chosen for action %t", ft.toString(), fontAction);

    // The tree to be evaluated needs its own symbol table before evaluation
    FontTreeClone replacer(ft);
    Tree *toBeEvaluated = fontAction;
    toBeEvaluated = toBeEvaluated->Do(replacer);

    // Evaluate the input tree
    TaoSave saveCurrent(current, this);
    xl_evaluate(fontScope, toBeEvaluated);
}


void Widget::updateFontDialog()
// ----------------------------------------------------------------------------
//   Pick font information from the selection
// ----------------------------------------------------------------------------
{
    if (!fontDialog)
        return;
    fontDialog->setCurrentFont(selectionFont);
}


QFileDialog *Widget::fileDialog = NULL;
Tree *Widget::fileChooser(Scope *scope, Tree *self, Tree *properties)
// ----------------------------------------------------------------------------
//   Draw a file chooser
// ----------------------------------------------------------------------------
{
    if (fileDialog)
    {
        delete fileDialog;
        fileDialog = NULL;
    }

    // Setup the color dialog
    fileDialog = new QFileDialog(this);
    fileDialog->setObjectName("fileDialog");
    // To be able to set the selectFileName programmatically.
    // Can be revisited when tests will be integrated in new module fmw.
    fileDialog->setOption(QFileDialog::DontUseNativeDialog, true);
    currentFileDialog = fileDialog;
    fileDialog->setModal(false);

    updateFileDialog(scope, properties);

    // Connect the dialog and show it
    connect(fileDialog, SIGNAL(fileSelected (const QString&)),
            this, SLOT(fileChosen(const QString &)));
    fileDialog->show();

    return XL::xl_true;
}


void Widget::updateFileDialog(Scope *scope, Tree *properties)
// ----------------------------------------------------------------------------
//   Execute code for a file dialog
// ----------------------------------------------------------------------------
//   The action for a file dialog contains names that can be shortcuts
//   to the actual PREFIX name in graphics.tbl. For instance, action in
//   a file dialog is a shortcut for file_chooser_action.
//   This function performs the replacement
{
    NameToNameReplacement map;

    map["action"]    = "file_chooser_action";
    map["directory"] = "file_chooser_directory";
    map["label"]     = "file_chooser_label";
    map["filter"]    = "file_chooser_filter";

    Tree *code = map.Replace(properties);
    xl_evaluate(scope, code);
}


Tree *Widget::setFileDialogAction(Scope *scope, Tree *self, Tree *action)
// ----------------------------------------------------------------------------
//   Set the action that will be execute when OK is pressed.
// ----------------------------------------------------------------------------
{
    record(widget, "Set file dialoc action %t", action);
    if (currentFileDialog)
    {
        Scope_p s = scope;
        Tree_p a = action;
        currentFileDialog->setProperty("TAO_ACTION", QVariant::fromValue(a));
        currentFileDialog->setProperty("TAO_SCOPE", QVariant::fromValue(s));
        return XL::xl_true;
    }
    return XL::xl_false;
}


Tree *Widget::setFileDialogDirectory(Tree *self, text dirname)
// ----------------------------------------------------------------------------
//   Set the directory to open first
// ----------------------------------------------------------------------------
{
    record(widget, "Set file dialog directory %s", dirname);
    if (currentFileDialog)
    {
        currentFileDialog->setDirectory(+dirname);
        return XL::xl_true;
    }
    return XL::xl_false;
}


Tree *Widget::setFileDialogFilter(Tree *self, text filters)
// ----------------------------------------------------------------------------
//   Set the file filters (file pattern, e.g. *.img)
// ----------------------------------------------------------------------------
{
    record(widget, "Set file dialog filters %s", filters);
    if (currentFileDialog)
    {
        currentFileDialog->setNameFilter(+filters);
        return XL::xl_true;
    }
    return XL::xl_false;
}


Tree *Widget::setFileDialogLabel(Tree *self, text label, text value)
// ----------------------------------------------------------------------------
//   Set labels on a file dialog
// ----------------------------------------------------------------------------
// 5 labels may be set : LookIn, FileName, FileType, Accept, Reject
{
    record(widget, "Set file dialog label %s value %s", label, value);
    if (currentFileDialog)
    {
        currentFileDialog->setLabelText(toDialogLabel[label], +value);
        return XL::xl_true;
    }
    return XL::xl_false;
}


void Widget::fileChosen(const QString & filename)
// ----------------------------------------------------------------------------
//   Slot called by the filechooser widget when a file is selected
// ----------------------------------------------------------------------------
{
    if (!currentFileDialog)
        return;

    Tree *action = currentFileDialog->property("TAO_ACTION").value<Tree_p>();
    Scope *scope = currentFileDialog->property("TAO_SCOPE").value<Scope_p>();
    currentFileDialog->close();
    record(widget, "File %s was chosen, action %t", filename, action);
    if (!action)
        return;

    // We override names 'filename', 'filepath', 'filepathname', 'relfilepath'
    QFileInfo file(filename);
    QString relFilePath = QDir((taoWindow())->currentProjectFolderPath()).
        relativeFilePath(file.absoluteFilePath());
    if (relFilePath.contains(".."))
    {
        QDir::home().
            relativeFilePath(file.absoluteFilePath());
        if (relFilePath.contains(".."))
        {
            relFilePath = file.absoluteFilePath();
        } else {
            relFilePath.prepend("~/");
        }
    }
    NameToTextReplacement map;

    map["file_name"] = +file.fileName();
    map["file_directory"] = +file.absolutePath();
    map["file_path"] = +file.absoluteFilePath();
    map["rel_file_path"] = +relFilePath;

    Tree *toBeEvaluated = map.Replace(action);

    // Evaluate the input tree
    TaoSave saveCurrent(current, this);
    xl_evaluate(scope, toBeEvaluated);
}


Tree *Widget::buttonGroup(Scope *scope, Tree *self,
                          bool exclusive, Tree *buttons)
// ----------------------------------------------------------------------------
//   Create a button group for radio buttons
// ----------------------------------------------------------------------------
{
    GroupInfo *grpInfo = buttons->GetInfo<GroupInfo>();
    if (!grpInfo)
    {
        grpInfo = new GroupInfo(scope, self, buttons, this);
        grpInfo->setExclusive(exclusive);
        buttons->SetInfo<GroupInfo>(grpInfo);
    }
    currentGroup = grpInfo;

    XL::FindChildAction findAction("action", 1);
    if (Tree* tmp = buttons->Do(findAction))
        if (Prefix *act = tmp->AsPrefix())
            act->left = new Name("button_group_action", act->left->Position());

    // Evaluate the input tree
    xl_evaluate(scope, buttons);
    currentGroup = NULL;

    return XL::xl_true;
}


Tree *Widget::setButtonGroupAction(Scope *scope, Tree *self, Tree *action)
// ----------------------------------------------------------------------------
//   Set the action to be executed by the current buttonGroup if any.
// ----------------------------------------------------------------------------
{
    if (currentGroup)
    {
        currentGroup->action = action;
        currentGroup->scope = scope;
    }
    return XL::xl_true;
}


Tree *Widget::groupBox(Scope *scope, Tree *self,
                       Real *x, Real *y, Real *w, Real *h,
                       Text *label, Tree *buttons)
// ----------------------------------------------------------------------------
//   Draw a group box in the curent frame
// ----------------------------------------------------------------------------
{
    XL::Save<Layout *> saveLayout(layout, layout->AddChild(selectionId()));

    groupBoxTexture(scope, self, *w, *h, label);

    GroupBoxSurface *surface = self->GetInfo<GroupBoxSurface>();
    layout->Add(new ClickThroughRectangle(Box(*x-*w/2, *y-*h/2, *w, *h),
                                          surface));
    if (currentShape)
        layout->Add(new WidgetManipulator(currentShape, x, y, w, h, surface));

    xl_evaluate(scope, buttons);

    surface->dirty = true;
    ((WidgetSurface*)surface)->bind();
    currentGridLayout = NULL;

    return XL::xl_true;
}


Integer* Widget::groupBoxTexture(Scope *scope, Tree *self,
                                 double w, double h, Text *label)
// ----------------------------------------------------------------------------
//   Make a texture out of a given group box
// ----------------------------------------------------------------------------
{
    if (w < 16) w = 16;
    if (h < 16) h = 16;

    // Get or build the current frame if we don't have one
    GroupBoxSurface *surface = self->GetInfo<GroupBoxSurface>();
    if (!surface)
    {
        surface = new GroupBoxSurface(scope, self, this);
        self->SetInfo<GroupBoxSurface> (surface);
    }

    currentGridLayout = surface->grid();


    // Resize to requested size, and bind texture
    surface->resize (w, h);
    uint texId = surface->bindButton(label);
    layout->Add(new FillTexture(texId));
    GL.TextureSize (w, h);

    return new Integer(texId, self->Position());
}


text Widget::currentDocumentFolder()
// ----------------------------------------------------------------------------
//   Return native path to current document folder
// ----------------------------------------------------------------------------
{
    Window *window = findTaoWidget()->taoWindow();
    return +QDir::toNativeSeparators(window->currentProjectFolderPath());
}


bool Widget::blink(double on, double off, double after)
// ----------------------------------------------------------------------------
//   Return true for 'on' seconds then false for 'off' seconds
// ----------------------------------------------------------------------------
{
    double runtime = Application::runTime();
    if (runtime <= after)
    {
        refreshOnAPI((int)QEvent::Timer, after - runtime);
        return true;
    }
    double time = Widget::currentTimeAPI();
    double mod = fmod(time, on + off);
    if (mod <= on)
    {
        refreshOnAPI((int)QEvent::Timer, time + on - mod);
        return true;
    }
    refreshOnAPI((int)QEvent::Timer, time + on + off - mod);
    return false;
}


Name *Widget::blink(Tree *self, Real *on, Real *off, Real *after)
// ----------------------------------------------------------------------------
//   Export 'blink' as a primitive
// ----------------------------------------------------------------------------
{
    return blink(on->value, off->value, after->value) ?
        XL::xl_true : XL::xl_false;
}


Name *Widget::hasLicense(Tree *self, Text *feature)
// ----------------------------------------------------------------------------
//   Export 'Licenses::Has' as a primitive
// ----------------------------------------------------------------------------
{
#ifdef CFG_NO_LICENSE
    return XL::xl_false;
#else
    return Licenses::Has(feature->value) ? XL::xl_true : XL::xl_false;
#endif
}


Name *Widget::checkLicense(Tree *self, Text *feature, Name *critical)
// ----------------------------------------------------------------------------
//   Export 'Licenses::Check' as a primitive
// ----------------------------------------------------------------------------
{
#ifdef CFG_NO_LICENSE
    return XL::xl_false;
#else
    bool crit = (critical == XL::xl_true) ? true : false;
    return Licenses::Check(feature->value, crit) ? XL::xl_true : XL::xl_false;
#endif
}


void Widget::setWatermarkText(text t, int w, int h)
// ----------------------------------------------------------------------------
//   Create a texture and make it the watermark of the current widget
// ----------------------------------------------------------------------------
{
    QImage image(w, h, QImage::Format_ARGB32);
    image.fill(0); // Transparent black
    QPainter painter;
    painter.begin(&image);
    QPainterPath path;
    path.addText(0, 0, QFont("Ubuntu", 40), +t);
    QRectF brect = path.boundingRect();
    path.translate((w - brect.width())/2, (h - brect.height())/2);
    painter.setBrush(QBrush(Qt::white));
    QPen pen(Qt::black);
    pen.setWidth(1);
    painter.setPen(pen);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawPath(path);
    painter.end();

    // Generate the GL texture
    GLAllStateKeeper save;
    QImage texture = QGLWidget::convertToGLFormat(image);
    if (!watermark)
        GL.GenTextures(1, &watermark);
    GL.BindTexture(GL_TEXTURE_2D, watermark);
    GL.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                  w, h, 0, GL_RGBA,
                  GL_UNSIGNED_BYTE, texture.bits());
    GL.BindTexture(GL_TEXTURE_2D, 0);

    watermarkText = t;
    watermarkWidth = w;
    watermarkHeight = h;

    IFTRACE(fileload)
        std::cerr << "Watermark created: text '" << t << "', texture id "
                  << watermark << "\n";
}


void Widget::setWatermarkTextAPI(text t, int w, int h)
// ----------------------------------------------------------------------------
//   Export setWatermarkText to the module API
// ----------------------------------------------------------------------------
{
    if (Tao()->showingEvaluationWatermark)
        return;
    Tao()->setWatermarkText(t, w, h);
}


void Widget::drawWatermark()
// ----------------------------------------------------------------------------
//   Draw watermark texture
// ----------------------------------------------------------------------------
{
    if (!watermark || !watermarkWidth || !watermarkHeight)
        return;
    GLAllStateKeeper save;
    GL.Color(1.0, 1.0, 1.0, 0.2);
    drawFullScreenTexture(watermarkWidth, watermarkHeight, watermark);
}


void Widget::drawFullScreenTexture(int texw, int texh, GLuint tex,
                                   bool centered)
// ----------------------------------------------------------------------------
//   Draw a texture centered over the full widget with wrapping enabled
// ----------------------------------------------------------------------------
{
    GL.Disable(GL_DEPTH_TEST);
    GL.DepthMask(GL_FALSE);
    GL.BindTexture(GL_TEXTURE_2D, tex);
    GL.TexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL.TexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GL.TexParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    GL.TexParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    GL.Enable(GL_TEXTURE_2D);
    GL.MatrixMode(GL_PROJECTION);
    GL.LoadIdentity();
    GL.MatrixMode(GL_MODELVIEW);
    GL.LoadIdentity();
    GL.LoadMatrix();
    float w = DisplayDriver::renderWidth(), h = DisplayDriver::renderHeight();
    float tw = w/texw, th = h/texh;
    float x1 = -tw/2, x2 = tw/2;
    float y1 = -th/2, y2 = th/2;
    if (centered)
    {
        x1 += 0.5; x2 += 0.5; y1 += 0.5; y2 += 0.5;
    }
    GL.Begin(GL_QUADS);
    GL.TexCoord(x1, y1);
    GL.Vertex  (-1, -1);
    GL.TexCoord(x2, y1);
    GL.Vertex   ( 1, -1);
    GL.TexCoord(x2, y2);
    GL.Vertex   ( 1,  1);
    GL.TexCoord(x1, y2);
    GL.Vertex   (-1,  1);
    GL.End();
    GL.Enable(GL_DEPTH_TEST);
    GL.DepthMask(GL_TRUE);
}


void Widget::drawWatermarkAPI()
// ----------------------------------------------------------------------------
//   Export drawWatermark to the module API
// ----------------------------------------------------------------------------
{
    Tao()->drawWatermark();
}



// ============================================================================
//
//    Chooser
//
// ============================================================================

Tree *Widget::chooser(Scope *scope, Tree *self, text caption)
// ----------------------------------------------------------------------------
//   Create a chooser with the given caption
// ----------------------------------------------------------------------------
//   Note: the current implementation doesn't prevent hierarchical choosers.
//   It's by design, I see only good reasons to disallow such hierarchies...
{
    Chooser *chooser = dynamic_cast<Chooser *> (activities);
    if (chooser)
        if (chooser->name == caption)
            return XL::xl_false;
    if (!xlProgram) return XL::xl_false;
    chooser = new Chooser(scope, caption, this);
    return XL::xl_true;
}


Tree *Widget::chooserChoice(Tree *self, text caption, Tree *command)
// ----------------------------------------------------------------------------
//   Create a chooser item and associate a command
// ----------------------------------------------------------------------------
{
    if (Chooser *chooser = dynamic_cast<Chooser *> (activities))
    {
        chooser->AddItem(caption, command);
        return XL::xl_true;
    }
    return XL::xl_false;
}


Tree *Widget::chooserCommands(Tree *self, text prefix, text label)
// ----------------------------------------------------------------------------
//   Add all commands in the current symbol table that have the given prefix
// ----------------------------------------------------------------------------
{
    if (!xlProgram)
        return XL::xl_false;

    if (Chooser *chooser = dynamic_cast<Chooser *> (activities))
    {
        chooser->AddCommands(prefix, label);
        return XL::xl_true;
    }
    return XL::xl_false;
}



Tree *Widget::chooserPages(Tree *self, Name *prefix, text label)
// ----------------------------------------------------------------------------
//   Add a list of pages to the chooser
// ----------------------------------------------------------------------------
{
    if (Chooser *chooser = dynamic_cast<Chooser *> (activities))
    {
        uint pnum = 1;

        page_list::iterator p;
        for (p = pageNames.begin(); p != pageNames.end(); p++)
        {
            text name = *p;
            Tree *action = new Prefix(prefix, new Text(name));
            std::ostringstream os;
            os << label << pnum << " " << name;
            std::string txt(os.str());
            chooser->AddItem(txt, action);
            if (pnum == pageShown)
                chooser->SetCurrentItem(txt);
            pnum++;
        }
        return XL::xl_true;
    }
    return XL::xl_false;
}


Tree *Widget::chooserBranches(Tree *self, Name *prefix, text label)
// ----------------------------------------------------------------------------
//   Add a list of branches to the chooser
// ----------------------------------------------------------------------------
{
#ifndef CFG_NOGIT
    Repository *repo = repository();
    Chooser *chooser = dynamic_cast<Chooser *> (activities);
    if (chooser && repo)
    {
        QStringList branches = repo->branches();
        foreach (QString branch, branches)
        {
            Tree *action = new Prefix(prefix, new Text(+branch));
            chooser->AddItem(label + +branch + "...", action);
        }
        return XL::xl_true;
    }
#endif
    return XL::xl_false;
}


Tree *Widget::chooserCommits(Tree *self, text branch, Name *prefix,
                             text label)
// ----------------------------------------------------------------------------
//   Add a list of commits to the chooser
// ----------------------------------------------------------------------------
{
#ifndef CFG_NOGIT
    Repository *repo = repository();
    Chooser *chooser = dynamic_cast<Chooser *> (activities);
    if (chooser && repo)
    {
        QList<Repository::Commit> commits = repo->history(+branch);
        commits.append(Repository::Commit(+branch));
        for (int n = commits.size() - 1; n >= 0; n--)
        {
            Repository::Commit c = commits[n];
            text ctext = +c.toString();
            Tree *action = new Prefix(prefix, new Text(+c.id));
            chooser->AddItem(label + ctext, action);
        }
        return XL::xl_true;
    }
#endif
    return XL::xl_false;
}


Tree *Widget::checkout(Tree *self, text what)
// ----------------------------------------------------------------------------
//   Checkout a branch or a commit. Called by chooser.
// ----------------------------------------------------------------------------
{
#ifndef CFG_NOGIT
    Repository *repo = repository();
    if (repo && repo->checkout(what))
        return XL::xl_true;
#endif
    return XL::xl_false;
}


Name *Widget::currentRepository(Tree *self)
// ----------------------------------------------------------------------------
//   Return true if we use a git repository with the current document
// ----------------------------------------------------------------------------
{
#ifndef CFG_NOGIT
    Repository *repo = repository();
    if (repo)
        return XL::xl_true;
#endif
    return XL::xl_false;
}


Tree *Widget::closeCurrentDocument(Tree *self)
// ----------------------------------------------------------------------------
//   Close the current document
// ----------------------------------------------------------------------------
{
    Window *window = taoWindow();
    QTimer::singleShot(0, window, SLOT(closeDocument()));
    return XL::xl_true;
}


Tree *Widget::quitTao(Tree *self)
// ----------------------------------------------------------------------------
//   Quit the application
// ----------------------------------------------------------------------------
{
    TaoApp->quit();
    return XL::xl_true;
}


// ============================================================================
//
//    Error management
//
// ============================================================================

Tree *Widget::runtimeError(Scope *scope, Tree *self, kstring msg, Tree *arg)
// ----------------------------------------------------------------------------
//   Display an error message from the input
// ----------------------------------------------------------------------------
{
    if (current)
    {
        // Stop refreshing
        current->inError = true;
#ifndef CFG_NOSRCEDIT
        if (current->xlProgram)
        {
            // Load source as plain text
            QString fname = +current->xlProgram->name;
            Window *window = Tao()->taoWindow();
            window->loadFileIntoSourceFileView(fname);
        }
#endif
    }
    return formulaRuntimeError(scope, self, msg, arg);
}


Tree *Widget::formulaRuntimeError(Scope *scope, Tree *self,
                                  kstring msg, Tree *arg)
// ----------------------------------------------------------------------------
//   Display a runtime error while executing a formula
// ----------------------------------------------------------------------------
{
    XL::Error err(msg, arg);

    if (current)
    {
        text message = err.Position() + ": " + err.Message();
        Window *window = Tao()->taoWindow();
        window->addError(+message);
        err.Display();
    }
    else
    {
        err.Display();
    }
    return XL::xl_false;
}


void Widget::clearErrors()
// ----------------------------------------------------------------------------
//   Clear all errors, e.g. because we reloaded a document
// ----------------------------------------------------------------------------
{
    inError = false;
    XL::MAIN->errors->Clear();
    taoWindow()->addSeparator("Reloaded");
}


void Widget::checkErrors()
// ----------------------------------------------------------------------------
//   Check if there were errors during evaluation, and display them if any
// ----------------------------------------------------------------------------
{
    if (XL::MAIN->HadErrors())
    {
        std::vector<XL::Error> errors = XL::MAIN->errors->errors;
        std::vector<XL::Error>::iterator ei;
        Window *window = taoWindow();
        XL::MAIN->errors->Clear();
        for (ei = errors.begin(); ei != errors.end(); ei++)
        {
            text pos = (*ei).Position();
            text err = (*ei).Message();
            text message = pos + ": " + err;
            window->addError(+message);
            text hint = +errorHint(+err);
            if (hint != "")
            {
                text message = pos + ": " + hint;
                window->addError(+message);
            }
        }
        inError = true;
    }
}



// ============================================================================
//
//   Menu management
//
// ============================================================================
// * Menu name philosophy :
// * The full name is used to register menus and menu items against
//   the menubar.  Those names are not displayed and must be unique.
// * Menu created by the XL programmer must be differentiated from the
//   originals ones because they have to be recreated or modified at
//   each loop of XL.  When top menus are deleted they recursively
//   delete their children (sub menus and menu items), so we have to
//   take care of sub menu at deletion time.
//
//
// * Menu and menu items lifecycle : Menus are created when the xl
//   program is executed the first time.  Menus display text can be
//   modified at each execution. At each loop, for each element (menu,
//   menu_item, toolbar,...) there name is looked for as a main window children,
//   if found, the order is checked against the registered value in
//   menuItems. If the order is OK, the label, etc are updated; if not
//   or not found at all a new element is created and registered.
// ============================================================================

Tree *Widget::menuItem(Scope *scope, Tree *self,
                       text name, text label, text iconFileName,
                       bool isCheckable, Text *isChecked, Tree *t)
// ----------------------------------------------------------------------------
//   Create a menu item
// ----------------------------------------------------------------------------
{
    // Do not update menus during transitions
    if (runningTransitionCode)
        return XL::xl_false;
    if (!currentMenu && !currentToolBar)
        return XL::xl_false;

    QString fullName = +name;

    if (QAction* act = taoWindow()->findChild<QAction*>(fullName))
    {
        // MenuItem found, update label, icon, checkmark if the order is OK.
        if (menuCount < menuItems.size() &&
            menuItems[menuCount] != NULL &&
            menuItems[menuCount]->fullname == fullName)
        {
            act->setText(+label);
            if (iconFileName != "")
                act->setIcon(QIcon(+iconFileName));
            else
                act->setIcon(QIcon());
            act->setChecked(strcasecmp(isChecked->value.c_str(), "true") == 0);

            menuCount++;
            return XL::xl_true;
        }

        // The name exist but it is not in the good order so clean it.
        delete act;
    }

    // Store the tree in the QAction.
    QVariant var = QVariant::fromValue(Tree_p(t));

    IFTRACE(menus)
    {
        std::cerr << "menuItem CREATION with name "
                  << fullName.toStdString()
                  << " at index " << menuCount << "\n";
        std::cerr.flush();
    }

    QAction * action;
    QWidget * par;
    if (currentMenu)
        par =  currentMenu;
    else
        par = currentToolBar;

    action = new QAction(+label, par);
    action->setData(var);


    // Set the item sensible to the selection
    if (fullName.startsWith("menu:selectW:"))
    {
        // Enabled action only if we need
        // (do not need if document is read only or noting is selected)
        action->setEnabled(hasSelection() && !isReadOnly());
        connect(this, SIGNAL(copyAvailableAndNotReadOnly(bool)),
                action, SLOT(setEnabled(bool)));
    }

    if (iconFileName != "")
        action->setIcon(QIcon(+iconFileName));
    else
        action->setIcon(QIcon());

    action->setCheckable(isCheckable);
    action->setChecked(strcasecmp(isChecked->value.c_str(), "true") == 0);
    action->setObjectName(fullName);

    if (menuCount >= menuItems.size())
        menuItems.resize(menuCount+10);

    if (menuItems[menuCount])
    {
        QAction*before = menuItems[menuCount]->action;
        if (currentMenu)
            currentMenu->insertAction(before, action);
        else
            currentToolBar->insertAction(before, action);

        delete menuItems[menuCount];
    }
    else
    {
        if (currentMenu)
            currentMenu->addAction(action);
        else
            currentToolBar->addAction(action);
    }

    menuItems[menuCount] = new MenuInfo(fullName, action);
    menuCount++;

    return XL::xl_true;
}



Tree * Widget::menuItemEnable(Tree *self, text name, bool enable)
// ----------------------------------------------------------------------------
//  Enable or disable a menu item
// ----------------------------------------------------------------------------
{
    // Do not update menus during transitions
    if (runningTransitionCode)
        return XL::xl_false;

    if (!currentMenu && !currentToolBar)
        return XL::xl_false;

    QString fullName = +name;
    if (QAction* act = taoWindow()->findChild<QAction*>(fullName))
    {
        act->setEnabled(enable);
        return XL::xl_true;
    }

    return XL::xl_false;
}



Tree *Widget::menu(text name, text label,
                   text iconFileName, bool isSubMenu)
// ----------------------------------------------------------------------------
// Add the menu to the current menu bar or create the contextual menu
// ----------------------------------------------------------------------------
{
    // Do not update menus during transitions
    if (runningTransitionCode)
        return XL::xl_false;

    bool isContextMenu = false;

    // Build the full name of the menu
    // Uses the current menu name, the given string and the isSubmenu.
    QString fullname = +name;
    if (fullname.startsWith(CONTEXT_MENU))
    {
        isContextMenu = true;
    }

    // If the menu is registered, no need to recreate it if same order as before
    // This is used at reload time.
    if (QMenu *tmp = taoWindow()->findChild<QMenu*>(fullname))
    {
        if (label == "" && iconFileName == "")
        {
            // Just set the current menu to the requested one
            currentMenu = tmp;
            return XL::xl_true;
        }

        if (menuCount < menuItems.size())
        {
            if (MenuInfo *menuInfo = menuItems[menuCount])
            {
                if (menuInfo->fullname == fullname)
                {
                    // Set the currentMenu and update the label and icon.
                    currentMenu = tmp;
                    if (label != menuInfo->title)
                    {
                        currentMenu->setTitle(+label);
                        menuInfo->title = label;
                    }
                    if (iconFileName != menuInfo->icon)
                    {
                        if (iconFileName != "")
                            currentMenu->setIcon(QIcon(+iconFileName));
                        else
                            currentMenu->setIcon(QIcon());
                        menuInfo->icon = iconFileName;
                    }
                    menuCount++;
                    return XL::xl_true;
                }
            }
        }
        // The name exist but it is not in the good order so clean it
        delete tmp;
    }

    QWidget *par = NULL;
    // The menu is not yet registered. Create it and set the currentMenu.
    if (isContextMenu)
    {
        currentMenu = new QMenu(taoWindow());
        connect(currentMenu, SIGNAL(triggered(QAction*)),
                this,        SLOT(userMenu(QAction*)));
    }
    else
    {
        if (isSubMenu && currentMenu)
            par = currentMenu;
        else if (currentMenuBar)
            par = currentMenuBar;
        else if (currentToolBar)
            par = currentToolBar;

        currentMenu = new QMenu(+label, par);
    }

    currentMenu->setObjectName(fullname);

    if (iconFileName != "")
        currentMenu->setIcon(QIcon(+iconFileName));

    if (menuCount >= menuItems.size())
        menuItems.resize(menuCount+10);

    if (par)
    {
        QAction *before = NULL;
        if (menuItems[menuCount])
        {
            before = menuItems[menuCount]->action;
        }
        else if (par == currentMenuBar)
        {
#if !defined(CFG_NOGIT) && !defined(CFG_NOEDIT)
            before = taoWindow()->shareMenu->menuAction();
#else
            before = taoWindow()->helpMenu->menuAction();
#endif
        }
        par->insertAction(before, currentMenu->menuAction());

        QToolButton* button = NULL;
        if (par == currentToolBar &&
            (button = dynamic_cast<QToolButton*>
             (currentToolBar-> widgetForAction(currentMenu->menuAction()))))
            button->setPopupMode(QToolButton::InstantPopup);
    }

    if (menuItems[menuCount])
        delete menuItems[menuCount];

    menuItems[menuCount] = new MenuInfo(fullname, currentMenu->menuAction());
    menuItems[menuCount]->title = label;
    menuItems[menuCount]->icon = iconFileName;

    record(menu, "Created menu named %s at index %u", fullname, menuCount);
    menuCount++;

    return XL::xl_true;
}


Tree * Widget::menuBar()
// ----------------------------------------------------------------------------
// Set currentMenuBar to the default menuBar.
// ----------------------------------------------------------------------------
{
    currentMenuBar = taoWindow()->menuBar();
    currentToolBar = NULL;
    currentMenu = NULL;
    return XL::xl_true;
}


Tree * Widget::toolBar(text name, text title, bool isFloatable, text location)
// ----------------------------------------------------------------------------
// Add the toolBar to the current widget
// ----------------------------------------------------------------------------
// The location is the prefered location for the toolbar.
// The supported values are [n|N]*, [e|E]*, [s|S]*, West or N, E, S, W, O
{
    // Do not update toolbars during transitions
    if (runningTransitionCode)
        return XL::xl_false;

    QString fullname = +name;
    Window *win = taoWindow();
    if (QToolBar *tmp = win->findChild<QToolBar*>(fullname))
    {
        if (menuCount < menuItems.size() &&
            menuItems[menuCount] != NULL &&
            menuItems[menuCount]->fullname == fullname)
        {
            // Set the currentMenu and update the label and icon.
            currentToolBar = tmp;
            menuCount++;
            currentMenuBar = NULL;
            currentMenu = NULL;
            return XL::xl_true;
        }

        // The name exist but it is not in the good order so remove it.
        delete tmp;
    }

    currentToolBar = win->addToolBar(+title);
    currentToolBar->setObjectName(fullname);
    currentToolBar->setFloatable(isFloatable);

    switch (location[0]) {
    case 'n':
    case 'N':
        win->addToolBarBreak(Qt::TopToolBarArea);
        win->addToolBar(Qt::TopToolBarArea, currentToolBar);
        break;
    case 'e':
    case 'E':
        win->addToolBarBreak(Qt::RightToolBarArea);
        win->addToolBar(Qt::RightToolBarArea, currentToolBar);
        break;
    case 's':
    case 'S':
        win->addToolBarBreak(Qt::BottomToolBarArea);
        win->addToolBar(Qt::BottomToolBarArea, currentToolBar);
        break;
    case 'w':
    case 'W':
    case 'o':
    case 'O':
        win->addToolBarBreak(Qt::LeftToolBarArea);
        win->addToolBar(Qt::LeftToolBarArea, currentToolBar);
        break;
    }

    if (QMenu* view = win->findChild<QMenu*>(TOOLBAR_MENU_NAME))
        view->addAction(currentToolBar->toggleViewAction());

    connect(currentToolBar, SIGNAL(actionTriggered(QAction*)),
            this, SLOT(userMenu(QAction*)));

    IFTRACE(menus)
    {
        std::cerr << "toolbar CREATION with name "
                  << fullname.toStdString()
                  << " at index " << menuCount << "\n";
        std::cerr.flush();
    }

    if (menuCount >= menuItems.size())
        menuItems.resize(menuCount+10);

    if (menuItems[menuCount])
        delete menuItems[menuCount];

    menuItems[menuCount] = new MenuInfo(fullname, currentToolBar);

    menuCount++;
    currentMenuBar = NULL;
    currentMenu = NULL;

    return XL::xl_true;
}


Tree * Widget::separator()
// ----------------------------------------------------------------------------
//   Add the separator to the current widget
// ----------------------------------------------------------------------------
{
    // Do not update toolbars during transitions
    if (runningTransitionCode)
        return XL::xl_false;

    QString fullname = QString("SEPARATOR_%1").arg(menuCount);

    if (QAction *tmp = taoWindow()->findChild<QAction*>(fullname))
    {
        if (menuCount < menuItems.size() &&
            menuItems[menuCount] != NULL &&
            menuItems[menuCount]->fullname == fullname)
        {
            menuCount++;
            return XL::xl_true;
        }

        delete tmp;
    }

    QWidget *par = NULL;
    if (currentMenu)
        par = currentMenu;
    else if (currentMenuBar)
        par = currentMenuBar;
    else if (currentToolBar)
        par = currentToolBar;

    QAction *act = new QAction(par);
    act->setSeparator(true);
    act->setObjectName(fullname);

    IFTRACE(menus)
    {
        std::cerr << "separator CREATION with name "
                  << fullname.toStdString()
                  << " at index " << menuCount << "\n";
        std::cerr.flush();
    }
    if (menuCount >= menuItems.size())
        menuItems.resize(menuCount+10);

    if (menuItems[menuCount])
    {
        if (par)
        {
            QAction *before = menuItems[menuCount]->action;
            par->insertAction(before, act);
        }
        delete menuItems[menuCount];
    }
    else
    {
        if (par)
            par->addAction(act);
    }
    menuItems[menuCount] = new MenuInfo(fullname, act);
    menuCount++;
    return XL::xl_true;
}



// ============================================================================
//
//    Tree selection management
//
// ============================================================================

Name *Widget::insert(Scope *scope, Tree *self, Tree *toInsert, text msg)
// ----------------------------------------------------------------------------
//    Insert at the end of page or program
// ----------------------------------------------------------------------------
{
    // Check if blocked because the source code window was edited
    if (!xlProgram)
    {
        QMessageBox::warning(this, tr("Insert"),
                             tr("Current document has no associated program. "
                                "Use \"Save as...\" to save it first."));
        return XL::xl_false;
    }

    if (!markChange(msg))
    {
        QMessageBox::warning(this, tr("Insert"),
                             tr("There are unsaved source code changes. "
                                "Use \"Save\" to save changes first."));
        return XL::xl_false;
    }

    if (isReadOnly())
    {
        QMessageBox::warning(this, tr("Insert"),
                             tr("Current document is read-only. Use "
                                "\"Save as...\" to make a modifiable copy."));
        return XL::xl_false;
    }

    // For 'insert { statement; }', we don't want the { } block
    if (Block *block = toInsert->AsBlock())
        toInsert = block->child;

    // Make sure the new objects appear selected next time they're drawn
    selectStatements(toInsert);

    // Start at the top of the program to find where we will insert
    Tree_p *top = &xlProgram->tree;
    Infix *parent  = NULL;

    // If we have a current page, insert only in that context
    if (Tree *page = pageTree)
    {
        // Restrict insertion to that page
        top = &pageTree;

        // The page instructions often runs a 'do' block
        if (Prefix *prefix = page->AsPrefix())
            if (Name *left = prefix->left->AsName())
                if (left->value == "do")
                    top = &prefix->right;

        // If the page code is a block, look inside
        if (Block *block = (*top)->AsBlock())
            top = &block->child;
    }

    // Descend on the right of the statements
    Tree *program = *top;
    if (!program)
    {
        *top = toInsert;
    }
    else if (top)
    {
        if (Infix *statements = program->AsInfix())
        {
            statements = statements->LastStatement();
            parent = statements;
            program = statements->right;
        }

        // Append at end of the statements
        Tree_p *what = parent ? &parent->right : top;
        *what = new Infix("\n", *what, toInsert);
    }
    else
    {
        xlProgram->tree = toInsert;
    }

    // Reload the program and mark the changes
    reloadProgram();

    return XL::xl_true;
}


Tree *Widget::copySelection()
// ----------------------------------------------------------------------------
//    Copy the selection from the tree
// ----------------------------------------------------------------------------
{
    if (!hasSelection() || !xlProgram || !xlProgram->tree)
        return NULL;

    CopySelection copy(this);

    return xlProgram->tree->Do(copy);
}


Name *Widget::deleteSelection(Scope *scope, Tree *self, text key)
// ----------------------------------------------------------------------------
//    Delete the selection (with text support)
// ----------------------------------------------------------------------------
{
    TaoSave saveCurrent(current, this);
    if (textSelection())
        return textEditKey(scope, self, key);
    deleteSelection();

    return XL::xl_true;
}


void Widget::deleteSelection()
// ----------------------------------------------------------------------------
//    Delete the selection (when selection is not text)
// ----------------------------------------------------------------------------
{
    // Check if the source was modified, if so, do not update the tree
    if (!xlProgram || !markChange("Deleted selection"))
        return;

    if (!hasSelection())
        return;

    Tree *what = xlProgram->tree;
    if (what)
    {
        DeleteSelectionAction del(this);
        what = what->Do(del);

        if (!what)
            xlProgram->tree = what;
        reloadProgram(what);
    }
    selection.clear();
    selectionTrees.clear();
}


Name *Widget::setAttribute(text name, Tree *attribute, text shape)
// ----------------------------------------------------------------------------
//    Insert the tree in all shapes in the selection
// ----------------------------------------------------------------------------
{
    // Check if the source code window was modified, if so do not change
    if (!markChange("Updated " + name + " attribute"))
        return XL::xl_false;

    // Attribute may be encapsulated in a block
    if (Block *block = attribute->AsBlock())
        attribute = block->child;

    if (TextSelect *sel = textSelection()) // Text selected
    {

        // Create tree with attribute and selected text.
        Prefix *p = new Prefix(new Name("text"),
                               new Text(+sel->cursor.
                                        document()->toPlainText(),
                                        "<<", ">>"));
        Infix *lf = new Infix("\n", attribute,
                              new Infix("\n", p, XL::xl_nil));

        // Current selected text must be erased because it will be re-inserted
        // with new formating
        sel->replacement = "";
        sel->replace = true;
        // New tree : format and text
        sel->replacement_tree = lf;
        // Draw...
        refresh();
        return XL::xl_true;
    }
    else if (xlProgram)
    {
        if (Tree *program = xlProgram->tree)
        {
            SetAttributeAction setAttrib(name, attribute, this, shape);
            program->Do(setAttrib);
            return XL::xl_true;
        }
    }
    return XL::xl_false;
}



// ============================================================================
//
//   Group management
//
// ============================================================================

Tree *Widget::group(Scope *scope, Tree *self, Tree *shapes)
// ----------------------------------------------------------------------------
//   Group objects together, make them selectable as a whole
// ----------------------------------------------------------------------------
{
    GroupLayout *group = new GroupLayout(this, self);
    group->id = selectionId();
    layout->Add(group);
    XL::Save<Layout *> saveLayout(layout, group);
    XL::Save<Tree_p>   saveShape (currentShape, self);
    if (selectNextTime.count(self))
    {
        selection[id]++;
        selectNextTime.erase(self);
    }

    Tree *result = saveAndEvaluate(scope, shapes);
    return result;
}


Tree *Widget::updateParentWithGroupInPlaceOfChild(Tree *parent,
                                                  Tree *child,
                                                  Tree *selected)
// ----------------------------------------------------------------------------
//   Replace 'child' with a group created from the selection
// ----------------------------------------------------------------------------
{
    Name *groupName = new Name("group");
    Tree *group = new Prefix(groupName,
                             new Block(selected, "I+", "I-"));

    Infix * inf = parent->AsInfix();
    if (inf)
    {
        if (inf->left == child)
            inf->left = group;
        else
            inf->right = group;

        return group;
    }

    Prefix * pref = parent->AsPrefix();
    if (pref)
    {
        if (pref->left == child)
            pref->left = group;
        else
            pref->right = group;

        return group;
    }

    Postfix * pos = parent->AsPostfix();
    if (pos)
    {
        if (pos->left == child)
            pos->left = group;
        else
            pos->right = group;

        return group;
    }

    Block * block = parent->AsBlock();
    if (block)
    {
        block->child = group;
        return group;
    }

    return NULL;

}


Name *Widget::groupSelection(Tree */*self*/)
// ----------------------------------------------------------------------------
//    Create the group from the selected objects
// ----------------------------------------------------------------------------
{
    // Check if there's no selection or if source window changed
    if (!xlProgram || !hasSelection() || !markChange("Selection grouped"))
        return XL::xl_false;

    Tree *selected = copySelection();
    // Find the first non-selected ancestor of the first element
    //      in the selection set.
    tree_set::iterator sel = selectionTrees.begin();
    Tree *child = *sel;
    Tree * parent = NULL;
    do {
        XL::FindParentAction getParent(child);
        parent = xlProgram->tree->Do(getParent);
    } while (parent && selectionTrees.count(parent) && (child = parent));

    // Check if we are not the only one
    if (!parent)

        return XL::xl_false;

    // Do the work
    Tree *theGroup = updateParentWithGroupInPlaceOfChild(parent, child, selected);
    if (!theGroup)
        return XL::xl_false;

    deleteSelection();
    selectStatements(theGroup);

    // Reload the program and mark the changes
    reloadProgram();

    return XL::xl_true;
}


bool Widget::updateParentWithChildrenInPlaceOfGroup(Tree *parent,
                                                    Prefix *group)
// ----------------------------------------------------------------------------
//    Helper function: Plug the group's child tree under the parent.
// ----------------------------------------------------------------------------
{
    Infix * inf = parent->AsInfix();
    Block * block = group->right->AsBlock();
    if (!block || !xlProgram)
        return false;

    // If the program is made only with this group
    if (group == xlProgram->tree)
    {
        xlProgram->tree = block->child;
        return true;
    }

    if (inf)
    {
        if (inf->left == group)
        {
            if (Infix * inf_child = block->child->AsInfix())
            {
                Tree *p_right = inf->right;
                Infix *last = inf_child->LastStatement();
                last->right = new Infix("\n",last->right, p_right);
                inf->left = inf_child->left;
                inf->right = inf_child->right;
            }
            else
            {
                inf->left = block->child;
            }
        }
        else
        {
            inf->right = block->child;
        }

        return true;
    }

    Prefix * pref = parent->AsPrefix();
    if (pref)
    {
        if (pref->left == group)
            pref->left = block->child;
        else
            pref->right = block->child;

        return true;
    }

    Postfix * pos = parent->AsPostfix();
    if (pos)
    {
        if (pos->left == group)
            pos->left = block->child;
        else
            pos->right = block->child;

        return true;
    }

    Block * blockPar = parent->AsBlock();
    if (blockPar)
    {
        blockPar->child = block->child;

        return true;
    }

    return false;

}

Name *Widget::ungroupSelection(Tree */*self*/)
// ----------------------------------------------------------------------------
//    Remove the group instruction from the source code
// ----------------------------------------------------------------------------
{
    // Check if there is no selection or if source window changed
    if (!xlProgram || !hasSelection() || !markChange("Selection ungrouped"))
        return XL::xl_false;

    tree_set::iterator sel = selectionTrees.begin();
    for( ;sel != selectionTrees.end(); sel++)
    {
        Prefix * groupTree = (*sel)->AsPrefix();
        if (!groupTree)
            continue;

        Name * name = groupTree->left->AsName();
        if (!name || name->value != "group")
            continue;

        XL::FindParentAction getParent(*sel);
        Tree * parent = xlProgram->tree->Do(getParent);
        // Check if we are not the only one
        if (!parent)
            continue;

        bool res = updateParentWithChildrenInPlaceOfGroup(parent, groupTree);
        if (!res)
            continue;
    }

    // Reload the program and mark the changes
    reloadProgram();

    return XL::xl_true;
}



// ============================================================================
//
//   Unit conversions
//
// ============================================================================

#define XL_RREAL(x)     return new Real(x, self->Position())

Real *Widget::fromCm(Tree *self, double cm)
// ----------------------------------------------------------------------------
//   Convert from cm to pixels
// ----------------------------------------------------------------------------
{
    XL_RREAL(cm * logicalDpiX() * (1.0 / 2.54));
}


Real *Widget::fromMm(Tree *self, double mm)
// ----------------------------------------------------------------------------
//   Convert from mm to pixels
// ----------------------------------------------------------------------------
{
    XL_RREAL(mm * logicalDpiX() * (0.1 / 2.54));
}


Real *Widget::fromIn(Tree *self, double in)
// ----------------------------------------------------------------------------
//   Convert from inch to pixels
// ----------------------------------------------------------------------------
{
    XL_RREAL(in * logicalDpiX());
}


Real *Widget::fromPt(Tree *self, double pt)
// ----------------------------------------------------------------------------
//   Convert from pt to pixels
// ----------------------------------------------------------------------------
{
    XL_RREAL(pt * logicalDpiX() * (1.0 / 72.0));
}


Real *Widget::fromPx(Tree *self, double px)
// ----------------------------------------------------------------------------
//   Convert from pixel (currently 1-1 mapping, could be based on scaling?)
// ----------------------------------------------------------------------------
{
    XL_RREAL(px);
}



// ============================================================================
//
//    Misc...
//
// ============================================================================

Tree *Widget::constant(Tree *self, Tree *tree)
// ----------------------------------------------------------------------------
//   Return a clone of the tree to make sure it is not modified
// ----------------------------------------------------------------------------
{
    MarkAsConstant(tree);
    return tree;
}


Name *Widget::taoFeatureAvailable(Tree *self, Name *name)
// ----------------------------------------------------------------------------
//   Check if a compile-time option is enabled
// ----------------------------------------------------------------------------
{
#ifndef CFG_NOGIT
    if (name->value == "git")
        return XL::xl_true;
#endif

    if (name->value == "mac")
#ifdef CONFIG_MACOSX
        return XL::xl_true;
#else
    return XL::xl_false;
#endif

    if (name->value == "linux")
#ifdef CONFIG_LINUX
        return XL::xl_true;
#else
    return XL::xl_false;
#endif

    if (name->value == "windows")
#ifdef CONFIG_MINGW
        return XL::xl_true;
#else
    return XL::xl_false;
#endif

    if (name->value == "vsync_ctl")
        return VSyncSupported() ? XL::xl_true : XL::xl_false;

    return XL::xl_false;
}


Text *Widget::GLVersion(Tree *self)
// ----------------------------------------------------------------------------
//   Return OpenGL supported version
// ----------------------------------------------------------------------------
{
    return new Text(GL.Version());
}


Name *Widget::isGLExtensionAvailable(Tree *self, text name)
// ----------------------------------------------------------------------------
//   Check is an OpenGL extensions is supported
// ----------------------------------------------------------------------------
{
    return isGLExtensionAvailable(name) ? XL::xl_true : XL::xl_false;
}


bool Widget::isGLExtensionAvailable(text name)
// ----------------------------------------------------------------------------
//   Module interface to isGLExtensionAvailable
// ----------------------------------------------------------------------------
{
    if (OpenGLState *opengl = dynamic_cast<OpenGLState *> (&GL))
    {
        kstring avail = opengl->extensionsAvailable.c_str();
        kstring req = name.c_str();
        bool isAvailable = (strstr(avail, req) != NULL);
        return isAvailable;
    }
    return false;
}


Name *Widget::hasDisplayModeText(Tree *self, text name)
// ----------------------------------------------------------------------------
//   Check if a display mode is available
// ----------------------------------------------------------------------------
{
    QStringList all = DisplayDriver::allDisplayFunctions();
    if (all.contains(+name))
        return XL::xl_true;
    return XL::xl_false;
}


Real *Widget::getWorldZ(Tree *self, Real *x, Real *y)
// ----------------------------------------------------------------------------
//   Get the depth buffer value in world coordinate for X and Y
// ----------------------------------------------------------------------------
{
    Point3 pos;
    double value = 0.0;
    layout->Add(new ConvertScreenCoordinates(self, *x, *y));
    if (CoordinatesInfo *info = self->GetInfo<CoordinatesInfo>())
        value = info->coordinates.z;
    return new Real(value, self->Position());
}


Real *Widget::getWorldCoordinates(Tree *self,
                                  Real *x, Real *y,
                                  Real *wx, Real *wy, Real *wz)
// ----------------------------------------------------------------------------
//   Get the depth buffer value in world coordinate for X and Y
// ----------------------------------------------------------------------------
{
    Point3 pos;
    layout->Add(new ConvertScreenCoordinates(self, *x, *y));
    if (CoordinatesInfo *info = self->GetInfo<CoordinatesInfo>())
    {
        wx->value = info->coordinates.x;
        wy->value = info->coordinates.y;
        wz->value = info->coordinates.z;
    }
    return wz;
}


Name *Widget::displaySet(Scope *scope, Tree *self, Tree *code)
// ----------------------------------------------------------------------------
//   Set a display option
// ----------------------------------------------------------------------------
{
    if (Infix *infix = code->AsInfix())
    {
        if (infix->name == ":=")
        {
            Name *name = infix->left->AsName();
            TreeList args;
            Tree *arg = infix->right;
            if (Block *block = arg->AsBlock())
                arg = block->child;
            arg = xl_evaluate(scope, arg);
            if (Integer *it = arg->AsInteger())
                arg = new Real(it->value);
            std::string strval;
            if (Real *rt = arg->AsReal())
            {
                std::ostringstream oss;
                oss << rt->value;
                strval = oss.str();
            }
            else if (Text *tt = arg->AsText())
            {
                strval = tt->value;
            }
            else if (Name *nt = arg->AsName())
            {
                strval = nt->value;
            }
            else
            {
                Ooops("Display driver argument $2 be text or number in $1",
                      self, arg);
                return XL::xl_false;
            }
            displayDriver->setOption(name->value, strval);
            return XL::xl_true;
        }
    }
    Ooops("Malformed display_set statement $1", self);
    return XL::xl_false;
}


Text *Widget::displayMode()
// ----------------------------------------------------------------------------
//   Return the name of the current display mode
// ----------------------------------------------------------------------------
{
    return new Text(+displayDriver->getDisplayFunction());
}


Name *Widget::readOnly()
// ----------------------------------------------------------------------------
//   Check if document is read only
// ----------------------------------------------------------------------------
{
    return isReadOnly() ? XL::xl_true : XL::xl_false;
}


Text *Widget::baseName(Tree *self, text filename)
// ----------------------------------------------------------------------------
// Returns the base name of a file without the path
// ----------------------------------------------------------------------------
{
    QFileInfo info(+filename);
    return new Text(+info.baseName(), self->Position());
}


Text *Widget::dirName(Tree *self, text filename)
// ----------------------------------------------------------------------------
// Returns the path of the specified filename
// ----------------------------------------------------------------------------
{
    QFileInfo info(+filename);
    return new Text(+info.path(), self->Position());
}


Name *Widget::openUrl(text url)
// ----------------------------------------------------------------------------
// Open url using an external application
// ----------------------------------------------------------------------------
{
    return QDesktopServices::openUrl(+url) ? XL::xl_true : XL::xl_false;
}


Name *Widget::screenShot(text filename, scale sz, bool withAlpha)
// ----------------------------------------------------------------------------
// Save current state of current drawing buffer into a file
// ----------------------------------------------------------------------------
{
    QString path(+filename);
    if (!QDir::isAbsolutePath(path))
    {
        QString dir(+currentDocumentFolder());
        path = QFileInfo(dir, path).absoluteFilePath();
    }
    screenShotPath = path;
    screenShotScale = sz;
    screenShotWithAlpha = withAlpha;
    return XL::xl_true;
}



// ============================================================================
//
//   Documentation generation
//
// ============================================================================

Text *Widget::generateDoc(Tree *self, Tree *tree, text defGrp)
// ----------------------------------------------------------------------------
//   Generate documentation for a given tree
// ----------------------------------------------------------------------------
{
    text result;
    if (defGrp.empty())
    {
        ExtractComment com;
        result = tree->Do(com);
    }
    else
    {
        ExtractDoc doc(defGrp);
        result = tree->Do(doc);
    }
    return new Text(result, self->Position());
}


#if 0
static void generateRewriteDoc(Widget *widget, XL::Rewrite *rewrite, text &com)
// ----------------------------------------------------------------------------
//   Generate documentation for a given rewrite
// ----------------------------------------------------------------------------
{
    if (rewrite->left)
    {
        Text *t1 = widget->generateDoc(rewrite->left, rewrite->left);
        com += t1->value;
    }

    if (rewrite->right)
    {
        Text *t2 = widget->generateDoc(rewrite->left, rewrite->right);
        com += t2->value;
    }

#if 0
    // REVISIT: Find the correct way to do that
    for (uint i = 0; i < REWRITE_HASH_SIZE; i++)
        if (XL::Rewrite *rw = rewrite->hash[i])
            generateRewriteDoc(widget, rw, com);
#endif
}
#endif


Text *Widget::generateAllDoc(Tree *self, text filename)
// ----------------------------------------------------------------------------
//   Generate documentation for all trees in the given file
// ----------------------------------------------------------------------------
{
    text      com = "";

    // Documentation from the context files (*.xl)
    for (auto &entry : XL::MAIN->files)
    {
        XL::SourceFile src = entry.second;
        if (!src.tree)
            continue;
        QFileInfo fi(+src.name);
        Text *t = generateDoc(self, src.tree, +fi.baseName());
        com += t->value;
    }

#if 0
    // REVISIT how to do that right now
    // Documentation from the primitives files (*.tbl)
    for (XL::Context *globals = xlr->context; globals; globals = globals->scope)
    {
        for (uint i = 0; i < REWRITE_HASH_SIZE; i++)
            if (XL::Rewrite *rw = globals->rewrites[i])
                generateRewriteDoc(this, rw, com);
    }
#endif

    // Write the result
    if (!filename.empty())
    {
        QFile file(+filename);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
            file.write(com.c_str());
        file.close();
    }

    return new Text(com, self->Position());
}



// ============================================================================
//
//   Running external processes
//
// ============================================================================

Name *Widget::runProcess(Tree *self, text name, text args)
// ----------------------------------------------------------------------------
//   Launch the given process
// ----------------------------------------------------------------------------
{
    QString qName = +name;
    if (!processMap.contains(qName))
    {
        QString qArgs = +args;
        QStringList qArgList = args == "" ? QStringList() : qArgs.split(' ');
        return runProcess(self, name, qArgList);
    }

    Process *process = processMap[qName];
    if (process->state() == QProcess::Running)
        return XL::xl_true;
    return XL::xl_false;
}


Name *Widget::runProcess(Tree *self, text name, QStringList &args)
// ----------------------------------------------------------------------------
//   Launch the given process
// ----------------------------------------------------------------------------
{
    // Extract the base name, e.g. turn ls#3 into ls
    text base = name;
    size_t hashIndex = base.find('#');
    if (hashIndex != std::string::npos)
        base = base.substr(0, hashIndex);

#ifndef CFG_NO_LICENSE
    // Check if running this process is allowed by the license file
    text feature = "RunProcess:" + base;
    if (!Licenses::Has(feature))
        return XL::xl_false;
#endif

    // Check if we already have a process by that name
    QString qName = +name;
    if (!processMap.contains(qName))
    {
        QString docPath = taoWindow()->currentProjectFolderPath();
        processMap[qName] = new Process(+base, args, docPath, true, 1024, true);
        return XL::xl_true;     // Return true at least once
    }

    Process *process = processMap[qName];
    if (process->state() == QProcess::Running)
        return XL::xl_true;
    return XL::xl_false;
}


Name *Widget::writeToProcess(Tree *self, text name, text args)
// ----------------------------------------------------------------------------
//   Write to the given process
// ----------------------------------------------------------------------------
{
    QString qName = +name;
    if (!processMap.contains(qName))
        return XL::xl_false;
    Process *process = processMap[qName];
    if (process->state() == QProcess::Running)
    {
        process->write(args.data(), args.length());
        return XL::xl_true;
    }
    return XL::xl_false;
}


Text *Widget::readFromProcess(Tree *self, text name, uint lines)
// ----------------------------------------------------------------------------
//   Read from the given process
// ----------------------------------------------------------------------------
{
    QString qName = +name;
    if (processMap.contains(qName))
    {
        Process *process = processMap[qName];
        QString result = process->getTail(lines);
        return new Text(+result, "\"", "\"", self->Position());
    }

    // Extract the base name, e.g. turn ls#3 into ls
    text base = name;
    size_t hashIndex = base.find('#');
    if (hashIndex != std::string::npos)
        base = base.substr(0, hashIndex);

#ifndef CFG_NO_LICENSE
    text feature = "RunProcess:" + base;
    if (!Licenses::Has(feature))
        return new Text("<no license for process " + name + ">",
                        "\"", "\"", self->Position());
#endif

    // Other case: report that the process does not exist
    return new Text("<process " + name + " does not exist>",
                    "\"", "\"", self->Position());
}


Name *Widget::dropProcess(Tree *self, text name)
// ----------------------------------------------------------------------------
//   Read from the given process
// ----------------------------------------------------------------------------
{
    QString qName = +name;
    if (processMap.contains(qName))
    {
        Process *process = processMap[qName];
        processMap.remove(qName);
        process->kill();
        delete process;
        return XL::xl_true;
    }
    return XL::xl_false;
}


Text *Widget::runRsync(Tree *self, text opt, text src, text dst)
// ----------------------------------------------------------------------------
//   Read from the given process
// ----------------------------------------------------------------------------
{
    static text path;
    static text ssh;

    if (path == "")
    {
        QStringList candidates;
        candidates << qApp->applicationDirPath() + "/rsync/rsync.exe"
                   << qApp->applicationDirPath() + "/rsync/rsync"
                   << "/usr/bin/rsync"
                   << "/bin/rsync"
                   << "/usr/local/bin/rsync"
                   << "/opt/local/bin/rsync";

        path = "<rsync not found>";
        foreach (QString p, candidates)
        {
            if (QFileInfo(p).exists())
            {
                path = +p;
                break;
            }
        }
    }

    if (ssh == "")
    {
        QStringList candidates;
        candidates << qApp->applicationDirPath() + "/rsync/ssh.exe"
                   << qApp->applicationDirPath() + "/rsync/ssh"
                   << "/usr/bin/ssh"
                   << "/bin/ssh"
                   << "/usr/local/bin/ssh"
                   << "/opt/local/bin/ssh";

        ssh = "<ssh not found>";
        foreach (QString p, candidates)
        {
            if (QFileInfo(p).exists())
            {
                ssh = +("\"" + p + "\"");
                break;
            }
        }
    }

    if (path == "<rsync not found>")
        return new Text(path, "\"", "\"", self->Position());

    QStringList args;
    if (opt != "")
        args << +opt;
    else
        args << "-aP";
    if (ssh != "<ssh not found>")
        args << "-e" << +ssh;
    args << +src << +dst;
    runProcess(self, path, args);
    return readFromProcess(self, path, 5);
}



// ============================================================================
//
//   Tree substitution / replacement helpers
//
// ============================================================================

Tree *NameToNameReplacement::DoName(Name *what)
// ----------------------------------------------------------------------------
//   Replace a name with another name
// ----------------------------------------------------------------------------
{
    std::map<text, text>::iterator found = map.find(what->value);
    if (found != map.end())
    {
        replaced = true;
        return new Name((*found).second, what->Position());
    }
    return new Name(what->value, what->Position());
}


Tree *  NameToNameReplacement::Replace(Tree *original)
// ----------------------------------------------------------------------------
//   Perform name replacement and give the result its own symbol table
// ----------------------------------------------------------------------------
{
    Tree *copy = original;
    copy = original->Do(*this);
    return copy;
}


Tree *NameToTextReplacement::DoName(Name *what)
// ----------------------------------------------------------------------------
//   Replace a name with a text
// ----------------------------------------------------------------------------
{
    std::map<text, text>::iterator found = map.find(what->value);
    if (found != map.end())
    {
        replaced = true;
        return new Text((*found).second, "\"", "\"", what->Position());
    }
    return new Name(what->value, what->Position());
}



// ============================================================================
//
//   Helper functions
//
// ============================================================================

void tao_widget_refresh(double delay)
// ----------------------------------------------------------------------------
//    Refresh the current widget
// ----------------------------------------------------------------------------
{
    TAO(refresh(delay));
}

}
