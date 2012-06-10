#ifndef TAO_MODULE_API_H
#define TAO_MODULE_API_H
// ****************************************************************************
//  module_api.h                                                   Tao project
// ****************************************************************************
//
//   File Description:
//
//    Interface between the Tao runtime and native modules
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
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao/module_info.h"
#include "coords3d.h"
#include "matrix.h"
#include "info.h"

// ========================================================================
//
//   Interface versioning
//
// ========================================================================

// The Tao module loader uses a versioning system similar to libtool's:
// http://www.gnu.org/software/libtool/manual/html_node/Libtool-versioning.html
// The loader will refuse to load a module library that does not pass the
// version compatibility check.
//
// Here are the rules when changing these numbers:
//
// - Update the version information only immediately before a public release
// - [ANY CHANGE] If any interfaces have been added, removed, or changed
//   since the last update, increment current.
// - [COMPATIBLE CHANGE] If any interfaces have been added since the last
//   public release, then increment age.
// - [INCOMPATIBLE CHANGE] If any interfaces have been removed or changed
//   since the last public release, then set age to 0.

#define TAO_MODULE_API_CURRENT   24
#define TAO_MODULE_API_AGE       5

// ========================================================================
//
//   Tao runtime interface
//
// ========================================================================

namespace XL
{
    struct Real;
}

