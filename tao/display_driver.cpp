// ****************************************************************************
//  display_driver.cpp                                             Tao project
// ****************************************************************************
//
//   File Description:
//
//    Implementation of the DisplayDriver class.
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
//  (C) 2011 Jerome Forissier <jerome@taodyne.com>
//  (C) 2011 Taodyne SAS
// ****************************************************************************

#include "display_driver.h"
#include "widget.h"
#include "tao_gl.h"
#include "tao_utf8.h"
#include "base.h"
#include "frame.h"  // TODO export to module API
#include <iostream>


namespace Tao {

// Static map of registered display functions and associated parameters
DisplayDriver::display_map DisplayDriver::map;


static void legacyDraw(void *)
// ----------------------------------------------------------------------------
//   Link to previous implementation of all display functions in Widget
// ----------------------------------------------------------------------------
//   Remove this when all supported formats are moved out of widget.cpp
{
    Widget::Tao()->legacyDraw();
}


DisplayDriver::DisplayDriver(Widget *widget)
        : widget(widget)
// ----------------------------------------------------------------------------
//   Constructor
// ----------------------------------------------------------------------------
{
    registerDisplayFunction("2D",                displayBackBuffer,
                                                 backBufferUse,
                                                 backBufferUnuse,
                                                 backBufferSetOpt);

    // Compatibility -- to be removed
    registerDisplayFunction("legacy",            legacyDraw);
    registerDisplayFunction("default",           legacyDraw);

    // Function to be used initially
    setDisplayFunction("default");
}


DisplayDriver::~DisplayDriver()
// ----------------------------------------------------------------------------
//   Destructor
// ----------------------------------------------------------------------------
{
    IFTRACE(displaymode)
        debug() << "Deactivating display function: " << +current.name
                << "@" << (void*)current.fn << "\n";

    if (current.unuse)
        current.unuse(current.obj);
}


void DisplayDriver::display()
// ----------------------------------------------------------------------------
//   Use currently active rendering function, or default, to draw scene
// ----------------------------------------------------------------------------
{
    Q_ASSERT(current.fn || !"No display function selected");
    return current.fn(current.obj);
}


bool DisplayDriver::setDisplayFunction(QString name)
// ----------------------------------------------------------------------------
//   Make a previously registered display function active
// ----------------------------------------------------------------------------
{
    bool found = false;
    if (name.isEmpty())
        name = "default";
    if (current.name == name)
        return true;
    if (map.contains(name))
    {
        if (current.unuse)
            current.unuse(current.obj);

        current = map[name];
        found = true;
        IFTRACE(displaymode)
            debug() << "Selecting display function: " << +name
                    << "@" << (void*)current.fn << "\n";
        if (current.use)
            current.obj = current.use();
    }
    return found;
}


bool DisplayDriver::setOption(std::string name, std::string val)
// ----------------------------------------------------------------------------
//   Pass option to display module
// ----------------------------------------------------------------------------
{
    bool ok = false;
    IFTRACE(displaymode)
        debug() << "Passing option to display function: " << +current.name
                << " \"" << name << "\"=\"" << val << "\"\n";
    if (current.setopt)
        ok = current.setopt(current.obj, name, val);
    if (!ok)
    {
        IFTRACE(displaymode)
            debug() << "Option not recognized or rejected\n";
    }
    return ok;
}


std::string DisplayDriver::getOption(std::string name)
// ----------------------------------------------------------------------------
//   Read option from display module
// ----------------------------------------------------------------------------
{
    std::string val;
    if (current.getopt)
        val = current.getopt(current.obj, name);
    IFTRACE(displaymode)
        debug() << "Read option from display function: " << +current.name
                << " \"" << name << "\"=\"" << val << "\"\n";
    return val;
}


std::ostream & DisplayDriver::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[Display Driver] ";
    return std::cerr;
}



// ============================================================================
//
//   Static display methods. To be later moved to modules, except default one.
//
// ============================================================================


