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

#include "coords4d.h"
#include "coords3d.h"
#include "color.h"
#include "matrix.h"
#include "graphic_state.h"
#include <stack>

TAO_BEGIN

struct OpenGLSave;              // Structure used to save/restore state
struct TextureState;
struct TexturesState;


// ============================================================================
//
//   Elements necessary for the OpenGL state
//
// ============================================================================

enum StateBits
{
#define GS(type, name)          STATE_BIT_##name,
#include "opengl_state.tbl"
    STATE_BIT_LAST,

#define GS(type, name)          STATE_##name = 1ULL << STATE_BIT_##name,
#include "opengl_state.tbl"
};

XL_CASSERT(STATE_BIT_LAST <= 64);


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



// ============================================================================
//
//   Clip planes
//
// ============================================================================


struct ClipPlaneState
// ----------------------------------------------------------------------------
//   Represent clip plane state
// ----------------------------------------------------------------------------
{
    ClipPlaneState(GLenum plane = GL_CLIP_PLANE0) : plane(plane), equation() {}
    bool operator==(const ClipPlaneState &o)
    {
        return equation == o.equation &&
               active == o.active;
    }
    bool operator!=(const ClipPlaneState &o) { return !operator==(o); }
    void Sync(ClipPlaneState &p, bool force=false);
    GLenum  plane;
    Vector4 equation;
    bool active;
};


struct ClipPlanesState
// ----------------------------------------------------------------------------
//   Represent clip planes state
// ----------------------------------------------------------------------------
{
    ClipPlanesState():  dirty(~0ULL), active(0), planes() {}
    bool operator==(const ClipPlanesState &o)
    {
        uint max = planes.size();
        if (max != o.planes.size())
            return false;
        for (uint i = 0; i < max; i++)
            if (planes[i] != o.planes[i])
                return false;
        return true;
    }
    bool operator!=(const ClipPlanesState &o) { return !operator==(o); }
    bool Sync(ClipPlanesState &ops);

    uint64                        dirty;
    uint64                        active;
    std::vector<ClipPlaneState>   planes;
};


// ============================================================================
// 
//   Texture units
// 
// ============================================================================

struct TextureUnitState
// ----------------------------------------------------------------------------
//    The state of a single texture unit
// ----------------------------------------------------------------------------
{
    TextureUnitState();
    void Set(GLenum cap, bool enabled)
    {
        switch (cap)
        {
        case GL_TEXTURE_1D:             tex1D = enabled; break;
        case GL_TEXTURE_2D:             tex2D = enabled; break;
        case GL_TEXTURE_3D:             tex3D = enabled; break;
        case GL_TEXTURE_CUBE_MAP:       texCube = enabled; break;
        default:                        Q_ASSERT(!"Invalid enum");
        }
    }

    bool operator==(const TextureUnitState &o)
    {
        return texture == o.texture &&
               target == o.target   &&
               mode == o.mode       &&
               tex1D   == o.tex1D   &&
               tex2D   == o.tex2D   &&
               tex3D   == o.tex3D   &&
               texCube == o.texCube &&
               matrix  == o.matrix;
    }
    bool operator!=(const TextureUnitState &o) { return !operator==(o); }
    void Sync(GLuint u, TextureUnitState &ns, TextureState &ot);

    GLuint      texture;
    GLenum      target, mode;
    Matrix4     matrix;
    bool        tex1D   : 1;
    bool        tex2D   : 1;
    bool        tex3D   : 1;
    bool        texCube : 1;

    // REVISIT: The following are here to assure a compatibility with
    // the XL language (these settings can be setted without binding textures)
    // and also with old versions of Tao.
    GLenum      minFilt, magFilt; // Default values in Tao depends from preferences
    bool        wrapS  : 1;       // Default value in Tao is false (in OpenGL, it's true)
    bool        wrapT  : 1;       // Default value in Tao is false
    bool        wrapR  : 1;       // Default value in Tao is false
};


#define MAX_TEXTURE_UNITS 64

struct TextureUnitsState
// ----------------------------------------------------------------------------
//    The state of all texture units
// ----------------------------------------------------------------------------
{
    TextureUnitsState(): dirty(~0ULL), active(0), units() {}
    bool operator==(const TextureUnitsState &o)
    {
        uint max = units.size();
        if (max != o.units.size())
            return false;
        for (uint i = 0; i < max; i++)
            if (units[i] != o.units[i])
                return false;
        return true;
    }
    bool operator!=(const TextureUnitsState &o) { return !operator==(o); }

    bool Sync(TexturesState &nts, TexturesState &ots, TextureUnitsState &ns);

public:
    uint64                          dirty;
    uint64                          active;
    std::vector<TextureUnitState>   units;
};


