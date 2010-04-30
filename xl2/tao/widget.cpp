// ****************************************************************************
//  widget.cpp							   Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "widget.h"
#include "tao.h"
#include "main.h"
#include "runtime.h"
#include "opcodes.h"
#include "gl_keepers.h"
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
#include "repository.h"
#include "application.h"
#include "tao_utf8.h"
#include "layout.h"
#include "page_layout.h"
#include "space_layout.h"
#include "shapes.h"
#include "text_drawing.h"
#include "shapes3d.h"
#include "path3d.h"
#include "attributes.h"
#include "transforms.h"
#include "undo.h"
#include "serializer.h"
#include "binpack.h"

#include <QToolButton>
#include <QtGui/QImage>
#include <cmath>
#include <QFont>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <QVariant>
#include <QtWebKit>
#include <sys/time.h>
#include <sys/stat.h>

#define TAO_CLIPBOARD_MIME_TYPE "application/tao-clipboard"

TAO_BEGIN

// ============================================================================
//
//   Widget life management
//
// ============================================================================

double Widget::zNear = 2000.0;
double Widget::zFar = 40000.0;


Widget::Widget(Window *parent, XL::SourceFile *sf)
// ----------------------------------------------------------------------------
//    Create the GL widget
// ----------------------------------------------------------------------------
    : QGLWidget(QGLFormat(QGL::SampleBuffers|QGL::AlphaChannel), parent),
      xlProgram(sf), inError(false), mustUpdateDialogs(false),
      space(NULL), layout(NULL), path(NULL),
      pageName(""), pageId(0), pageTotal(0), pageTree(NULL),
      currentShape(NULL),
      currentGridLayout(NULL),
      currentGroup(NULL), activities(NULL),
      id(0), charId(0), capacity(1), manipulator(0),
      wasSelected(false),
      event(NULL), focusWidget(NULL), keyboardModifiers(0),
      currentMenu(NULL), currentMenuBar(NULL),currentToolBar(NULL),
      orderedMenuElements(QVector<MenuInfo*>(10, NULL)), order(0),
      colorAction(NULL), fontAction(NULL),
      timer(this), idleTimer(this),
      pageStartTime(CurrentTime()), pageRefresh(86400),
      tmin(~0ULL), tmax(0), tsum(0), tcount(0),
      nextSave(now()), nextCommit(nextSave), nextSync(nextSave),
      nextPull(nextSave)
{
    // Make sure we don't fill background with crap
    setAutoFillBackground(false);

    // Make this the current context for OpenGL
    makeCurrent();

    // Create the main page we draw on
    space = new SpaceLayout(this);
    layout = space;

    // Prepare the timers
    connect(&timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    connect(&idleTimer, SIGNAL(timeout()), this, SLOT(dawdle()));
    idleTimer.start(100);

    // Receive notifications for focus
    connect(qApp, SIGNAL(focusChanged (QWidget *, QWidget *)),
            this,  SLOT(appFocusChanged(QWidget *, QWidget *)));
    setFocusPolicy(Qt::StrongFocus);

    // Prepare the menubar
    currentMenuBar = parent->menuBar();
    connect(parent->menuBar(),  SIGNAL(triggered(QAction*)),
            this,               SLOT(userMenu(QAction*)));
}


Widget::~Widget()
// ----------------------------------------------------------------------------
//   Destroy the widget
// ----------------------------------------------------------------------------
{
    delete space;
    delete path;
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
    // Run all activities, which will get them a chance to update refresh
    for (Activity *a = activities; a; a = a->Idle()) ;

    // We will only auto-save and commit if we have a valid repository
    Repository *repo           = repository();
    XL::Main   *xlr            = XL::MAIN;

    // Check if we need to refresh something
    double idleInterval = 0.001 * idleTimer.interval();
    double remaining = pageRefresh - idleInterval;
    if (remaining <= idleInterval &&
        (!timer.isActive() || remaining <= timer.interval() * 0.001))
    {
        if (remaining <= 0)
            remaining = 0.001;
        timer.stop();
        timer.setSingleShot(true);
        timer.start(1000 * remaining);
        remaining = 86400;
    }
    pageRefresh = remaining;

    if (xlProgram && xlProgram->changed)
    {
        text txt = *xlProgram->tree.tree;
        Window *window = (Window *) parentWidget();
        window->setText(+txt);
        if (!repo)
            xlProgram->changed = false;
    }

    // Check if there's something to save
    ulonglong tick = now();
    longlong saveDelay = longlong(nextSave - tick);
    if (repo && saveDelay < 0 && repo->idle())
    {
        XL::source_files::iterator it;
        for (it = xlr->files.begin(); it != xlr->files.end(); it++)
        {
            XL::SourceFile &sf = (*it).second;
            writeIfChanged(sf);
        }

        // Record when we will save file again
        nextSave = tick + xlr->options.save_interval * 1000;
    }

    // Check if there's something to commit
    longlong commitDelay = longlong (nextCommit - tick);
    if (repo && commitDelay < 0 && repo->state == Repository::RS_NotClean)
    {
        doCommit();
    }

    // Check if there's something to merge from the remote repository
    // REVISIT: sync: what if several widgets share the same repository?
    longlong pullDelay = longlong (nextPull - tick);
    if (repo && pullDelay < 0 && repo->state == Repository::RS_Clean)
    {
        repo->pull();
        nextPull = now() + repo->pullInterval * 1000;
    }

    // Check if there's something to reload
    longlong syncDelay = longlong(nextSync - tick);
    if (syncDelay < 0)
    {
        refreshProgram();
        syncDelay = tick + xlr->options.sync_interval * 1000;
    }

    // Once we are done, do a garbage collection
    XL::Context::context->CollectGarbage();
}


void Widget::draw()
// ----------------------------------------------------------------------------
//    Redraw the widget
// ----------------------------------------------------------------------------
{
    // Timing
    ulonglong before = now();
    event = NULL;

    // Setup the initial drawing environment
    double w = width(), h = height();
    setup(w, h);
    pageW = (21.0 / 2.54) * logicalDpiX(); // REVISIT
    pageH = (29.7 / 2.54) * logicalDpiY();
    flowName = "";
    flows.clear();
    pageId = 0;
    pageTree = NULL;
    lastPageName = "";

    // Clear the background
    glClearColor (1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Layout::polygonOffset = 0;

    // Make sure we compile the selection the first time
    static bool first = true;
    if (xlProgram && first)
    {
        XL::Symbols *s = xlProgram->symbols;
        double x = 0;
        (XL::XLCall("draw_selection"), x,x,x,x).build(s);
        (XL::XLCall("draw_selection"), x,x,x,x,x,x).build(s);
        (XL::XLCall("draw_widget_selection"), x,x,x,x).build(s);
        (XL::XLCall("draw_widget_selection"), x,x,x,x,x,x).build(s);
        (XL::XLCall("draw_3D_selection"), x,x,x,x,x,x).build(s);
        (XL::XLCall("draw_handle"), x, x, x).build(s);
        first = false;
    }

    // If there is a program, we need to run it
    pageRefresh = 86400;        // 24 hours
    runProgram();

    // Check if we want to refresh something
    ulonglong after = now();
    double remaining = pageRefresh - 1e-6 * (after - before) - 0.001;
    if (remaining <= 0)
        remaining = 0.001;
    timer.setSingleShot(true);
    timer.start(1000 * remaining);

    // Timing
    elapsed(before, after);

    // Render all activities, e.g. the selection rectangle
    SpaceLayout selectionSpace(this);
    XL::LocalSave<Layout *> saveLayout(layout, &selectionSpace);
    glDisable(GL_DEPTH_TEST);
    for (Activity *a = activities; a; a = a->Display()) ;
    selectionSpace.Draw(NULL);
    glEnable(GL_DEPTH_TEST);

    // Update page count for next run
    pageTotal = pageId;

    // If we must update dialogs, do it now
    if (mustUpdateDialogs)
    {
        mustUpdateDialogs = false;
        if (colorDialog)
            updateColorDialog();
        if (fontDialog)
            updateFontDialog();
    }
}


void Widget::runProgram()
// ----------------------------------------------------------------------------
//   Run the current XL program
// ----------------------------------------------------------------------------
{
    // Don't run anything if we detected errors running previously
    if (inError)
        return;

    // Reset the selection id for the various elements being drawn
    focusWidget = NULL;

    // Run the XL program associated with this widget
    current = this;
    QTextOption alignCenter(Qt::AlignCenter);
    IFTRACE(memory)
        std::cerr << "Run, Drawing::count = " << space->count << ", ";
    space->Clear();
    IFTRACE(memory)
        std::cerr << "cleared, count = " << space->count << ", ";
    XL::LocalSave<Layout *> saveLayout(layout, space);
    selectionTrees.clear();
    id = charId = 0;

    if (xlProgram)
    {
        if (Tree *prog = xlProgram->tree.tree)
        {
            xl_evaluate(prog);

            // Clean the end of the old menu list.
            for  ( ; order < orderedMenuElements.count(); order++)
            {
                delete orderedMenuElements[order];
                orderedMenuElements[order] = NULL;
            }

            // Reset the order value.
            order          = 0;
            currentMenu    = NULL;
            currentToolBar = NULL;
            currentMenuBar = ((Window*)parent())->menuBar();
        }
    }

    // Remember how many elements are drawn on the page, plus arbitrary buffer
    if (id + charId + 100 > capacity)
        capacity = id + charId + 100;
    else if (id + charId + 50 < capacity / 2)
        capacity = capacity / 2;

    // After we are done, draw the space with all the drawings in it
    id = charId = 0;
    space->Draw(NULL);
    IFTRACE(memory)
        std::cerr << "Draw, count = " << space->count << "\n";
    id = charId = 0;
    space->DrawSelection(NULL);

    // Clipboard management
    checkCopyAvailable();
}


void Widget::identifySelection()
// ----------------------------------------------------------------------------
//   Draw the elements in global space for selection purpose
// ----------------------------------------------------------------------------
{
    id = charId = 0;
    space->Identify(NULL);
}


void Widget::updateSelection()
// ----------------------------------------------------------------------------
//   Redraw selection in order to perform text editing operations
// ----------------------------------------------------------------------------
{
    id = charId = 0;
    space->DrawSelection(NULL);
}


static void printWidget(QWidget *w)
// ----------------------------------------------------------------------------
//   Print a widget for debugging purpose
// ----------------------------------------------------------------------------
{
    printf("%p", w);
    if (w)
        printf(" (%s)", w->metaObject()->className());
}



void Widget::appFocusChanged(QWidget *prev, QWidget *next)
// ----------------------------------------------------------------------------
//   Notifications when focus changes
// ----------------------------------------------------------------------------
{
    IFTRACE(focus)
    {
        printf("Focus "); printWidget(prev); printf ("->"); printWidget(next);
        const QObjectList &children = this->children();
        QObjectList::const_iterator it;
        printf("\nChildren:");
        for (it = children.begin(); it != children.end(); it++)
        {
            printf(" ");
            printWidget((QWidget *) *it);
        }
        printf("\n");
    }
}



void Widget::checkCopyAvailable()
// ----------------------------------------------------------------------------
//   Emit a signal when clipboard can copy or cut something (or cannot anymore)
// ----------------------------------------------------------------------------
{
    bool isSelected = selected();
    if (wasSelected != isSelected)
    {
        emit copyAvailable(isSelected);
        wasSelected = isSelected;
    }
}


bool Widget::canPaste()
// ----------------------------------------------------------------------------
//   Is current clibpoard data in a suitable format to be pasted?
// ----------------------------------------------------------------------------
{
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    return (mimeData->hasFormat(TAO_CLIPBOARD_MIME_TYPE));
}


void Widget::cut()
// ----------------------------------------------------------------------------
//   Cut current selection into clipboard
// ----------------------------------------------------------------------------
{
    copy();
    IFTRACE(clipboard)
        std::cerr << "Clipboard: deleting selection\n";
    deleteSelection();
}


void Widget::copy()
// ----------------------------------------------------------------------------
//   Copy current selection into clipboard
// ----------------------------------------------------------------------------
{
    if (!hasSelection())
        return;

    // Build a single tree from all the selected sub-trees
    std::set<Tree *>::reverse_iterator i = selectionTrees.rbegin();
    XL::Tree *tree = (*i++);
    for ( ; i != selectionTrees.rend(); i++)
        tree = new XL::Infix("\n", (*i), tree);

    IFTRACE(clipboard)
    {
        std::cerr << "Clipboard: copying:\n";
        XL::Renderer render(std::cerr);
        render.SelectStyleSheet("debug.stylesheet");
        render.Render(tree);
    }

    // Serialize the tree
    std::string ser;
    std::ostringstream ostr;
    XL::Serializer serializer(ostr);
    tree->Do(serializer);
    ser += ostr.str();

    // Encapsulate serialized tree as MIME data
    QByteArray binData;
    binData.append(ser.data(), ser.length());
    QMimeData *mimeData = new QMimeData;
    mimeData->setData(TAO_CLIPBOARD_MIME_TYPE, binData);

    // Transfer into clipboard
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setMimeData(mimeData);
}


void Widget::paste()
// ----------------------------------------------------------------------------
//   Paste the clipboard content at the current selection
// ----------------------------------------------------------------------------
{
    // Does clipboard contain Tao stuff?)
    if (!canPaste())
        return;

    // Read clipboard content
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();

    // Extract serialized tree
    QByteArray binData = mimeData->data(TAO_CLIPBOARD_MIME_TYPE);
    std::string ser(binData.data(), binData.length());

    // De-serialize
    std::istringstream istr(ser);
    XL::Deserializer deserializer(istr);
    XL::Tree *tree = deserializer.ReadTree();
    if (!deserializer.IsValid())
        return;

    IFTRACE(clipboard)
    {
        std::cerr << "Clipboard: pasting:\n";
        XL::Renderer render(std::cerr);
        render.SelectStyleSheet("debug.stylesheet");
        render.Render(tree);
    }

    // Insert tree at current selection, or at end of current page
    // TODO: paste with an offset to avoid exactly overlapping objects
    insert(NULL, tree);

    // Deselect previous selection
    selection.clear();
    selectionTrees.clear();

    // Make sure the new objects appear selected next time they're drawn
    XL::Infix *i;
    XL::Tree  *t = tree;
    selectNextTime.clear();
    for (i = tree->AsInfix(); i ; t = i->right, i = i->right->AsInfix())
        selectNextTime.insert(i->left);
    selectNextTime.insert(t);
}


void Widget::userMenu(QAction *p_action)
// ----------------------------------------------------------------------------
//   User menu slot activation
// ----------------------------------------------------------------------------
{
    if (!p_action)
        return;

    IFTRACE(menus)
        std::cout << "Action " << p_action->objectName().toStdString()
                  << " (" << p_action->text().toStdString() << ") activated\n";

    QVariant var =  p_action->data();
    if (!var.isValid())
        return;

    markChanged(+("Menu '" + p_action->text() + "' selected"));

    current = this;
    XL::TreeRoot t = var.value<XL::TreeRoot >();
    xl_evaluate(t.tree);        // Typically will insert something...
}


bool Widget::refresh(double delay)
// ----------------------------------------------------------------------------
//   Refresh the screen after the given time interval
// ----------------------------------------------------------------------------
{
    if (pageRefresh > delay)
    {
        pageRefresh = delay;
        return true;
    }
    return false;
}


void Widget::commitSuccess(QString id, QString msg)
// ----------------------------------------------------------------------------
//   Document was succesfully committed to repository (see doCommit())
// ----------------------------------------------------------------------------
{
    Window *window = (Window *) parentWidget();
    window->undoStack->push(new UndoCommand(repository(), id, msg));
}



// ============================================================================
//
//   OpenGL setup
//
// ============================================================================

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
    space->space = Box3(-width/2, -height/2, 0, width, height, 0);
    tmax = tsum = tcount = 0;
    tmin = ~tmax;
}


void Widget::paintGL()
// ----------------------------------------------------------------------------
//    Repaint the contents of the window
// ----------------------------------------------------------------------------
{
    draw();
    glShowErrors();
}


void Widget::setup(double w, double h, Box *picking)
// ----------------------------------------------------------------------------
//   Setup an initial environment for drawing
// ----------------------------------------------------------------------------
{
    // Setup viewport
    glViewport(0, 0, w, h);

    // Setup the projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Restrict the picking area if any is given as input
    if (picking)
    {
        GLint viewport[4] = { 0, 0, w, h };
        Box b = *picking;
        b.Normalize();
        Vector size = b.upper - b.lower;
        Point center = b.lower + size / 2;
        gluPickMatrix(center.x, center.y, size.x+1, size.y+1, viewport);
    }

    // Setup the frustrum for the projection
    double zNear = Widget::zNear, zFar = Widget::zFar;
    double eyeX = 0.0, eyeY = 0.0, eyeZ = zNear;
    double centerX = 0.0, centerY = 0.0, centerZ = 0.0;
    double upX = 0.0, upY = 1.0, upZ = 0.0;
    glFrustum (-w/2, w/2, -h/2, h/2, zNear, zFar);
    gluLookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ);
    glTranslatef(0.0, 0.0, -zNear);
    glScalef(2.0, 2.0, 2.0);

    // Setup the model view matrix so that 1.0 unit = 1px
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Reset default GL parameters
    setupGL();
}


