#ifndef GRAPHIC_STATE_H
#define GRAPHIC_STATE_H
// ****************************************************************************
//  graphic_state.h                                                Tao project
// ****************************************************************************
//
//   File Description:
//
//    Graphic state interface for native modules
//
//
//
//
//
//
//
// ****************************************************************************
// This file may be used in accordance with the terms and conditions contained
// in the Tao Presentations license agreement, or alternatively, in a signed
// license agreement between you and Taodyne SAS.
//  (C) 2012 Baptiste Soulisse <soulisse.baptiste@taodyne.com>
//  (C) 2012 Taodyne SAS
// ****************************************************************************

/*

1. What is the purpose of this class?

The main goal of GraphicState is to define some wrappers
for the more usal GL and GLU functions in order to prevent redundant
OpenGL state changes and improve performances in Tao Presentations.

In a more technical way, it defines three differents states:
    - The next state: It contains all changes requested by users
    when calling graphic state functions (GL.X).

    - The current state: It contains a cache of some graphical settings.
    When synchronising the graphic state, we compare the next state with this cache,
    and if some settings are different, then only at this moment we send it
    to the graphic card. By this way, we prevent redundant changes and
    improve strongly perfomances.

    - The classic OpenGL state: This is the real state in the graphic card.

2. How can it be used in modules?

To use the optimised graphic state in a module, it is necessary add
the following code in the corresponding .cpp file in order to define
the GraphicState pointer:

    #include <tao/module_api.h>
    #include <tao/graphic_state.h>
    extern "C" DLL_PUBLIC Tao::GraphicState * graphic_state = NULL;
    #define GL (*graphic_state)

Then just use e.g., GL.Viewport() to get the optimized version
of glViewport().

To prevent display problem, you can use GraphicSave to save/restore
to a correct GL state. It is the equivalent to glPush/glPop routines.

e.g:
   GraphicSave* save = GL.Save();
   GL.Viewport()
   GL.Restore(save);

By this way, the viewport is restored to its previous value.


3. Can we used direct OpenGL calls ? Under which conditions?

Of course, direct OpenGl calls can always be used in modules, even with some
function of the optimised GraphicState but under some conditions:

    3.1 Case of direct OpenGL calls only

The only and important condition to use direct OpenGL calls in a module is
to assure to restore the correct OpenGL state when we exit the module.
A good way is for instance to use direct OpenGL calls between glPush/glPop routines.
This condition is fundamental in order to avoid conflict with other modules.

e.g:
     glPushAttrib();           // Save OpenGL attributes
     glColor(0.0, 1.0, 0.0);   // Change color using direct OpenGL calls
     glDrawArray()             // Draw shape
     glPopAttrib();            // Restore color to its previous value (red)

    3.2 Mix of GraphicState and direct OpenGL calls

To manage this mix, it is necessary to synchronise GraphicState before
using direct OpenGL calls to assure to have the correct state.
To make this synchronisation, just call GL.Sync().

e.g:
    GL.Color()       // Change color
    GL.BindTexture() // Bind texture
    GL.Sync()        // Synchronise GraphicState
    glDrawArrays()   // Draw using direct OpenGL calls

Moreovever, as explained in 3.1, it is fundamental to save/restore
the correct state when using direct OpenGL calls to prevent conflicts
with the optimised graphic state (thanks to glPush/glPop routines for instance).

wrong code:
     GL.Color(1.0, 0.0, 0.0);       // Change color to red in GraphicState
     GL.Sync();                     // Synchronise GraphicState
     glColor(0.0, 1.0, 0.0);        // Change red color to green one
     GL.Color(1.0, 0.0, 0.0);       // Color will be still green as the current
                                    // one in GraphicState is red...

good code:
     GL.Color(1.0, 0.0, 0.0);       // Change color to red in GraphicState
     GL.Sync();                     // Synchronise GraphicState
     glPushAttrib();                // Save OpenGL attributes
     glColor(0.0, 1.0, 0.0);        // Change color using direct OpenGL calls
     glPopAttrib();                 // Restore color to its previous value (red)
     GL.Color(1.0, 0.0, 0.0);       // Don't do anything as the color is already
                                    // red in GraphicState (and in OpenGL).

other good code:
     GL.Color(1.0, 0.0, 0.0);       // Change color to red in GraphicState
     GL.Sync();                     // Synchronise GraphicState
     glPushAttrib();                // Save OpenGL attributes
     glColor(0.0, 1.0, 0.0);        // Change color using direct OpenGL calls
     GraphicSave* save = GL.Save(); // Save current graphic state
     GL.Color();                    // Change color in graphic state
     GL.Sync();                     // Synchronise graphic state
     GL.Restore(save);              // Restore color in graphic state
     GL.Sync();                     // Resynchronise graphic state to assure a consistent with real OpenGL state
     glPopAttrib();                 // Restore OpenGL color to its previous value (red)
     GL.Color(1.0, 0.0, 0.0);       // Don't do anything as the color is already
                                    // red in GraphicState (and in OpenGL).


4. When do I have to synchronise GraphicState?

Excepted for the case mentionned above (3.2), it is not necessary to
call GL.Sync() each time you use GraphicState given that drawing wrappers
already made it implicitly. Just use GraphicState functions as if you
would used OpenGL ones.

*/

