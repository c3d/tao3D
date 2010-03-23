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
#include "widget-surface.h"
#include "window.h"
#include "treeholder.h"
#include "apply-changes.h"
#include "activity.h"
#include "selection.h"
#include "drag.h"
#include "shapename.h"
#include "treeholder.h"
#include "menuinfo.h"
#include "repository.h"
#include "application.h"
#include "process.h"

#include <QtGui/QImage>
#include <cmath>
#include <QFont>
#include <iostream>
#include <QVariant>
#include <QtWebKit>
#include <sys/time.h>
#include <sys/stat.h>

#define Z_NEAR  2000.0
#define Z_FAR  40000.0

TAO_BEGIN

// ============================================================================
//
//   Widget life management
//
// ============================================================================

Widget::Widget(Window *parent, XL::SourceFile *sf)
// ----------------------------------------------------------------------------
//    Create the GL widget
// ----------------------------------------------------------------------------
    : QGLWidget(QGLFormat(QGL::SampleBuffers|QGL::AlphaChannel), parent),
      xlProgram(sf), timer(this), idleTimer(this), currentMenu(NULL),
      frame(NULL), mainFrame(NULL), activities(NULL),
      page_start_time(CurrentTime()),
      id(0), capacity(0), selector(0), activeSelector(0),
      event(NULL), focusWidget(NULL),
      whatsNew(""), reloadProgram(false),
      tmin(~0ULL), tmax(0), tsum(0), tcount(0),
      nextSave(now()), nextCommit(nextSave), nextSync(nextSave)
{
    // Make sure we don't fill background with crap
    setAutoFillBackground(false);

    // Make this the current context for OpenGL
    makeCurrent();
    mainFrame = new Frame;

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

    // Make sure we have at least a selector name for "move"
    selectors["move"] = 1;
    selectorNames.push_back("move");    // Index 1
}


Widget::~Widget()
// ----------------------------------------------------------------------------
//   Destroy the widget
// ----------------------------------------------------------------------------
{
    delete mainFrame;
}



// ============================================================================
//
//   Widget basic events (painting, mause, ...)
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
    mainFrame->Resize(width, height);
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


void Widget::updateProgram(XL::SourceFile *source)
// ----------------------------------------------------------------------------
//   Change the XL program, clean up stuff along the way
// ----------------------------------------------------------------------------
{
    xlProgram = source;
    refreshProgram();
}


void Widget::refreshProgram()
// ----------------------------------------------------------------------------
//   Check if any of the source files we depend on changed
// ----------------------------------------------------------------------------
{
    if (!xlProgram)
        return;

    Repository *repository = TaoApp->library();
    Tree *prog = xlProgram->tree.tree;
    if (!prog)
        return;

    // Loop on imported files
    import_set iset;
    if (ImportedFilesChanged(prog, iset, false))
    {
        import_set::iterator it;
        bool needBigHammer = false;
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
                if (repository)
                {
                    replacement = repository->read(fname);
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
            } // If file modified
        } // For all files

        // If we were not successful with simple changes, reload everything...
        if (needBigHammer)
        {
            for (it = iset.begin(); it != iset.end(); it++)
            {
                XL::SourceFile &sf = **it;
                text fname = sf.name;
                XL::MAIN->LoadFile(fname);
            }
        }
    }

    // Perform a good old garbage collection to clean things up
    XL::Context::context->CollectGarbage();
}


void Widget::markChanged(text reason)
// ----------------------------------------------------------------------------
//    Record that the program changed
// ----------------------------------------------------------------------------
{
    if (whatsNew.find(reason) == whatsNew.npos)
    {
        if (whatsNew.length())
            whatsNew += "\n";
        whatsNew += reason;
    }
    if (xlProgram)
    {
        if (Tree *prog = xlProgram->tree.tree)
        {
            import_set done;
            ImportedFilesChanged(prog, done, true);
        }
    }
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
    double zNear = Z_NEAR, zFar = Z_FAR;
    double eyeX = 0.0, eyeY = 0.0, eyeZ = zNear;
    double centerX = 0.0, centerY = 0.0, centerZ = 0.0;
    double upX = 0.0, upY = 1.0, upZ = 0.0;
    glFrustum (-w/2, w/2, -h/2, h/2, zNear, zFar);
    gluLookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ);
    glTranslatef(0.0, 0.0, -zNear);
    glScalef(2.0, 2.0, 2.0);

    glGetIntegerv(GL_DEPTH_BITS, &depthBits);
    switch (depthBits) 
    {
        case 32:
            depthBitsMax = UINT32_MAX;
            break;
        default:
            depthBitsMax = (1u<<depthBits) - 1;
    }

    // Setup the model view matrix so that 1.0 unit = 1px
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Reset default GL parameters
    setupGL();

    // Initial state
    depth = 0.0;
    state.frameWidth = w;
    state.frameHeight = h;
    state.charFormat = QTextCharFormat();
    state.charFormat.setForeground(Qt::black);
    state.charFormat.setBackground(Qt::white);
    state.depthDelta = 0.05;
    state.selectable = true;
    state.filled = true;
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
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_RECTANGLE_ARB);
    glDisable(GL_CULL_FACE);
}


coord Widget::zBuffer(coord z, int pos)
// ----------------------------------------------------------------------------
//   Calculate minimal z increment depending on the GL_DEPTH_BITS 
// ----------------------------------------------------------------------------
{
    long b = 50 * pos; // Don't touch this settings unless you exactly know 
                       // what you are going
    return b2z( z2b(z) - b );   
}
 
double Widget::z2b(coord z)
{
    double n = Z_NEAR, f = Z_FAR;
    double s = double(depthBitsMax);
    double zn = 2.0 * (n - z);
    return floor(s * (f - (f * n / zn)) / (f - n) + 0.5);
}

double  Widget::b2z(ulong b)
{
    double n = Z_NEAR, f = Z_FAR;
    double s = double(depthBitsMax);
    double zn = f * n / ((double(b) / s) * (f - n) - f);
    return n + zn / 2.0;
}


void Widget::dawdle()
// ----------------------------------------------------------------------------
//   Operations to do when idle (in the background)
// ----------------------------------------------------------------------------
{
    // Run all activities, which will get them a chance to update refresh
    for (Activity *a = activities; a; a = a->Idle()) ;

    // We will only auto-save and commit if we have a valid repository
    Repository *repository     = TaoApp->library();
    XL::Main   *xlr            = XL::MAIN;
    bool        savedSomething = false;

    if (xlProgram && xlProgram->changed)
    {
        text txt = *xlProgram->tree.tree;
        Window *window = (Window *) parentWidget();
        window->setText(+txt);
    }

    // Check if there's something to save
    ulonglong tick = now();
    longlong saveDelay = longlong(nextSave - tick);
    if (repository && saveDelay < 0)
    {
        XL::source_files::iterator it;
        for (it = xlr->files.begin(); it != xlr->files.end(); it++)
        {
            XL::SourceFile &sf = (*it).second;
            text fname = sf.name;
            if (sf.changed)
            {
                if (repository->write(fname, sf.tree.tree))
                {
                    // Mark the tree as no longer changed
                    sf.changed = false;
                    savedSomething = true;

                    // Record that we need to commit it sometime soon
                    repository->change(fname);
                    IFTRACE(filesync)
                        std::cerr << "Changed " << fname << "\n";

                    // Record time when file was changed
                    struct stat st;
                    stat (fname.c_str(), &st);
                    sf.modified = st.st_mtime;
                }
            }
        }

        // Record when we will save file again
        nextSave = tick + xlr->options.save_interval * 1000;
    }

    // Check if there's something to commit
    longlong commitDelay = longlong (nextCommit - tick);
    if (savedSomething && commitDelay < 0)
    {
        // If we saved anything, then commit changes
        IFTRACE(filesync)
            std::cerr << "Commit: " << whatsNew << "\n";
        if (repository->commit(whatsNew))
        {
            whatsNew = "";
            nextCommit = tick + xlr->options.commit_interval * 1000;
            savedSomething = false;
        }
    }

    // Check if there's something to reload
    longlong syncDelay = longlong(nextSync - tick);
    if (syncDelay < 0)
    {
        refreshProgram();
        syncDelay = tick + xlr->options.sync_interval * 1000;
    }
}


