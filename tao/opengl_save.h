#ifndef OPENGL_SAVE
#define OPENGL_SAVE
// ****************************************************************************
//  opengl_save.h                                                 Tao project
// ****************************************************************************
//
//   File Description:
//
//     Save and restore the OpennGL graphic state transparently
//     This replaces functions like glPushMatric or glPopAttribs
//
//
//
//
//
//
//
// ****************************************************************************
//  (C) 2011 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2011 Taodyne SAS
// ****************************************************************************

#include "opengl_state.h"

TAO_BEGIN

struct OpenGLSave : GraphicSave
// ----------------------------------------------------------------------------
//   Demand-based saving of the state held in a GraphicState
// ----------------------------------------------------------------------------
{
#define OpenGLSave_All ~0ULL

public:
    OpenGLSave(OpenGLState *gs = NULL);
    OpenGLSave(ulonglong which = OpenGLSave_All, OpenGLState *gs = NULL);
    ~OpenGLSave();

public:
#define GS(type, name)                          \
    void save_##name(const type &value)         \
    {                                           \
        if (saving_##name && !saved_##name)     \
        {                                       \
            name##_stack.push_back(value);      \
            saved_##name = true;                \
        }                                       \
    }
#include "opengl_state.tbl"

public:
    OpenGLState *gs;
#define GS(type, name)                          \
    bool saved_##name:1;                        \
    bool saving_##name:1;
#include "opengl_state.tbl"
    OpenGLSave *oldSave;

#define GS(type, name)                          \
    static std::vector<type> name##_stack;
#include "opengl_state.tbl"
};



// ============================================================================
//
//    Inline implementation
//
// ============================================================================

inline OpenGLSave::OpenGLSave(OpenGLState *gs)
// ----------------------------------------------------------------------------
//   Prepare to save the state
// ----------------------------------------------------------------------------
//   Initialize all save_X flags to false, save graphic state and link in list
    : gs(gs ? gs : gs = ((OpenGLState *) &GL)),
#define GS(type, name)  saved_##name(false), saving_##name(true),
#include "opengl_state.tbl"
      oldSave(gs->save)
{
    gs->save = this;
}


inline OpenGLSave::OpenGLSave(ulonglong flags, OpenGLState *gs)
// ----------------------------------------------------------------------------
//   Prepare to save the state
// ----------------------------------------------------------------------------
//   Initialize all save_X flags to false, save graphic state and link in list
    : gs(gs ? gs : gs = ((OpenGLState *) &GL)),
#define GS(type, name)                          \
      saved_##name(false),                      \
      saving_##name(flags & STATE_##name),
#include "opengl_state.tbl"
      oldSave(gs->save)
{
    gs->save = this;
}


inline OpenGLSave::~OpenGLSave()
// ----------------------------------------------------------------------------
//   Restore what needs to be saved
// ----------------------------------------------------------------------------
{
#define GS(type, name)                                  \
    if (saved_##name)                                   \
    {                                                   \
        gs->name = name##_stack.back();                 \
        gs->name##_isDirty = true;                      \
        name##_stack.pop_back();                        \
    }
#include "opengl_state.tbl"
    gs->save = oldSave;
}

TAO_END

#endif // GRAPHIC_SAVE