namespace Tao {

struct ModuleApi
// ------------------------------------------------------------------------
//   API exported by the Tao runtime for use by modules
// ------------------------------------------------------------------------
{
    typedef void (*render_fn)(void *arg);
    typedef void (*delete_fn)(void *arg);

    // Add a rendering callback to the current layout. Callback will be
    // invoked when it's time to draw the layout.
    // This function is typically used by XL primitives that need to draw
    // something using OpenGL calls.
    bool (*scheduleRender)(render_fn callback, void *arg);

    // Request that current layout be refreshed on specified event
    // If event_type is QEvent::Timer, next_refresh is used to specify when
    // the next refresh should occur:
    //   - if next_refresh is -1.0, refresh will occur after the
    // "default refresh" period (cf. builtin default_refresh). Unless the
    // default_refresh value is changed, this means: as soon as possible.
    //   - if next_refresh is not -1.0, it is the absolute date of the
    // next refresh. Use currentTime() if needed, for instance to request
    // a refresh in 5 seconds, set next_refresh = currentTime() + 5.0.
    // When event_type is not QEvent::Timer, next_refresh is ignored.
    bool (*refreshOn)(int event_type, double next_refresh);

    // Return the current time, in seconds. The returned value normally
    // has millisecond precision.
    double (*currentTime)();

    // Check if an OpenGL extension, given by its name, is avaible in Tao.
    bool (*isGLExtensionAvailable)(text name);

    // Like scheduleRender, but current layout takes ownership of arg:
    // when layout is destroyed, delete_fn is called with arg.
    bool (*addToLayout)(render_fn callback, void *arg, delete_fn del);

    // Like addToLayout, but uses an other callback to identify object
    // under cursor.
    bool (*AddToLayout2)(render_fn callback, render_fn identify,
                         void *arg, delete_fn del);

    // Show a control box to manipulate the object
    bool (*addControlBox)(XL::Real *x, XL::Real *y, XL::Real *z,
                          XL::Real *w, XL::Real *h, XL::Real *d);

    // Allow to enable specified texture coordinates before a drawing
    // according to current application parameters.
    // After drawing, texture coordinates have to be disable.
    bool (*EnableTexCoords)(double* texCoord);

    // Allow to disable texture coordinates after a drawing.
    bool (*DisableTexCoords)();

    // Get the last activated texture unit
    unsigned int (*TextureUnit)();

    // Get the bimasks of all activated texture units
    // in the current layout.
    unsigned int (*TextureUnits)();

    // Set the bimasks of all activated texture units in
    // the current layout.
    void (*SetTextureUnits)(uint64 textureUnits);

    // Check if a texture is bound at the specified unit
    bool (*HasTexture)(unsigned int unit);

    // Allow to bind a new texture in Tao thanks to its id and its type.
    // For a 2D texture, use BindTexture2D
    // Always returns false.
    // Note : Can not be call during drawing.
    bool (*BindTexture)(unsigned int id, unsigned int type);

    // Adds a "bind 2D texture" command to the current layout.
    // width and height are the dimensions of the texture in pixels.
    // Note : Can not be call during drawing.
    void (*BindTexture2D)(unsigned int id,
                          unsigned int width, unsigned int height);

    // Allow to apply current textures during a drawing.
    bool (*SetTextures)();

    // Allow to add a shader define by its id during a drawing.
    bool (*SetShader)(int id);

    // Allow to set fill color during a drawing according
    // to the current layout attributes.
    bool (*SetFillColor)();

    // Allow to set line color during a drawing according
    // to the current layout attributes.
    bool (*SetLineColor)();

    // Allow to enable or deactivate pixel blur
    // on textures of the current layout.
    // It corresponds to GL_LINEAR/GL_NEAREST parameters.
    bool (*HasPixelBlur)(bool enable);

    // Get the bimasks of all activated lights in the current layout.
    unsigned int (*EnabledLights)();

    // Get the current model matrix.
    // Note : Can not be call during drawing
    Matrix4 (*ModelMatrix)();

    // Mark object for deletion by the main thread.
    // All classes derived from XL::Info that perform OpenGL calls in their
    // destructor MUST use this function in their Delete() method, as follows:
    // class Foo : public XL::Info
    // {
    //      Foo() { }
    //     ~Foo() { /* Some OpenGL calls like glDeleteTextures... */ }
    //     virtual void Delete() { tao->deferredDelete(this); }
    // }
    // Otherwise, the XL garbage collector may run the destructor in its own
    // thread, possibly resulting in OpenGL thread conflicts.
    void (*deferredDelete)(XL::Info * obj);

    // Make the OpenGL context of the current Tao widget be the current context
    void (*makeGLContextCurrent)();

    // ------------------------------------------------------------------------
    //   API for display modules
    // ------------------------------------------------------------------------

    // In the following protoypes, obj is the value returned by the
    // display_use_fn function.
    typedef void   (*display_fn)(void *obj);
    typedef void * (*display_use_fn)();
    typedef void   (*display_unuse_fn)(void *obj);
    typedef bool   (*display_setopt_fn)(void *obj, std::string name,
                                        std::string val);
    typedef std::string
                   (*display_getopt_fn)(void *obj, std::string name);

    // Register a display function (if module is a display module).
    // Display can later be activated by primitive: display_function <name>
    // Returns true on success, false on error (e.g., name already used)
    //   - fn is called for each frame to be displayed
    //   - use is called once when fn is about to be used.
    //     Return (void *)(~0L) to indicated an error.
    //   - unuse is called when Tao stops using fn
    //   - setopt is called when Tao needs to set a display option
    //   - getopt is called when Tao needs to get a display option
    bool (*registerDisplayFunction)(std::string name, display_fn fn,
                                    display_use_fn use,
                                    display_unuse_fn unuse,
                                    display_setopt_fn setopt,
                                    display_getopt_fn getopt);

    // Create another name for an existing display function
    bool (*registerDisplayFunctionAlias)(std::string name,
                                         std::string existing);

    // Call glClearColor() with the color currently specified by the program.
    void (*setGlClearColor)();

    // Setup default OpenGL parameters before drawing.
    void (*setupGl)();

    // Display all pending OpenGL errors (if any), in the error window.
    void (*showGlErrors)();

    // Switch GL_STEREO on or off for the current OpenGL display context.
    // The current context becomes invalid and a new one is created with
    // or without stereo buffers.
    // Returns true on success.
    bool (*setStereo)(bool on);

    // setProjectionMatrix, setModelViewMatrix
    //
    // Helper functions to define the geometry for a given camera.
    // Cameras lie on a straight line (not on an arc) and are spaced evenly by
    // the eyeSeparation() distance. The center of the camera segment is the
    // 'pos' point returned by getCamera(pos, target, up). Cameras are globally
    // aimed at the 'target' point, but they are parallel -- there is no inward
    // rotation or vergence point.
    // This effectively implements the "asymmetric frustum parallel axis"
    // projection method, which is recommended for stereoscopic and
    // autostereoscopic setups.
    //
    // w and h are the width and height in pixels of the frustum
    // plane at the target point. numCameras is the total number of cameras,
    // i is the camera number for which the view is to be rendered (i must be
    // between 1 and numCameras).
    void (*setProjectionMatrix)(int w, int h, int i, int numCameras);
    void (*setModelViewMatrix)(int i, int numCameras);

    // Draw the current page.
    void (*drawScene)();

    // Draw object selection.
    void (*drawSelection)();

    // Draw activities (command menu, selection rectangle, display
    // statistics)
    void (*drawActivities)();

    // Get camera characteristics. pos, target, up and/or toScreen may be NULL.
    void (*getCamera)(Point3 *pos, Point3 *target, Vector3 *up,
                      double *toScreen);

    // The height, in pixels, of the image to be rendered.
    int  (*renderHeight)();

    // The width, in pixels, of the image to be rendered.
    int  (*renderWidth)();

    // The z coordinate of the near clipping plane.
    double (*zNear)();

    // The z coordinate of the far clipping plane.
    double (*zFar)();

    // The current zoom factor.
    double (*zoom)();

    // For stereoscopic or multi-view settings: the distance between
    // the left and the right eye (or camera).
    double (*eyeSeparation)();

    // Get current eye
    int (*getCurrentEye)();

    // Get total of eyes
    int (*getEyesNumber)();

    // Tell Tao if the current point of view is to be used for selection/
    // mouse activities. When rendering multiple views per frame, you
    // normally set this for only one view.
    // When this setting is true, the drawScene() call will unproject the
    // mouse coordinates into the 3D space. It is true by default.
    void   (*doMouseTracking)(bool on);

    // Override viewport setting during mouse tracking.
    // This viewport is used in drawScene() when doMouseTracking(true), and
    // only for the current frame.
    // The current GL viewport is used by default.
    void   (*setMouseTrackingViewport)(int x, int y, int w, int h);

    // Set the text to be used as a watermark for the current widget.
    // Creates a texture of (w x h) pixels and writes text, centered.
    void   (*setWatermarkText)(std::string t, int w, int h);

    // Draw a watermark on top of the current scene. GL depth test and depth
    // mask are disabled. The text is set by setWatermarkText().
    void   (*drawWatermark)();


    // ------------------------------------------------------------------------
    //   Rendering to framebuffer/texture
    // ------------------------------------------------------------------------

    typedef struct fbo_s fbo;

    // Create a new framebuffer object.
    // Default internal format is GL_RGBA8.
    ModuleApi::fbo *   (*newFrameBufferObject)(uint w, uint h);

    // Delete a framebuffer object.
    void               (*deleteFrameBufferObject)(ModuleApi::fbo * obj);

    // Resize a framebuffer object.
    void               (*resizeFrameBufferObject)(ModuleApi::fbo * obj,
                                                      uint w, uint h);

    // Make a framebuffer object the current rendering target.
    // After this call, OpenGL will draw into the framebuffer.
    void               (*bindFrameBufferObject)(ModuleApi::fbo * obj);

    // Switch OpenGL rendering back to the default rendering target.
    void               (*releaseFrameBufferObject)(ModuleApi::fbo * obj);

    // Make a framebuffer object available as a texture.
    // After this call, the texture is bound to target GL_TEXTURE_2D
    // and target is enabled. That is:
    //   glBindTexture(GL_TEXTURE_2D, id);
    //   glEnable(GL_TEXTURE_2D);
    unsigned int       (*frameBufferObjectToTexture)(ModuleApi::fbo * obj);

    // Make a framebuffer attachment available as a GL_TEXTURE_2d texture.
    // The attachment parameter must be one of the following:
    //   GL_COLOR_ATTACHMENT0: returned texture contains the FBO color buffer
    //   GL_DEPTH_ATTACHMENT: returned texture contains the FBO depth buffer
    // Note that contrary to frameBufferObjectToTexture above, the texture
    // is NOT bound nor enabled.
    unsigned int       (*frameBufferAttachmentToTexture)(ModuleApi::fbo * obj,
                                                         int attachment);

    // ------------------------------------------------------------------------
    //   Licence checking
    // ------------------------------------------------------------------------

    // Return true if a valid license is found for the requested feature name
    bool (*hasLicense)(std::string featureName);

    // Return true if a valid license is found for the requested feature name,
    // false otherwise. When no license is found, an information dialog
    // is shown. 'critical' changes the appearance of the dialog (icon...).
    // Set it to true if the feature won't work at all, or false if the module
    // will work in degraded mode.
    bool (*checkLicense)(std::string featureName, bool critical);

    // Blink if the application has been running longer that specified duration.
    // Return true if (current_time % (on + off)) <= on and the application was
    // started more that 'after' seconds ago, false otherwise.
    // Note: calls refreshOn to refresh automatically on next transition.
    bool (*blink)(double on, double off, double after);

    // Returns the number of seconds since the application was started.
    double (*taoRunTime)();

    // ------------------------------------------------------------------------
    //   Current document info
    // ------------------------------------------------------------------------

    // Return the full path (native format) to the current document folder
    std::string (*currentDocumentFolder)();

    // ------------------------------------------------------------------------
    //   Licence checking (continued)
    // ------------------------------------------------------------------------

    // If the current build of Tao is Impress, return true. If not Impress:
    // return true if a valid license is found for the requested feature name,
    // false otherwise. When no license is found, an information dialog
    // is shown.
    bool (*checkImpressOrLicense)(std::string featureName);

    // ------------------------------------------------------------------------
    //   Main window
    // ------------------------------------------------------------------------

    // Return the index of the screen that contains the largest part of the
    // main window (see QDesktopWidget::screenNumber()).
    int (*screenNumber)();

    // The main display area of the Tao Presentations window can handle
    // several widgets (see QStackedWidget).
    // A module can temporarily replace the default display widget with its
    // own using the following functions.
    // widget must be a pointer to a QWidget object.
    //    Add a QWidget to the list. Note: ownership is transferred.
    void (*addWidget)(void * widget);
    //    Remove a QWidget from the list (and take back ownership of the
    //    object). If widget is the current one, the default Tao widget becomes
    //    visible again.
    void (*removeWidget)(void * widget);
    //    Make a Qwidget the active one in the main Tao window. widget must
    //    be a pointer previously passed to addWidget, or NULL to
    //    revert to the default display.
    void (*setCurrentWidget)(void * widget);

    // ------------------------------------------------------------------------
    //   Rendering to framebuffer/texture
    // ------------------------------------------------------------------------

    // Create a new framebuffer object with a specified internal format.
    ModuleApi::fbo *   (*newFrameBufferObjectWithFormat)(uint w, uint h,
                                                         uint format);



    // Post a user event to the graphical widget. See also refreshOn().
    // Please allocate eventType with QEvent::registerEventType() to avoid
    // conflicts with other modules or the main application.
    void (*postEvent)(int eventType);
};

}