void Widget::setupGL()
// ----------------------------------------------------------------------------
//   Setup default GL parameters
// ----------------------------------------------------------------------------
{
    // Setup other
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glEnable(GL_POLYGON_OFFSET_LINE);
    glEnable(GL_POLYGON_OFFSET_POINT);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glLineWidth(1);
    glLineStipple(1, -1);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_RECTANGLE_ARB);
    glDisable(GL_CULL_FACE);
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
    if (QObject *focus = focusWidget)
        return focus->event(event);
    return false;
}


bool Widget::forwardEvent(QMouseEvent *event)
// ----------------------------------------------------------------------------
//   Forward event to the focus proxy if there is any, adjusting coordinates
// ----------------------------------------------------------------------------
{
    if (QObject *focus = focusWidget)
    {
        int x = event->x();
        int y = event->y();
        int w = focusWidget->width();
        int h = focusWidget->height();

        Point3 u = unproject(x, y, 0);
        QMouseEvent local(event->type(), QPoint(u.x + w/2, h/2 - u.y),
                          event->button(), event->buttons(),
                          event->modifiers());
        return focus->event(&local);
    }
    return false;
}


static text keyName(QKeyEvent *event)
// ----------------------------------------------------------------------------
//   Return the properly formatted key name for a key event
// ----------------------------------------------------------------------------
{
    // Try to find if there is a callback in the code for this key
    text name = +event->text();
    text ctrl = "";             // Name for Control, Meta and Alt

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
    static Qt::KeyboardModifiers modifiers = 0;
    if (event->type() == QEvent::KeyPress)
        modifiers = event->modifiers();
    if (modifiers)
    {
        if (ctrl == "")
        {
            if (name.length() != 1 && (modifiers & Qt::ShiftModifier))
                name = "Shift-" + name;
            ctrl = name;
        }
        else
        {
            int shift = modifiers & Qt::ShiftModifier;
            if (shift && shift != modifiers)
                name = ctrl = "Shift-" + ctrl;
        }
        if (modifiers & Qt::ControlModifier)
            name = ctrl = "Control-" + ctrl;
        if (modifiers & Qt::AltModifier)
            name = ctrl = "Alt-" + ctrl;
        if (modifiers & Qt::MetaModifier)
            name = ctrl = "Meta-" + ctrl;
    }

    return name;
}


void Widget::keyPressEvent(QKeyEvent *event)
// ----------------------------------------------------------------------------
//   A key is pressed
// ----------------------------------------------------------------------------
{
    EventSave save(this->event, event);
    keyboardModifiers = event->modifiers();

    // Forward it down the regular event chain
    if (forwardEvent(event))
        return;

    // Now call "key" in the current context
    text name = keyName(event);
    XL::Symbols *syms = xlProgram ? xlProgram->symbols : XL::Symbols::symbols;
    (XL::XLCall ("key"), name) (syms);
}


void Widget::keyReleaseEvent(QKeyEvent *event)
// ----------------------------------------------------------------------------
//   A key is released
// ----------------------------------------------------------------------------
{
    EventSave save(this->event, event);
    keyboardModifiers = event->modifiers();

    // Forward it down the regular event chain
    if (forwardEvent(event))
        return;

    // Now call "key" in the current context with the ~ prefix
    text name = "~" + keyName(event);
    XL::Symbols *syms = xlProgram ? xlProgram->symbols : XL::Symbols::symbols;
    (XL::XLCall ("key"), name) (syms);
}


void Widget::mousePressEvent(QMouseEvent *event)
// ----------------------------------------------------------------------------
//   Mouse button click
// ----------------------------------------------------------------------------
{
    EventSave save(this->event, event);
    keyboardModifiers = event->modifiers();

    QMenu * contextMenu = NULL;
    uint    button      = (uint) event->button();
    int     x           = event->x();
    int     y           = event->y();

    // Create a selection if left click and nothing going on right now
    if (button == Qt::LeftButton)
        new Selection(this);

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
            contextMenu = parent()->findChild<QMenu*>(CONTEXT_MENU);
            break;
        case Qt::ShiftModifier :
            contextMenu = parent()->findChild<QMenu*>(SHIFT_CONTEXT_MENU);
            break;
        case Qt::ControlModifier :
            contextMenu = parent()->findChild<QMenu*>(CONTROL_CONTEXT_MENU);
            break;
        case Qt::AltModifier :
            contextMenu = parent()->findChild<QMenu*>(ALT_CONTEXT_MENU);
            break;
        case Qt::MetaModifier :
            contextMenu = parent()->findChild<QMenu*>(META_CONTEXT_MENU);
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
    EventSave save(this->event, event);
    keyboardModifiers = event->modifiers();

    uint button = (uint) event->button();
    int x = event->x();
    int y = event->y();

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
    EventSave save(this->event, event);
    keyboardModifiers = event->modifiers();
    bool active = event->buttons() != Qt::NoButton;
    int x = event->x();
    int y = event->y();

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
    EventSave save(this->event, event);
    keyboardModifiers = event->modifiers();

    // Create a selection if left click and nothing going on right now
    uint    button      = (uint) event->button();
    int     x           = event->x();
    int     y           = event->y();
    if (button == Qt::LeftButton)
        new Selection(this);

    // Send the click to all activities
    for (Activity *a = activities; a; a = a->Click(button, 2, x, y)) ;

    forwardEvent(event);
}


void Widget::wheelEvent(QWheelEvent *event)
// ----------------------------------------------------------------------------
//   Mouse wheel
// ----------------------------------------------------------------------------
{
    EventSave save(this->event, event);
    keyboardModifiers = event->modifiers();
    forwardEvent(event);
}


void Widget::timerEvent(QTimerEvent *event)
// ----------------------------------------------------------------------------
//    Timer expired
// ----------------------------------------------------------------------------
{
    EventSave save(this->event, event);
    forwardEvent(event);
}



// ============================================================================
//
//    XL program management
//
// ============================================================================

void Widget::updateProgram(XL::SourceFile *source)
// ----------------------------------------------------------------------------
//   Change the XL program, clean up stuff along the way
// ----------------------------------------------------------------------------
{
    xlProgram = source;
    refreshProgram();
    inError = false;
}


void Widget::applyAction(XL::Action &action)
// ----------------------------------------------------------------------------
//   Applies an action on the tree and all the dependents
// ----------------------------------------------------------------------------
{
    if (!xlProgram)
        return;
    Tree *prog = xlProgram->tree.tree;
    if (!prog)
        return;

    // Lookup imported files
    import_set iset;
    ImportedFilesChanged(prog, iset, false);

    import_set::iterator it;
    for (it = iset.begin(); it != iset.end(); it++)
    {
        XL::SourceFile &sf = **it;
        if (sf.tree.tree)
            sf.tree.tree->Do(action);
    }
}


void Widget::reloadProgram(XL::Tree *newProg)
// ----------------------------------------------------------------------------
//   Set the program to reload
// ----------------------------------------------------------------------------
{
    Tree *prog = xlProgram->tree.tree;
    if (newProg)
    {
        // Check if we can simply change some parameters in the tree
        ApplyChanges changes(newProg);
        if (!prog->Do(changes))
        {
            // Need a big hammer, i.e. reload the complete program
            newProg->Set<XL::SymbolsInfo>(prog->Get<XL::SymbolsInfo>());
            xlProgram->tree.tree = newProg;
            prog = newProg;
        }
        inError = false;
    }
    else
    {
        // We want to force a clone so that we recompile everything
        XL::NormalizedClone clone;
        newProg = prog->Do(clone);
        newProg->Set<XL::SymbolsInfo>(prog->Get<XL::SymbolsInfo>());
        xlProgram->tree.tree = newProg;
        prog = newProg;
    }

    // Now update the window
    text txt = *prog;
    Window *window = (Window *) parentWidget();
    window->setText(+txt);
}


void Widget::renormalizeProgram()
// ----------------------------------------------------------------------------
//   Remove elements in the program that make it look not good, e.g. -(-x))
// ----------------------------------------------------------------------------
{
    XL::NormalizedClone norm;
    Tree *prog = xlProgram->tree.tree;
    prog = prog->Do(norm);
    reloadProgram(prog);
}


void Widget::refreshProgram()
// ----------------------------------------------------------------------------
//   Check if any of the source files we depend on changed
// ----------------------------------------------------------------------------
{
    if (!xlProgram)
        return;

    Repository *repo = repository();
    Tree *prog = xlProgram->tree.tree;
    if (!prog)
        return;

    // Loop on imported files
    import_set iset;
    if (ImportedFilesChanged(prog, iset, false))
    {
        import_set::iterator it;
        bool needBigHammer = false;
        bool loadError     = false;
        for (it = iset.begin(); it != iset.end(); it++)
        {
            XL::SourceFile &sf = **it;
            text fname = sf.name;
            struct stat st;
            stat (fname.c_str(), &st);

            if (st.st_mtime > sf.modified)
            {
                IFTRACE(filesync)
                    std::cerr << "File " << fname << " changed\n";

                Tree *replacement = NULL;
                if (repo)
                {
                    replacement = repo->read(fname);
                }
                else
                {
                    XL::Syntax syntax(XL::MAIN->syntax);
                    XL::Positions &positions = XL::MAIN->positions;
                    XL::Errors &errors = XL::MAIN->errors;
                    XL::Parser parser(fname.c_str(), syntax, positions, errors);
                    replacement = parser.Parse();
                }

                if (!replacement)
                {
                    // Uh oh, file went away?
                    needBigHammer = true;
                }
                else
                {
                    // Check if we can simply change some parameters in file
                    ApplyChanges changes(replacement);
                    if (!sf.tree.tree->Do(changes))
                        needBigHammer = true;

                    // Record new modification time
                    sf.modified = st.st_mtime;

                    IFTRACE(filesync)
                    {
                        if (needBigHammer)
                            std::cerr << "Need to reload everything.\n";
                        else
                            std::cerr << "Surgical replacement worked\n";
                    }
                } // Replacement checked

                if (fname == xlProgram->name)
                {
                    // Update source file view
                    Window *window = (Window *) parentWidget();
                    loadError = !window->loadFileIntoSourceFileView(+fname);
                    if (loadError)
                    {
                        IFTRACE(filesync)
                            std::cerr << "Main program file could not be read\n";
                        // FIXME: now source file view is cleared, but drawing is
                        // still there -> how to delete main tree?
                    }
                }

                // If a file was modified, we need to refresh the screen
                refresh();

            } // If file modified
        } // For all files

        // If we were not successful with simple changes, reload everything...
        if (needBigHammer)
        {
            if (!loadError)
            {
                for (it = iset.begin(); it != iset.end(); it++)
                {
                    XL::SourceFile &sf = **it;
                    text fname = sf.name;
                    XL::MAIN->LoadFile(fname);
                    inError = false;
                }
            }
        }
    }
}


