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

#include <QAbstractButton>
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
    MenuInfo(QString name, QAction *act);
    MenuInfo(QString name, QToolBar *bar);
    ~MenuInfo();

public:
    QString        fullname; // the widget full name.
    QAction      * p_action; // The action associated with the widget
    QToolBar     * p_toolbar; // The toolbar
    text           title;
    text           icon;

public slots:
    void actionDestroyed();

};


struct GroupInfo : QButtonGroup, XL::Info
// ----------------------------------------------------------------------------
// QGroupButton associated to an XL tree
// ----------------------------------------------------------------------------
{
    Q_OBJECT;

public:
    typedef GroupInfo * data_t;

    GroupInfo(XL::Tree *t, QWidget * parent) :
            QButtonGroup(parent), XL::Info(), tree(t), action(NULL)
    {
        connect(this, SIGNAL(buttonClicked(QAbstractButton*)),
                this, SLOT(bClicked(QAbstractButton*)));
    }
    ~GroupInfo()
    {
        if (tree)
            tree->Remove(this);
    }

    operator data_t() { return this; }

public slots:
    void bClicked(QAbstractButton* button);

public:
    XL::Tree *tree;
    XL::TreeRoot *action;
};

}

#endif // MENUINFO_H