void DisplayDriver::displayBackBuffer(void *obj)
// ----------------------------------------------------------------------------
//   Default, usual 2D rendering into OpenGL back buffer
// ----------------------------------------------------------------------------
{
    BackBufferParams * o = (BackBufferParams *)obj;
    Q_ASSERT(obj || !"Back buffer display routine received NULL object");

    int n = o->bogusQuadBuffer ? 2 : 1;

    for (int i = 1; i <= n; i++)
    {
        // Setup viewport
        int w = renderWidth();
        int h = renderHeight();
        glViewport(0, 0, w, h);

        // Setup projection and modelview matrices
        setProjectionMatrix(w, h);
        setModelViewMatrix();

        // Select draw buffer
        if (o->bogusQuadBuffer)
            glDrawBuffer(i == 1 ? GL_BACK_LEFT : GL_BACK_RIGHT);
        else
            glDrawBuffer(GL_BACK);

        // Clear color and depth information
        setGlClearColor();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set suitable GL parameters for drawing
        setupGl();

        // Draw scene, selection, and activities
        drawScene();
        drawSelection();
        drawActivities();
    }
}


void * DisplayDriver::backBufferUse()
// ----------------------------------------------------------------------------
//   2D back buffer display is about to be used: allocate context
// ----------------------------------------------------------------------------
{
    return new BackBufferParams;
}


void DisplayDriver::backBufferUnuse(void *obj)
// ----------------------------------------------------------------------------
//   Done with 2D back buffer display
// ----------------------------------------------------------------------------
{
    delete (BackBufferParams *)obj;
}


bool DisplayDriver::backBufferSetOpt(void * obj,
                                     std::string name,
                                     std::string val)
// ----------------------------------------------------------------------------
//   Parse module option
// ----------------------------------------------------------------------------
{
    bool ok = false;
    BackBufferParams * o = (BackBufferParams *)obj;
    if (name == "bogusquadbuffers")
    {
        ok = true;
        if (val == "1" || val == "on")
            o->bogusQuadBuffer = true;
        else
        if (val == "" || val == "0" || val == "off")
            o->bogusQuadBuffer = false;
        else
            ok = false;
    }
    return ok;
}



// ============================================================================
//
//   Static methods exported to module API
//
// ============================================================================


bool DisplayDriver::registerDisplayFunction(std::string name,
                                            ModuleApi::display_fn fn,
                                            ModuleApi::display_use_fn use,
                                            ModuleApi::display_unuse_fn unuse,
                                            ModuleApi::display_setopt_fn setopt,
                                            ModuleApi::display_getopt_fn getopt)
// ----------------------------------------------------------------------------
//   Add a display function to the list of known functions
// ----------------------------------------------------------------------------
{
    IFTRACE(displaymode)
        debug() << "Registering display function: " << name << "@"
                << (void*)fn << "\n";
    QString nam = +name;
    if (map.contains(nam))
    {
        IFTRACE(displaymode)
            debug() << "Error: name already in use\n";
        return false;
    }
    DisplayParams p(nam, fn, use, unuse, setopt, getopt);
    map[nam] = p;
    return true;
}


void DisplayDriver::drawScene()
// ----------------------------------------------------------------------------
//   Draw all objects in the scene
// ----------------------------------------------------------------------------
{
    Widget::Tao()->drawScene();
}


void DisplayDriver::drawSelection()
// ----------------------------------------------------------------------------
//   Draw selection items for all objects (selection boxes, manipulators)
// ----------------------------------------------------------------------------
{
    Widget::Tao()->drawSelection();
}


void DisplayDriver::drawActivities()
// ----------------------------------------------------------------------------
//   Draw chooser, selection rectangle
// ----------------------------------------------------------------------------
{
    Widget::Tao()->drawActivities();
}


void DisplayDriver::setGlClearColor()
// ----------------------------------------------------------------------------
//   Call glClearColor with the color specified in the widget
// ----------------------------------------------------------------------------
{
    Widget::Tao()->setGlClearColor();
}


void DisplayDriver::setupGl()
// ----------------------------------------------------------------------------
//   Set default GL parameters before drawing the scene
// ----------------------------------------------------------------------------
{
    Widget::Tao()->setupGL();
}


void DisplayDriver::showGlErrors()
// ----------------------------------------------------------------------------
//   Display all OpenGL errors in the error window
// ----------------------------------------------------------------------------
{
    Widget::Tao()->showGlErrors();
}


