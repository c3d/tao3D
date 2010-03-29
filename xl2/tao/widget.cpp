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
#include "treeholder.h"
#include "apply_changes.h"
#include "activity.h"
#include "selection.h"
#include "drag.h"
#include "manipulator.h"
#include "treeholder.h"
#include "menuinfo.h"
#include "repository.h"
#include "application.h"
#include "process.h"
#include "layout.h"
#include "page_layout.h"
#include "space_layout.h"
#include "shapes.h"
#include "text_drawing.h"
#include "shapes3d.h"
#include "path3d.h"
#include "attributes.h"
#include "transforms.h"

#include <QtGui/QImage>
#include <cmath>
#include <QFont>
#include <iostream>
#include <QVariant>
#include <QtWebKit>
#include <sys/time.h>
#include <sys/stat.h>

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
      xlProgram(sf),
      space(NULL), layout(NULL), path(NULL),
      activities(NULL),
      id(0), capacity(0), manipulator(0),
      event(NULL), focusWidget(NULL),
      currentMenu(NULL), currentMenuBar(NULL),
      whatsNew(""), reloadProgram(false),
      timer(this), idleTimer(this),
      pageStartTime(CurrentTime()),
      tmin(~0ULL), tmax(0), tsum(0), tcount(0),
      nextSave(now()), nextCommit(nextSave), nextSync(nextSave)
{
    // Make sure we don't fill background with crap
    setAutoFillBackground(false);

    // Make this the current context for OpenGL
    makeCurrent();

    // Create the main page we draw on
    space = new SpaceLayout(this);

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
    if (repo && saveDelay < 0)
    {
        XL::source_files::iterator it;
        for (it = xlr->files.begin(); it != xlr->files.end(); it++)
        {
            XL::SourceFile &sf = (*it).second;
            if (writeIfChanged(sf))
                savedSomething = true;
        }

        // Record when we will save file again
        nextSave = tick + xlr->options.save_interval * 1000;
    }

    // Check if there's something to commit
    longlong commitDelay = longlong (nextCommit - tick);
    if (savedSomething && commitDelay < 0)
    {
        // If we saved anything, then commit changes
        if (doCommit())
            savedSomething = false;
    }

    // Check if there's something to reload
    longlong syncDelay = longlong(nextSync - tick);
    if (syncDelay < 0)
    {
        refreshProgram();
        syncDelay = tick + xlr->options.sync_interval * 1000;
    }
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
        if (repo && repo->write(fname, sf.tree.tree))
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
    }
    return false;
}


