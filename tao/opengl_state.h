#ifndef OPENGL_STATE_H
#define OPENGL_STATE_H
// ****************************************************************************
//  opengl_state.h                                                 Tao project
// ****************************************************************************
//
//   File Description:
//
//     Manage OpenGL state
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
//  (C) 2012 Baptiste Soulisse <soulisse.baptiste@taodyne.com>
//  (C) 2012 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2012 Taodyne SAS
// ****************************************************************************

#include "coords3d.h"
#include "color.h"
#include "matrix.h"
#include "graphic_state.h"
#include <stack>

TAO_BEGIN

struct OpenGLSave;              // Structure used to save/restore state



// ============================================================================
//
//   Elements necessary for the OpenGL state
//
// ============================================================================

enum StateBits
{
#define GS(type, name)          STATE_BIT_##name,
#include "opengl_state.tbl"
#define GS(type, name)          STATE_##name = 1ULL << STATE_BIT_##name,
#include "opengl_state.tbl"
};


struct ViewportState
// ----------------------------------------------------------------------------
//    Represent the viewport information in a more readable way
// ----------------------------------------------------------------------------
{
    ViewportState(int x, int y, int w, int h): x(x), y(y), w(w), h(h) {}
    bool operator==(const ViewportState &o)
    {
        return x==o.x && y==o.y && w==o.w && h==o.h;
    }
    bool operator!=(const ViewportState &o) { return !operator==(o); }
    int x, y, w, h;
};


struct LineStippleState
// ----------------------------------------------------------------------------
//    Represent line stipple information
// ----------------------------------------------------------------------------
{
    LineStippleState(GLint f, GLushort p): factor(f), pattern(p) {}
    bool operator==(const LineStippleState &o)
    {
        return factor == o.factor && pattern == o.pattern;
    }
    bool operator!=(const LineStippleState &o) { return !operator==(o); }
    GLint factor;
    GLushort pattern;
};


struct BlendFunctionState
// ----------------------------------------------------------------------------
//   Represent blend function state
// ----------------------------------------------------------------------------
{
    BlendFunctionState(GLenum srcRgb, GLenum destRgb, GLenum srcA, GLenum dstA)
        : srcRgb(srcRgb), destRgb(destRgb), srcAlpha(srcA), destAlpha(dstA) {}
    bool operator==(const BlendFunctionState &o)
    {
        return (srcRgb == o.srcRgb && destRgb == o.destRgb &&
                srcAlpha == o.srcAlpha && destAlpha == o.destAlpha);
    }
    bool operator!=(const BlendFunctionState &o) { return !operator==(o); }
    GLenum srcRgb;
    GLenum destRgb;
    GLenum srcAlpha;
    GLenum destAlpha;
};


struct AlphaFunctionState
// ----------------------------------------------------------------------------
//   Represent alpha function state
// ----------------------------------------------------------------------------
{
    AlphaFunctionState(GLenum func, GLfloat ref): func(func), ref(ref) {}
    bool operator==(const AlphaFunctionState &o)
    {
        return func == o.func && ref == o.ref;
    }
    bool operator!=(const AlphaFunctionState &o) { return !operator==(o); }
    GLenum func;
    GLfloat ref;
};


struct TextureState
// ----------------------------------------------------------------------------
//   The state of a single texture (single texture unit)
// ----------------------------------------------------------------------------
{
    TextureState();
    bool operator ==(const TextureState &o)
    {
        return (type == o.type && mode == o.mode && id == o.id &&
                /* ignore unit, width and height on purpose */
                minFilt == o.minFilt && magFilt == o.magFilt &&
                matrix == o.matrix &&
                active == o.active &&
                wrapS == o.wrapS && wrapT == o.wrapT && wrapR == o.wrapR &&
                mipmap == o.mipmap);
    }
    bool operator!=(const TextureState &o) { return !operator==(o); }
    void Sync(GLuint unit, const TextureState &newState, bool force = false);

public:
    GLenum      type, mode;
    GLuint      unit, id;
    GLuint      width, height;
    GLenum      minFilt, magFilt;
    Matrix4     matrix;
    bool        active : 1;
    bool        wrapS  : 1;
    bool        wrapT  : 1;
    bool        wrapR  : 1;
    bool        mipmap : 1;
};


#define MAX_TEXTURE_UNITS 64
struct TexturesState
// ----------------------------------------------------------------------------
//    The state of all textures on all texture units
// ----------------------------------------------------------------------------
{
    TexturesState(): dirty(~0ULL), textures() {}
    bool operator==(const TexturesState &o)
    {
        uint max = textures.size();
        if (max != o.textures.size())
            return false;
        for (uint i = 0; i < max; i++)
            if (textures[i] != o.textures[i])
                return false;
        return true;
    }
    bool operator!=(const TexturesState &o) { return !operator==(o); }
    TexturesState &operator=(const TexturesState &o);
    void Sync(TexturesState &newState);

public:
    ulonglong dirty;
    std::vector<TextureState>   textures;
};