void DisplayDriver::getCamera(Point3 *pos, Point3 *target, Vector3 *up)
// ----------------------------------------------------------------------------
//   Get camera characteristics
// ----------------------------------------------------------------------------
{
    Widget::Tao()->getCamera(pos, target, up);
}


static inline int even(int x)
// ----------------------------------------------------------------------------
//   Largest even integer smaller or equal to x
// ----------------------------------------------------------------------------
{
    return (x - (int)fmod(x, 2));
}


int DisplayDriver::renderHeight()
// ----------------------------------------------------------------------------
//   Window height for rendering purposes
// ----------------------------------------------------------------------------
//   Dimension in pixels is rounded down to an even value to avoid display
//   artifacts as the window is resized (half pixels)
{
    int h = Widget::Tao()->height();
    return even(h);
}


int DisplayDriver::renderWidth()
// ----------------------------------------------------------------------------
//   Window width for rendering purposes
// ----------------------------------------------------------------------------
//   Dimension in pixels is rounded down to an even value to avoid display
//   artifacts as the window is resized (half pixels)
{
    int w = Widget::Tao()->width();
    return even(w);
}


double DisplayDriver::zNear()
// ----------------------------------------------------------------------------
//   Near plane
// ----------------------------------------------------------------------------
{
    return Widget::Tao()->zNear;
}


double DisplayDriver::zFar()
// ----------------------------------------------------------------------------
//   Far plane
// ----------------------------------------------------------------------------
{
    return Widget::Tao()->zFar;
}


double DisplayDriver::zoom()
// ----------------------------------------------------------------------------
//   Current zoom factor
// ----------------------------------------------------------------------------
{
    return Widget::Tao()->zoom;
}


double DisplayDriver::eyeSeparation()
// ----------------------------------------------------------------------------
//   Distance between eyes
// ----------------------------------------------------------------------------
{
    return Widget::Tao()->eyeDistance;
}


void DisplayDriver::setProjectionMatrix(int w, int h, int i, int numCameras)
// ----------------------------------------------------------------------------
//   Set frustum for the given camera
// ----------------------------------------------------------------------------
{
    // Read camera position
    Point3 cameraPosition;
    Point3 cameraTarget;
    Vector3 cameraUpVector;
    getCamera(&cameraPosition, &cameraTarget, &cameraUpVector);

    // Setup the projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    Vector3 toTarget = cameraTarget - cameraPosition;
    double distance = toTarget.Length();
    double nearRatio = zNear()/distance;
    double delta = stereoDelta(i, numCameras);
    double shift = -(eyeSeparation() * delta) * nearRatio;
    double f = 0.5 * nearRatio / zoom();
    glFrustum (-w*f + shift, w*f + shift, -h*f, h*f, zNear(), zFar());
}


void DisplayDriver::setModelViewMatrix(int i, int numCameras)
// ----------------------------------------------------------------------------
//   Set modelview matrix for the given camera
// ----------------------------------------------------------------------------
{
    // Read camera position
    Point3 cameraPosition;
    Point3 cameraTarget;
    Vector3 cameraUpVector;
    getCamera(&cameraPosition, &cameraTarget, &cameraUpVector);

    // Setup the model-view matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    double delta = stereoDelta(i, numCameras);
    double shift = eyeSeparation() * delta;

    gluLookAt(cameraPosition.x + shift, cameraPosition.y, cameraPosition.z,
              cameraTarget.x + shift, cameraTarget.y ,cameraTarget.z,
              cameraUpVector.x, cameraUpVector.y, cameraUpVector.z);
}


double DisplayDriver::stereoDelta(int i, int numCameras)
// ----------------------------------------------------------------------------
//  Compute x-axis shift for the given camera (multiples of eye distance)
// ----------------------------------------------------------------------------
//  Examples: with 3 cameras: -1, 0, 1
//            with 4 cameras: -1.5, -0.5, 0.5, 1.5
{
    // First camera is i=1
    i--;
    i = i - numCameras/2;
    double delta = i;
    if (numCameras % 2 == 1)
        delta -= 0.5;
    delta += 0.5;
    return delta;
}


}