// ========================================================================
//
//   Module interface
//
// ========================================================================

namespace XL
{
    struct Context;
}

namespace Tao
{
    typedef int (*module_init_fn)   (const Tao::ModuleApi *,
                                     const Tao::ModuleInfo *);
    typedef int (*enter_symbols_fn) (XL::Context *);
    typedef int (*delete_symbols_fn)(XL::Context *);
    typedef int (*module_exit_fn)   ();
    typedef int (*module_preferences_fn) ();
}

extern "C"
// ------------------------------------------------------------------------
//   API exported by modules for use by the Tao runtime
// ------------------------------------------------------------------------
{
    // Called once immediately after the module library is loaded.
    // Return 0 on success.
    // [Optional]
    // [ModuleInfo is only valid during this call]
    int module_init(const Tao::ModuleApi *a, const Tao::ModuleInfo *m);

    // Predefined error codes for module_init
    enum init_error
    {
        no_error = 0,
        error_invalid_license
    };

    // Called when module is imported to let the module extend the XL symbol
    // table (for instance, add new XL commands) in a given context.
    // Return 0 on success.
    // [Optional]
    // [May be automatically generated by the tbl_gen script]
    int enter_symbols(XL::Context *c);

    // Called once before module is unloaded to let module remove its
    // primitives from the XL symbol table.
    // Return 0 on success.
    // [Optional]
    // [May be automatically generated by the tbl_gen script]
    int delete_symbols(XL::Context *c);

    // Called when the module library is about to be unloaded.
    // Return 0 on success.
    // [Optional]
    int module_exit();

    // Called when the user wants to access the module's
    // configuration/preference page.
    // This function may use the Qt GUI to show a configuration
    // dialog, and may save settings using the QSettings class.
    // Return 0 on success.
    // [Optional]
    int show_preferences();
}

#endif // TAO_MODULE_API_H
