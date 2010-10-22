#ifndef TOOL_WINDOW_H
#define TOOL_WINDOW_H
// ****************************************************************************
//  tool_window.h                                                  Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao.h"
#include <QWidget>
#include <QPoint>

namespace Tao {


class ToolWindow : public QWidget
// ----------------------------------------------------------------------------
//   A window with small title bar and borders suitable for tools
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    ToolWindow(const QString &title, QWidget *parent = 0,
                   const QString &objName = "")
        : QWidget(parent, Qt::Tool), needPlacement(true)
    {
        setWindowTitle(title);
        if (!objName.isEmpty())
            setObjectName(objName);
    }

    ToolWindow(QWidget *parent = 0)
        : QWidget(parent, Qt::Tool), needPlacement(true)
    {
    }

    virtual void   setVisible(bool visible);
    bool           createVisible();
    bool           doClose();

public slots:
    bool           close();
    void           closeEvent(QCloseEvent *);

signals:
    void           visibilityChanged(bool visible);

private:
    void           choosePosition();
    bool           restoreSavedGeometry();

private:
    bool           needPlacement;

private:
    static QPoint  where;
    static int     screenH;
};

}

#endif // TOOL_WINDOW_H