void Widget::draw()
// ----------------------------------------------------------------------------
//    Redraw the widget
// ----------------------------------------------------------------------------
{
    // Timing
    ulonglong t = now();
    event = NULL;

    // Need to setup initial context for the activities
    XL::LocalSave<Frame *> saveFrame (frame, mainFrame);

    // Setup the initial drawing environment
    double w = width(), h = height();
    setup(w, h);

    // Clear the background
    glClearColor (1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // If there is a program, we need to run it
    runProgram();

    // Timing
    elapsed(t);

    // Render all activities, e.g. the selection rectangle
    glDisable(GL_DEPTH_TEST);
    for (Activity *a = activities; a; a = a->Display()) ;
}


void Widget::runProgram()
// ----------------------------------------------------------------------------
//   Run the current XL program
// ----------------------------------------------------------------------------
{
    // Reset the selection id for the various elements being drawn
    id = 0;
    focusWidget = NULL;
    state.paintDevice = this;

    // Run the XL program associated with this widget
    current = this;
    QTextOption alignCenter(Qt::AlignCenter);
    TextFlow mainFlow(alignCenter);
    XL::LocalSave<TextFlow *> saveFlow(state.flow, &mainFlow);
    XL::LocalSave<Frame *> saveFrame (frame, mainFrame);

    try
    {
        if (xlProgram)
        {
            if (Tree *prog = xlProgram->tree.tree)
            {
                if (reloadProgram)
                {
                    XL::NormalizedClone cloneAction;
                    Tree *copy = prog->Do(cloneAction);
                    copy->Set<XL::SymbolsInfo>(prog->Get<XL::SymbolsInfo>());
                    xlProgram->tree.tree = copy;
                    prog = copy;
                    reloadProgram = false;
                }
                xl_evaluate(prog);
            }
        }
    }
    catch (XL::Error &e)
    {
        xlProgram = NULL;
        QMessageBox::warning(this, tr("Runtime error"),
                             tr("Error executing the program:\n%1")
                             .arg(QString::fromStdString(e.Message())));
    }
    catch(...)
    {
        xlProgram = NULL;
        QMessageBox::warning(this, tr("Runtime error"),
                             tr("Unknown error executing the program"));
    }

    // After we are done, flush the frame and over-paint it
    mainFrame->Paint();

    // Once we are done, do a garbage collection
    XL::Context::context->CollectGarbage();

    // Remember how many elements are drawn on the page, plus arbitrary buffer
    if (id > capacity)
        capacity = id + 100;
    else if (id + 50 < capacity / 2)
        capacity = capacity / 2;
}


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


ulonglong Widget::elapsed(ulonglong since, bool stats, bool show)
// ----------------------------------------------------------------------------
//    Record how much time passed since last measurement
// ----------------------------------------------------------------------------
{
    ulonglong t = now() - since;
    if (t == 0)
        t = 1; // Because windows lies

    if (stats)
    {
        if (tmin > t) tmin = t;
        if (tmax < t) tmax = t;
        tsum += t;
        tcount++;
    }

    if (show)
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


bool Widget::forwardEvent(QEvent *event)
// ----------------------------------------------------------------------------
//   Forward event to the focus proxy if there is any
// ----------------------------------------------------------------------------
{
    if (QObject *focus = focusWidget)
        return focus->event(event);
    return false;
}

void Widget::userMenu(QAction *p_action)
// ----------------------------------------------------------------------------
//   user menu slot activation
// ----------------------------------------------------------------------------
{
    if (!p_action)
        return;

    IFTRACE(menus)
    {
        std::cout << "Action " << p_action->objectName().toStdString()
                << " (" << p_action->text().toStdString() << ") activated\n";
    }
    QVariant var =  p_action->data();
    if (!var.isValid())
        return;

    TreeHolder t = var.value<TreeHolder >();
    xlProgram->tree.tree = new XL::Infix("\n", xlProgram->tree.tree, t.tree);

    markChanged("Menu clicked, added program element");
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
        int hh = height();

        Point3 u = unproject(x, hh-y, 0);
        QMouseEvent local(event->type(), QPoint(u.x + w/2, h/2 - u.y),
                          event->button(), event->buttons(),
                          event->modifiers());
        return focus->event(&local);
    }
    return false;
}


void Widget::keyPressEvent(QKeyEvent *event)
// ----------------------------------------------------------------------------
//   A key is pressed
// ----------------------------------------------------------------------------
{
    EventSave save(this->event, event);

    // Check if there is an activity that deals with it
    uint key = (uint) event->key();
    for (Activity *a = activities; a; a = a->Key(key, true)) ;

    // Forward it down the regular event chain
    forwardEvent(event);
}


void Widget::keyReleaseEvent(QKeyEvent *event)
// ----------------------------------------------------------------------------
//   A key is released
// ----------------------------------------------------------------------------
{
    EventSave save(this->event, event);

    // Check if there is an activity that deals with it
    uint key = (uint) event->key();
    for (Activity *a = activities; a; a = a->Key(key, false)) ;

    // Forward it down the regular event chain
    forwardEvent(event);
}

void Widget::mousePressEvent(QMouseEvent *event)
// ----------------------------------------------------------------------------
//   Mouse button click
// ----------------------------------------------------------------------------
{
    EventSave save(this->event, event);

    QMenu * contextMenu = NULL;
    uint    button      = (uint) event->button();
    int     x           = event->x();
    int     y           = event->y();

    // Create a selection if left click and nothing going on right now
    if (!activities && button == Qt::LeftButton)
        new Selection(this);

    // Send the click to all activities
    for (Activity *a = activities; a; a = a->Click(button, true, x, y)) ;

    if (button ==  Qt::RightButton)
    {
        switch (event->modifiers())
        {
        default :
        case Qt::NoModifier :
            {
                contextMenu = parent()->findChild<QMenu*>(CONTEXT_MENU);
                break;
            }
        case Qt::ShiftModifier :
            {
                contextMenu = parent()->findChild<QMenu*>(SHIFT_CONTEXT_MENU);
                break;
            }
        case Qt::ControlModifier :
            {
                contextMenu = parent()->findChild<QMenu*>(CONTROL_CONTEXT_MENU);
                break;
            }
        case Qt::AltModifier :
            {
                contextMenu = parent()->findChild<QMenu*>(ALT_CONTEXT_MENU);
                break;
            }
        case Qt::MetaModifier :
            {
                contextMenu = parent()->findChild<QMenu*>(META_CONTEXT_MENU);
                break;
            }
        }

        if (contextMenu)
            contextMenu->exec(event->globalPos());
    }

    // Pass the event down the event chain
    if (!contextMenu)
        forwardEvent(event);
}


void Widget::mouseReleaseEvent(QMouseEvent *event)
// ----------------------------------------------------------------------------
//   Mouse button is released
// ----------------------------------------------------------------------------
{
    EventSave save(this->event, event);

    uint button = (uint) event->button();
    int x = event->x();
    int y = event->y();

    // Check if there is an activity that deals with it
    for (Activity *a = activities; a; a = a->Click(button, false, x, y)) ;

    // Pass the event down the event chain
    forwardEvent(event);
}


void Widget::mouseMoveEvent(QMouseEvent *event)
// ----------------------------------------------------------------------------
//    Mouse move
// ----------------------------------------------------------------------------
{
    EventSave save(this->event, event);
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
    forwardEvent(event);
}


void Widget::wheelEvent(QWheelEvent *event)
// ----------------------------------------------------------------------------
//   Mouse wheel
// ----------------------------------------------------------------------------
{
    EventSave save(this->event, event);
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
//   XLR runtime entry points
//
// ============================================================================

#pragma GCC diagnostic ignored "-Wunused-parameter"

Widget *Widget::current = NULL;
typedef XL::Tree Tree;

Tree *Widget::status(Tree *self, text caption)
// ----------------------------------------------------------------------------
//   Set the status line of the window
// ----------------------------------------------------------------------------
{
    Window *window = (Window *) parentWidget();
    window->statusBar()->showMessage(QString::fromStdString(caption));
    return XL::xl_true;
}


Tree *Widget::rotate(Tree *self, double ra, double rx, double ry, double rz)
// ----------------------------------------------------------------------------
//    Rotation along an arbitrary axis
// ----------------------------------------------------------------------------
{
    glRotatef(ra, rx, ry, rz);
    return XL::xl_true;
}


Tree *Widget::translate(Tree *self, double rx, double ry, double rz)
// ----------------------------------------------------------------------------
//     Translation along three axes
// ----------------------------------------------------------------------------
{
    glTranslatef(rx, ry, rz);
    return XL::xl_true;
}


Tree *Widget::scale(Tree *self, double sx, double sy, double sz)
// ----------------------------------------------------------------------------
//     Scaling along three axes
// ----------------------------------------------------------------------------
{
    glScalef(sx, sy, sz);
    return XL::xl_true;
}


Tree *Widget::depthDelta(Tree *self, double dx)
// ----------------------------------------------------------------------------
//   Change the delta we use for the depth
// ----------------------------------------------------------------------------
{
    state.depthDelta = dx;
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
    timer.setSingleShot(true);
    timer.start(1000 * delay);
    return XL::xl_true;
}


Tree *Widget::locally(Tree *self, Tree *child)
// ----------------------------------------------------------------------------
//   Evaluate the child tree while preserving the OpenGL context
// ----------------------------------------------------------------------------
{
    GLAndWidgetKeeper save(this);
    Tree *result = xl_evaluate(child);
    return result;
}


Tree *Widget::pagesize(Tree *self, uint w, uint h)
// ----------------------------------------------------------------------------
//    Set the bit size for the page textures
// ----------------------------------------------------------------------------
{
    // Little practical point in ever creating textures bigger than viewport
    if (w > (uint) width())    w = width();
    if (h > (uint) height())   h = height();
    state.frameWidth = w;
    state.frameHeight = h;
    return XL::xl_true;
}


Tree *Widget::page(Tree *self, Tree *p)
// ----------------------------------------------------------------------------
//  Evaluate the tree in a frame with the given size
// ----------------------------------------------------------------------------
{
    uint w = state.frameWidth, h = state.frameHeight;
    Frame *cairo = self->GetInfo<Frame>();
    FrameInfo *frame = self->GetInfo<FrameInfo>();
    if (!frame)
    {
        frame = new FrameInfo(w,h);
        self->SetInfo<FrameInfo> (frame);
    }

    Tree *result = NULL;

    // Do to a bug in the NVIDIA kernel driver on MacOSX, we need to avoid the
    // following code when in GL_SELECT mode or die.
    if (!event)
    {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();

        frame->resize(w,h);

        frame->begin();
        {
            // Clear the background and setup initial state
            setup(w, h);

            if (!cairo)
            {
                cairo = new Frame;
                self->SetInfo<Frame>(cairo);
            }

            XL::LocalSave<QPaintDevice *> sv(state.paintDevice,
                                             frame->render_fbo);
            XL::LocalSave<Frame *> svc(this->frame, cairo);
            result = xl_evaluate(p);
        }
        cairo->Paint();
        frame->end();

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glPopAttrib();

        frame->bind();
    }

    return result;
}


XL::Integer *Widget::page_width(Tree *self)
// ----------------------------------------------------------------------------
//   Return the width of the page
// ----------------------------------------------------------------------------
{
    return new Integer(width());
}


XL::Integer *Widget::page_height(Tree *self)
// ----------------------------------------------------------------------------
//   Return the height of the page
// ----------------------------------------------------------------------------
{
    return new Integer(height());
}


Tree *Widget::time(Tree *self)
// ----------------------------------------------------------------------------
//   Return a fractional time, including milliseconds
// ----------------------------------------------------------------------------
{
    return new XL::Real(CurrentTime());
}


Tree *Widget::page_time(Tree *self)
// ----------------------------------------------------------------------------
//   Return a fractional time, including milliseconds
// ----------------------------------------------------------------------------
{
    return new XL::Real(CurrentTime() - page_start_time);
}


XL::Name *Widget::selectable(Tree *self, bool new_selectable)
// ----------------------------------------------------------------------------
//   Return current selectable state, and set new one
// ----------------------------------------------------------------------------
{
    Name *result = state.selectable ? XL::xl_true : XL::xl_false;
    state.selectable = new_selectable;
    return result;
}


XL::Name *Widget::selectorName(Tree *self, Text &name)
// ----------------------------------------------------------------------------
//   Assign the selector name
// ----------------------------------------------------------------------------
{
    if (!selector)
    {
        if (Tree *o = Ooops("Selector '$1' used outside of selection", &name))
            if (Name *n = o->AsName())
                return n;
        return XL::xl_false;
    }

    GLuint selectorId;
    if (selectors.count(name.value) == 0)
    {
        // Already identified, return existing selector ID
        selectorId = selectors[name.value];
    }
    else
    {
        // First time we see it, return a new selector ID
        selectorId = selectors.size();
        selectors[name.value] = selectorId;
        selectorNames.push_back(name.value);
    }

    // Load selector name on the OpenGL selection stack
    glLoadName(selectorId);

    return XL::xl_true;
}


GLuint Widget::shapeId()
// ----------------------------------------------------------------------------
//   Return an identifier for the shape in selections
// ----------------------------------------------------------------------------
//   We assign an identifier only if we are selectable and if we are not
//   rendering in an off-screen buffer of some sort
{
    return state.selectable && state.paintDevice == this && !selector
        ? ++id
        : 0;
}


bool Widget::selected()
// ----------------------------------------------------------------------------
//   Test if the current shape is selected
// ----------------------------------------------------------------------------
{
    return state.selectable && state.paintDevice == this
        ? selection.count(id) > 0
        : false;
}


void Widget::select()
// ----------------------------------------------------------------------------
//    Select the current shape if we are in selectable state
// ----------------------------------------------------------------------------
{
    if (state.selectable && state.paintDevice == this && !selector)
        selection.insert(id);
}


void Widget::requestFocus(QWidget *widget)
// ----------------------------------------------------------------------------
//   Some other widget request the focus
// ----------------------------------------------------------------------------
{
    if (!focusWidget)
    {
        focusWidget = widget;
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


Vector3 Widget::dragDelta()
// ----------------------------------------------------------------------------
//   Compute the drag delta based on the current Drag object if there is any
// ----------------------------------------------------------------------------
{
    Vector3 result;
    recordProjection();
    if (Drag *d = dynamic_cast<Drag *>(activities))
    {
        double x1 = d->x1;
        double y1 = d->y1;
        double x2 = d->x2;
        double y2 = d->y2;
        int hh = height();

        Point3 u1 = unproject(x1, hh-y1, 0);
        Point3 u2 = unproject(x2, hh-y2, 0);
        result = u2 - u1;

        // Clamp amplification resulting from reverse projection
        const double maxAmp = 5.0;
        double ampX = fabs(result.x) / (fabs(x2-x1) + 0.01);
        double ampY = fabs(result.y) / (fabs(y2-y1) + 0.01);
        if (ampX > maxAmp)
            result *= maxAmp/ampX;
        if (ampY > maxAmp)
            result *= maxAmp/ampY;
    }
    return result;
}


text Widget::dragSelector()
// ----------------------------------------------------------------------------
//   Return the name of the drag selector if there's any, or "none" otherwise
// ----------------------------------------------------------------------------
{
    if (activeSelector)
    {
        assert (activeSelector <= selectorNames.size());
        return selectorNames[activeSelector - 1];
    }
    return "none";
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

    XL::LocalSave<bool>   save_selectable(state.selectable, false);
    XL::LocalSave<GLuint> save_selector(selector, selectors["move"]);

    glPushName(selector);
    if (bounds.Depth() > 0)
    {
        GLAttribKeeper save;
        setupGL();
        glDisable(GL_DEPTH_TEST);
        (XL::XLCall("draw_" + selName), c.x, c.y, c.z, w, h, d) (symbols);
    }
    else
    {
        GLAttribKeeper save;
        glDisable(GL_DEPTH_TEST);
        (XL::XLCall("draw_" + selName), c.x, c.y, w, h) (symbols);
    }
    glPopName();
}


void Widget::loadName(bool load)
// ----------------------------------------------------------------------------
//   Load a name on the GL stack
// ----------------------------------------------------------------------------
{
    if (!load)
        depth += state.depthDelta;

    if (!selector)
    {
        if (state.selectable && load)
            glLoadName(shapeId());
        else
            glLoadName(0);
    }
}


Box3 Widget::bbox(coord x, coord y, coord w, coord h)
// ----------------------------------------------------------------------------
//   Return a selection bounding box for a flat shape
// ----------------------------------------------------------------------------
{
    return Box3(x-w/2, y-h/2, 0, w, h, 0);
}


Box3 Widget::bbox(coord x, coord y, coord z, coord w, coord h, coord d)
// ----------------------------------------------------------------------------
//   Return a selection bounding box for a 3D shape
// ----------------------------------------------------------------------------
{
    return Box3(x-w/2, y-h/2, z-d/2, w, h, d);
}


Tree *Widget::filled(Tree *self)
// ----------------------------------------------------------------------------
//   Select filled polygon mode
// ----------------------------------------------------------------------------
{
    state.filled = true;
    return XL::xl_true;
}


Tree *Widget::hollow(Tree *self)
// ----------------------------------------------------------------------------
//   Select hollow polygon mode
// ----------------------------------------------------------------------------
{
    state.filled = false;
    return XL::xl_true;
}


Tree *Widget::linewidth(Tree *self, double lw)
// ----------------------------------------------------------------------------
//    Select the line width for OpenGL
// ----------------------------------------------------------------------------
{
    glLineWidth(lw);
    frame->LineWidth(lw);
    return XL::xl_true;
}


Tree *Widget::color(Tree *self, double r, double g, double b, double a)
// ----------------------------------------------------------------------------
//    Set the RGBA color
// ----------------------------------------------------------------------------
{
    glColor4f(r,g,b,a);
    frame->Color(r,g,b,a);
    return XL::xl_true;
}


Tree *Widget::textColor(Tree *self,
                        double r, double g, double b, double a,
                        bool isFg)
// ----------------------------------------------------------------------------
//    Set the RGBA color
// ----------------------------------------------------------------------------
{
      // Set color for text layout
    const double amp=255.9;
    QColor qcolor(floor(amp*r),floor(amp*g),floor(amp*b),floor(amp*a));

    if (isFg)
    {
        state.charFormat.setForeground(qcolor);
    } else {
        state.charFormat.setBackground(qcolor);
    }

    // For Cairo
    GLStateKeeper save;
    frame->Color(r,g,b,a);

    return XL::xl_true;
}


Tree *Widget::evalInGlMode(GLenum mode, Tree *child)
// ----------------------------------------------------------------------------
//   Evaluate the child tree (typically, a list of vertexes) in given GL mode
// ----------------------------------------------------------------------------
{
    GLAndWidgetKeeper save(this);
    glBegin(mode);
    xl_evaluate(child);
    glEnd();
    return XL::xl_true;
}


Tree *Widget::polygon(Tree *self, Tree *child)
// ----------------------------------------------------------------------------
//   Evaluate the child tree within a polygon
// ----------------------------------------------------------------------------
{
    return evalInGlMode(state.filled ? GL_POLYGON : GL_LINE_LOOP, child);
}


Tree *Widget::points(Tree *self, Tree *child)
// ----------------------------------------------------------------------------
//   Evaluate the child tree in GL_POINTS mode
// ----------------------------------------------------------------------------
{
    return evalInGlMode(GL_POINTS, child);
}

Tree *Widget::lines(Tree *self, Tree *child)
// ----------------------------------------------------------------------------
//   Evaluate the child tree in GL_LINES mode
// ----------------------------------------------------------------------------
{
    return evalInGlMode(GL_LINES, child);
}


Tree *Widget::line_strip(Tree *self, Tree *child)
// ----------------------------------------------------------------------------
//   Evaluate the child tree in GL_LINE_STRIP mode
// ----------------------------------------------------------------------------
{
    return evalInGlMode(GL_LINE_STRIP, child);
}


Tree *Widget::line_loop(Tree *self, Tree *child)
// ----------------------------------------------------------------------------
//   Evaluate the child tree in GL_LINE_LOOP mode
// ----------------------------------------------------------------------------
{
    return evalInGlMode(GL_LINE_LOOP, child);
}


Tree *Widget::triangles(Tree *self, Tree *child)
// ----------------------------------------------------------------------------
//   Evaluate the child tree in GL_TRIANGLES mode
// ----------------------------------------------------------------------------
{
    return evalInGlMode(GL_TRIANGLES, child);
}


Tree *Widget::quads(Tree *self, Tree *child)
// ----------------------------------------------------------------------------
//   Evaluate the child tree in GL_QUADS mode
// ----------------------------------------------------------------------------
{
    return evalInGlMode(GL_QUADS, child);
}


Tree *Widget::quad_strip(Tree *self, Tree *child)
// ----------------------------------------------------------------------------
//   Evaluate the child tree in GL_QUAD_STRIP mode
// ----------------------------------------------------------------------------
{
    return evalInGlMode(GL_QUAD_STRIP, child);
}


Tree *Widget::triangle_fan(Tree *self, Tree *child)
// ----------------------------------------------------------------------------
//   Evaluate the child tree in GL_TRIANGLE_FAN mode
// ----------------------------------------------------------------------------
{
     return evalInGlMode(GL_TRIANGLE_FAN, child);
}


Tree *Widget::triangle_strip(Tree *self, Tree *child)
// ----------------------------------------------------------------------------
//   Evaluate the child tree in GL_TRIANGLE_STRIP mode
// ----------------------------------------------------------------------------
{
     return evalInGlMode(GL_TRIANGLE_STRIP, child);
}


Tree *Widget::vertex(Tree *self, double x, double y, double z)
// ----------------------------------------------------------------------------
//     GL vertex
// ----------------------------------------------------------------------------
{
    glVertex3f(x, y, z);
    return XL::xl_true;
}



// ============================================================================
//
//    Texture management
//
// ============================================================================

Tree *Widget::texture(Tree *self, text img)
// ----------------------------------------------------------------------------
//     Build a GL texture out of an image file
// ----------------------------------------------------------------------------
{
    if (img == "")
    {
        glDisable(GL_TEXTURE_2D);
        return XL::xl_true;
    }

    ImageTextureInfo *rinfo = self->GetInfo<ImageTextureInfo>();
    if (!rinfo)
    {
        rinfo = new ImageTextureInfo(this);
        self->SetInfo<ImageTextureInfo>(rinfo);
    }
    rinfo->bind(img);
    return XL::xl_true;
}


Tree *Widget::svg(Tree *self, text img)
// ----------------------------------------------------------------------------
//    Draw an image in SVG format
// ----------------------------------------------------------------------------
//    The image may be animated, in which case we will get repaintNeeded()
//    signals that we send to our 'draw()' so that we redraw as needed.
{
    SvgRendererInfo *rinfo = self->GetInfo<SvgRendererInfo>();
    if (!rinfo)
    {
        rinfo = new SvgRendererInfo(this);
        self->SetInfo<SvgRendererInfo>(rinfo);
    }
    rinfo->bind(img);
    return XL::xl_true;
}


Tree *Widget::texCoord(Tree *self, double x, double y)
// ----------------------------------------------------------------------------
//     GL texture coordinate
// ----------------------------------------------------------------------------
{
    glTexCoord2f(x, y);
    return XL::xl_true;
}


Tree *Widget::sphere(Tree *self,
                     real_r x, real_r y, real_r z, real_r r,
                     integer_r nslices, integer_r nstacks)
// ----------------------------------------------------------------------------
//     GL sphere
// ----------------------------------------------------------------------------
{
    ShapeName name(this, bbox(x,y,z, 2*r,2*r,2*r), "3D_selection");
    name.x(x).y(y).w(r).h(r).y(z, "depth");

    GLUquadric *q = gluNewQuadric();
    gluQuadricTexture (q, true);
    glPushMatrix();
    glTranslatef(x,y,z);
    glRotatef(-90.0, 1.0, 0.0, 0.0);
    gluSphere(q, r, nslices, nstacks);
    glPopMatrix();
    gluDeleteQuadric(q);
    return XL::xl_true;
}


void Widget::widgetVertex(double x, double y, double tx, double ty)
// ----------------------------------------------------------------------------
//   A vertex, including texture coordinate
// ----------------------------------------------------------------------------
{
    texCoord(NULL, tx, ty);
    vertex(NULL, x, y, depth);
}


void Widget::circularVertex(double cx, double cy, double r,
                            double x, double y,
                            double tx0, double ty0, double tx1, double ty1)
// ----------------------------------------------------------------------------
//   A circular vertex, including texture coordinate
// ----------------------------------------------------------------------------
//   x range between -1 and 1, y between -1 and 1
//   cx and cy are the center of the circle, r its radius
{
    widgetVertex(cx + r * x,
                 cy + r * y,
                 tx0 + ((x + 1.0) / 2.0 * (tx1 - tx0)),
                 ty0 + ((y + 1.0) / 2.0 * (ty1 - ty0)));
}


void Widget::circularSectorN(double cx, double cy, double r,
                            double tx0, double ty0, double tx1, double ty1,
                            int sq, int nq)
// ----------------------------------------------------------------------------
//     Draw a circular sector of N/4th of a circle
// ----------------------------------------------------------------------------
//   We use a reduced Bresenham-like algorithm for circles (midpoint circle)
//
//   For now the sector is limited to multiples of 1/4th of circle. For
//   example, an angle of 280 will draw 3/4 of a circle.
{
    // The two first values configure how precise the circle is
    int step = 10;                // Triangles generated every <step> points
    double grid = 1.0 / 500.0;    // Tolerance for points on the circle
    double error, x, y, s;


    for (int q = 0; q < nq; q++)
    {
    error = -1.0;
    x = 1.0;
    y = 0;
    s = step;

    while (x > 0)
    {
        if (++s >= step)
        {
        s = 0;
        switch ((sq + q) % 4)
        {
            case 3:
                circularVertex(cx, cy, r,  y, -x, tx0, ty0, tx1, ty1);
                break;
            case 2:
                circularVertex(cx, cy, r, -x, -y, tx0, ty0, tx1, ty1);
                break;
            case 1:
                circularVertex(cx, cy, r, -y,  x, tx0, ty0, tx1, ty1);
                break;
            case 0:
                circularVertex(cx, cy, r,  x,  y, tx0, ty0, tx1, ty1);
                break;
        }
        }
        if (error >= 0)
        {
            x -= grid;
            error -= x + x;
        }
        else
        {
            y += grid;
            error += y + y;
        }
    }
    }
    switch ((sq + nq) % 4)
    {
        case 3:
            circularVertex(cx, cy, r,  0, -1, tx0, ty0, tx1, ty1);
            break;
        case 2:
            circularVertex(cx, cy, r, -1,  0, tx0, ty0, tx1, ty1);
            break;
        case 1:
            circularVertex(cx, cy, r,  0,  1, tx0, ty0, tx1, ty1);
            break;
        case 0:
            circularVertex(cx, cy, r,  1,  0, tx0, ty0, tx1, ty1);
            break;
    }
}


Tree *Widget::glcircle(Tree *self, real_r cx, real_r cy, real_r r)
// ----------------------------------------------------------------------------
//     GL circle centered around (cx,cy), radius r
// ----------------------------------------------------------------------------
{
    ShapeName name(this, bbox(cx, cy, 2*r, 2*r));
    name.x(cx).y(cy).w(r).h(r);

    glBegin(state.filled ? GL_POLYGON : GL_LINE_LOOP);
    circularSectorN(cx, cy, r, 0, 0, 1, 1, 0, 4);
    glEnd();

    return XL::xl_true;
}


Tree *Widget::glcircularSector(Tree *self,
                             real_r cx, real_r cy, real_r r,
                             real_r a, real_r b)
// ----------------------------------------------------------------------------
//     GL circular sector centered around (cx,cy), radius r and two angles a, b
// ----------------------------------------------------------------------------
{
    ShapeName name(this, bbox(cx, cy, 2*r, 2*r));
    name.x(cx).y(cy).w(r).h(r);

    double db = b;
    double da = a;
    while (db < da)
    {
        db += 360;
    }
    int nq = int((db-da) / 90);                 // Number of quadrants to draw
    if (nq > 4)
    {
        nq = 4;
    }

    while (da < 0)
    {
        da += 360;
    }
    int sq = (int(da / 90) % 4);                // Starting quadrant

    glBegin(state.filled ? GL_POLYGON : GL_LINE_LOOP);
    circularVertex(cx, cy, r, 0, 0, 0, 0, 1, 1);    // The center
    circularSectorN(cx, cy, r, 0, 0, 1, 1, sq, nq);
    glEnd();

    return XL::xl_true;
 }



Tree *Widget::glroundedRectangle(Tree *self,
                               real_r cx, real_r cy,
                               real_r w, real_r h, real_r r)
// ----------------------------------------------------------------------------
//     GL rounded rectangle with radius r for the rounded corners
// ----------------------------------------------------------------------------
{
    ShapeName name(this, bbox(cx, cy, w, h));
    name.x(cx).y(cy).w(w).h(h);

    if (r <= 0) return glrectangle(self, cx, cy, w, h);
    if (r > w/2) r = w/2;
    if (r > h/2) r = h/2;

    double x0  = cx-w/2;
    double x0r = x0+r;
    double x1  = cx+w/2;
    double x1r = x1-r;

    double y0  = cy-h/2;
    double y0r = y0+r;
    double y1  = cy+h/2;
    double y1r = y1-r;

    double tx0  = 0;
    double tx0r = 0+r/w;
    double tx0d = 0+2*r/w;
    double tx1  = 1;
    double tx1r = 1-r/w;
    double tx1d = 1-2*r/w;

    double ty0  = 0;
    double ty0r = 0+r/h;
    double ty0d = 0+2*r/h;
    double ty1  = 1;
    double ty1r = 1-r/h;
    double ty1d = 1-2*r/h;

    glBegin(state.filled ? GL_POLYGON : GL_LINE_LOOP);
    {
        widgetVertex(x1, y1r, tx1, ty1r);

        circularSectorN(x1r, y1r, r, tx1d, ty1d, tx1, ty1, 0, 1);

        widgetVertex(x1r, y1, tx1r, ty1);
        widgetVertex(x0r, y1, tx0r, ty1);

        circularSectorN(x0r, y1r, r, tx0, ty1d, tx0d, ty1, 1, 1);

        widgetVertex(x0, y1r, tx0, ty1r);
        widgetVertex(x0, y0r, tx0, ty0r);

        circularSectorN(x0r, y0r, r, tx0, ty0, tx0d, ty0d, 2, 1);

        widgetVertex(x0r, y0, tx0r, ty0);
        widgetVertex(x1r, y0, tx1r, ty0);

        circularSectorN(x1r, y0r, r, tx1d, ty0, tx1, ty0d, 3, 1);

        widgetVertex(x1, y0r, tx1, ty0r);
    }
    glEnd();

    return XL::xl_true;
}


Tree *Widget::glrectangle(Tree *self, real_r cx, real_r cy, real_r w, real_r h)
// ----------------------------------------------------------------------------
//     GL rectangle centered around (cx,cy), width w, height h
// ----------------------------------------------------------------------------
{
    ShapeName name(this, bbox(cx, cy, w, h));
    name.x(cx).y(cy).w(w).h(h);

    glBegin(state.filled ? GL_POLYGON : GL_LINE_LOOP);
    {
        widgetVertex(cx-w/2, cy-h/2, 0, 0);
        widgetVertex(cx+w/2, cy-h/2, 1, 0);
        widgetVertex(cx+w/2, cy+h/2, 1, 1);
        widgetVertex(cx-w/2, cy+h/2, 0, 1);
    }
    glEnd();

    return XL::xl_true;
}


Tree *Widget::regularStarPolygon(Tree *self, real_r cx, real_r cy, real_r r,
                                 integer_r p, integer_r q)
// ----------------------------------------------------------------------------
//     GL regular p-side star polygon {p/q} centered around (cx,cy), radius r
// ----------------------------------------------------------------------------
{
    ShapeName name(this, bbox(cx, cy, 2*r, 2*r));
    name.x(cx).y(cy).w(r).h(r);

    if (p < 2 || q < 1 || q > (p-1)/2)
        return XL::xl_false;

    double R_r = cos( q*M_PI/p ) / cos( (q-1)*M_PI/p );
    double R = r * R_r;

    GLuint mode = state.filled ? GL_TRIANGLE_FAN : GL_LINE_LOOP;
    glBegin(mode);
    {
        if (state.filled)
            circularVertex(cx, cy, r, 0, 0, 0, 0, 1, 1);    // The center

        for (int i = 0; i < p; i++)
        {
            circularVertex(cx, cy, r,
                    cos( i * 2*M_PI/p + M_PI_2),
                    sin( i * 2*M_PI/p + M_PI_2),
                    0, 0, 1, 1);

            circularVertex(cx, cy, R,
                    cos( i * 2*M_PI/p + M_PI_2 + M_PI/p),
                    sin( i * 2*M_PI/p + M_PI_2 + M_PI/p),
                    (1-R_r)/2, (1-R_r)/2, (1+R_r)/2, (1+R_r)/2);
        }

        if (state.filled)
            circularVertex(cx, cy, r, 0, 1, 0, 0, 1, 1);    // Closing the star
    }
    glEnd();

    return XL::xl_true;
}


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


Tree *Widget::font(Tree *self, text description)
// ----------------------------------------------------------------------------
//   Select a font family
// ----------------------------------------------------------------------------
{
    QFont font = state.charFormat.font();
    font.fromString((QString::fromStdString(description)));
    state.charFormat.setFont(font);
    GLStateKeeper save;
    frame->Font(description);
    return XL::xl_true;
}


Tree *Widget::fontSize(Tree *self, double size)
// ----------------------------------------------------------------------------
//   Select a font size
// ----------------------------------------------------------------------------
{
    state.charFormat.setFontPointSize(size);
    GLStateKeeper save;
    frame->FontSize(size);
    return XL::xl_true;
}


Tree *Widget::fontPlain(Tree *self)
// ----------------------------------------------------------------------------
//   Select whether this is italic or not
// ----------------------------------------------------------------------------
{
    state.charFormat.setFontItalic(false);
    state.charFormat.setFontWeight(QFont::Normal);
    state.charFormat.setFontUnderline(false);
    state.charFormat.setFontOverline(false);
    state.charFormat.setFontStrikeOut(false);
    return XL::xl_true;
}


Tree *Widget::fontItalic(Tree *self, bool italic)
// ----------------------------------------------------------------------------
//   Select whether this is italic or not
// ----------------------------------------------------------------------------
{
    state.charFormat.setFontItalic(italic);
    return XL::xl_true;
}


Tree *Widget::fontBold(Tree *self, bool bold)
// ----------------------------------------------------------------------------
//   Select whether the font is bold or not
// ----------------------------------------------------------------------------
{
    state.charFormat.setFontWeight( bold ? QFont::Bold : QFont::Normal);
    return XL::xl_true;
}


Tree *Widget::fontUnderline(Tree *self, bool underline)
// ----------------------------------------------------------------------------
//    Select whether we underline a font
// ----------------------------------------------------------------------------
{
    state.charFormat.setFontUnderline(underline);
    return XL::xl_true;
}


Tree *Widget::fontOverline(Tree *self, bool overline)
// ----------------------------------------------------------------------------
//    Select whether we draw an overline
// ----------------------------------------------------------------------------
{
    state.charFormat.setFontOverline(overline);
    return XL::xl_true;
}


Tree *Widget::fontStrikeout(Tree *self, bool strikeout)
// ----------------------------------------------------------------------------
//    Select whether we strikeout a font
// ----------------------------------------------------------------------------
{
    state.charFormat.setFontStrikeOut(strikeout);
    return XL::xl_true;
}


Tree *Widget::fontStretch(Tree *self, int stretch)
// ----------------------------------------------------------------------------
//    Set font streching factor
// ----------------------------------------------------------------------------
{
    //state.charFormat.setFontStretch(stretch);
    return XL::xl_true;
}


Tree *Widget::align(Tree *self, int align)
// ----------------------------------------------------------------------------
//   Set text alignment
// ----------------------------------------------------------------------------
{
    Qt::Alignment old = state.flow->paragraphOption.alignment();
    if (align & Qt::AlignHorizontal_Mask)
        old &= ~Qt::AlignHorizontal_Mask;
    if (align & Qt::AlignVertical_Mask)
        old &= ~Qt::AlignVertical_Mask;
    align |= old;
    state.flow->paragraphOption.setAlignment(Qt::Alignment(align));
    return XL::xl_true;
}


Tree *Widget::textSpan(Tree *self, text content)
// ----------------------------------------------------------------------------
//   Insert a block of text with the current definition of font, color, ...
// ----------------------------------------------------------------------------
{
    state.flow->addText(QString::fromUtf8(content.c_str(), content.length()),
                        state.charFormat);
    return XL::xl_true;
}


Tree *Widget::flow(Tree *self)
// ----------------------------------------------------------------------------
//   Create a new text flow
// ----------------------------------------------------------------------------
{
    TextFlow *thisFlow = self->GetInfo<TextFlow>();
    if (!thisFlow)
    {
        thisFlow = new TextFlow(state.flow->paragraphOption);
        self->SetInfo<TextFlow> (thisFlow);
    }
    state.flow = thisFlow;

    return XL::xl_true;
}


Tree *Widget::frameTexture(Tree *self, double w, double h)
// ----------------------------------------------------------------------------
//   Make a texture out of the current text layout
// ----------------------------------------------------------------------------
{
    if (w < 16) w = 16;
    if (h < 16) h = 16;

    // Get or build the current frame if we don't have one
    FrameInfo *frame = self->GetInfo<FrameInfo>();
    if (!frame)
    {
        frame = new FrameInfo(w,h);
        self->SetInfo<FrameInfo> (frame);
    }

    if (1)
    {
        GLStateKeeper save;

        frame->resize(w,h);
        frame->begin();
        {
            // Clear the background and setup initial state
            setup(w, h);
            XL::LocalSave<QPaintDevice *> sv(state.paintDevice,
                                             frame->render_fbo);

            TextFlow *flow = state.flow;
            qreal lineY = 0, lineHeight = 0, topY = flow->topLineY;

            flow->setText(flow->completeText);
            flow->setAdditionalFormats(flow->formats);
            flow->setTextOption(flow->paragraphOption);
            flow->beginLayout();

            while(true)
            {
                // Create a new line
                QTextLine line = flow->createLine();
                if (!line.isValid())
                    break;
                line.setLineWidth(w);
                line.setPosition(QPoint(0, lineY - topY));
                lineHeight = line.height();
                lineY += lineHeight;
            }

            flow->topLineY = lineY;
            flow->endLayout();

            QPainter painter(state.paintDevice);
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
            painter.setRenderHint(QPainter::TextAntialiasing, true);
            flow->draw(&painter, QPoint(0,0));
            flow->clear();
        }
        frame->end();
    } // GLStateKeeper

    // Bind the resulting texture
    frame->bind();

    return XL::xl_true;
}


Tree *Widget::framePaint(Tree *self, real_r x, real_r y, real_r w, real_r h)
// ----------------------------------------------------------------------------
//   Draw a frame with the current text flow
// ----------------------------------------------------------------------------
{
    GLAttribKeeper save(GL_TEXTURE_BIT);
    frameTexture(self, w, h);

    // Draw a rectangle with the resulting texture
    ShapeName name(this, bbox(x, y, w, h));
    name.x(x).y(y).w(w).h(h);
    glBegin(GL_QUADS);
    {
        widgetVertex(x-w/2, y-h/2, 0, 0);
        widgetVertex(x+w/2, y-h/2, 1, 0);
        widgetVertex(x+w/2, y+h/2, 1, 1);
        widgetVertex(x-w/2, y+h/2, 0, 1);
    }
    glEnd();

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
        surface = new WebViewSurface(this);
        url->SetInfo<WebViewSurface> (surface);
    }

    // Resize to requested size, and bind texture
    surface->resize(w,h);
    surface->bind(url, progress);

    return XL::xl_true;
}


Tree *Widget::urlPaint(Tree *self,
                       real_r x, real_r y, real_r w, real_r h,
                       Text *url, Integer *progress)
// ----------------------------------------------------------------------------
//   Draw a URL in the curent frame
// ----------------------------------------------------------------------------
{
    GLAttribKeeper save(GL_TEXTURE_BIT);
    ShapeName name(this, bbox(x, y, 0, w, h, 0), "widget_selection");
    name.x(x).y(y).w(w).h(h);
    urlTexture(self, w, h, url, progress);

    // Draw a rectangle with the resulting texture
    glBegin(GL_QUADS);
    {
        widgetVertex(x-w/2, y-h/2, 0, 0);
        widgetVertex(x+w/2, y-h/2, 1, 0);
        widgetVertex(x+w/2, y+h/2, 1, 1);
        widgetVertex(x-w/2, y+h/2, 0, 1);
    }
    glEnd();

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
        surface = new LineEditSurface(this);
        txt->SetInfo<LineEditSurface> (surface);
    }

    // Resize to requested size, and bind texture
    surface->resize(w,h);
    surface->bind(txt);

    return XL::xl_true;
}


Tree *Widget::lineEdit(Tree *self,
                       real_r x, real_r y, real_r w, real_r h,
                       Text *txt)
// ----------------------------------------------------------------------------
//   Draw a line editor in the curent frame
// ----------------------------------------------------------------------------
{
    GLAttribKeeper save(GL_TEXTURE_BIT);
    ShapeName name(this, bbox(x,y,0,w,h,0), "widget_selection");
    name.x(x).y(y).w(w).h(h);

    lineEditTexture(self, w, h, txt);

    // Draw a rectangle with the resulting texture
    glBegin(GL_QUADS);
    {
        widgetVertex(x-w/2, y-h/2, 0, 0);
        widgetVertex(x+w/2, y-h/2, 1, 0);
        widgetVertex(x+w/2, y+h/2, 1, 1);
        widgetVertex(x-w/2, y+h/2, 0, 1);
    }
    glEnd();

    return XL::xl_true;
}


Tree *Widget::qtrectangle(Tree *self, real_r x, real_r y, real_r w, real_r h)
// ----------------------------------------------------------------------------
//    Draw a rectangle using the Qt primitive
// ----------------------------------------------------------------------------
{
    ShapeName name(this, bbox(x, y, w, h));
    name.x(x).y(y).w(w).h(h);

    QPainter painter(state.paintDevice);
    QPen pen(QColor(Qt::red));
    pen.setWidth(4);
    painter.setPen(pen);
    painter.drawRect(QRectF(x,y,w,h));

    return XL::xl_true;
}


Tree *Widget::qttext(Tree *self, double x, double y, text s)
// ----------------------------------------------------------------------------
//    Draw a text using the Qt text primitive
// ----------------------------------------------------------------------------
{
    QPainter painter(state.paintDevice);
    setAutoFillBackground(false);
    if (selected())
        painter.setPen(Qt::darkRed);
    else
        painter.setPen(Qt::darkBlue);

    QFont font("Arial");
    font.setPointSizeF(24);
    painter.setFont(font);
    painter.drawText(QPointF(x,y), Utf8(s));

    return XL::xl_true;
}


Tree *Widget::moveTo(Tree *self, double x, double y, bool isRelative)
// ----------------------------------------------------------------------------
//   Move to the given Cairo coordinates
// ----------------------------------------------------------------------------
{
    frame->MoveTo(x,y, isRelative);
    return XL::xl_true;
}


Tree *Widget::Ktext(Tree *self, text s)
// ----------------------------------------------------------------------------
//    Text at the current cursor position
// ----------------------------------------------------------------------------
{
    frame->Text(s);
    return XL::xl_true;
}


Tree *Widget::rectangle(Tree *self, real_r x, real_r y, real_r w, real_r h)
// ----------------------------------------------------------------------------
//    Draw a rectangle using Cairo
// ----------------------------------------------------------------------------
{
    ShapeName name(this, bbox(x,y,w,h));
    name.x(x).y(y).w(w).h(h);
    frame->Rectangle(x, y, w, h);
    return XL::xl_true;
}


Tree *Widget::stroke(Tree *self)
// ----------------------------------------------------------------------------
//    Stroke the current path
// ----------------------------------------------------------------------------
{
    frame->Stroke();
    return XL::xl_true;
}


Tree *Widget::clear(Tree *self)
// ----------------------------------------------------------------------------
//    Clear the current frame
// ----------------------------------------------------------------------------
{
    frame->Clear();
    return XL::xl_true;
}

#define K_STROKE 0x1
#define K_FILL   0x2
Tree *Widget::buildPath(Tree *self, Tree *path, int strokeOrFill)
// ----------------------------------------------------------------------------
//    Build and stroke and / or fill a path
// ----------------------------------------------------------------------------
{
    if (! path) return XL::xl_false;

    GLAttribKeeper save;

    frame->CleanPath();

    xl_evaluate(path);

    if (strokeOrFill & K_STROKE)
        frame->StrokePreserve();
    if (strokeOrFill & K_FILL)
        frame->FillPreserve();

 //   ShapeName name(this, frame->bbox());
  //  name.x(x).y(y).w(w).h(h);

    frame->bbox();
    frame->CleanPath();

    return XL::xl_true;
}

Tree *Widget::arc(Tree *self,
                   real_r x,
                   real_r y,
                   real_r r,
                   real_r a1,
                   real_r a2,
                   bool isPositive)
// ----------------------------------------------------------------------------
//    Add an arc to the current path
// ----------------------------------------------------------------------------
{
//    ShapeName name(this, bbox(x, y, 2*r, 2*r));
//    name.x(x).y(y).w(r).h(r);

    frame->Arc(x, y, r, a1, a2, isPositive);

    return XL::xl_true;
}

Tree *Widget::curveTo(Tree *self,
                      real_r x1, real_r y1,
                      real_r x2, real_r y2,
                      real_r x3, real_r y3, bool isRelative)
// ----------------------------------------------------------------------------
//    Add a curve to the current path
// ----------------------------------------------------------------------------
{
    frame->CurveTo(x1, y1, x2, y2, x3, y3, isRelative);

    return XL::xl_true;
}

Tree *Widget::lineTo(Tree *self,real_r x, real_r y, bool isRelative)
// ----------------------------------------------------------------------------
//    Add a line to the current path
// ----------------------------------------------------------------------------
{
    frame->LineTo(x, y, isRelative);
    return XL::xl_true;
}

Tree *Widget::closePath(Tree *self)
// ----------------------------------------------------------------------------
//    Close the current path
// ----------------------------------------------------------------------------
{
    frame->ClosePath();
    return XL::xl_true;
}


Tree *Widget::circle(Tree *self, real_r cx, real_r cy, real_r r)
// ----------------------------------------------------------------------------
//   Cairo circle centered around (cx,cy), radius r
// ----------------------------------------------------------------------------
{
    ShapeName name(this, bbox(cx, cy, 2*r, 2*r));
    name.x(cx).y(cy).w(r).h(r);

    GLAttribKeeper save;

    frame->Arc(cx, cy, r, 0, 2*M_PI, true);
    if (state.filled)
        frame->Fill();
    else
        frame->Stroke();
    return XL::xl_true;
}


Tree *Widget::circularSector(Tree *self,
                             real_r cx, real_r cy, real_r r,
                             real_r a, real_r b)
// ----------------------------------------------------------------------------
//   Cairo circular sector centered around (cx,cy), radius r and two angles a, b
// ----------------------------------------------------------------------------
{
    ShapeName name(this, bbox(cx, cy, 2*r, 2*r));
    name.x(cx).y(cy).w(r).h(r);

    GLAttribKeeper save;

    frame->MoveTo(cx, cy, false);
    frame->Arc(cx, cy, r, a, b, true);
    frame->ClosePath();
    if (state.filled)
        frame->Fill();
    else
        frame->Stroke();

    return XL::xl_true;
 }



Tree *Widget::roundedRectangle(Tree *self,
                               real_r cx, real_r cy,
                               real_r w, real_r h, real_r r)
// ----------------------------------------------------------------------------
//   Cairo rounded rectangle with radius r for the rounded corners
// ----------------------------------------------------------------------------
{
    ShapeName name(this, bbox(cx, cy, w, h));
    name.x(cx).y(cy).w(w).h(h);

    GLAttribKeeper save;

    if (r <= 0) return rectangle(self, cx, cy, w, h);
    if (r > w/2) r = w/2;
    if (r > h/2) r = h/2;

    double w0  = w-2*r;
    double h0  = h-2*r;
    double x0  = cx+r - w/2;
    double y0  = cy+r - h/2;

    frame->Arc(x0   , y0   , r, M_PI    , 3*M_PI_2, true);
    frame->Arc(x0+w0, y0   , r, 3*M_PI_2, 0       , true);
    frame->Arc(x0+w0, y0+h0, r, 0       , M_PI_2  , true);
    frame->Arc(x0   , y0+h0, r, M_PI_2  , M_PI    , true);
    frame->ClosePath();
    if (state.filled)
        frame->Fill();
    else
        frame->Stroke();

    return XL::xl_true;
}

// ============================================================================
//
//   Menu management
//
// ============================================================================
// * Menu name philosophy :
// * The full name is used to register menus and menu items against
//   the menubar.  Those names are not displayed.
// * Menu created by the XL programmer must be differentiated from the
//   originals ones because they have to be recreated or modified at
//   each loop of XL.  When top menus are deleted they recursively
//   delete their children (sub menus and menu items), so we have to
//   take care of sub menu at deletion time.
//   Regarding those constraints, main menus are prefixed with _TOP_MENU_,
//   sub menus are prefixed by _SUB_MENU_. Then each menu item and sub menu are
//   prefixed by the "current menu" name (this current menu may itself be a
//   submenu). Each part of the name are separated by a /.
//
// * Menu and menu items lifecycle : Menus are created when the xl
//   program is executed the first time.  Menus display text can be
//   modified at each execution.  Menus are destroyed when the xl
//   program is invalidated.  At save time, the old xl program is
//   invalidated and the new one is executed for the first time.
// ============================================================================

Tree *Widget::menuItem(Tree *self, text s, Tree *t)
// ----------------------------------------------------------------------------
//   Create a menu item
// ----------------------------------------------------------------------------
{
    if (!currentMenu)
        return XL::xl_false;

    QString fullName = currentMenu->objectName() +
                      "/" +
                      QString::fromStdString(s);

    if (parent()->findChild<QAction*>(fullName))
    {
        IFTRACE(menus)
        {
            std::cout<< "MenuItem " << s
                     << " found in current MenuBar with fullname "
                     << fullName.toStdString() << "\n";
            std::cout.flush();
        }
        return XL::xl_true;
    }

    // Get or build the current Menu if we don't have one
    MenuInfo *menuInfo = self->GetInfo<MenuInfo>();

    // Store a copy of the tree in the QAction.
    XL::TreeClone cloner;
    XL::Tree *copy = t->Do(cloner);
    QVariant var = QVariant::fromValue(TreeHolder(copy));

    if (menuInfo)
    {
        // The name of the menuItem has changed.
        IFTRACE(menus)
        {
            std::cout << "menuInfo found, old name is "
                      << menuInfo->fullName << " new name is "
                      << fullName.toStdString() << "\n";
            std::cout.flush();
        }
        menuInfo->action->setText(QString::fromStdString(s));
        menuInfo->action->setObjectName(fullName);
        menuInfo->action->setData(var);
        menuInfo->fullName = fullName.toStdString();
        return XL::xl_true;
    }

    menuInfo = new MenuInfo(currentMenu, fullName.toStdString());
    self->SetInfo<MenuInfo> (menuInfo);

    IFTRACE(menus)
    {
        std::cout << "menuItem creation with name "
                  << fullName.toStdString() << "\n";
        std::cout.flush();
    }

    QAction * p_action = currentMenu->addAction(QString::fromStdString(s));
    menuInfo->action = p_action;
    p_action->setData(var);
    p_action->setObjectName(fullName);

    return XL::xl_true;
}

Tree *Widget::menu(Tree *self, text s, bool isSubMenu)
// ----------------------------------------------------------------------------
// Add the menu to the current menu bar or create the contextual menu
// ----------------------------------------------------------------------------
{
    bool isContextMenu = false;

    // Build the full name of the menu
    // Uses the current menu name, the given string and the isSubmenu.
    QString fullname = QString::fromStdString(s);
    if (isSubMenu && currentMenu)
    {
        fullname.prepend(currentMenu->objectName() +'/');
        fullname.replace(TOPMENU, SUBMENU);

    }
    else if (fullname.startsWith(CONTEXT_MENU))
    {
        isContextMenu = true;
    }
    else
    {
        fullname.prepend( TOPMENU );
    }

    // Get or build the current Menu if we don't have one
    MenuInfo *menuInfo = self->GetInfo<MenuInfo>();


    // If the menu is registered, no need to recreate it.
    // This is used at reload time, recreate the MenuInfo if required.
    if (QMenu *tmp = parent()->findChild<QMenu*>(fullname))
    {
        currentMenu = tmp;
        if (!menuInfo)
        {
            menuInfo = new MenuInfo(isContextMenu ? NULL : currentMenuBar,
                                    currentMenu,
                                    fullname.toStdString());
            self->SetInfo<MenuInfo> (menuInfo);
            menuInfo->action = currentMenu->menuAction();

        }
        return XL::xl_true;
    }

    // The menu is not yet registered.
    // The name may have change but not the content
    // (in the loop of the XL program execution).
    if (menuInfo)
    {
        // The menu exists : update its info
        currentMenu = menuInfo->menu;
        menuInfo->action->setText(QString::fromStdString(s));
        menuInfo->menu->setObjectName(fullname);
        menuInfo->fullName = fullname.toStdString();
        return XL::xl_true;
    }

    // The menu is not existing. Creating it.
    if (isContextMenu)
    {
        currentMenu = new QMenu((Window*)parent());
        connect(currentMenu, SIGNAL(triggered(QAction*)),
                this,        SLOT(userMenu(QAction*)));

    }
    else if (isSubMenu)
        currentMenu = currentMenu->addMenu(QString::fromStdString(s));
    else
        currentMenu = currentMenuBar->addMenu(QString::fromStdString(s));

    currentMenu->setObjectName(fullname);

    menuInfo = new MenuInfo(isContextMenu ? NULL : currentMenuBar,
                            currentMenu,
                            fullname.toStdString());
    self->SetInfo<MenuInfo> (menuInfo);
    menuInfo->action = currentMenu->menuAction();

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
    return XL::xl_true;
}


XL::Name *Widget::deleteSelection(Tree *self)
// ----------------------------------------------------------------------------
//    Delete the selection
// ----------------------------------------------------------------------------
{
    return XL::xl_true;
}

TAO_END
