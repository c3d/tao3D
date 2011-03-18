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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao.h"
#include <QDialog>
#include <QPoint>

namespace Tao {


class ToolWindow : public QDialog
// ----------------------------------------------------------------------------
//   A window with small title bar and borders suitable for tools
// ----------------------------------------------------------------------------
{
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
