#ifndef VSYNC_X11_H
#define VSYNC_X11_H
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

#include "tao.h"

class QWidget;

TAO_BEGIN

bool enableVSyncX11(QWidget *widget, bool enable);

TAO_END

#endif