void Widget::markChanged(text reason)
// ----------------------------------------------------------------------------
//    Record that the program changed
// ----------------------------------------------------------------------------
{
    Repository *repo = repository();
    if (repo)
        repo->markChanged(reason);
    if (xlProgram)
    {
        if (Tree *prog = xlProgram->tree.tree)
        {
            import_set done;
            ImportedFilesChanged(prog, done, true);

            import_set::iterator f;
            for (f = done.begin(); f != done.end(); f++)
            {
                XL::SourceFile &sf = **f;
                if (&sf != xlProgram && sf.changed)
                    writeIfChanged(sf);
            }

            // Now update the window
            text txt = *prog;
            Window *window = (Window *) parentWidget();
            window->setText(+txt);
        }
    }

    // Cause the screen to redraw
    refresh(0);
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

        if (repo->write(fname, sf.tree.tree))
        {
            // Mark the tree as no longer changed
            sf.changed = false;

            // Record that we need to commit it sometime soon
            repo->change(fname);
            IFTRACE(filesync)
                std::cerr << "Changed " << fname << "\n";

            // Record time when file was changed
            struct stat st;
            stat (fname.c_str(), &st);
            sf.modified = st.st_mtime;

            return true;
        }

        IFTRACE(filesync)
            std::cerr << "Could not write " << fname << " to repository\n";
    }
    return false;
}


bool Widget::doCommit(bool immediate)
// ----------------------------------------------------------------------------
//   Commit files previously written to repository and reset next commit time
// ----------------------------------------------------------------------------
{
    IFTRACE(filesync)
            std::cerr << "Commit: " << repository()->whatsNew << "\n";
    bool done;
    done = immediate ? repository()->commit() : repository()->asyncCommit();
    if (done)
    {
        XL::Main *xlr = XL::MAIN;
        nextCommit = now() + xlr->options.commit_interval * 1000;

        Window *window = (Window *) parentWidget();
        window->markChanged(false);

        return true;
    }
    return false;
}


Repository * Widget::repository()
// ----------------------------------------------------------------------------
//   Return the repository associated with the current document (may be NULL)
// ----------------------------------------------------------------------------
{
    Window * win = (Window *)parentWidget();
    return win->repository();
}


