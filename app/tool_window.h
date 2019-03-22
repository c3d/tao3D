#ifndef TOOL_WINDOW_H
#define TOOL_WINDOW_H
// *****************************************************************************
// tool_window.h                                                   Tao3D project
// *****************************************************************************
//
// File description:
//
//    A window suitable to display tools.
//    ToolWindows typically have a reduced title bar and small or no borders.
//    They also have the ability to save their geometry, so that they are
//    restored identically when the application is restarted.
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2013-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
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

#include "tao.h"
#include <QDialog>
#include <QPoint>

namespace Tao {


struct ToolWindow : QDialog
// ----------------------------------------------------------------------------
//   A window with small title bar and borders suitable for tools
// ----------------------------------------------------------------------------
{
private:
    Q_OBJECT

public:
    ToolWindow(const QString &title, QWidget *parent = 0,
                   const QString &objName = "")
        : QDialog(parent), needPlacement(true), toggleView(NULL)
    {
        setWindowFlags(flags());
        setWindowTitle(title);
        if (!objName.isEmpty())
            setObjectName(objName);
    }

    ToolWindow(QWidget *parent = 0)
        : QDialog(parent), needPlacement(true), toggleView(NULL)
    {
        setWindowFlags(flags());
    }

    virtual void   setVisible(bool visible);
    bool           createVisible();
    bool           doClose();
    QAction *      toggleViewAction();

public slots:
    bool           close();
    void           closeEvent(QCloseEvent *);

signals:
    void           visibilityChanged(bool visible);

private:
    void           choosePosition();
    bool           restoreSavedGeometry();
    Qt::WindowFlags flags()
    {
        Qt::WindowFlags flags = windowFlags();
        flags |= Qt::CustomizeWindowHint;
        flags &= ~Qt::WindowMaximizeButtonHint;
        return flags;
    }

private:
    bool           needPlacement;
    QAction *      toggleView;

private:
    static QPoint  where;
    static int     screenH;
};

}

#endif // TOOL_WINDOW_H