// ============================================================================
//
//   Client active texture units
//
// ============================================================================

struct ClientTextureUnitState
// ----------------------------------------------------------------------------
//    The state of a single client texture unit (for texture coordinates)
// ----------------------------------------------------------------------------
{
    ClientTextureUnitState()
        :   texCoordArray(false) {}

    void Set(GLenum cap, bool enabled)
    {
        switch (cap)
        {
        case GL_TEXTURE_COORD_ARRAY:    texCoordArray = enabled; break;
        default:                        Q_ASSERT(!"Invalid enum");
        }
    }

    bool operator==(const ClientTextureUnitState &o)
    {
        return texCoordArray == o.texCoordArray;
    }

    bool operator!=(const ClientTextureUnitState &o) { return !operator==(o); }
    void Sync(ClientTextureUnitState &ns, bool force);

    bool        texCoordArray   : 1;
};


#define MAX_TEXTURE_COORDS 8

struct ClientTextureUnitsState
// ----------------------------------------------------------------------------
//    The state of all client texture units
// ----------------------------------------------------------------------------
{
    ClientTextureUnitsState(): dirty(~0ULL), active(0), units() {}
    bool operator==(const ClientTextureUnitsState &o)
    {
        uint max = units.size();
        if (max != o.units.size())
            return false;
        for (uint i = 0; i < max; i++)
            if (units[i] != o.units[i])
                return false;
        return true;
    }
    bool operator!=(const ClientTextureUnitsState &o) { return !operator==(o); }

    bool Sync(ClientTextureUnitsState &ns, uint clientActiveUnit);

public:
    uint64                                dirty;
    uint64                                active;
    std::vector<ClientTextureUnitState>   units;
};


// ============================================================================
// 
//   Textures
// 
// ============================================================================

struct TextureState
// ----------------------------------------------------------------------------
//   The state of a single texture
// ----------------------------------------------------------------------------
{
    TextureState(GLuint id = 0);
    bool operator ==(const TextureState &o)
    {
        return (type == o.type && id == o.id &&
                /* ignore unit, width and height on purpose */
                active == o.active &&
                minFilt == o.minFilt && magFilt == o.magFilt &&
                wrapS == o.wrapS && wrapT == o.wrapT && wrapR == o.wrapR);
    }
    bool operator!=(const TextureState &o) { return !operator==(o); }
    void Sync(TextureState &newState);

public:
    GLenum      type;
    GLuint      id;
    GLuint      width, height, depth;
    bool        active : 1;
    GLenum      minFilt, magFilt;
    bool        wrapS  : 1;
    bool        wrapT  : 1;
    bool        wrapR  : 1;

    // REVISIT: The following are here temporarily (for Layout use).
    // They really belong to TextureUnit, which is done for the OpenGLState.
    // That is the reason they are ignored in operator==
    GLuint      unit;
    GLenum      mode;
};


struct TexturesState
// ----------------------------------------------------------------------------
//    The state of all textures
// ----------------------------------------------------------------------------
//    This is also used in OpenGLSave to record the previous state of the
//    textures that have been changed.
{
    TexturesState(): textures() { textures[0] = TextureState(); }

public:
    typedef std::map<GLuint, TextureState> texture_map;
    typedef std::set<GLuint> texture_set;

public:
    texture_map textures;
    texture_set dirty;
};



// ============================================================================
// 
//    Light state
// 
// ============================================================================

struct LightState
// ----------------------------------------------------------------------------
//   The state of a single light
// ----------------------------------------------------------------------------
{
    LightState(uint id = 0);
    bool operator ==(const LightState &o)
    {
        return (id == o.id &&
                ambient == o.ambient &&
                diffuse == o.diffuse &&
                specular == o.specular  &&
                position == o.position &&
                spotDirection == o.spotDirection &&
                spotCutoff == o.spotCutoff &&
                spotExponent == o.spotExponent &&
                constantAttenuation == o.constantAttenuation &&
                linearAttenuation == o.linearAttenuation &&
                quadricAttenuation == o.quadricAttenuation &&
                active == o.active);
    }
    bool operator!=(const LightState &o) { return !operator==(o); }
    void Sync(const LightState &newState, bool force = false);

public:
    GLuint      id;
    Tao::Color  ambient;
    Tao::Color  diffuse;
    Tao::Color  specular;
    Vector4     position;
    Vector3     spotDirection;
    GLfloat     spotExponent;
    GLfloat     spotCutoff;
    GLfloat     constantAttenuation;
    GLfloat     linearAttenuation;
    GLfloat     quadricAttenuation;
    bool        active : 1;
};