XL::Tree *Widget::get(Tree *shape, text name, text topName)
// ----------------------------------------------------------------------------
//   Find an attribute in the current shape or returns NULL
// ----------------------------------------------------------------------------
{
    // Can't get attributes without a current shape
    if (!shape)
        return NULL;

    // The current shape has to be a 'shape' prefix
    XL::Prefix *shapePrefix = shape->AsPrefix();
    if (!shapePrefix)
        return NULL;
    Name *shapeName = shapePrefix->left->AsName();
    if (!shapeName || shapeName->value != topName)
        return NULL;

    // Take the right child. If it's a block, extract the block
    Tree *child = shapePrefix->right;
    if (XL::Block *block = child->AsBlock())
        child = block->child;

    // Now loop on all statements, looking for the given name
    while (child)
    {
        Tree *what = child;

        // Check if we have \n or ; infix
        XL::Infix *infix = child->AsInfix();
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
        if (XL::Prefix *prefix = what->AsPrefix())
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


bool Widget::set(Tree *shape, text name, Tree *value, text topName)
// ----------------------------------------------------------------------------
//   Set an attribute in the current shape, return true if successful
// ----------------------------------------------------------------------------
{
    // Can't get attributes without a current shape
    if (!shape)
        return false;

    // The current shape has to be a 'shape' prefix
    XL::Prefix *shapePrefix = shape->AsPrefix();
    if (!shapePrefix)
        return false;
    Name *shapeName = shapePrefix->left->AsName();
    if (!shapeName || shapeName->value != topName)
        return false;

    // Take the right child. If it's a block, extract the block
    Tree **addr = &shapePrefix->right;
    Tree *child = *addr;
    if (XL::Block *block = child->AsBlock())
    {
        addr = &block->child;
        child = *addr;
    }
    Tree **topAddr = addr;

    // Now loop on all statements, looking for the given name
    while (child)
    {
        Tree *what = child;

        // Check if we have \n or ; infix
        XL::Infix *infix = child->AsInfix();
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
            if (XL::Prefix *prefix = what->AsPrefix())
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
    *topAddr = new XL::Infix("\n", value, *topAddr);
    reloadProgram();
    return true;
}


bool Widget::get(Tree *shape, text name, XL::tree_list &args, text topName)
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
    XL::Prefix *prefix = attrib->AsPrefix();
    if (!prefix)
        return false;           // ??? This shouldn't happen

    // Get attribute arguments and decompose them into 'args'
    Tree *argsTree = prefix->right;
    while (XL::Infix *infix = argsTree->AsInfix())
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


bool Widget::set(Tree *shape, text name, XL::tree_list &args, text topName)
// ----------------------------------------------------------------------------
//   Set the arguments, building the comma-separated list
// ----------------------------------------------------------------------------
{
    Tree *call = new XL::Name(name);
    if (uint arity = args.size())
    {
        Tree *argsTree = args[0];
        for (uint a = 1; a < arity; a++)
            argsTree = new XL::Infix(",", argsTree, args[a]);
        call = new XL::Prefix(call, argsTree);
    }

    return set(shape, name, call, topName);
}


bool Widget::get(Tree *shape, text name, attribute_args &args, text topName)
// ----------------------------------------------------------------------------
//   Get the arguments, decomposing args in a comma-separated list
// ----------------------------------------------------------------------------
{
    // Get the trees
    XL::tree_list treeArgs;
    if (!get(shape, name, treeArgs, topName))
        return false;

    // Convert from integer or tree values
    XL::tree_list::iterator i;
    for (i = treeArgs.begin(); i != treeArgs.end(); i++)
    {
        Tree *arg = *i;
        if (!arg->IsConstant())
            arg = xl_evaluate(arg);
        if (XL::Real *asReal = arg->AsReal())
            args.push_back(asReal->value);
        else if (XL::Integer *asInteger = arg->AsInteger())
            args.push_back(asInteger->value);
        else return false;
    }

    return true;
}


bool Widget::set(Tree *shape, text name, attribute_args &args, text topName)
// ----------------------------------------------------------------------------
//   Set the arguments, building the comma-separated list
// ----------------------------------------------------------------------------
{
    Tree *call = new XL::Name(name);
    if (uint arity = args.size())
    {
        Tree *argsTree = new XL::Real(args[0]);
        for (uint a = 1; a < arity; a++)
            argsTree = new XL::Infix(",", argsTree, new XL::Real(args[a]));
        call = new XL::Prefix(call, argsTree);
    }

    return set(shape, name, call, topName);
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


ulonglong Widget::elapsed(ulonglong since, ulonglong until,
                          bool stats, bool show)
// ----------------------------------------------------------------------------
//    Record how much time passed since last measurement
// ----------------------------------------------------------------------------
{
    ulonglong t = until - since;
    if (t == 0)
        t = 1; // Because Windows lies

    if (stats)
    {
        if (tmin > t) tmin = t;
        if (tmax < t) tmax = t;
        tsum += t;
        tcount++;
    }

    if (show && (tcount & 15) == 0)
    {
        char buffer[80];
        snprintf(buffer, sizeof(buffer),
                 "Duration=%llu-%llu (~%f) %5.2f-%5.2f FPS (~%5.2f)",
                 tmin, tmax, double(tsum )/ tcount,
                 (100000000ULL / tmax)*0.01,
                 (100000000ULL / tmin)*0.01,
                 (100000000ULL * tcount / tsum) * 0.01);
        Window *window = (Window *) parentWidget();
        window->statusBar()->showMessage(QString(buffer));
    }

    return t;
}



// ============================================================================
//
//    Selection management
//
// ============================================================================

uint Widget::selected(uint i)
// ----------------------------------------------------------------------------
//   Test if the current shape is selected
// ----------------------------------------------------------------------------
{
    return i && selection.count(i) > 0 ? selection[i] : 0;
}


uint Widget::selected(Layout *layout)
// ----------------------------------------------------------------------------
//   Test if the current shape is selected
// ----------------------------------------------------------------------------
{
    return selected(layout->id);
}


void Widget::select(uint id, uint count)
// ----------------------------------------------------------------------------
//    Select the current shape if we are in selectable state
// ----------------------------------------------------------------------------
{
    if (id && id != ~0U)
    {
        if (count)
            selection[id] = count;
        else
            selection.erase(id);
    }
}


void Widget::deleteFocus(QWidget *widget)
// ----------------------------------------------------------------------------
//   Make sure we don't keep a focus on a widget that was deleted
// ----------------------------------------------------------------------------
{
    if (focusWidget == widget)
        focusWidget = NULL;
}


void Widget::requestFocus(QWidget *widget, coord x, coord y)
// ----------------------------------------------------------------------------
//   Some other widget request the focus
// ----------------------------------------------------------------------------
{
    if (!focusWidget)
    {
        GLMatrixKeeper saveGL;
        Vector3 v = layout->Offset() + Vector3(x, y, 0);
        focusWidget = widget;
        glTranslatef(v.x, v.y, v.z);
        recordProjection();
        QFocusEvent focusIn(QEvent::FocusIn, Qt::ActiveWindowFocusReason);
        QObject *fin = focusWidget;
        fin->event(&focusIn);
    }
}


void Widget::recordProjection()
// ----------------------------------------------------------------------------
//   Record the transformation matrix for the current projection
// ----------------------------------------------------------------------------
{
    glGetDoublev(GL_PROJECTION_MATRIX, focusProjection);
    glGetDoublev(GL_MODELVIEW_MATRIX, focusModel);
    glGetIntegerv(GL_VIEWPORT, focusViewport);
}


Point3 Widget::unproject (coord x, coord y, coord z)
// ----------------------------------------------------------------------------
//   Convert mouse clicks into 3D planar coordinates for the focus object
// ----------------------------------------------------------------------------
{
    // Adjust between mouse and OpenGL coordinate systems
    y = height() - y;

    // Get 3D coordinates for the near plane based on window coordinates
    GLdouble x3dn, y3dn, z3dn;
    gluUnProject(x, y, 0.0,
                 focusModel, focusProjection, focusViewport,
                 &x3dn, &y3dn, &z3dn);

    // Same with far-plane 3D coordinates
    GLdouble x3df, y3df, z3df;
    gluUnProject(x, y, 1.0,
                 focusModel, focusProjection, focusViewport,
                 &x3df, &y3df, &z3df);

    GLfloat zDistance = z3dn - z3df;
    if (zDistance == 0.0)
        zDistance = 1.0;
    GLfloat ratio = (z3dn - z) / zDistance;
    GLfloat x3d = x3dn + ratio * (x3df - x3dn);
    GLfloat y3d = y3dn + ratio * (y3df - y3dn);

    return Point3(x3d, y3d, z);
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


void Widget::drawSelection(const Box3 &bnds, text selName)
// ----------------------------------------------------------------------------
//    Draw a 2D or 3D selection with the given coordinates
// ----------------------------------------------------------------------------
{
    // Symbols where we will find the selection code
    XL::Symbols *symbols = XL::Symbols::symbols;
    if (xlProgram)
        symbols = xlProgram->symbols;

    Box3 bounds(bnds);
    bounds.Normalize();

    coord w = bounds.Width();
    coord h = bounds.Height();
    coord d = bounds.Depth();
    Point3 c  = bounds.Center();
    SpaceLayout selectionSpace(this);

    XL::LocalSave<Layout *> saveLayout(layout, &selectionSpace);
    XL::LocalSave<GLuint>   saveId(id, ~0);
    GLAttribKeeper          saveGL;
    glDisable(GL_DEPTH_TEST);
    if (bounds.Depth() > 0)
        (XL::XLCall("draw_" + selName), c.x, c.y, c.z, w, h, d) (symbols);
    else
        (XL::XLCall("draw_" + selName), c.x, c.y, w, h) (symbols);
    selectionSpace.Draw(NULL);
    glEnable(GL_DEPTH_TEST);
}


void Widget::drawHandle(const Point3 &p, text handleName)
// ----------------------------------------------------------------------------
//    Draw the handle of a 2D or 3D selection
// ----------------------------------------------------------------------------
{
    // Symbols where we will find the selection code
    XL::Symbols *symbols = XL::Symbols::symbols;
    if (xlProgram)
        symbols = xlProgram->symbols;

    SpaceLayout selectionSpace(this);
    XL::LocalSave<Layout *> saveLayout(layout, &selectionSpace);
    GLAttribKeeper          saveGL;
    glDisable(GL_DEPTH_TEST);
    selectionSpace.id = ~0U;
    (XL::XLCall("draw_" + handleName), p.x, p.y, p.z) (symbols);
    selectionSpace.Draw(NULL);
    glEnable(GL_DEPTH_TEST);
}



// ============================================================================
//
//   XLR runtime entry points
//
// ============================================================================

#pragma GCC diagnostic ignored "-Wunused-parameter"

Widget *Widget::current = NULL;
typedef XL::Tree Tree;

XL::Text *Widget::page(Tree *self, text name, Tree *body)
// ----------------------------------------------------------------------------
//   Start a new page, returns the previously named page
// ----------------------------------------------------------------------------
{
    // We start with first page if we had no page set
    if (pageName == "")
        pageName = name;

    // Increment pageId
    pageId++;

    // If the page is set, then we display it
    if (pageName == name)
    {
        // Initialize back-link
        pageShown = pageId;
        pageLinks.clear();
        if (pageId > 1)
            pageLinks["PageUp"] = lastPageName;
        pageTree = body;
        xl_evaluate(body);
    }
    else if (pageName == lastPageName)
    {
        // We are executing the page following the current one:
        // Check if PageDown is set, otherwise set current page as default
        if (pageLinks.count("PageDown") == 0)
            pageLinks["PageDown"] = name;
    }

    lastPageName = name;
    return new Text(pageName);
}


XL::Text *Widget::pageLink(Tree *self, text key, text name)
// ----------------------------------------------------------------------------
//   Indicate the chaining of pages, returns previous information
// ----------------------------------------------------------------------------
{
    text old = pageLinks[key];
    pageLinks[key] = name;
    return new Text(old);
}


XL::Text *Widget::pageLabel(Tree *self)
// ----------------------------------------------------------------------------
//   Return the label of the current page
// ----------------------------------------------------------------------------
{
    return new Text(pageName);
}


XL::Integer *Widget::pageNumber(Tree *self)
// ----------------------------------------------------------------------------
//   Return the number of the current page
// ----------------------------------------------------------------------------
{
    return new Integer(pageShown);
}


XL::Integer *Widget::pageCount(Tree *self)
// ----------------------------------------------------------------------------
//   Return the number of pages in the current document
// ----------------------------------------------------------------------------
{
    return new Integer(pageTotal);
}


XL::Real *Widget::pageWidth(Tree *self)
// ----------------------------------------------------------------------------
//   Return the width of the page
// ----------------------------------------------------------------------------
{
    return new Real(pageW);
}


XL::Real *Widget::pageHeight(Tree *self)
// ----------------------------------------------------------------------------
//   Return the height of the page
// ----------------------------------------------------------------------------
{
    return new Real(pageH);
}


XL::Real *Widget::frameWidth(Tree *self)
// ----------------------------------------------------------------------------
//   Return the width of the current layout frame
// ----------------------------------------------------------------------------
{
    return new Real(layout->Bounds(layout).Width());
}


XL::Real *Widget::frameHeight(Tree *self)
// ----------------------------------------------------------------------------
//   Return the height of the current layout frame
// ----------------------------------------------------------------------------
{
    return new Real(layout->Bounds(layout).Height());
}


XL::Real *Widget::frameDepth(Tree *self)
// ----------------------------------------------------------------------------
//   Return the depth of the current layout frame
// ----------------------------------------------------------------------------
{
    return new Real(layout->Bounds(layout).Depth());
}


XL::Real *Widget::windowWidth(Tree *self)
// ----------------------------------------------------------------------------
//   Return the width of the window in which we display
// ----------------------------------------------------------------------------
{
    return new Real(width());
}


XL::Real *Widget::windowHeight(Tree *self)
// ----------------------------------------------------------------------------
//   Return the height of window in which we display
// ----------------------------------------------------------------------------
{
    return new Real(height());
}


XL::Real *Widget::time(Tree *self)
// ----------------------------------------------------------------------------
//   Return a fractional time, including milliseconds
// ----------------------------------------------------------------------------
{
    refresh(0.1);
    return new XL::Real(CurrentTime());
}


XL::Real *Widget::pageTime(Tree *self)
// ----------------------------------------------------------------------------
//   Return a fractional time, including milliseconds
// ----------------------------------------------------------------------------
{
    refresh(0.1);
    return new XL::Real(CurrentTime() - pageStartTime);
}


Tree *Widget::locally(Tree *self, Tree *child)
// ----------------------------------------------------------------------------
//   Evaluate the child tree while preserving the current state
// ----------------------------------------------------------------------------
{
    XL::LocalSave<Layout *> save(layout, layout->AddChild(layout->id));
    Tree *result = xl_evaluate(child);
    return result;
}


Tree *Widget::shape(Tree *self, Tree *child)
// ----------------------------------------------------------------------------
//   Evaluate the child and mark the current shape
// ----------------------------------------------------------------------------
{
    XL::LocalSave<Layout *> saveLayout(layout, layout->AddChild(newId()));
    XL::LocalSave<Tree *>   saveShape (currentShape, self);
    if (selectNextTime.count(self))
    {
        selection[id]++;
        selectNextTime.erase(self);
    }
    Tree *result = xl_evaluate(child);
    return result;
}


static inline XL::Real &r(double x) { return *new XL::Real(x); }


Tree *Widget::rotatex(Tree *self, real_r rx)
// ----------------------------------------------------------------------------
//   Rotate around X
// ----------------------------------------------------------------------------
{
    return rotate(self, rx, r(1), r(0), r(0));
}


Tree *Widget::rotatey(Tree *self, real_r ry)
// ----------------------------------------------------------------------------
//   Rotate around Y
// ----------------------------------------------------------------------------
{
    return rotate(self, ry, r(0), r(1), r(0));
}


Tree *Widget::rotatez(Tree *self, real_r rz)
// ----------------------------------------------------------------------------
//   Rotate around Z
// ----------------------------------------------------------------------------
{
    return rotate(self, rz, r(0), r(0), r(1));
}


Tree *Widget::rotate(Tree *self, real_r ra, real_r rx, real_r ry, real_r rz)
// ----------------------------------------------------------------------------
//    Rotation along an arbitrary axis
// ----------------------------------------------------------------------------
{
    layout->Add(new Rotation(ra, rx, ry, rz));
    layout->hasMatrix = true;
    return XL::xl_true;
}


Tree *Widget::translatex(Tree *self, real_r x)
// ----------------------------------------------------------------------------
//   Translate along X
// ----------------------------------------------------------------------------
{
    return translate(self, x, r(0), r(0));
}


Tree *Widget::translatey(Tree *self, real_r y)
// ----------------------------------------------------------------------------
//   Translate along Y
// ----------------------------------------------------------------------------
{
    return translate(self, r(0), y, r(0));
}


Tree *Widget::translatez(Tree *self, real_r z)
// ----------------------------------------------------------------------------
//   Translate along Z
// ----------------------------------------------------------------------------
{
    return translate(self, r(0), r(0), z);
}


Tree *Widget::translate(Tree *self, real_r tx, real_r ty, real_r tz)
// ----------------------------------------------------------------------------
//     Translation along three axes
// ----------------------------------------------------------------------------
{
    layout->Add(new Translation(tx, ty, tz));
    layout->hasMatrix = true;
    return XL::xl_true;
}


Tree *Widget::rescalex(Tree *self, real_r x)
// ----------------------------------------------------------------------------
//   Rescale along X
// ----------------------------------------------------------------------------
{
    return rescale(self, x, r(1), r(1));
}


Tree *Widget::rescaley(Tree *self, real_r y)
// ----------------------------------------------------------------------------
//   Rescale along Y
// ----------------------------------------------------------------------------
{
    return rescale(self, r(1), y, r(1));
}


Tree *Widget::rescalez(Tree *self, real_r z)
// ----------------------------------------------------------------------------
//   Rescale along Z
// ----------------------------------------------------------------------------
{
    return rescale(self, r(1), r(1), z);
}


Tree *Widget::rescale(Tree *self, real_r sx, real_r sy, real_r sz)
// ----------------------------------------------------------------------------
//     Scaling along three axes
// ----------------------------------------------------------------------------
{
    layout->Add(new Scale(sx, sy, sz));
    layout->hasMatrix = true;
    return XL::xl_true;
}


XL::Name *Widget::depthTest(XL::Tree *self, bool enable)
// ----------------------------------------------------------------------------
//   Change the delta we use for the depth
// ----------------------------------------------------------------------------
{
    GLboolean old;
    glGetBooleanv(GL_DEPTH_TEST, &old);
    if (enable)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
    return old ? XL::xl_true : XL::xl_false;
}


Tree *Widget::refresh(Tree *self, double delay)
// ----------------------------------------------------------------------------
//    Refresh after the given number of seconds
// ----------------------------------------------------------------------------
{
    return refresh (delay) ? XL::xl_true : XL::xl_false;
}


XL::Name *Widget::fullScreen(XL::Tree *self, bool fs)
// ----------------------------------------------------------------------------
//   Switch to full screen
// ----------------------------------------------------------------------------
{
    bool oldFs = isFullScreen();
    Window *window = (Window *) parentWidget();
    window->switchToFullScreen(fs);
    return oldFs ? XL::xl_true : XL::xl_false;
}


XL::Name *Widget::toggleFullScreen(XL::Tree *self)
// ----------------------------------------------------------------------------
//   Switch to full screen
// ----------------------------------------------------------------------------
{
    return fullScreen(self, !isFullScreen());
}


XL::Integer * Widget::polygonOffset(Tree *self,
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


Tree *Widget::lineColor(Tree *self, double r, double g, double b, double a)
// ----------------------------------------------------------------------------
//    Set the RGBA color for lines
// ----------------------------------------------------------------------------
{
    layout->Add(new LineColor(r, g, b, a));
    return XL::xl_true;
}


Tree *Widget::lineWidth(Tree *self, double lw)
// ----------------------------------------------------------------------------
//    Select the line width for OpenGL
// ----------------------------------------------------------------------------
{
    layout->Add(new LineWidth(lw));
    layout->hasAttributes = true;
    return XL::xl_true;
}


Tree *Widget::lineStipple(Tree *self, uint16 pattern, uint16 scale)
// ----------------------------------------------------------------------------
//    Select the line stipple pattern for OpenGL
// ----------------------------------------------------------------------------
{
    layout->Add(new LineStipple(pattern, scale));
    layout->hasAttributes = true;
    return XL::xl_true;
}


Tree *Widget::fillColor(Tree *self, double r, double g, double b, double a)
// ----------------------------------------------------------------------------
//    Set the RGBA color for fill
// ----------------------------------------------------------------------------
{
    layout->Add(new FillColor(r, g, b, a));
    return XL::xl_true;
}


Tree *Widget::fillTexture(Tree *self, text img)
// ----------------------------------------------------------------------------
//     Build a GL texture out of an image file
// ----------------------------------------------------------------------------
{
    GLuint texId = 0;

    if (img != "")
    {
        ImageTextureInfo *rinfo = self->GetInfo<ImageTextureInfo>();
        if (!rinfo)
        {
            rinfo = new ImageTextureInfo(this);
            self->SetInfo<ImageTextureInfo>(rinfo);
        }
        texId = rinfo->bind(img);
    }

    layout->Add(new FillTexture(texId));
    layout->hasAttributes = true;
    return XL::xl_true;
}


Tree *Widget::fillTextureFromSVG(Tree *self, text img)
// ----------------------------------------------------------------------------
//    Draw an image in SVG format
// ----------------------------------------------------------------------------
//    The image may be animated, in which case we will get repaintNeeded()
//    signals that we send to our 'draw()' so that we redraw as needed.
{
    GLuint texId = 0;
    if (img != "")
    {
        SvgRendererInfo *rinfo = self->GetInfo<SvgRendererInfo>();
        if (!rinfo)
        {
            rinfo = new SvgRendererInfo(this);
            self->SetInfo<SvgRendererInfo>(rinfo);
        }
        texId = rinfo->bind(img);
    }
    layout->Add(new FillTexture(texId));
    layout->hasAttributes = true;
    return XL::xl_true;
}


Tree *Widget::image(Tree *self, real_r x, real_r y, real_r w, real_r h,
                    text filename)
//----------------------------------------------------------------------------
//  Make an image
//----------------------------------------------------------------------------
//  If w or h is 0 then the image width or height is used and assigned to it.
{
    GLuint texId = 0;
    XL::LocalSave<Layout *> saveLayout(layout, layout->AddChild(layout->id));

    ImageTextureInfo *rinfo = self->GetInfo<ImageTextureInfo>();
    if (!rinfo)
    {
        rinfo = new ImageTextureInfo(this);
        self->SetInfo<ImageTextureInfo>(rinfo);
    }
    texId = rinfo->bind(filename);
    if (w.value <= 0)
        w.value = rinfo->width;
    if (h.value <= 0)
        h.value = rinfo->height;

    layout->Add(new FillTexture(texId));
    layout->hasAttributes = true;

    Rectangle shape(Box(x-w/2, y-h/2, w, h));
    layout->Add(new Rectangle(shape));

    if (currentShape)
        layout->Add(new ControlRectangle(currentShape, x, y, w, h));

    return XL::xl_true;
}



// ============================================================================
//
//    Path management
//
// ============================================================================

Tree *Widget::newPath(Tree *self, Tree *child)
// ----------------------------------------------------------------------------
//   Evaluate the child tree within a polygon
// ----------------------------------------------------------------------------
{
    if (path)
        return Ooops("Path '$1' while evaluating a path", self);

    TesselatedPath *localPath = new TesselatedPath(GLU_TESS_WINDING_ODD);
    XL::LocalSave<GraphicPath *> save(path, localPath);
    layout->Add(localPath);
    Tree *result = xl_evaluate(child);
    return result;
}


Tree *Widget::moveTo(Tree *self, real_r x, real_r y, real_r z)
// ----------------------------------------------------------------------------
//    Add a 'moveTo' to the current path
// ----------------------------------------------------------------------------
{
    if (path)
    {
        path->moveTo(Point3(x,y,z));
        path->AddControl(self, x, y, z);
    }
    else
    {
        layout->Add(new MoveTo(x, y, z));
    }
    return XL::xl_true;
}


Tree *Widget::lineTo(Tree *self, real_r x, real_r y, real_r z)
// ----------------------------------------------------------------------------
//    Add a 'lineTo' to the current path
// ----------------------------------------------------------------------------
{
    if (!path)
        return Ooops("No path for '$1'", self);
    path->lineTo(Point3(x,y,z));
    path->AddControl(self, x, y, z);
    return XL::xl_true;
}


Tree *Widget::curveTo(Tree *self,
                      real_r cx, real_r cy, real_r cz,
                      real_r x, real_r y, real_r z)
// ----------------------------------------------------------------------------
//    Add a quadric curveTo to the current path
// ----------------------------------------------------------------------------
{
    if (!path)
        return Ooops("No path for '$1'", self);
    path->curveTo(Point3(cx, cy, cz), Point3(x,y,z));
    path->AddControl(self, x, y, z);
    path->AddControl(self, cx, cy, cz);
    return XL::xl_true;
}


Tree *Widget::curveTo(Tree *self,
                      real_r c1x, real_r c1y, real_r c1z,
                      real_r c2x, real_r c2y, real_r c2z,
                      real_r x, real_r y, real_r z)
// ----------------------------------------------------------------------------
//    Add a cubic curveTo to the current path
// ----------------------------------------------------------------------------
{
    if (!path)
        return Ooops("No path for '$1'", self);
    path->curveTo(Point3(c1x, c1y, c1z), Point3(c2x, c2y, c2z), Point3(x,y,z));
    path->AddControl(self, x, y, z);
    path->AddControl(self, c1x, c1y, c1z);
    path->AddControl(self, c2x, c2y, c2z);
    return XL::xl_true;
}


Tree *Widget::moveToRel(Tree *self, real_r x, real_r y, real_r z)
// ----------------------------------------------------------------------------
//    Add a relative moveTo
// ----------------------------------------------------------------------------
{
    if (path)
    {
        path->moveTo(Vector3(x,y,z));
        path->AddControl(self, x, y, z);
    }
    else
    {
        layout->Add(new MoveToRel(x, y, z));
    }
    return XL::xl_true;
}


Tree *Widget::lineToRel(Tree *self, real_r x, real_r y, real_r z)
// ----------------------------------------------------------------------------
//    Add a 'lineTo' to the current path
// ----------------------------------------------------------------------------
{
    if (!path)
        return Ooops("No path for '$1'", self);
    path->lineTo(Vector3(x,y,z));
    path->AddControl(self, x, y, z);
    return XL::xl_true;
}


Tree *Widget::pathTextureCoord(Tree *self, real_r x, real_r y, real_r r)
// ----------------------------------------------------------------------------
//    Add a texture coordinate to the path
// ----------------------------------------------------------------------------
{
    return XL::Ooops ("Path texture coordinate '$1' not supported yet", self);
}


Tree *Widget::pathColor(Tree *self, real_r r, real_r g, real_r b, real_r a)
// ----------------------------------------------------------------------------
//   Add a color element to the path
// ----------------------------------------------------------------------------
{
    return XL::Ooops ("Path color coordinate '$1' not supported yet", self);
}


Tree *Widget::closePath(Tree *self)
// ----------------------------------------------------------------------------
//    Close the path back to its origin
// ----------------------------------------------------------------------------
{
    if (!path)
        return Ooops("No path for '$1'", self);
    path->close();
    return XL::xl_true;
}



// ============================================================================
//
//    2D primitives that can be in a path or standalone
//
// ============================================================================

Tree *Widget::rectangle(Tree *self, real_r x, real_r y, real_r w, real_r h)
// ----------------------------------------------------------------------------
//    Draw a rectangle
// ----------------------------------------------------------------------------
{
    Rectangle shape(Box(x-w/2, y-h/2, w, h));
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new Rectangle(shape));

    if (currentShape)
        layout->Add(new ControlRectangle(currentShape, x, y, w, h));

    return XL::xl_true;
}


Tree *Widget::isoscelesTriangle(Tree *self,
                                real_r x, real_r y, real_r w, real_r h)
// ----------------------------------------------------------------------------
//    Draw an isosceles triangle
// ----------------------------------------------------------------------------
{
    IsoscelesTriangle shape(Box(x-w/2, y-h/2, w, h));
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new IsoscelesTriangle(shape));

    if (currentShape)
        layout->Add(new ControlRectangle(currentShape, x, y, w, h));

    return XL::xl_true;
}


Tree *Widget::rightTriangle(Tree *self, real_r x, real_r y, real_r w, real_r h)
// ----------------------------------------------------------------------------
//    Draw a right triangle
// ----------------------------------------------------------------------------
{
    RightTriangle shape(Box(x-w/2, y-h/2, w, h));
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new RightTriangle(shape));

    if (currentShape)
        layout->Add(new ControlRectangle(currentShape, x, y, w, h));

    return XL::xl_true;
}


Tree *Widget::ellipse(Tree *self, real_r cx, real_r cy, real_r w, real_r h)
// ----------------------------------------------------------------------------
//   Circle centered around (cx,cy), size w * h
// ----------------------------------------------------------------------------
{
    Ellipse shape(Box(cx-w/2, cy-h/2, w, h));
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new Ellipse(shape));

    if (currentShape)
        layout->Add(new ControlRectangle(currentShape, cx, cy, w, h));

    return XL::xl_true;
}


Tree *Widget::ellipseArc(Tree *self,
                         real_r cx, real_r cy, real_r w, real_r h,
                         real_r start, real_r sweep)
// ----------------------------------------------------------------------------
//   Circular sector centered around (cx,cy)
// ----------------------------------------------------------------------------
{
    EllipseArc shape(Box(cx-w/2, cy-h/2, w, h), start, sweep);
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new EllipseArc(shape));

    if (currentShape)
        layout->Add(new ControlRectangle(currentShape, cx, cy, w, h));

    return XL::xl_true;
}


Tree *Widget::roundedRectangle(Tree *self,
                               real_r cx, real_r cy,
                               real_r w, real_r h, real_r r)
// ----------------------------------------------------------------------------
//   Rounded rectangle with radius r for the rounded corners
// ----------------------------------------------------------------------------
{
    RoundedRectangle shape(Box(cx-w/2, cy-h/2, w, h), r);
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new RoundedRectangle(shape));

    if (currentShape)
        layout->Add(new ControlRoundedRectangle(currentShape, cx,cy,w,h, r));


    return XL::xl_true;
}


