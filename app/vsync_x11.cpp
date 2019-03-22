// *****************************************************************************
// vsync_x11.cpp                                                   Tao3D project
// *****************************************************************************
//
// File description:
//
//    "Sync to VBlank" function for X11.
//
//
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2013, Jérôme Forissier <jerome@taodyne.com>
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

#include "vsync_x11.h"
#include <QWidget>
#if QT_VERSION < 0x050000
#include <QX11Info>
#endif
#include <GL/glx.h>
#include <iostream>

TAO_BEGIN

bool enableVSyncX11(QWidget *widget, bool enable)
// ----------------------------------------------------------------------------
//   Enable or disable VSync, return true on success
// ----------------------------------------------------------------------------
{
    static bool resolved = false;
    typedef int (*setSwapIntervalFunc)(int interval);
    static setSwapIntervalFunc set_fn = NULL;
    if (!resolved)
    {
#if QT_VERSION < 0x050000
        const char *ext = glXGetClientString(widget->x11Info().display(),
                                             GLX_EXTENSIONS);
        QString extensions(ext);
        if (extensions.contains("GLX_MESA_swap_control"))
        {
            set_fn = (setSwapIntervalFunc)
                    glXGetProcAddress((const GLubyte*) "glXSwapIntervalMESA");
        }
        else if (extensions.contains("GLX_SGI_swap_control"))
        {
            set_fn = (setSwapIntervalFunc)
                    glXGetProcAddress((const GLubyte*) "glXSwapIntervalSGI");
        }
#else // QT5 has no easy access to the X11 dpy
        (void) widget;
        set_fn = (setSwapIntervalFunc)
            glXGetProcAddress((const GLubyte*) "glXSwapIntervalMESA");
        if (!set_fn)
            set_fn = (setSwapIntervalFunc)
                glXGetProcAddress((const GLubyte*) "glXSwapIntervalSGI");
#endif // QT5
        resolved = true;
    }
    if (set_fn)
    {
        int swapInterval = enable ? 1 : 0;
        if (!set_fn(swapInterval))
            return true;
    }
    return false;
}

TAO_END
