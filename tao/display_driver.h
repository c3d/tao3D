#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H
// ****************************************************************************
//  display_driver.h                                               Tao project
// ****************************************************************************
//
//   File Description:
//
//    Wrapper class for the final rendering function(s) used by a widget to
//    display each frame.
//    This class contains a default implementation which produces a 2D
//    projection, as well as pointers to more sophisticated algorithms
//    typically implemented by modules, for instance to support stereoscopic
//    or multiple views displays.
//    This class also provides entry points for drawing routines in the module
//    API.
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2011 Jerome Forissier <jerome@taodyne.com>
//  (C) 2011 Taodyne SAS
// ****************************************************************************


//#include "tao/module_api.h"
#include "tao.h"
#include "display_driver.h"
#include "widget.h"
#include "tao/module_api.h"
#include "frame.h"
#include <QMap>

QT_BEGIN_NAMESPACE
class QGLShaderProgram;
QT_END_NAMESPACE

TAO_BEGIN

class DisplayDriver
// ----------------------------------------------------------------------------
//   Convert a 3D scene to a format suitable for the display hardware
// ----------------------------------------------------------------------------
{
public:

public:
    DisplayDriver(Widget *widget);
    ~DisplayDriver();

    // Methods used by Tao
    void                display();
    bool                setDisplayFunction(QString name);
    bool                setOption(std::string name, std::string val);
    std::string         getOption(std::string name);

public:
    // Methods exported by the module API for use by display modules
    // See module_api.h
    static bool         registerDisplayFunction(std::string name,
                                                ModuleApi::display_fn fn,
                                                ModuleApi::display_use_fn use = NULL,
                                                ModuleApi::display_unuse_fn unuse = NULL,
                                                ModuleApi::display_setopt_fn setopt = NULL,
                                                ModuleApi::display_getopt_fn getopt = NULL);
    static void         drawScene();
    static void         drawSelection();
    static void         drawActivities();
    static void         setGlClearColor();
    static void         setupGl();
    static void         showGlErrors();
    static void         getCamera(Point3 *pos, Point3 *target, Vector3 *up);
    static int          renderHeight();
    static int          renderWidth();
    static double       zNear();
    static double       zFar();
    static double       zoom();
    static double       eyeSeparation();
    static void         setProjectionMatrix(int w, int h,
                                            int i = 1, int numCameras = 1);
    static void         setModelViewMatrix (int i = 1, int numCameras = 1);

protected:
    static std::ostream & debug();

protected:

    // The default display is plain double-buffered OpenGL (2D)

    struct BackBufferParams
    {
        BackBufferParams() : bogusQuadBuffer(false) {}
        bool bogusQuadBuffer;
    };
    static void         displayBackBuffer(void *);
    static void *       backBufferUse();
    static void         backBufferUnuse(void *arg);
    static bool         backBufferSetOpt(void * obj,
                                         std::string name,
                                         std::string val);
    static bool         bogusQuadBuffer;

    static double       stereoDelta(int i, int numCameras);

protected:
    struct DisplayParams
    {
        DisplayParams()
            : name(), fn(NULL), use(NULL), unuse(NULL),
              setopt(NULL), getopt(NULL), obj(NULL) {}
        DisplayParams(QString name,
                      ModuleApi::display_fn fn,
                      ModuleApi::display_use_fn use = NULL,
                      ModuleApi::display_unuse_fn unuse = NULL,
                      ModuleApi::display_setopt_fn setopt = NULL,
                      ModuleApi::display_getopt_fn getopt = NULL,
                      void * obj = NULL)
            : name(name), fn(fn), use(use), unuse(unuse),
              setopt(setopt), getopt(getopt), obj(obj) {}

        QString                       name;
        ModuleApi::display_fn         fn;
        ModuleApi::display_use_fn     use;
        ModuleApi::display_unuse_fn   unuse;
        ModuleApi::display_setopt_fn  setopt;
        ModuleApi::display_getopt_fn  getopt;
        void *                        obj;
    };
    typedef QMap<QString, DisplayParams>  display_map;

protected:
    Widget *              widget;
    DisplayParams         current;

protected:
    static display_map    map;
};

TAO_END

#endif // DISPLAY_DRIVER_H