Tree *Widget::ellipticalRectangle(Tree *self,
                                  real_r cx, real_r cy,
                                  real_r w, real_r h, real_r r)
// ----------------------------------------------------------------------------
//   Elliptical rectangle with ratio r for the elliptic sides
// ----------------------------------------------------------------------------
{
    EllipticalRectangle shape(Box(cx-w/2, cy-h/2, w, h), r);
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
                    real_r cx, real_r cy, real_r w, real_r h,
                    real_r ax, real_r ary)
// ----------------------------------------------------------------------------
//   Arrow
// ----------------------------------------------------------------------------
{
    Arrow shape(Box(cx-w/2, cy-h/2, w, h), ax, ary);
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new Arrow(shape));

    if (currentShape)
        layout->Add(new ControlArrow(currentShape, cx, cy, w, h, ax, ary));

    return XL::xl_true;
}


Tree *Widget::doubleArrow(Tree *self,
                          real_r cx, real_r cy, real_r w, real_r h,
                          real_r ax, real_r ary)
// ----------------------------------------------------------------------------
//   Double arrow
// ----------------------------------------------------------------------------
{
    DoubleArrow shape(Box(cx-w/2, cy-h/2, w, h), ax, ary);
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new DoubleArrow(shape));

    if (currentShape)
        layout->Add(new ControlArrow(currentShape, cx,cy,w,h, ax,ary, true));

    return XL::xl_true;
}


Tree *Widget::starPolygon(Tree *self,
                          real_r cx, real_r cy, real_r w, real_r h,
                          integer_r p, integer_r q)
// ----------------------------------------------------------------------------
//     GL regular p-side star polygon {p/q} centered around (cx,cy)
// ----------------------------------------------------------------------------
{
    StarPolygon shape(Box(cx-w/2, cy-h/2, w, h), p, q);
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new StarPolygon(shape));

    if (currentShape)
        layout->Add(new ControlPolygon(currentShape, cx, cy, w, h, p));

    return XL::xl_true;
}


Tree *Widget::star(Tree *self,
                   real_r cx, real_r cy, real_r w, real_r h,
                   integer_r p, real_r r)
// ----------------------------------------------------------------------------
//     GL regular p-side star centered around (cx,cy), inner radius ratio r
// ----------------------------------------------------------------------------
{
    Star shape(Box(cx-w/2, cy-h/2, w, h), p, r);
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new Star(shape));

    if (currentShape)
        layout->Add(new ControlStar(currentShape, cx, cy, w, h, p, r));

    return XL::xl_true;
}


Tree *Widget::speechBalloon(Tree *self,
                            real_r cx, real_r cy, real_r w, real_r h,
                            real_r r, real_r ax, real_r ay)
// ----------------------------------------------------------------------------
//   Speech balloon with radius r for rounded corners, and point a for the tail
// ----------------------------------------------------------------------------
{
    SpeechBalloon shape(Box(cx-w/2, cy-h/2, w, h), r, ax, ay);
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new SpeechBalloon(shape));

    if (currentShape)
        layout->Add(new ControlBalloon(currentShape, cx, cy, w, h, r, ax, ay));

    return XL::xl_true;
}


Tree *Widget::callout(Tree *self,
                      real_r cx, real_r cy, real_r w, real_r h,
                      real_r r, real_r ax, real_r ay, real_r d)
