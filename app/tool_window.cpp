// *****************************************************************************
// tool_window.cpp                                                 Tao3D project
// *****************************************************************************
//
// File description:
//
//    Positioning tool windows.
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
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
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

#include "tool_window.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QSettings>
#include <QRect>
#include <QAction>

TAO_BEGIN


QPoint ToolWindow::where;
int    ToolWindow::screenH = 0;


void ToolWindow::choosePosition()
// ----------------------------------------------------------------------------
//   A dumb window placement algorithm, used only once (before pos is saved)
// ----------------------------------------------------------------------------
{
    if (screenH == 0)
    {
        QDesktopWidget * desktop = QApplication::desktop();
        QRect screenSize = desktop->screenGeometry(parentWidget());
        screenH = screenSize.height();
        where.setX(12);
        where.setY(parentWidget()->pos().y() + 24);
    }

    move(where);
    show();
    where += QPoint(0, frameGeometry().height() + 24);

    if (where.y() > screenH)
        where.setY(0);
}


void ToolWindow::setVisible(bool visible)
// ----------------------------------------------------------------------------
//   Show or hide a tool window, first call restores saved position
// ----------------------------------------------------------------------------
{
    bool wasVisible = isVisible();
    if (visible == wasVisible)
        return;

    if (needPlacement)
    {
        needPlacement = false;
        if (!restoreSavedGeometry())
            choosePosition();
    }
    QWidget::setVisible(visible);
    emit visibilityChanged(visible);
}


bool ToolWindow::createVisible()
// ----------------------------------------------------------------------------
//   Should the window be made visible when created? (read saved settings)
// ----------------------------------------------------------------------------
{
    bool ret = false;
    QSettings settings;
    settings.beginGroup("windows/" + objectName());
    ret = settings.value("visible", QVariant(false)).toBool();
    settings.endGroup();
    return ret;
}


bool ToolWindow::restoreSavedGeometry()
// ----------------------------------------------------------------------------
//   Read and apply previously saved position and size
// ----------------------------------------------------------------------------
{
    if (objectName().isEmpty())
        return false;

    QSettings settings;
    QByteArray geom;
    settings.beginGroup("windows/" + objectName());
    geom = settings.value("geometry").toByteArray();
    bool ok = restoreGeometry(geom);
    settings.endGroup();

    return ok;
}


bool ToolWindow::doClose()
// ----------------------------------------------------------------------------
//    (Really) close the window
// ----------------------------------------------------------------------------
{
    return QWidget::close();
}


bool ToolWindow::close()
// ----------------------------------------------------------------------------
//    Closing a tool window simply hides it
// ----------------------------------------------------------------------------
{
    hide();
    return true;
}


void ToolWindow::closeEvent(QCloseEvent *)
// ----------------------------------------------------------------------------
//   Save windows geometry on close
// ----------------------------------------------------------------------------
{
    QSettings settings;
    settings.beginGroup("windows/" + objectName());
    settings.setValue("geometry", saveGeometry());
    settings.setValue("visible", !isHidden());
    settings.endGroup();
}


QAction * ToolWindow::toggleViewAction()
// ----------------------------------------------------------------------------
//   Return a checkable action that can be used to show or hide the tool window
// ----------------------------------------------------------------------------
{
    if (!toggleView)
    {
        toggleView = new QAction(windowTitle(), this);
        toggleView->setCheckable(true);
        connect(toggleView, SIGNAL(triggered(bool)),
                this,       SLOT(setVisible(bool)));
        connect(this,       SIGNAL(visibilityChanged(bool)),
                toggleView, SLOT(setChecked(bool)));
    }
    return toggleView;
}

TAO_END
