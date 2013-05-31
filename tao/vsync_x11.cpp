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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2013 Jerome Forissier <jerome@taodyne.com>
//  (C) 2013 Taodyne SAS
// ****************************************************************************

#include "vsync_x11.h"
#include <QWidget>
#include <QX11Info>
#include "tao/GL/glxew.h"
#include <iostream>

TAO_BEGIN

bool enableVSyncX11(QWidget *widget, bool enable)
// ----------------------------------------------------------------------------
//   Enable or disable VSync, return true on success
// ----------------------------------------------------------------------------
{
    static bool resolved = false;
    typedef int (*setSwapIntervalFunc)(int interval);
    typedef int (*getSwapIntervalFunc)(int interval);
    static setSwapIntervalFunc set_fn = NULL;
    if (!resolved)
    {
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