// ----------------------------------------------------------------------------
//   Callout with radius r for corners, and point a, width b for the tail
// ----------------------------------------------------------------------------
{
    Callout shape(Box(cx-w/2, cy-h/2, w, h), r, ax, ay, d);
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


XL::Tree *Widget::debugBinPacker(Tree *self, uint w, uint h, Tree *t)
// ----------------------------------------------------------------------------
//   Debug the bin packer
// ----------------------------------------------------------------------------
{
    BinPacker binpack(w, h);
    GraphicPath *path = new GraphicPath;

    struct BinPackerTest : XL::Action
    {
        BinPackerTest(GraphicPath *path, BinPacker &bp)
            : path(path), bp(bp), w(0) {}

        virtual Tree *Do (Tree *what) { return what; }
        XL::Integer *DoInteger (XL::Integer *what)
        {
            if (!w)
            {
                w = what->value;
            }
            else
            {
                BinPacker::Rect rect;
                if (bp.Allocate(w, what->value, rect))
                {
                    path->moveTo(Point3(rect.x1, rect.y1, 0));
                    path->lineTo(Point3(rect.x1, rect.y2, 0));
                    path->lineTo(Point3(rect.x2, rect.y2, 0));
                    path->lineTo(Point3(rect.x2, rect.y1, 0));
                    path->close();
                }
                w = 0;
            }
            return what;
        }
        GraphicPath *path;
        BinPacker   &bp;
        uint         w;
    } binPackerTest(path, binpack);

    t->Do(binPackerTest);
    layout->Add(path);

    return XL::xl_false;
}



// ============================================================================
//
//    3D primitives
//
// ============================================================================

Tree *Widget::sphere(Tree *self,
                     real_r x, real_r y, real_r z,
                     real_r w, real_r h, real_r d,
                     integer_r slices, integer_r stacks)
// ----------------------------------------------------------------------------
//     GL sphere
// ----------------------------------------------------------------------------
{
    layout->Add(new Sphere(Box3(x-w/2, y-h/2, z-d/2, w,h,d), slices, stacks));
    if (currentShape)
        layout->Add (new ControlBox(currentShape, x, y, z, w, h, d));
    return XL::xl_true;
}


Tree *Widget::cube(Tree *self,
                   real_r x, real_r y, real_r z,
                   real_r w, real_r h, real_r d)
// ----------------------------------------------------------------------------
//    A simple cubic box
// ----------------------------------------------------------------------------
{
    layout->Add(new Cube(Box3(x-w/2, y-h/2, z-d/2, w,h,d)));
    if (currentShape)
        layout->Add(new ControlBox(currentShape, x, y, z, w, h, d));
    return XL::xl_true;
}


Tree *Widget::cone(Tree *self,
                   real_r x, real_r y, real_r z,
                   real_r w, real_r h, real_r d)
// ----------------------------------------------------------------------------
//    A simple cone
// ----------------------------------------------------------------------------
{
    layout->Add(new Cone(Box3(x-w/2, y-h/2, z-d/2, w,h,d)));
    if (currentShape)
        layout->Add(new ControlBox(currentShape, x, y, z, w, h, d));
    return XL::xl_true;
}



// ============================================================================
//
//    Text and font
//
// ============================================================================

Tree * Widget::textBox(Tree *self,
                       real_r x, real_r y, real_r w, real_r h, Tree *prog)
// ----------------------------------------------------------------------------
//   Create a new page layout and render text in it
// ----------------------------------------------------------------------------
{
    PageLayout *tbox = new PageLayout(this);
    tbox->space = Box3(x - w/2, y-h/2, 0, w, h, 0);
    layout->Add(tbox);
    flows[flowName] = tbox;

    if (currentShape)
    {
        tbox->id = layout->id;
        layout->Add(new ControlRectangle(currentShape, x, y, w, h));
    }

    XL::LocalSave<Layout *> save(layout, tbox);
    return xl_evaluate(prog);
}


Tree *Widget::textOverflow(Tree *self,
                           real_r x, real_r y, real_r w, real_r h)
// ----------------------------------------------------------------------------
//   Overflow text box for the rest of the current text flow
// ----------------------------------------------------------------------------
{
    // Add page layout overflow rectangle
    PageLayoutOverflow *overflow =
        new PageLayoutOverflow(Box(x - w/2, y-h/2, w, h), this, flowName);
    layout->Add(overflow);
    if (currentShape)
        layout->Add(new ControlRectangle(currentShape, x, y, w, h));

    return XL::xl_true;
}


XL::Text *Widget::textFlow(Tree *self, text name)
// ----------------------------------------------------------------------------
//    Set the name of the current text flow
// ----------------------------------------------------------------------------
{
    text oldName = flowName;
    flowName = name;
    return new XL::Text(oldName);
}


Tree *Widget::textSpan(Tree *self, text_r contents)
// ----------------------------------------------------------------------------
//   Insert a block of text with the current definition of font, color, ...
// ----------------------------------------------------------------------------
{
    if (path)
        TextSpan(&contents).Draw(*path, layout);
    else
        layout->Add(new TextSpan(&contents));
    return XL::xl_true;
}


Tree *Widget::font(Tree *self, text description)
// ----------------------------------------------------------------------------
//   Select a font family
// ----------------------------------------------------------------------------
{
    layout->font.fromString(+description);
    layout->Add(new FontChange(layout->font));
    return XL::xl_true;
}


Tree *Widget::fontSize(Tree *self, double size)
// ----------------------------------------------------------------------------
//   Select a font size
// ----------------------------------------------------------------------------
{
    layout->font.setPointSizeF(size);
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
    return XL::xl_true;
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
    return XL::xl_true;
}


Tree *Widget::fontBold(Tree *self, scale amount)
// ----------------------------------------------------------------------------
//   Select whether the font is bold or not
// ----------------------------------------------------------------------------
//   Qt weight values range from 0 to 99 with 50 = regular
{
    amount = clamp(amount, 0, 99);
    layout->font.setWeight(QFont::Weight(amount));
    layout->Add(new FontChange(layout->font));
    return XL::xl_true;
}


Tree *Widget::fontUnderline(Tree *self, scale amount)
// ----------------------------------------------------------------------------
//    Select whether we underline a font
// ----------------------------------------------------------------------------
//    Qt doesn't support setting the size of the underline, it's on or off
{
    layout->font.setUnderline(bool(amount));
    layout->Add(new FontChange(layout->font));
    return XL::xl_true;
}


Tree *Widget::fontOverline(Tree *self, scale amount)
// ----------------------------------------------------------------------------
//    Select whether we draw an overline
// ----------------------------------------------------------------------------
//    Qt doesn't support setting the size of the overline, it's on or off
{
    layout->font.setOverline(bool(amount));
    layout->Add(new FontChange(layout->font));
    return XL::xl_true;
}


Tree *Widget::fontStrikeout(Tree *self, scale amount)
// ----------------------------------------------------------------------------
//    Select whether we strikeout a font
// ----------------------------------------------------------------------------
//    Qt doesn't support setting the size of the strikeout, it's on or off
{
    layout->font.setStrikeOut(bool(amount));
    layout->Add(new FontChange(layout->font));
    return XL::xl_true;
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
    return XL::xl_true;
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


Tree *Widget::justify(Tree *self, scale amount, uint axis)
// ----------------------------------------------------------------------------
//   Change justification along the given axis
// ----------------------------------------------------------------------------
{
    layout->Add(new JustificationChange(amount, jaxis(axis)));
    return XL::xl_true;
}


Tree *Widget::center(Tree *self, scale amount, uint axis)
// ----------------------------------------------------------------------------
//   Change centering along the given axis
// ----------------------------------------------------------------------------
{
    layout->Add(new CenteringChange(amount, jaxis(axis)));
    return XL::xl_true;
}


Tree *Widget::spread(Tree *self, scale amount, uint axis)
// ----------------------------------------------------------------------------
//   Change the spread along the given axis
// ----------------------------------------------------------------------------
{
    layout->Add(new SpreadChange(amount, jaxis(axis)));
    return XL::xl_true;
}


Tree *Widget::spacing(Tree *self, scale amount, uint axis)
// ----------------------------------------------------------------------------
//   Change the spacing along the given axis
// ----------------------------------------------------------------------------
{
    layout->Add(new SpacingChange(amount, jaxis(axis)));
    return XL::xl_true;
}


Tree *Widget::drawingBreak(Tree *self, Drawing::BreakOrder order)
// ----------------------------------------------------------------------------
//   Change the spacing along the given axis
// ----------------------------------------------------------------------------
{
    layout->Add(new DrawingBreak(order));
    return XL::xl_true;
}


XL::Name *Widget::textEditKey(Tree *self, text key)
// ----------------------------------------------------------------------------
//   Send a key to the activities
// ----------------------------------------------------------------------------
{
    // Check if we are changing pages here...
    if (pageLinks.count(key))
    {
        pageName = pageLinks[key];
        selection.clear();
        selectionTrees.clear();
        delete textSelection();
        delete drag();
        return XL::xl_true;
    }

    for (Activity *a = activities; a; a = a->Key(key)) ;
    return XL::xl_true;
}





// ============================================================================
//
//   Frames and widgets
//
// ============================================================================

Tree *Widget::status(Tree *self, text caption)
// ----------------------------------------------------------------------------
//   Set the status line of the window
// ----------------------------------------------------------------------------
{
    Window *window = (Window *) parentWidget();
    window->statusBar()->showMessage(+caption);
    return XL::xl_true;
}


Tree *Widget::framePaint(Tree *self,
                         real_r x, real_r y, real_r w, real_r h,
                         Tree *prog)
// ----------------------------------------------------------------------------
//   Draw a frame with the current text flow
// ----------------------------------------------------------------------------
{
    XL::LocalSave<Layout *> saveLayout(layout, layout->AddChild());
    Tree *result = frameTexture(self, w, h, prog);

    // Draw a rectangle with the resulting texture
    layout->Add(new Rectangle(Box(x-w/2, y-h/2, w, h)));
    if (currentShape)
        layout->Add(new FrameManipulator(currentShape, x, y, w, h));
    return result;
}


Tree *Widget::frameTexture(Tree *self, double w, double h, Tree *prog)
// ----------------------------------------------------------------------------
//   Make a texture out of the current text layout
// ----------------------------------------------------------------------------
{
    if (w < 16) w = 16;
    if (h < 16) h = 16;

    // Get or build the current frame if we don't have one
    FrameInfo *frame = self->GetInfo<FrameInfo>();
    Tree *result = XL::xl_false;
    if (!frame)
    {
        frame = new FrameInfo(w,h);
        self->SetInfo<FrameInfo> (frame);
    }

    do
    {
        GLAllStateKeeper saveGL;
        XL::LocalSave<Layout *> saveLayout(layout, layout->NewChild());

        // Clear the background and setup initial state
        frame->resize(w,h);
        setup(w, h);
        result = xl_evaluate(prog);

        // Draw the layout in the frame context
        frame->begin();
        layout->Draw(NULL);
        frame->end();

        // Delete the layout (it's not a child of the outer layout)
        delete layout;
        layout = NULL;
    } while (0); // State keeper and layout

    // Bind the resulting texture
    GLuint tex = frame->bind();
    layout->Add(new FillTexture(tex));
    layout->hasAttributes = true;

    return result;
}


Tree *Widget::urlPaint(Tree *self,
                       real_r x, real_r y, real_r w, real_r h,
                       Text *url, Integer *progress)
// ----------------------------------------------------------------------------
//   Draw a URL in the curent frame
// ----------------------------------------------------------------------------
{
    XL::LocalSave<Layout *> saveLayout(layout, layout->AddChild(layout->id));
    urlTexture(self, w, h, url, progress);
    WebViewSurface *surface = url->GetInfo<WebViewSurface>();
    layout->Add(new ClickThroughRectangle(Box(x-w/2, y-h/2, w, h)));
    if (currentShape)
        layout->Add(new WidgetManipulator(currentShape, x, y, w, h, surface));
    return XL::xl_true;
}


Tree *Widget::urlTexture(Tree *self, double w, double h,
                         Text *url, Integer *progress)
// ----------------------------------------------------------------------------
//   Make a texture out of a given URL
// ----------------------------------------------------------------------------
{
    if (w < 16) w = 16;
    if (h < 16) h = 16;

    // Get or build the current frame if we don't have one
    WebViewSurface *surface = url->GetInfo<WebViewSurface>();
    if (!surface)
    {
        surface = new WebViewSurface(url, this);
        url->SetInfo<WebViewSurface> (surface);
    }

    // Resize to requested size, and bind texture
    surface->resize(w,h);
    GLuint tex = surface->bind(url, progress);
    layout->Add(new FillTexture(tex));
    layout->hasAttributes = true;

    return XL::xl_true;
}


Tree *Widget::lineEdit(Tree *self,
                       real_r x, real_r y, real_r w, real_r h,
                       Text *txt)
// ----------------------------------------------------------------------------
//   Draw a line editor in the curent frame
// ----------------------------------------------------------------------------
{
    XL::LocalSave<Layout *> saveLayout(layout, layout->AddChild(layout->id));
    lineEditTexture(self, w, h, txt);
    LineEditSurface *surface = txt->GetInfo<LineEditSurface>();
    layout->Add(new ClickThroughRectangle(Box(x-w/2, y-h/2, w, h)));
    if (currentShape)
        layout->Add(new WidgetManipulator(currentShape, x, y, w, h, surface));
    return XL::xl_true;
}


Tree *Widget::lineEditTexture(Tree *self, double w, double h, Text *txt)
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
        surface = new LineEditSurface(txt, this);
        txt->SetInfo<LineEditSurface> (surface);
    }

    // Resize to requested size, and bind texture
    surface->resize(w,h);
    GLuint tex = surface->bind(txt);
    layout->Add(new FillTexture(tex));
    layout->hasAttributes = true;

    return XL::xl_true;
}

Tree *Widget::radioButton(Tree *self,
                          real_r x,real_r y, real_r w,real_r h,
                          Text *name, text_p lbl, Text* sel, Tree *act)
// ----------------------------------------------------------------------------
//   Draw a radio button in the curent frame
// ----------------------------------------------------------------------------
{
    XL::LocalSave<Layout *> saveLayout(layout, layout->AddChild(layout->id));
    radioButtonTexture(self, w, h, name, lbl, sel, act);
    return abstractButton(self, name, x, y, w, h);
}


Tree *Widget::radioButtonTexture(Tree *self, double w, double h, Text *name,
                                 Text *lbl, Text* sel, Tree *act)
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
        surface = new RadioButtonSurface(name, this, +name->value);
        name->SetInfo<AbstractButtonSurface> (surface);
    }

    // Resize to requested size, and bind texture
    surface->resize(w,h);
    GLuint tex = surface->bind(lbl, act, sel);
    layout->Add(new FillTexture(tex));
    layout->hasAttributes = true;

    return XL::xl_true;
}


Tree *Widget::checkBoxButton(Tree *self, real_r x,real_r y, real_r w, real_r h,
                             Text *name, text_p lbl, Text* sel, Tree *act)
// ----------------------------------------------------------------------------
//   Draw a check button in the curent frame
// ----------------------------------------------------------------------------
{
    XL::LocalSave<Layout *> saveLayout(layout, layout->AddChild(layout->id));
    checkBoxButtonTexture(self, w, h, name, lbl, sel, act);
    return abstractButton(self, name, x, y, w, h);
}


Tree *Widget::checkBoxButtonTexture(Tree *self, double w, double h, Text *name,
                                    Text *lbl, Text* sel, Tree *act)
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
        surface = new CheckBoxSurface(name, this, +name->value);
        name->SetInfo<AbstractButtonSurface> (surface);
    }

    // Resize to requested size, and bind texture
    surface->resize(w,h);
    GLuint tex = surface->bind(lbl, act, sel);
    layout->Add(new FillTexture(tex));
    layout->hasAttributes = true;

    return XL::xl_true;
}


Tree *Widget::pushButton(Tree *self, real_r x, real_r y, real_r w, real_r h,
                         Text *name, Text *lbl, Tree* act)
// ----------------------------------------------------------------------------
//   Draw a push button in the curent frame
// ----------------------------------------------------------------------------
{
    XL::LocalSave<Layout *> saveLayout(layout, layout->AddChild(layout->id));
    pushButtonTexture(self, w, h, name, lbl, act);
    return abstractButton(self, name, x, y, w, h);
}


Tree *Widget::pushButtonTexture(Tree *self, double w, double h, Text *name,
                                Text *lbl, Tree *act)
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
        surface = new PushButtonSurface(name, this, +name->value);
        name->SetInfo<AbstractButtonSurface> (surface);
    }

    // Resize to requested size, and bind texture
    surface->resize(w,h);
    GLuint tex = surface->bind(lbl, act, NULL);
    layout->Add(new FillTexture(tex));
    layout->hasAttributes = true;

    return XL::xl_true;
}