#define MAX_LIGHTS 8
struct LightsState
// ----------------------------------------------------------------------------
//    The state of all lights
// ----------------------------------------------------------------------------
{
    LightsState(): dirty(~0ULL), active(0), lights() {}
    bool operator==(const LightsState &o)
    {
        uint max = lights.size();
        if (max != o.lights.size())
            return false;
        for (uint i = 0; i < max; i++)
            if (lights[i] != o.lights[i])
                return false;
        return true;
    }
    bool operator!=(const LightsState &o) { return !operator==(o); }
    void Sync(LightsState &nl);

public:
    uint64                    dirty;
    uint64                    active;
    std::vector<LightState>   lights;
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
    virtual ~OpenGLState()              { if (current == this) current = NULL; }

    // Return the current graphic state (one per widget)
    static OpenGLState *State()         { return current; }
    void   MakeCurrent();
    uint   VendorID()                   { return vendorID; }
    bool   IsATIOpenGL()                { return vendorID == ATI; }

    // Saving and restoring state
    virtual GraphicSave *       Save();
    virtual void                Restore(GraphicSave *saved);

    // Apply pending state changes
    virtual void                Sync(uint64 which = ~0ULL);
    virtual void                Invalidate(uint64 which = ~0ULL);

    std::ostream & debug();

    // Return attributes of state
    virtual uint   MaxTextureCoords()   { return maxTextureCoords; }
    virtual uint   MaxTextureUnits()    { return maxTextureUnits; }
    virtual uint   MaxTextureSize()     { return maxTextureSize; }
    virtual text   Vendor()             { return vendor; }
    virtual text   Renderer()           { return renderer; }
    virtual text   Version()            { return version; }
    virtual coord* ModelViewMatrix()    { return mvMatrix.Data(false); }
    virtual coord* ProjectionMatrix()   { return projMatrix.Data(false); }
    virtual int*   Viewport()           { return (int *) &viewport.x; }
    virtual void   Get(GLenum pname, GLboolean * params);
    virtual void   Get(GLenum pname, GLfloat * params);
    virtual void   Get(GLenum pname, GLint * params);

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
    virtual void ClientActiveTexture(GLenum tex);
    virtual void DrawElements(GLenum  mode, int count, GLenum type, const GLvoid *  indices);
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
    virtual void DrawPixels(GLsizei width, GLsizei height, GLenum format,
                            GLenum type, const GLvoid *data);


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

    // Stencil
    virtual void ClearStencil(GLint s)     { glClearStencil(s); }
    virtual void StencilFunc(GLenum func,
                             GLint ref,
                             GLuint mask)  { glStencilFunc(func, ref, mask); }
    virtual void StencilOp(GLenum sfail,
                           GLenum dpfail,
                           GLenum dppass)  { glStencilOp(sfail, dpfail, dppass); }

    // Clipping plane
    virtual void ClipPlane(GLenum plane, const GLdouble *equation);

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
    virtual void GetUniformIndices(uint prg, GLsizei count, const char** names, GLuint* indices);
    virtual void GetActiveUniform(uint prg, uint id, uint bufSize, GLsizei* length,
                                  GLsizei* size, GLenum* type, char *name);
    virtual int  GetAttribLocation(uint program, const char* name);
    virtual void VertexAttrib1fv(uint id, const float *v);
    virtual void VertexAttrib2fv(uint id, const float *v);
    virtual void VertexAttrib3fv(uint id, const float *v);
    virtual void VertexAttrib4fv(uint id, const float *v);

    virtual int  GetUniformLocation(uint program, const char* name);
    virtual void Uniform(uint id, float v);
    virtual void Uniform(uint id, float v0, float v1);
    virtual void Uniform(uint id, float v0, float v1, float v2);
    virtual void Uniform(uint id, float v0, float v1, float v2, float v3);
    virtual void Uniform(uint id, int v);
    virtual void Uniform(uint id, int v0, int v1);
    virtual void Uniform(uint id, int v0, int v1, int v2);
    virtual void Uniform(uint id, int v0, int v1, int v2, int v3);
    virtual void Uniform(uint id, uint v);
    virtual void Uniform(uint id, uint v0, uint v1);
    virtual void Uniform(uint id, uint v0, uint v1, uint v2);
    virtual void Uniform(uint id, uint v0, uint v1, uint v2, uint v3);
    virtual void Uniform1fv(uint id, GLsizei size, const float* v);
    virtual void Uniform2fv(uint id, GLsizei size, const float* v);
    virtual void Uniform3fv(uint id, GLsizei size, const float* v);
    virtual void Uniform4fv(uint id, GLsizei size, const float* v);
    virtual void Uniform1iv(uint id, GLsizei size, const int* v);
    virtual void Uniform2iv(uint id, GLsizei size, const int* v);
    virtual void Uniform3iv(uint id, GLsizei size, const int* v);
    virtual void Uniform4iv(uint id, GLsizei size, const int* v);
    virtual void UniformMatrix2fv(uint id, GLsizei size,
                                  bool transp, const float* m);
    virtual void UniformMatrix3fv(uint id, GLsizei size,
                                  bool transp, const float* m);
    virtual void UniformMatrix4fv(uint id, GLsizei size,
                                  bool transp, const float* m);

    // Textures
    virtual void ActiveTexture(GLenum id);
    virtual void GenTextures(uint n, GLuint *  textures);
    virtual void DeleteTextures(uint n, GLuint *  textures);
    virtual void BindTexture(GLenum type, GLuint id);
    virtual void TexParameter(GLenum type, GLenum pname, GLint param);
    virtual void TexEnv(GLenum type, GLenum pname, GLint param);
    virtual void TexImage2D(GLenum target, GLint level, GLint internalformat,
                            GLsizei width, GLsizei height, GLint border,
                            GLenum format, GLenum type,
                            const GLvoid *pixels );
    virtual void CompressedTexImage2D(GLenum target, GLint level,
                                      GLenum internalformat,
                                      GLsizei width, GLsizei height,
                                      GLint border, GLsizei imgSize,
                                      const GLvoid *data);    
    virtual void TexImage3D(GLenum target, GLint level, GLint internalformat,
                            GLsizei width, GLsizei height, GLsizei depth, GLint border,
                            GLenum format, GLenum type,
                            const GLvoid *pixels );

    virtual void TextureSize(uint width, uint height, uint depth = 0);
    virtual uint TextureWidth();
    virtual uint TextureHeight();
    virtual uint TextureDepth();
    virtual uint TextureType();
    virtual uint TextureMode();
    virtual uint TextureID();
    virtual void ActivateTextureUnits(uint64 mask);
    virtual uint ActiveTextureUnitIndex();
    virtual uint ActiveTextureUnitsCount();
    virtual uint64 ActiveTextureUnits();
    virtual void HasPixelBlur(bool enable);
    virtual void GenerateMipmap(GLenum target);

    // Lighting
    void SetLight(GLenum light, bool active);
    virtual void Light(GLenum light, GLenum pname, const float* params);

public:
#define GS(type, name)                          \
    inline void SetCached_##name(type name)     \
    {                                           \
        this->name = name;                      \
        name##_isDirty = false;                 \
    }
#include "opengl_state.tbl"

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
    enum VendorID           vendorID;
    GLuint                  maxTextureCoords,maxTextureUnits,maxTextureSize;
    text                    vendor, renderer, version, extensionsAvailable;
    TexturesState           currentTextures;
    TextureUnitsState       currentTextureUnits;
    ClientTextureUnitsState currentClientTextureUnits;
    LightsState             currentLights;
    ClipPlanesState         currentClipPlanes;

#define GS(type, name)                          \
    type name;
#define GFLAG(name)                             \
    bool glflag_##name:1;
#define GCLIENTSTATE(name)                      \
    bool glclientstate_##name:1;
#include "opengl_state.tbl"

public:
    static text          vendorsList[LAST_VENDOR];
    static uint ShowErrors(kstring msg = NULL);

public:
    // Return the current client texture unit state
    ClientTextureUnitState & ClientActiveTexture();

    // Return the current texture state
    TextureState &      ActiveTexture();
    TextureUnitState &  ActiveTextureUnit(bool isDirty = true);

    // Return the corresponding clip plane state
    ClipPlaneState &    ClipPlane(GLenum plane);

private:
    // Structure used to push/pop state
    friend class OpenGLSave;
    OpenGLSave *save;

    // Return the current texture matrix
    Matrix4 &           TextureMatrix()
    {
        return ActiveTextureUnit().matrix;
    }

    // Singleton
    OpenGLState(const OpenGLState &other);
    static OpenGLState *        current;
};

// Shortcut to the current state
#define GL  (*Tao::OpenGLState::State())

TAO_END

#endif // GRAPHIC_STATE_H
