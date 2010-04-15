#ifndef MENUINFO_H
#define MENUINFO_H
// ****************************************************************************
//  menuinfo.h                                                      Tao project
// ****************************************************************************
//
//   File Description:
//
//    Associate Qt menu data to XL trees
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
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include <string>
#include <QButtonGroup>
#include <QMenu>
#include <QToolBar>
#include <QAction>
#include <QMainWindow>
#include "tree.h"


namespace Tao {

struct MenuInfo : QObject
// ----------------------------------------------------------------------------
//  Qt menu data.
//  This objects holds either a menu/submenu/menuitem using p_parent and
//    p_action or a toolbar using p_window and p_toolbar.
// ----------------------------------------------------------------------------
{
    Q_OBJECT;
public:
    MenuInfo(QString name, QWidget *wid, QAction *act);
    MenuInfo(QString name, QMainWindow *win, QToolBar *bar);
    ~MenuInfo();

public:
    QString        fullname; // the widget full name.
    QWidget      * p_parent; //  the parent hosting the menu, menuItem
    QAction      * p_action; // The action associated with the widget
    QMainWindow  * p_window; // the window hosting the toolbar (if any)
    QToolBar     * p_toolbar; // The toolbar

public slots:
    void actionDestroyed(QObject * obj);

};


struct CleanMenuInfo : XL::Action
// ----------------------------------------------------------------------------
//   Remove the MenuInfo from all the trees
// ----------------------------------------------------------------------------
{
    XL::Tree * Do(XL::Tree *what);
};

struct GroupInfo : QButtonGroup, XL::Info
// ----------------------------------------------------------------------------
// QGroupButton associated to an XL tree
// ----------------------------------------------------------------------------
{
    typedef GroupInfo * data_t;

public:
    GroupInfo(XL::Tree *t, QWidget * parent) :
            QButtonGroup(parent), XL::Info(), tree(t){}
    ~GroupInfo()
    {
        if (tree)
            tree->Remove(this);
    }

    operator data_t() { return this; }

public:
    XL::Tree *tree;
};

}

#endif // MENUINFO_H
