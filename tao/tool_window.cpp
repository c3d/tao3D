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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tool_window.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QSettings>
#include <QRect>

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

TAO_END