#include "coords3d.h"
#include "matrix.h"
#include "tao_gl.h"

TAO_BEGIN


struct GraphicSave
// ----------------------------------------------------------------------------
//   Used to save/restore a graphic state
// ----------------------------------------------------------------------------
{
    GraphicSave()               {}
    virtual ~GraphicSave()      {}
};


struct GraphicState
// ----------------------------------------------------------------------------
//   Interface to manage graphic states
// ----------------------------------------------------------------------------
{
    GraphicState()              {}
    virtual ~GraphicState()     {}


    // Saving and restoring graphic state
    virtual GraphicSave *       Save() = 0;
    virtual void                Restore(GraphicSave *saved) = 0;

    // Apply pending state changes
    virtual void                Sync(uint64 which = ~0ULL) = 0;
    virtual void                Invalidate(uint64 which = ~0ULL) = 0;


    // ========================================================================
    //
    //   Useful functions (not defined in OpenGL specs)
    //
    // ========================================================================

    virtual text   Vendor() = 0;                                             // Return graphic card vendor
    virtual text   Renderer() = 0;                                           // Return current OpenGL renderer
    virtual text   Version() = 0;                                            // Return current OpenGL version
    virtual coord* ModelViewMatrix() = 0;                                    // Return current modelview matrix
    virtual coord* ProjectionMatrix() = 0;                                   // Return current projection matrix
    virtual int*   Viewport() = 0;                                           // Return current viewport
    virtual uint   MaxTextureCoords() = 0;                                   // Return the maximum number of texture coordinates
    virtual uint   MaxTextureUnits() = 0;                                    // Return the maximum number of texture units
    virtual uint   MaxTextureSize() = 0;                                     // Return the maximum size of textures
    virtual void   TextureSize(uint width, uint height, uint depth = 0) = 0; // Set size of current active texture
    virtual uint   TextureWidth() = 0;                                       // Return width of current active texture
    virtual uint   TextureHeight() = 0;                                      // Return height of current active texture
    virtual uint   TextureDepth() = 0;                                       // Return depth of current active texture
    virtual uint   TextureType() = 0;                                        // Return type of current active texture
    virtual uint   TextureMode() = 0;                                        // Return mode of current active texture
    virtual uint   TextureID() = 0;                                          // Return id of current active texture
    virtual void   ActivateTextureUnits(uint64 mask) = 0;                    // Activate texture units marked in the mask
    virtual uint   ActiveTextureUnitIndex() = 0;                             // Return the ID of the current active texture
    virtual uint   ActiveTextureUnitsCount() = 0;                            // Return the number of bound and active textures
    virtual uint64 ActiveTextureUnits() = 0;                                 // Return the mask of active texture units
    virtual void   HasPixelBlur(bool enable) = 0;                            // Enable or disable pixel blur on textures
    virtual uint64 LightsMask() = 0;                                         // Return the mask of enabled lights



    // ========================================================================
    //
    //   OpenGL/GLU wrappers (use GL.X instead of glX/gluX)
    //
    // ========================================================================


    // Get graphic attributes
    virtual void   Get(GLenum pname, GLboolean * params) = 0;
    virtual void   Get(GLenum pname, GLfloat * params) = 0;
    virtual void   Get(GLenum pname, GLint * params) = 0;

    // Matrix management
    virtual void   MatrixMode(GLenum mode) = 0;
    virtual void   LoadMatrix() = 0;
    virtual void   LoadIdentity() = 0;

    // Transformations
    virtual void Translate(double x, double y, double z) = 0;
    virtual void Rotate(double a, double x, double y, double z)  = 0;
    virtual void Scale(double x, double y, double z) = 0;

    // Camera management
    virtual bool Project(coord objx, coord objy, coord objz,
                         const coord* mv, const coord* proj,
                         const int* viewport,
                         coord *winx, coord *winy, coord *winz) = 0;
    virtual bool UnProject(coord winx, coord winy, coord winz,
                           const coord* mv, const coord* proj,
                           const int* viewport,
                           coord *objx, coord *objy, coord *objz) = 0;
    virtual void PickMatrix(float x, float y, float width, float height,
                            int viewport[4]) = 0;
    virtual void Frustum(float left, float right, float bottom, float top,
                         float nearZ, float farZ) = 0;
    virtual void Perspective(float fovy, float aspect, float nearZ, float farZ) = 0;
    virtual void Ortho(float left, float right, float bottom,
                       float top, float nearZ, float farZ) = 0;
    virtual void Ortho2D(float left, float right, float bottom, float top) = 0;
    virtual void LookAt(float eyeX, float eyeY, float eyeZ,
                        float centerX, float centerY, float centerZ,
                        float upX, float upY, float upZ) = 0;
    virtual void LookAt(Vector3 eye, Vector3 center, Vector3 up) = 0;

    // Drawing functions
    virtual void DrawBuffer(GLenum mode) = 0;
    virtual void Begin(GLenum mode) = 0;
    virtual void End() = 0;
    virtual void Vertex(coord x, coord y, coord z = 0, coord w = 1.0) = 0;
    virtual void Vertex3v(const coord* v) = 0;
    virtual void Normal(coord nx, coord ny, coord nz) = 0;
    virtual void TexCoord(coord s, coord t) = 0;
    virtual void MultiTexCoord3v(GLenum target, const coord *array) = 0;
    virtual void EnableClientState(GLenum cap) = 0;
    virtual void DisableClientState(GLenum cap) = 0;
    virtual void ClientActiveTexture(GLenum tex) = 0;
    virtual void DrawElements(GLenum  mode, int count, GLenum type, const GLvoid *  indices) = 0;
    virtual void DrawArrays(GLenum mode, int first, int count) = 0;
    virtual void VertexPointer(int size, GLenum type, int stride,
                               const void* pointer) = 0;
    virtual void NormalPointer(GLenum type, int stride,
                               const void* pointer) = 0;
    virtual void TexCoordPointer(int size, GLenum type, int stride,
                                 const void* pointer) = 0;
    virtual void ColorPointer(int size, GLenum type, int stride,
                              const void* pointer) = 0;
    virtual void NewList(uint list, GLenum mode) = 0;
    virtual void EndList() = 0;
    virtual uint GenLists(uint range) = 0;
    virtual void DeleteLists(uint list, uint range) = 0;
    virtual void CallList(uint list) = 0;
    virtual void CallLists(uint size, GLenum type, const void* lists) = 0;
    virtual void ListBase(uint base) = 0;
    virtual void Bitmap(uint  width,  uint  height, coord  xorig,
                        coord  yorig,  coord  xmove, coord  ymove,
                        const uchar *  bitmap) = 0;
    virtual void DrawPixels(GLsizei width, GLsizei height, GLenum format,
                            GLenum type, const GLvoid *data) = 0;

    // Attributes management
    virtual void Viewport(int x, int y, int w, int h) = 0;
    virtual void RasterPos(coord x, coord y, coord z = 0, coord w = 1) = 0;
    virtual void WindowPos(coord x, coord y, coord z = 0, coord w = 1) = 0;
    virtual void PixelStorei(GLenum pname,  int param) = 0;
    virtual void PointSize(coord size) = 0;
    virtual void Color(float r, float g, float b, float a = 1.0) = 0;
    virtual void Materialfv(GLenum face, GLenum pname, const GLfloat *val) = 0;
    virtual void ClearColor(float r, float g, float b, float a = 1.0) = 0;
    virtual void Clear(GLuint mask) = 0;
    virtual void LineWidth(float width) = 0;
    virtual void LineStipple(GLint factor, GLushort pattern) = 0;
    virtual void CullFace(GLenum mode) = 0;
    virtual void FrontFace(GLenum mode) = 0;
    virtual void DepthMask(GLboolean flag) = 0;
    virtual void DepthFunc(GLenum func) = 0;
    virtual void ShadeModel(GLenum mode) = 0;
    virtual void Hint(GLenum target, GLenum mode) = 0;
    virtual void Enable(GLenum cap) = 0;
    virtual void Disable(GLenum cap) = 0;

    // Blend
    virtual void BlendFunc(GLenum sfactor, GLenum dfactor) = 0;
    virtual void BlendFuncSeparate(GLenum sRgb, GLenum dRgb,
                                   GLenum sAlpha, GLenum dAlpha) = 0;
    virtual void BlendEquation(GLenum mode) = 0;

    // Alpha
    virtual void AlphaFunc(GLenum func, float ref) = 0;

    // Stencil
    virtual void ClearStencil(GLint s) = 0;
    virtual void StencilFunc(GLenum func, GLint ref, GLuint mask) = 0;
    virtual void StencilOp(GLenum sfail, GLenum dpfail, GLenum dppass) = 0;

    // Clipping plane
    virtual void ClipPlane(GLenum plane, const GLdouble *equation) = 0;

    // Selection
    virtual int  RenderMode(GLenum mode) = 0;
    virtual void SelectBuffer(int size, uint* buffer) = 0;
    virtual void InitNames() = 0;
    virtual void LoadName(uint name) = 0;
    virtual void PushName(uint name) = 0;
    virtual void PopName() = 0;

    // Shaders
    virtual void UseProgram(uint prg) = 0;
    virtual void GetProgram(uint prg, GLenum pname, int* params) = 0;
    virtual void GetActiveUniform(uint prg, uint id, uint bufSize, GLsizei* length,
                                  GLsizei* size, GLenum* type, char *name) = 0;

    virtual int  GetAttribLocation(uint program, const char* name) = 0;
    virtual void VertexAttrib1fv(uint id, const float *v) = 0;
    virtual void VertexAttrib2fv(uint id, const float *v) = 0;
    virtual void VertexAttrib3fv(uint id, const float *v) = 0;
    virtual void VertexAttrib4fv(uint id, const float *v) = 0;

    virtual int  GetUniformLocation(uint program, const char* name) = 0;
    virtual void Uniform(uint id, float v) = 0;
    virtual void Uniform(uint id, float v0, float v1) = 0;
    virtual void Uniform(uint id, float v0, float v1, float v2) = 0;
    virtual void Uniform(uint id, float v0, float v1, float v2, float v3) = 0;
    virtual void Uniform(uint id, int v) = 0;
    virtual void Uniform(uint id, int v0, int v1) = 0;
    virtual void Uniform(uint id, int v0, int v1, int v2) = 0;
    virtual void Uniform(uint id, int v0, int v1, int v2, int v3) = 0;
    virtual void Uniform(uint id, uint v) = 0;
    virtual void Uniform(uint id, uint v0, uint v1) = 0;
    virtual void Uniform(uint id, uint v0, uint v1, uint v2) = 0;
    virtual void Uniform(uint id, uint v0, uint v1, uint v2, uint v3) = 0;
    virtual void Uniform1fv(uint id, GLsizei size, const float* v) = 0;
    virtual void Uniform2fv(uint id, GLsizei size, const float* v) = 0;
    virtual void Uniform3fv(uint id, GLsizei size, const float* v) = 0;
    virtual void Uniform4fv(uint id, GLsizei size, const float* v) = 0;
    virtual void Uniform1iv(uint id, GLsizei size, const int* v) = 0;
    virtual void Uniform2iv(uint id, GLsizei size, const int* v) = 0;
    virtual void Uniform3iv(uint id, GLsizei size, const int* v) = 0;
    virtual void Uniform4iv(uint id, GLsizei size, const int* v) = 0;
    virtual void UniformMatrix2fv(uint id, GLsizei size,
                                  bool transp, const float* m) = 0;
    virtual void UniformMatrix3fv(uint id, GLsizei size,
                                  bool transp, const float* m) = 0;
    virtual void UniformMatrix4fv(uint id, GLsizei size,
                                  bool transp, const float* m) = 0;


    // Textures
    virtual void ActiveTexture(GLenum id) = 0;
    virtual void GenTextures(uint n, GLuint *  textures) = 0;
    virtual void DeleteTextures(uint n, GLuint *  textures) = 0;
    virtual void BindTexture(GLenum type, GLuint id) = 0;
    virtual void TexParameter(GLenum type, GLenum pname, GLint param) = 0;
    virtual void TexEnv(GLenum type, GLenum pname, GLint param) = 0;
    virtual void TexGen(GLenum coord, GLenum pname, GLint param) = 0;
    virtual void TexImage2D(GLenum target, GLint level, GLint internalformat,
                            GLsizei width, GLsizei height, GLint border,
                            GLenum format, GLenum type,
                            const GLvoid *pixels ) = 0;
    virtual void CompressedTexImage2D(GLenum target, GLint level,
                                      GLenum internalformat,
                                      GLsizei width, GLsizei height,
                                      GLint border, GLsizei imgSize,
                                      const GLvoid *data) = 0;
    virtual void TexImage3D(GLenum target, GLint level, GLint internalformat,
                            GLsizei width, GLsizei height, GLsizei depth, GLint border,
                            GLenum format, GLenum type,
                            const GLvoid *pixels ) = 0;
    virtual void CopyTexImage2D(GLenum target, GLint level, GLenum format,
                                GLint x, GLint y, GLsizei width, GLsizei height,
                                GLint border) = 0;
    virtual void CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset,
                                   GLint yoffset, GLint x, GLint y,
                                   GLsizei width, GLsizei height) = 0;
    virtual void CopyTexSubImage3D(GLenum target, GLint level, GLint xoffset,
                                   GLint yoffset, GLint zoffset, GLint x, GLint y,
                                   GLsizei width, GLsizei height) = 0;
    virtual void GenerateMipmap(GLenum target) = 0;

    // Lighting
    virtual void   Light(GLenum light, GLenum pname, const float* params) = 0;
    virtual void   LightModel(GLenum pname, GLuint param) = 0;
};

TAO_END

#endif // GRAPHIC_STATE_H
