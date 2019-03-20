// ****************************************************************************
//  vsync_x11.h                                                    Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This software is licensed under the GNU General Public License v3.
// See file COPYING for details.
//  (C) 2013 Jerome Forissier <jerome@taodyne.com>
//  (C) 2013 Taodyne SAS
// ****************************************************************************

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
