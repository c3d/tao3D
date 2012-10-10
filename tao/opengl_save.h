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

template <class State>
struct OpenGLChanges
// ----------------------------------------------------------------------------
//   A recording of the changes for a given state type
// ----------------------------------------------------------------------------
//   This class records how a given value changes over time.
//
//   The default implementation shown here simply creates a stack of values,
//   where the top of the stack is the last value to restore in the graphics
//   state. This implementation is efficient for small states, e.g. colors,
//   or for "atomic" states, e.g. matrices.
//
//   For more complex states like texture changes, it is more efficient to
//   record changes as "deltas", in which case we will use a specialization
//   of this class. If we have 200 textures active, and we change the state
//   of texture 103, teh 'change_type' will only contain changes for texture
//   103, not changes for any other texture, saving both memory and CPU time.
{
    // By default, we record a change as a state
    typedef State               change_type;

    // By default, we record changes with a vector containing old values
    typedef std::vector<State>  changes_type;
    changes_type                changes;

public:
    // Default 'save' function records a change
    inline bool save(const change_type &change, bool alreadySaved)
    {
        if (!alreadySaved)
            changes.push_back(change);
        return true;
    }

    // Default 'restore' function reverts old state, returns true if dirty
    inline bool restore(State &state)
    {
        state = changes.back();
        changes.pop_back();
        return true;
    }
};


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
#define GS(type, name)                                                  \
    inline void save_##name(const OpenGLChanges<type>::change_type &c)  \
    {                                                                   \
        if (saving_##name)                                              \
            saved_##name = name##_changes.save(c, saved_##name);        \
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
    static OpenGLChanges<type> name##_changes;
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
#define GS(type, name)                                          \
    if (saved_##name)                                           \
        gs->name##_isDirty = name##_changes.restore(gs->name);
// The following two are necessary because we can't reference bitfields
#define GBITFIELD(name)                                         \
    if (saved_##name)                                           \
    {                                                           \
        bool tmp = gs->name;                                    \
        gs->name##_isDirty = name##_changes.restore(tmp);       \
        gs->name = tmp;                                         \
    }
#define GFLAG(name)             GBITFIELD(glflag_##name) 
#define GCLIENTSTATE(name)      GBITFIELD(glclientstate_##name)
#include "opengl_state.tbl"
#undef GBITFIELD

    gs->save = oldSave;
}

TAO_END

#endif // GRAPHIC_SAVE
