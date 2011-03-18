// ****************************************************************************
//  tool_window.cpp                                                Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

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