// ============================================================================
//
//   The OpenGL state itself
//
// ============================================================================

struct OpenGLState : GraphicState
// ----------------------------------------------------------------------------
//   Class to manage graphic states
// ----------------------------------------------------------------------------
{
    OpenGLState();

    // Saving and restoring state
    virtual GraphicSave *       Save();
    virtual void                Restore(GraphicSave *saved);
    virtual void                Sync(ulonglong which = ~0ULL);

    // Return attributes of state
    virtual uint   MaxTextureCoords()       { return maxTextureCoords; }
    virtual uint   MaxTextureUnits()        { return maxTextureUnits; }
    virtual text   Vendor()                 { return vendor; }
    virtual text   Renderer()               { return renderer; }
    virtual text   Version()                { return version; }
    virtual uint   VendorID()               { return vendorID; }
    virtual bool   IsATIOpenGL()            { return vendorID == ATI; }
    virtual coord* ModelViewMatrix()        { return mvMatrix.Data(false); }
    virtual coord* ProjectionMatrix()       { return projMatrix.Data(false); }
    virtual int*   Viewport()               { return (int *) &viewport.x; }

    // Matrix management
    virtual void   MatrixMode(GLenum mode);
    virtual void   LoadMatrix();
    virtual void   LoadIdentity();
    virtual void   MultMatrices(const coord *m1,const coord *m2,coord *result);
    virtual void   MultMatrixVec(const coord matrix[16], const coord in[4],
                                 coord out[4]);
    virtual bool   InvertMatrix(const GLdouble m[16], GLdouble invOut[16]);
    virtual void   PrintMatrix(GLuint model);

    // Transformations
    virtual void Translate(double x, double y, double z);
    virtual void Rotate(double a, double x, double y, double z);
    virtual void Scale(double x, double y, double z);

    // Camera management
    virtual bool Project(coord objx, coord objy, coord objz,
                         const coord* mv, const coord* proj,
                         const int* viewport,
                         coord *winx, coord *winy, coord *winz);
    virtual bool UnProject(coord winx, coord winy, coord winz,
                           const coord* mv, const coord* proj,
                           const int* viewport,
                           coord *objx, coord *objy, coord *objz);
    virtual void PickMatrix(float x, float y, float width, float height,
                            int viewport[4]);
    virtual void Frustum(float left, float right, float bottom, float top,
                         float nearZ, float farZ);
    virtual void Perspective(float fovy, float aspect, float nearZ, float farZ);
    virtual void Ortho(float left, float right, float bottom,
                       float top, float nearZ, float farZ);
    virtual void Ortho2D(float left, float right, float bottom, float top);
    virtual void LookAt(float eyeX, float eyeY, float eyeZ,
                        float centerX, float centerY, float centerZ,
                        float upX, float upY, float upZ);
    virtual void LookAt(Vector3 eye, Vector3 center, Vector3 up);

    // Draw functions
    virtual void DrawBuffer(GLenum  mode);
    virtual void Begin(GLenum mode);
    virtual void End();
    virtual void Vertex(coord x, coord y, coord z = 0, coord w = 1);
    virtual void Vertex3v(const coord* array);
    virtual void Normal(coord nx, coord ny, coord nz);
    virtual void TexCoord(coord s, coord t);
    virtual void MultiTexCoord3v(GLenum target, const coord *array);
    virtual void EnableClientState(GLenum cap);
    virtual void DisableClientState(GLenum cap);
    virtual void DrawArrays(GLenum mode, int first, int count);
    virtual void VertexPointer(int size, GLenum type, int stride,
                               const void* pointer);
    virtual void NormalPointer(GLenum type, int stride,
                               const void* pointer);
    virtual void TexCoordPointer(int size, GLenum type, int stride,
                                 const void* pointer);
    virtual void ColorPointer(int size, GLenum type, int stride,
                              const void* pointer);
    virtual void NewList(uint list, GLenum mode);
    virtual void EndList();
    virtual uint GenLists(uint range);
    virtual void DeleteLists(uint list, uint range);
    virtual void CallList(uint list);
    virtual void CallLists(uint size, GLenum type, const void* pointer);
    virtual void ListBase(uint base);
    virtual void Bitmap(uint  width,  uint  height, coord  xorig,
                        coord  yorig,  coord  xmove, coord  ymove,
                        const uchar *  bitmap);


    // Attributes management
    virtual void Viewport(int x, int y, int w, int h);
    virtual void RasterPos(coord x, coord y, coord z = 0, coord w = 1);
    virtual void WindowPos(coord x, coord y, coord z = 0, coord w = 1);
    virtual void PixelStorei(GLenum pname,  int param);
    virtual void PointSize(coord size);
    virtual void Color(float r, float g, float b, float a = 1.0);
    virtual void Materialfv(GLenum face, GLenum pname, const GLfloat *val);
    virtual void ClearColor(float r, float g, float b, float a = 1.0);
    virtual void Clear(GLuint mask);
    virtual void LineWidth(float width);
    virtual void LineStipple(GLint factor, GLushort pattern);
    virtual void CullFace(GLenum mode);
    virtual void DepthMask(GLboolean flag);
    virtual void DepthFunc(GLenum func);
    virtual void ShadeModel(GLenum mode);
    virtual void Hint(GLenum target, GLenum mode);
    virtual void Enable(GLenum cap);
    virtual void Disable(GLenum cap);

    // Blend
    virtual void BlendFunc(GLenum sfactor, GLenum dfactor);
    virtual void BlendFuncSeparate(GLenum sRgb, GLenum dRgb,
                                   GLenum sAlpha, GLenum dAlpha);
    virtual void BlendEquation(GLenum mode);

    // Alpha
    virtual void AlphaFunc(GLenum func, float ref);

    // Selection
    virtual int  RenderMode(GLenum mode);
    virtual void SelectBuffer(int size, uint* buffer);
    virtual void InitNames();
    virtual void LoadName(uint name);
    virtual void PushName(uint name);
    virtual void PopName();

    // Shader functions
    virtual void UseProgram(uint prg);
    virtual void GetProgram(uint prg, GLenum pname, int* params);
    virtual void GetActiveUniform(uint prg, uint id, uint bufSize, GLsizei* length,
                                  GLsizei* size, GLenum* type, char *name);

    virtual int  GetAttribLocation(uint program, const char* name);
    virtual void VertexAttrib1fv(uint id, const float *v)      { glVertexAttrib1fv(id, v); }
    virtual void VertexAttrib2fv(uint id, const float *v)      { glVertexAttrib2fv(id, v); }
    virtual void VertexAttrib3fv(uint id, const float *v)      { glVertexAttrib3fv(id, v); }
    virtual void VertexAttrib4fv(uint id, const float *v)      { glVertexAttrib4fv(id, v); }

    virtual int  GetUniformLocation(uint program, const char* name);
    virtual void Uniform1i(uint id, int v)                     { glUniform1i(id, v); }
    virtual void Uniform1fv(uint id, GLsizei size, const float* v) { glUniform1fv(id, size, v); }
    virtual void Uniform2fv(uint id, GLsizei size, const float* v) { glUniform2fv(id, size, v); }
    virtual void Uniform3fv(uint id, GLsizei size, const float* v) { glUniform3fv(id, size, v); }
    virtual void Uniform4fv(uint id, GLsizei size, const float* v) { glUniform4fv(id, size, v); }
    virtual void UniformMatrix2fv(uint id, GLsizei size,
                                  bool transp, const float* m) { glUniformMatrix2fv(id, size, transp, m); }
    virtual void UniformMatrix3fv(uint id, GLsizei size,
                                  bool transp, const float* m) { glUniformMatrix3fv(id, size, transp, m); }
    virtual void UniformMatrix4fv(uint id, GLsizei size,
                                  bool transp, const float* m) { glUniformMatrix4fv(id, size, transp, m); }

    // Textures
    virtual void ActiveTexture(GLenum id);
    virtual void BindTexture(GLenum type, GLuint id);
    virtual void TexParameteri(GLenum type, GLenum pname, GLint param);
    virtual void TexEnvi(GLenum type, GLenum pname, GLint param);
    virtual void TexImage2D(GLenum target, GLint level, GLint internalformat,
                            GLsizei width, GLsizei height, GLint border,
                            GLenum format, GLenum type,
                            const GLvoid *pixels );
    virtual void CompressedTexImage2D(GLenum target, GLint level,
                                      GLenum internalformat,
                                      GLsizei width, GLsizei height,
                                      GLint border, GLsizei imgSize,
                                      const GLvoid *data);

    std::ostream & debug();

public:
#define GS(type, name)                          \
    bool name##_isDirty : 1;
#include "opengl_state.tbl"

public:
    enum VendorID
    {
        UNKNOWN = 0,
        ATI,
        NVIDIA,
        INTEL,
        LAST_VENDOR
    };

public:
    enum VendorID vendorID;
    GLuint        maxTextureCoords, maxTextureUnits;
    text          vendor, renderer, version, extensionsAvailable;
    TexturesState currentTextures;
    TextureState *currentUnit;

#define GS(type, name)                          \
    type name;
#define GFLAG(name)                             \
    bool glflag_##name:1;
#include "opengl_state.tbl"

public:
    static text          vendorsList[LAST_VENDOR];
    static uint ShowErrors();

private:
    // Structure used to push/pop state
    friend class OpenGLSave;
    OpenGLSave *save;

    // Return the current texture state and texture matrix
    TextureState &ActiveTexture();
    Matrix4&    TextureMatrix()
    {
        return ActiveTexture().matrix;
    }
};

TAO_END

#endif // GRAPHIC_STATE_H