Tree *Widget::abstractButton(Tree *self, Text *name,
                             real_r x, real_r y, real_r w, real_r h)
// ----------------------------------------------------------------------------
//   Draw any button in the curent frame
// ----------------------------------------------------------------------------
{
    AbstractButtonSurface *surface = name->GetInfo<AbstractButtonSurface>();

    if (currentGroup &&
        ! currentGroup->buttons().contains((QAbstractButton*)surface->widget))
    {
        currentGroup->addButton((QAbstractButton*)surface->widget);
    }

    if (currentGridLayout)
    {
        currentGridLayout->addWidget(surface->widget, y, x);
        return XL::xl_true;
    }

    layout->Add(new ClickThroughRectangle(Box(x-w/2, y-h/2, w, h)));
    if (currentShape)
        layout->Add(new WidgetManipulator(currentShape, x, y, w, h, surface));

    return XL::xl_true;
}


QColorDialog *Widget::colorDialog = NULL;
Tree *Widget::colorChooser(Tree *self, text treeName, Tree *action)
// ----------------------------------------------------------------------------
//   Draw a color chooser
// ----------------------------------------------------------------------------
{
    if (colorDialog)
    {
        delete colorDialog;
        colorDialog = NULL;
    }

    colorAction.tree = action;
    colorName = treeName;

    // Setup the color dialog
    colorDialog = new QColorDialog(this);
    colorDialog->setOption(QColorDialog::ShowAlphaChannel, true);
    colorDialog->setModal(false);
    colorDialog->setOption(QColorDialog::DontUseNativeDialog, false);
    updateColorDialog();

    // Connect the dialog and show it
    connect(colorDialog, SIGNAL(colorSelected (const QColor&)),
            this, SLOT(colorChosen(const QColor &)));
    connect(colorDialog, SIGNAL(currentColorChanged (const QColor&)),
            this, SLOT(colorChosen(const QColor &)));
    colorDialog->show();


    return XL::xl_true;
}


void Widget::colorChosen(const QColor & col)
// ----------------------------------------------------------------------------
//   Slot called by the color widget when a color is selected
// ----------------------------------------------------------------------------
{
    if (!colorAction.tree)
        return;

    IFTRACE (widgets)
    {
        std::cerr << "Color "<< col.name().toStdString()
                  << "was chosen for reference "<< colorAction.tree << "\n";
    }

    // We override names 'red', 'green', 'blue' and 'alpha' in the input tree
    struct ColorTreeClone : XL::TreeClone
    {
        ColorTreeClone(const QColor &c) : color(c){}
        XL::Tree *DoName(XL::Name *what)
        {
            if (what->value == "red")
                return new XL::Real(color.redF(), what->Position());
            if (what->value == "green")
                return new XL::Real(color.greenF(), what->Position());
            if (what->value == "blue")
                return new XL::Real(color.blueF(), what->Position());
            if (what->value == "alpha")
                return new XL::Real(color.alphaF(), what->Position());

            return new XL::Name(what->value, what->Position());
        }
        QColor color;
    } replacer(col);

    // The tree to be evaluated needs its own symbol table before evaluation
    XL::Tree *toBeEvaluated = colorAction.tree;
    XL::Symbols *syms = toBeEvaluated->Get<XL::SymbolsInfo>();
    if (!syms)
        syms = XL::Symbols::symbols;
    syms = new XL::Symbols(syms);
    toBeEvaluated = toBeEvaluated->Do(replacer);
    toBeEvaluated->Set<XL::SymbolsInfo>(syms);

    // Evaluate the input tree
    xl_evaluate(toBeEvaluated);
}


void Widget::updateColorDialog()
// ----------------------------------------------------------------------------
//   Pick colors from the selection
// ----------------------------------------------------------------------------
{
    if (!colorDialog)
        return;

    // Get the default color from the first selected shape
    for (std::set<Tree *>::iterator i = selectionTrees.begin();
         i != selectionTrees.end();
         i++)
    {
        attribute_args color;
        if (get(*i, colorName, color) && color.size() == 4)
        {
            QColor qc;
            qc.setRgbF(color[0], color[1], color[2], color[3]);
            colorDialog->setCurrentColor(qc);
            break;
        }
    }
}


QFontDialog *Widget::fontDialog = NULL;
Tree *Widget::fontChooser(Tree *self, Tree *action)
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
    connect(fontDialog, SIGNAL(fontSelected (const QFont&)),
            this, SLOT(fontChosen(const QFont &)));

    fontDialog->setModal(false);
    fontDialog->show();
    fontAction.tree = action;

    return XL::xl_true;
}


void Widget::fontChosen(const QFont& ft)
// ----------------------------------------------------------------------------
//    A font was selected. Evaluate the action.
// ----------------------------------------------------------------------------
{
    if (!fontAction.tree)
        return;

    IFTRACE (widgets)
    {
        std::cerr << "Font "<< ft.toString().toStdString()
                  << "was chosen for reference "<< fontAction.tree << "\n";
    }

    struct FontTreeClone : XL::TreeClone
    {
        FontTreeClone(const QFont &f) : font(f){}
        XL::Tree *DoName(XL::Name *what)
        {
            if (what->value == "family")
                return new XL::Text(font.family().toStdString(),
                                    "\"" ,"\"",what->Position());
            if (what->value == "pointSize")
                return new XL::Integer(font.pointSize(), what->Position());
            if (what->value == "weight")
                return new XL::Integer(font.weight(), what->Position());
            if (what->value == "italic")
            {
                return new XL::Name(font.italic() ?
                                      XL::xl_true->value :
                                      XL::xl_false->value,
                                      what->Position());
            }

            return new XL::Name(what->value, what->Position());
        }
        QFont font;
    } replacer(ft);

    // The tree to be evaluated needs its own symbol table before evaluation
    XL::Tree *toBeEvaluated = fontAction.tree;
    XL::Symbols *syms = toBeEvaluated->Get<XL::SymbolsInfo>();
    if (!syms)
        syms = XL::Symbols::symbols;
    syms = new XL::Symbols(syms);
    toBeEvaluated = toBeEvaluated->Do(replacer);
    toBeEvaluated->Set<XL::SymbolsInfo>(syms);

    // Evaluate the input tree
    xl_evaluate(toBeEvaluated);
}


void Widget::updateFontDialog()
// ----------------------------------------------------------------------------
//   Pick font information from the selection
// ----------------------------------------------------------------------------
{
    if (!fontDialog)
        return;
}


Tree *Widget::colorChooser(Tree *self, real_r x, real_r y, real_r w, real_r h,
                           Tree *action)
// ----------------------------------------------------------------------------
//   Draw a color chooser
// ----------------------------------------------------------------------------
{
    XL::LocalSave<Layout *> saveLayout(layout, layout->AddChild(layout->id));

    colorChooserTexture(self, w, h, action);

    ColorChooserSurface *surface = self->GetInfo<ColorChooserSurface>();
    layout->Add(new ClickThroughRectangle(Box(x-w/2, y-h/2, w, h)));
    if (currentShape)
        layout->Add(new WidgetManipulator(currentShape, x, y, w, h, surface));
    return XL::xl_true;
}


Tree *Widget::colorChooserTexture(Tree *self, double w, double h, Tree *action)
// ----------------------------------------------------------------------------
//   Make a texture out of a given color chooser
// ----------------------------------------------------------------------------
{
    if (w < 16) w = 16;
    if (h < 16) h = 16;

    // Get or build the current frame if we don't have one
    ColorChooserSurface *surface = self->GetInfo<ColorChooserSurface>();
    if (!surface)
    {
        surface = new ColorChooserSurface(self, this, action);
        self->SetInfo<ColorChooserSurface> (surface);
    }

    // Resize to requested size, and bind texture
    surface->resize(w,h);
    GLuint tex = surface->bind();
    layout->Add(new FillTexture(tex));
    layout->hasAttributes = true;

    return XL::xl_true;
}


Tree *Widget::fontChooser(Tree *self, real_r x, real_r y, real_r w, real_r h,
                           Tree *action)
// ----------------------------------------------------------------------------
//   Draw a color chooser
// ----------------------------------------------------------------------------
{
    XL::LocalSave<Layout *> saveLayout(layout, layout->AddChild(layout->id));

    fontChooserTexture(self, w, h, action);

    FontChooserSurface *surface = self->GetInfo<FontChooserSurface>();
    layout->Add(new ClickThroughRectangle(Box(x-w/2, y-h/2, w, h)));
    if (currentShape)
        layout->Add(new WidgetManipulator(currentShape, x, y, w, h, surface));
    return XL::xl_true;
}


Tree *Widget::fontChooserTexture(Tree *self, double w, double h,
                                  Tree *action)
// ----------------------------------------------------------------------------
//   Make a texture out of a given color chooser
// ----------------------------------------------------------------------------
{
    if (w < 16) w = 16;
    if (h < 16) h = 16;

    // Get or build the current frame if we don't have one
    FontChooserSurface *surface = self->GetInfo<FontChooserSurface>();
    if (!surface)
    {
        surface = new FontChooserSurface(self, this, action);
        self->SetInfo<FontChooserSurface> (surface);
    }

    // Resize to requested size, and bind texture
    surface->resize(w,h);
    GLuint tex = surface->bind();
    layout->Add(new FillTexture(tex));
    layout->hasAttributes = true;

    return XL::xl_true;
}


Tree *Widget::buttonGroup(Tree *self, bool exclusive, Tree *buttons)
// ----------------------------------------------------------------------------
//   Create a button group for radio buttons
// ----------------------------------------------------------------------------
{
    GroupInfo *grpInfo = buttons->GetInfo<GroupInfo>();
    if (!grpInfo)
    {
        grpInfo = new GroupInfo(buttons, this);
        grpInfo->setExclusive(exclusive);
        buttons->SetInfo<GroupInfo>(grpInfo);
    }
    currentGroup = grpInfo;
    xl_evaluate(buttons);
    currentGroup = NULL;

    return XL::xl_true;
}


Tree*Widget::setAction(Tree *self, Tree *action)
// ----------------------------------------------------------------------------
//   Set the action to be executed by the current buttonGroup if any.
// ----------------------------------------------------------------------------
{
    if (currentGroup && currentGroup->action == NULL)
    {
        currentGroup->action = new XL::TreeRoot(action);
    }

    return XL::xl_true;
}


Tree *Widget::groupBox(Tree *self,
                       real_r x, real_r y, real_r w, real_r h,
                       Text *lbl, Tree *buttons)
// ----------------------------------------------------------------------------
//   Draw a group box in the curent frame
// ----------------------------------------------------------------------------
{
    XL::LocalSave<Layout *> saveLayout(layout, layout->AddChild(layout->id));

    groupBoxTexture(self, w, h, lbl);

    GroupBoxSurface *surface = self->GetInfo<GroupBoxSurface>();
    layout->Add(new ClickThroughRectangle(Box(x-w/2, y-h/2, w, h)));
    if (currentShape)
        layout->Add(new WidgetManipulator(currentShape, x, y, w, h, surface));

    xl_evaluate(buttons);

    surface->dirty = true;
    ((WidgetSurface*)surface)->bind();
    currentGridLayout = NULL;

    return XL::xl_true;
}


Tree *Widget::groupBoxTexture(Tree *self, double w, double h, Text *lbl)
// ----------------------------------------------------------------------------
//   Make a texture out of a given push button
// ----------------------------------------------------------------------------
{
    if (w < 16) w = 16;
    if (h < 16) h = 16;


    // Get or build the current frame if we don't have one
    GroupBoxSurface *surface = self->GetInfo<GroupBoxSurface>();
    if (!surface)
    {
        currentGridLayout = new QGridLayout();
        currentGridLayout->setObjectName("groupBox layout");
        surface = new GroupBoxSurface(self, this, currentGridLayout);
        self->SetInfo<GroupBoxSurface> (surface);
    }
    else
    {
        currentGridLayout = surface->grid;
    }

    // Resize to requested size, and bind texture
    surface->resize(w,h);
    GLuint tex = surface->bind(lbl);
    layout->Add(new FillTexture(tex));
    layout->hasAttributes = true;

    return XL::xl_true;
}


Tree *Widget::videoPlayer(Tree *self,
                          real_r x, real_r y, real_r w, real_r h, Text *url)
// ----------------------------------------------------------------------------
//   Make a video player
// ----------------------------------------------------------------------------
{
    XL::LocalSave<Layout *> saveLayout(layout, layout->AddChild(layout->id));
    videoPlayerTexture(self, w, h, url);
    VideoPlayerSurface *surface = self->GetInfo<VideoPlayerSurface>();
    layout->Add(new ClickThroughRectangle(Box(x-w/2, y-h/2, w, h)));
    if (currentShape)
        layout->Add(new WidgetManipulator(currentShape, x, y, w, h, surface));

    return XL::xl_true;

}


Tree *Widget::videoPlayerTexture(Tree *self, real_r w, real_r h, Text *url)
// ----------------------------------------------------------------------------
//   Make a video player texture
// ----------------------------------------------------------------------------
{
    if (w < 16) w = 16;
    if (h < 16) h = 16;

    // Get or build the current frame if we don't have one
    VideoPlayerSurface *surface = self->GetInfo<VideoPlayerSurface>();
    if (!surface)
    {
        surface = new VideoPlayerSurface(self, this);
        self->SetInfo<VideoPlayerSurface> (surface);
    }

    // Resize to requested size, and bind texture
    surface->resize(w,h);
    GLuint tex = surface->bind(url);
    layout->Add(new FillTexture(tex));
    layout->hasAttributes = true;

    return XL::xl_true;
}


