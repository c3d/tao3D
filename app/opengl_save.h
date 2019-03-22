#ifndef OPENGL_SAVE
#define OPENGL_SAVE
// *****************************************************************************
// opengl_save.h                                                   Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2012-2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2012-2013,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2012, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

#include "opengl_state.h"

TAO_BEGIN

// ============================================================================
//
//    Class recording individual changes in the OpenGL state
//
// ============================================================================

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


template<>
struct OpenGLChanges<TexturesState>
// ----------------------------------------------------------------------------
//    For texture states, we only record textures that changed
// ----------------------------------------------------------------------------
{
    // By default, we record a change as a state
    typedef TextureState                change_type;

    // By default, we record changes with a vector containing old values
    typedef std::vector<TexturesState>  changes_type;
    changes_type                        changes;

public:
    // Record a texture change
    inline bool save(const change_type &tex, bool alreadySaved)
    {
        if (!alreadySaved)
        {
            // First time we save a texture on this stack level:
            // create save area
            TexturesState empty;
            changes.push_back(empty);
        }

        // Already have a valid state on the top of the stack, update it
        // but only if we didn't already save that texture at this level
        TexturesState &ts = changes.back();
        if (ts.textures.count(tex.id) == 0)
            ts.textures[tex.id] = tex;

        return true;
    }

    // Restore a texture change
    inline bool restore(TexturesState &state)
    {
        TexturesState &saved = changes.back();
        TexturesState::texture_map &texs = saved.textures;
        TexturesState::texture_map::iterator it;
        for (it = texs.begin(); it != texs.end(); it++)
        {
            GLuint id = (*it).first;
            TextureState &tex  = (*it).second;
            TextureState &ntex = state.textures[id];
            tex.mipMap = ntex.mipMap; // Mipmap is the only setting not restored
            ntex = tex;
            state.dirty.insert(id);
        }
        changes.pop_back();
        return true;
    }
};



// ============================================================================
//
//   Class recording and restoring the OpenGL state
//
// ============================================================================

struct OpenGLSave : GraphicSave
// ----------------------------------------------------------------------------
//   Demand-based saving of the state held in a GraphicState
// ----------------------------------------------------------------------------
{
#define OpenGLSave_All ~0ULL

public:
    OpenGLSave(OpenGLState *gs = NULL);
    OpenGLSave(uint64 which = OpenGLSave_All, OpenGLState *gs = NULL);
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


inline OpenGLSave::OpenGLSave(uint64 flags, OpenGLState *gs)
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