bool Widget::doCommit()
// ----------------------------------------------------------------------------
//   Commit files previously written to repository and reset next commit time
// ----------------------------------------------------------------------------
{
    IFTRACE(filesync)
            std::cerr << "Commit: " << whatsNew << "\n";
    if (repository()->commit(whatsNew))
    {
        XL::Main *xlr = XL::MAIN;
        whatsNew = "";
        nextCommit = now() + xlr->options.commit_interval * 1000;
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


void Widget::draw()
// ----------------------------------------------------------------------------
//    Redraw the widget
// ----------------------------------------------------------------------------
{
    // Timing
    ulonglong t = now();
    event = NULL;

    // Setup the initial drawing environment
    double w = width(), h = height();
    setup(w, h);

    // Clear the background
    glClearColor (1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
        (XL::XLCall("draw_handle"), x, x).build(s);
        first = false;
    }

    // If there is a program, we need to run it
    runProgram();

    // Timing
    elapsed(t);

    // Render all activities, e.g. the selection rectangle
    SpaceLayout selectionSpace(this);
    XL::LocalSave<Layout *> saveLayout(layout, &selectionSpace);
    glDisable(GL_DEPTH_TEST);
    for (Activity *a = activities; a; a = a->Display()) ;
    selectionSpace.Draw(NULL);
}


void Widget::runProgram()
// ----------------------------------------------------------------------------
//   Run the current XL program
// ----------------------------------------------------------------------------
{
    // Reset the selection id for the various elements being drawn
    focusWidget = NULL;

    // Run the XL program associated with this widget
    current = this;
    QTextOption alignCenter(Qt::AlignCenter);
    space->Clear();
    XL::LocalSave<Layout *> saveLayout(layout, space);

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

    // After we are done, draw the space with all the drawings in it
    id = 0;
    space->Draw(NULL);
    id = 0;
    space->DrawSelection(NULL);

    // Once we are done, do a garbage collection
    XL::Context::context->CollectGarbage();

    // Remember how many elements are drawn on the page, plus arbitrary buffer
    if (id > capacity)
        capacity = id + 100;
    else if (id + 50 < capacity / 2)
        capacity = capacity / 2;
}


void Widget::identifySelection()
// ----------------------------------------------------------------------------
//   Draw the elements in global space for selection purpose
// ----------------------------------------------------------------------------
{
    id = 0;
    space->Identify(NULL);
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

    TreeHolder t = var.value<TreeHolder >();
    xl_evaluate(t.tree);        // Typically will insert something...
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


ulonglong Widget::elapsed(ulonglong since, bool stats, bool show)
// ----------------------------------------------------------------------------
//    Record how much time passed since last measurement
// ----------------------------------------------------------------------------
{
    ulonglong t = now() - since;
    if (t == 0)
        t = 1; // Because Windows lies

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



// ============================================================================
//
//    Selection management
//
// ============================================================================

uint Widget::selected()
// ----------------------------------------------------------------------------
//   Test if the current shape is selected
// ----------------------------------------------------------------------------
{
    return id && selection.count(id) > 0 ? selection[id] : 0;
}


void Widget::select(uint count)
// ----------------------------------------------------------------------------
//    Select the current shape if we are in selectable state
// ----------------------------------------------------------------------------
{
    selection[id] = count;
}


void Widget::requestFocus(QWidget *widget, coord x, coord y)
// ----------------------------------------------------------------------------
//   Some other widget request the focus
// ----------------------------------------------------------------------------
{
    if (!focusWidget)
    {
        GLMatrixKeeper saveGL;
        focusWidget = widget;
        glTranslatef(x, y, 0);
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

    try
    {
        XL::LocalSave<Layout *> saveLayout(layout, &selectionSpace);
        XL::LocalSave<GLuint>   saveId(id, ~0);
        GLAttribKeeper          saveGL;
        glDisable(GL_DEPTH_TEST);
        if (bounds.Depth() > 0)
            (XL::XLCall("draw_" + selName), c.x, c.y, c.z, w, h, d) (symbols);
        else
            (XL::XLCall("draw_" + selName), c.x, c.y, w, h) (symbols);
        selectionSpace.Draw(NULL);
    }
    catch(XL::Error &e)
    {
    }
    catch(...)
    {
    }
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
    try
    {
        XL::LocalSave<Layout *> saveLayout(layout, &selectionSpace);
        XL::LocalSave<GLuint>   saveId(id, ~0U);
        GLAttribKeeper          saveGL;
        glDisable(GL_DEPTH_TEST);
        (XL::XLCall("draw_" + handleName), p.x, p.y) (symbols);
        selectionSpace.Draw(NULL);
    }
    catch(XL::Error &e)
    {
    }
    catch(...)
    {
    }
}



// ============================================================================
//
//   XLR runtime entry points
//
// ============================================================================

#pragma GCC diagnostic ignored "-Wunused-parameter"

Widget *Widget::current = NULL;
typedef XL::Tree Tree;

XL::Integer *Widget::pageWidth(Tree *self)
// ----------------------------------------------------------------------------
//   Return the width of the page
// ----------------------------------------------------------------------------
{
    return new Integer(width());
}


XL::Integer *Widget::pageHeight(Tree *self)
// ----------------------------------------------------------------------------
//   Return the height of the page
// ----------------------------------------------------------------------------
{
    return new Integer(height());
}


XL::Real *Widget::frameWidth(Tree *self)
// ----------------------------------------------------------------------------
//   Return the width of the current layout frame
// ----------------------------------------------------------------------------
{
    return new Real(layout->Bounds().Width());
}


XL::Real *Widget::frameHeight(Tree *self)
// ----------------------------------------------------------------------------
//   Return the height of the current layout frame
// ----------------------------------------------------------------------------
{
    return new Real(layout->Bounds().Height());
}


XL::Real *Widget::frameDepth(Tree *self)
// ----------------------------------------------------------------------------
//   Return the depth of the current layout frame
// ----------------------------------------------------------------------------
{
    return new Real(layout->Bounds().Depth());
}


XL::Real *Widget::time(Tree *self)
// ----------------------------------------------------------------------------
//   Return a fractional time, including milliseconds
// ----------------------------------------------------------------------------
{
    return new XL::Real(CurrentTime());
}


XL::Real *Widget::pageTime(Tree *self)
// ----------------------------------------------------------------------------
//   Return a fractional time, including milliseconds
// ----------------------------------------------------------------------------
{
    return new XL::Real(CurrentTime() - pageStartTime);
}


Tree *Widget::locally(Tree *self, Tree *child)
// ----------------------------------------------------------------------------
//   Evaluate the child tree while preserving the OpenGL context
// ----------------------------------------------------------------------------
{
    XL::LocalSave<Layout *> save(layout, layout->NewChild());
    Tree *result = xl_evaluate(child);
    save.saved->Add(layout);
    return result;
}


Tree *Widget::rotate(Tree *self, double ra, double rx, double ry, double rz)
// ----------------------------------------------------------------------------
//    Rotation along an arbitrary axis
// ----------------------------------------------------------------------------
{
    layout->Add(new Rotation(ra, rx, ry, rz));
    return XL::xl_true;
}


Tree *Widget::translate(Tree *self, double rx, double ry, double rz)
// ----------------------------------------------------------------------------
//     Translation along three axes
// ----------------------------------------------------------------------------
{
    layout->Add(new Translation(rx, ry, rz));
    return XL::xl_true;
}


Tree *Widget::rescale(Tree *self, double sx, double sy, double sz)
// ----------------------------------------------------------------------------
//     Scaling along three axes
// ----------------------------------------------------------------------------
{
    layout->Add(new Scale(sx, sy, sz));
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

    XL::LocalSave<GraphicPath *> save(path, new GraphicPath);
    Tree *result = xl_evaluate(child);
    layout->Add(path);

    return result;
}


Tree *Widget::moveTo(Tree *self, real_r x, real_r y, real_r z)
// ----------------------------------------------------------------------------
//    Add a 'moveTo' to the current path
// ----------------------------------------------------------------------------
{
    if (path)
        path->moveTo(Point3(x,y,z));
    else
        layout->Add(new MoveTo(x, y, z));
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
    return XL::xl_true;
}


Tree *Widget::moveToRel(Tree *self, real_r x, real_r y, real_r z)
// ----------------------------------------------------------------------------
//    Add a relative moveTo
// ----------------------------------------------------------------------------
{
    if (path)
        path->moveTo(Vector3(x,y,z));
    else
        layout->Add(new MoveToRel(x, y, z));
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
//    Draw a rectangle using Cairo
// ----------------------------------------------------------------------------
{
    Rectangle shape(Box(x-w/2, y-h/2, w, h));
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new ControlRectangle(x, y, w, h, new Rectangle(shape)));

    return XL::xl_true;
}


Tree *Widget::ellipse(Tree *self, real_r cx, real_r cy, real_r w, real_r h)
// ----------------------------------------------------------------------------
//   Cairo circle centered around (cx,cy), radius r
// ----------------------------------------------------------------------------
{
    Ellipse shape(Box(cx-w/2, cy-h/2, w, h));
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new ControlRectangle(cx, cy, w, h, new Ellipse(shape)));

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
        layout->Add(new ControlRectangle(cx, cy, w, h, new EllipseArc(shape)));

    return XL::xl_true;
 }



Tree *Widget::roundedRectangle(Tree *self,
                               real_r cx, real_r cy,
                               real_r w, real_r h, real_r rx, real_r ry)
// ----------------------------------------------------------------------------
//   Cairo rounded rectangle with radius r for the rounded corners
// ----------------------------------------------------------------------------
{
    RoundedRectangle shape(Box(cx-w/2, cy-h/2, w, h), rx, ry);
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new ControlRectangle(cx, cy, w, h,
                                         new RoundedRectangle(shape)));

    return XL::xl_true;
}



Tree *Widget::starPolygon(Tree *self,
                          real_r cx, real_r cy, real_r w, real_r h,
                          integer_r p, integer_r q)
// ----------------------------------------------------------------------------
//     GL regular p-side star polygon {p/q} centered around (cx,cy), radius r
// ----------------------------------------------------------------------------
{
    if (p < 2 || q == 0 || q > (p-1)/2 || q < -(p-1)/2)
        return ellipse(self, cx, cy, w, h); // Show something else in its place

    StarPolygon shape(Box(cx-w/2, cy-h/2, w, h), p, q);
    if (path)
        shape.Draw(*path);
    else
        layout->Add(new ControlRectangle(cx, cy, w, h,
                                         new StarPolygon(shape)));

    return XL::xl_true;
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
    layout->Add (new Sphere(Box3(x-w/2, y-h/2, z-d/2, w,h,d), slices, stacks));
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
    return XL::xl_true;
}



// ============================================================================
//
//    Text and font
//
// ============================================================================

Tree *Widget::textSpan(Tree *self, text_r content)
// ----------------------------------------------------------------------------
//   Insert a block of text with the current definition of font, color, ...
// ----------------------------------------------------------------------------
{
    layout->Add(new TextSpan(+content, layout->font));
    return XL::xl_true;
}


Tree *Widget::font(Tree *self, text description)
// ----------------------------------------------------------------------------
//   Select a font family
// ----------------------------------------------------------------------------
{
    layout->font.fromString(+description);
    return XL::xl_true;
}


Tree *Widget::fontSize(Tree *self, double size)
// ----------------------------------------------------------------------------
//   Select a font size
// ----------------------------------------------------------------------------
{
    layout->font.setPointSizeF(size);
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
    return XL::xl_true;
}


Tree *Widget::fontUnderline(Tree *self, scale amount)
// ----------------------------------------------------------------------------
//    Select whether we underline a font
// ----------------------------------------------------------------------------
//    Qt doesn't support setting the size of the underline, it's on or off
{
    layout->font.setUnderline(bool(amount));
    return XL::xl_true;
}


Tree *Widget::fontOverline(Tree *self, scale amount)
// ----------------------------------------------------------------------------
//    Select whether we draw an overline
// ----------------------------------------------------------------------------
//    Qt doesn't support setting the size of the overline, it's on or off
{
    layout->font.setOverline(bool(amount));
    return XL::xl_true;
}


Tree *Widget::fontStrikeout(Tree *self, scale amount)
// ----------------------------------------------------------------------------
//    Select whether we strikeout a font
// ----------------------------------------------------------------------------
//    Qt doesn't support setting the size of the strikeout, it's on or off
{
    layout->font.setStrikeOut(bool(amount));
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
    window->statusBar()->showMessage(QString::fromStdString(caption));
    return XL::xl_true;
}


Tree *Widget::framePaint(Tree *self,
                         real_r x, real_r y, real_r w, real_r h,
                         Tree *prog)
// ----------------------------------------------------------------------------
//   Draw a frame with the current text flow
// ----------------------------------------------------------------------------
{
    XL::LocalSave<Layout *> saveLayout(layout, layout->NewChild());
    Tree *result = frameTexture(self, w, h, prog);

    // Draw a rectangle with the resulting texture
    layout->Add(new FrameManipulator(x, y, w, h,
                                     new Rectangle(Box(x-w/2, y-h/2, w, h))));
    saveLayout.saved->Add(layout);
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

        frame->resize(w,h);
        frame->begin();
        { 
           // Clear the background and setup initial state
            setup(w, h);
            try
            {
                result = xl_evaluate(prog);
            }
            catch(...)
            {
                std::cerr << "Error evaluating frame program\n";
            }
        }
        layout->Draw(NULL);

        frame->end();
    } while (0); // State keeper and layout

    // Bind the resulting texture
    GLuint tex = frame->bind();
    layout->Add(new FillTexture(tex));

    return result;
}


Tree *Widget::urlPaint(Tree *self,
                       real_r x, real_r y, real_r w, real_r h,
                       Text *url, Integer *progress)
// ----------------------------------------------------------------------------
//   Draw a URL in the curent frame
// ----------------------------------------------------------------------------
{
    XL::LocalSave<Layout *> saveLayout(layout, layout->NewChild());
    urlTexture(self, w, h, url, progress);
    WidgetSurface *surface = url->GetInfo<WidgetSurface>();
    layout->Add(new WidgetManipulator(x, y, w, h, surface));
    saveLayout.saved->Add(layout);
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
    GLuint tex = surface->bind(url, progress);
    layout->Add(new FillTexture(tex));

    return XL::xl_true;
}


Tree *Widget::lineEdit(Tree *self,
                       real_r x, real_r y, real_r w, real_r h,
                       Text *txt)
// ----------------------------------------------------------------------------
//   Draw a line editor in the curent frame
// ----------------------------------------------------------------------------
{
    XL::LocalSave<Layout *> saveLayout(layout, layout->NewChild());

    lineEditTexture(self, w, h, txt);
    WidgetSurface *surface = txt->GetInfo<WidgetSurface>();
    layout->Add(new WidgetManipulator(x, y, w, h, surface));
    saveLayout.saved->Add(layout);
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
    GLuint tex = surface->bind(txt);
    layout->Add(new FillTexture(tex));

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