// ============================================================================
//
//    Error management
//
// ============================================================================
Tree *Widget::runtimeError(Tree *self, text msg, Tree *arg)
// ----------------------------------------------------------------------------
//   Display an error message from the input
// ----------------------------------------------------------------------------
{
    inError = true;             // Stop refreshing
    XL::Error err(msg, arg, NULL, NULL);
    QMessageBox::warning(this, tr("Runtime error"),
                         tr("Error executing the program:\n%1")
                         .arg(+err.Message()));
    return XL::xl_false;
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
//   orderedMenuElements. If the order is OK, the label, etc are updated; if not
//   or not found at all a new element is created and registered.
// ============================================================================

Tree *Widget::menuItem(Tree *self, text name, text lbl, text iconFileName,
                       bool isCheckable, Text *isChecked, Tree *t)
// ----------------------------------------------------------------------------
//   Create a menu item
// ----------------------------------------------------------------------------
{
    if (!currentMenu && !currentToolBar)
        return XL::xl_false;

    QString fullName = +name;

    if (QAction* act = parent()->findChild<QAction*>(fullName))
    {
        // MenuItem found, update label, icon, checkmark if the order is OK.
        if (order < orderedMenuElements.size() &&
            orderedMenuElements[order] != NULL &&
            orderedMenuElements[order]->fullname == fullName)
        {
            act->setText(+lbl);
            if (iconFileName != "")
                act->setIcon(QIcon(+iconFileName));
            else
                act->setIcon(QIcon());
            act->setChecked(strcasecmp(isChecked->value.c_str(), "true") == 0);

            order++;
            return XL::xl_true;
        }

        // The name exist but it is not in the good order so clean it.
        delete act;
    }

    // Store the tree in the QAction.
    QVariant var = QVariant::fromValue(XL::TreeRoot(t));

    IFTRACE(menus)
    {
        std::cerr << "menuItem CREATION with name "
                  << fullName.toStdString() << " and order " << order << "\n";
        std::cerr.flush();
    }

    QAction * p_action;
    QWidget * par;
    if (currentMenu)
        par =  currentMenu;
    else
        par = currentToolBar;
    p_action = new QAction(+lbl, par);

    p_action->setData(var);

    if (iconFileName != "")
        p_action->setIcon(QIcon(+iconFileName));
    else
        p_action->setIcon(QIcon());

    p_action->setCheckable(isCheckable);
    p_action->setChecked(strcasecmp(isChecked->value.c_str(), "true") == 0);
    p_action->setObjectName(fullName);

    if (order >= orderedMenuElements.size())
        orderedMenuElements.resize(order+10);

    if (orderedMenuElements[order])
    {
        QAction*before = orderedMenuElements[order]->p_action;
        if (currentMenu)
            currentMenu->insertAction(before, p_action);
        else
            currentToolBar->insertAction(before, p_action);

        delete orderedMenuElements[order];
    }
    else
    {
        if (currentMenu)
            currentMenu->addAction(p_action);
        else
            currentToolBar->addAction(p_action);
    }

    orderedMenuElements[order] = new MenuInfo(fullName,
                                              p_action);
    order++;

    return XL::xl_true;
}


Tree *Widget::menu(Tree *self, text name, text lbl,
                   text iconFileName, bool isSubMenu)
// ----------------------------------------------------------------------------
// Add the menu to the current menu bar or create the contextual menu
// ----------------------------------------------------------------------------
{
    bool isContextMenu = false;

    // Build the full name of the menu
    // Uses the current menu name, the given string and the isSubmenu.
    QString fullname = +name;
    if (fullname.startsWith(CONTEXT_MENU))
    {
        isContextMenu = true;
    }

    // If the menu is registered, no need to recreate it if the order is exact.
    // This is used at reload time.
    if (QMenu *tmp = parent()->findChild<QMenu*>(fullname))
    {
        if (lbl == "" && iconFileName == "")
        {
            // Just set the current menu to the requested one
            currentMenu = tmp;
            return XL::xl_true;
        }

        if (order < orderedMenuElements.size())
        {
            if (MenuInfo *menuInfo = orderedMenuElements[order])
            {
                if (menuInfo->fullname == fullname)
                {
                    // Set the currentMenu and update the label and icon.
                    currentMenu = tmp;
                    if (lbl != menuInfo->title)
                    {
                        currentMenu->setTitle(+lbl);
                        menuInfo->title = lbl;
                    }
                    if (iconFileName != menuInfo->icon)
                    {
                        if (iconFileName != "")
                            currentMenu->setIcon(QIcon(+iconFileName));
                        else
                            currentMenu->setIcon(QIcon());
                        menuInfo->icon = iconFileName;
                    }
                    order++;
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
        currentMenu = new QMenu((Window*)parent());
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

        currentMenu = new QMenu(+lbl, par);
    }

    currentMenu->setObjectName(fullname);

    if (iconFileName != "")
        currentMenu->setIcon(QIcon(+iconFileName));

    if (order >= orderedMenuElements.size())
        orderedMenuElements.resize(order+10);

    if (par)
    {
        if (orderedMenuElements[order])
        {
            QAction *before = orderedMenuElements[order]->p_action;
            par->insertAction(before, currentMenu->menuAction());
        }
        else
        {
            par->addAction(currentMenu->menuAction());
        }

        QToolButton* button = NULL;
        if (par == currentToolBar &&
            (button = dynamic_cast<QToolButton*>
             (currentToolBar-> widgetForAction(currentMenu->menuAction()))))
            button->setPopupMode(QToolButton::InstantPopup);
    }

    if (orderedMenuElements[order])
        delete orderedMenuElements[order];

    orderedMenuElements[order] = new MenuInfo(fullname,
                                              currentMenu->menuAction());
    orderedMenuElements[order]->title = lbl;
    orderedMenuElements[order]->icon = iconFileName;

    IFTRACE(menus)
    {
        std::cerr << "menu CREATION with name "
                  << fullname.toStdString() << " and order " << order << "\n";
        std::cerr.flush();
    }

    order++;

    return XL::xl_true;
}


Tree * Widget::menuBar(Tree *self)
// ----------------------------------------------------------------------------
// Set the currentManueBar to the default menuBar.
// ----------------------------------------------------------------------------
{
    currentMenuBar = ((Window *)parent())->menuBar();
    currentToolBar = NULL;
    currentMenu = NULL;
    return XL::xl_true;
}


Tree * Widget::toolBar(Tree *self, text name, text title, bool isFloatable,
                       text location)
// ----------------------------------------------------------------------------
// Add the toolBar to the current widget
// ----------------------------------------------------------------------------
// The location is the prefered location for the toolbar.
// The supported values are [n|N]*, [e|E]*, [s|S]*, West or N, E, S, W, O
{
    QString fullname = +name;
    Window *win = (Window *)parent();
    if (QToolBar *tmp = win->findChild<QToolBar*>(fullname))
    {
        if (order < orderedMenuElements.size() &&
            orderedMenuElements[order] != NULL &&
            orderedMenuElements[order]->fullname == fullname)
        {
            // Set the currentMenu and update the label and icon.
            currentToolBar = tmp;
            order++;
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

    if (QMenu* view = win->findChild<QMenu*>(VIEW_MENU_NAME))
        view->addAction(currentToolBar->toggleViewAction());

    connect(currentToolBar, SIGNAL(actionTriggered(QAction*)),
            this, SLOT(userMenu(QAction*)));

    IFTRACE(menus)
    {
        std::cerr << "toolbar CREATION with name "
                  << fullname.toStdString() << " and order " << order << "\n";
        std::cerr.flush();
    }

    if (order >= orderedMenuElements.size())
        orderedMenuElements.resize(order+10);

    if (orderedMenuElements[order])
        delete orderedMenuElements[order];

    orderedMenuElements[order] = new MenuInfo(fullname, currentToolBar);

    order++;
    currentMenuBar = NULL;
    currentMenu = NULL;

    return XL::xl_true;
}


Tree * Widget::separator(Tree *self)
// ----------------------------------------------------------------------------
//   Add the separator to the current widget
// ----------------------------------------------------------------------------
{

    QString fullname = QString("SEPARATOR_%1").arg(order);

    if (QAction *tmp = parent()->findChild<QAction*>(fullname))
    {
        if (order < orderedMenuElements.size() &&
            orderedMenuElements[order] != NULL &&
            orderedMenuElements[order]->fullname == fullname)
        {
//            IFTRACE(menus)
//            {
//                std::cerr << "separator found with name "
//                          << fullname.toStdString() << " and order "
//                          << order << "\n";
//                std::cerr.flush();
//            }
            order++;
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
                  << fullname.toStdString() << " and order " << order << "\n";
        std::cerr.flush();
    }
    if (order >= orderedMenuElements.size())
        orderedMenuElements.resize(order+10);

    if (orderedMenuElements[order])
    {
        if (par)
        {
            QAction *before = orderedMenuElements[order]->p_action;
            par->insertAction(before, act);
        }
        delete orderedMenuElements[order];
    }
    else
    {
        if (par)
            par->addAction(act);
    }
    orderedMenuElements[order] = new MenuInfo(fullname, act);
    order++;
    return XL::xl_true;
}



// ============================================================================
//
//    Tree selection management
//
// ============================================================================

XL::Name *Widget::insert(Tree *self, Tree *toInsert)
// ----------------------------------------------------------------------------
//    Insert the tree after the selection, assuming there is only one
// ----------------------------------------------------------------------------
{
    if (!xlProgram)
        return XL::xl_false;

    Tree *program = xlProgram->tree.tree;
    if (XL::Block *block = toInsert->AsBlock())
        toInsert = block->child;

    InsertAtSelectionAction insert(this, toInsert, pageTree);
    Tree *afterInsert = program->Do(insert);

    // If we never hit the selection during the insert, append
    if (insert.toInsert)
    {
        Tree **top = &afterInsert;
        XL::Infix *parent  = NULL;
        if (pageTree)
        {
            if (XL::Prefix *prefix = pageTree->AsPrefix())
                if (XL::Name *left = prefix->left->AsName())
                    if (left->value == "do")
                        pageTree = prefix->right;
            if (XL::Block *block = pageTree->AsBlock())
                pageTree = block->child;

            top = &pageTree;
        }

        program = *top;
        while (true)
        {
            XL::Infix *infix = program->AsInfix();
            if (!infix)
                break;
            if (infix->name != ";" && infix->name != "\n")
                break;
            parent = infix;
            program = infix->right;
         }

        Tree **what = parent ? &parent->right : top;
        *what = new XL::Infix("\n", *what, toInsert);
    }

    reloadProgram(afterInsert);
    markChanged("Inserted tree");

    return XL::xl_true;
}


XL::Name *Widget::deleteSelection(Tree *self, text key)
// ----------------------------------------------------------------------------
//    Delete the selection (with text support)
// ----------------------------------------------------------------------------
{
    if (textSelection())
        return textEditKey(self, key);

    deleteSelection();

    return XL::xl_true;
}


void Widget::deleteSelection()
// ----------------------------------------------------------------------------
//    Delete the selection (when selection is not text)
// ----------------------------------------------------------------------------
{
    DeleteSelectionAction del(this);
    XL::Tree *what = xlProgram->tree.tree;
    what = what->Do(del);
    reloadProgram(what);
    markChanged("Deleted selection");
    selection.clear();
    selectionTrees.clear();
}


XL::Name *Widget::setAttribute(Tree *self,
                               text name, Tree *attribute,
                               text shape)
// ----------------------------------------------------------------------------
//    Insert the tree in all shapes in the selection
// ----------------------------------------------------------------------------
{
    if (!xlProgram)
        return XL::xl_false;

    Tree *program = xlProgram->tree.tree;
    if (XL::Block *block = attribute->AsBlock())
        attribute = block->child;

    SetAttributeAction setAttrib(name, attribute, this, shape);
    program->Do(setAttrib);
    reloadProgram();
    markChanged("Updated " + name + " attribute");

    return XL::xl_true;
}



// ============================================================================
//
//   Unit conversions
//
// ============================================================================

XL::Real *Widget::fromCm(Tree *self, double cm)
// ----------------------------------------------------------------------------
//   Convert from cm to pixels
// ----------------------------------------------------------------------------
{
    XL_RREAL(cm * logicalDpiX() * (1.0 / 2.54));
}


XL::Real *Widget::fromMm(Tree *self, double mm)
// ----------------------------------------------------------------------------
//   Convert from mm to pixels
// ----------------------------------------------------------------------------
{
    XL_RREAL(mm * logicalDpiX() * (0.1 / 2.54));
}


XL::Real *Widget::fromIn(Tree *self, double in)
// ----------------------------------------------------------------------------
//   Convert from inch to pixels
// ----------------------------------------------------------------------------
{
    XL_RREAL(in * logicalDpiX());
}


XL::Real *Widget::fromPt(Tree *self, double pt)
// ----------------------------------------------------------------------------
//   Convert from pt to pixels
// ----------------------------------------------------------------------------
{
    XL_RREAL(pt * logicalDpiX() * (1.0 / 72.0));
}


XL::Real *Widget::fromPx(Tree *self, double px)
// ----------------------------------------------------------------------------
//   Convert from pixel to pixels (trivial)
// ----------------------------------------------------------------------------
{
    XL_RREAL(px);
}

TAO_END


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
