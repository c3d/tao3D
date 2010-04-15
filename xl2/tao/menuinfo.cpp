// ****************************************************************************
//  menuinfo.cpp                                                    Tao project
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

#include "tao.h"
#include "menuinfo.h"
#include "options.h"
#include <iostream>

TAO_BEGIN

MenuInfo::MenuInfo(QString name, QWidget *wid, QAction *act) :
        fullname(name), p_parent(wid), p_action(act),
        p_window(NULL), p_toolbar(NULL)
{
    connect(p_action, SIGNAL(destroyed(QObject *)),
            this, SLOT(actionDestroyed(QObject*)));
}

MenuInfo::MenuInfo(QString name, QMainWindow *win, QToolBar *bar) :
        fullname(name), p_parent(NULL), p_action(NULL),
        p_window(win), p_toolbar(bar)
{
    connect(p_toolbar, SIGNAL(destroyed(QObject *)),
            this, SLOT(actionDestroyed(QObject*)));
}

MenuInfo::~MenuInfo()
// ----------------------------------------------------------------------------
//   Delete a menu entry
// ----------------------------------------------------------------------------
{

    QObject *tmp_par = NULL;
    if (p_action)
    {
 //       p_parent->removeAction(p_action);
        delete p_action;
        p_action = NULL;
        tmp_par = p_parent;
    }
    if (p_toolbar)
    {
//        p_window->removeToolBar(p_toolbar);
        delete p_toolbar;
        p_toolbar = NULL;
        tmp_par = p_window;
    }

    IFTRACE(menus)
    {
        std::cerr << fullname.toStdString() << " deleted";
        if (tmp_par)
        {
            if (tmp_par->findChild<QObject*>(fullname))
                std::cerr << " still registered \n";
            else
                std::cerr << " well removed from parent \n";
        }
        else
        {
            std::cerr << "No more action nor toolbar \n";
        }

    }

    actionDestroyed(NULL);
    fullname = "";
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
void MenuInfo::actionDestroyed(QObject *obj)
{
    IFTRACE(menus)
    {
        std::cerr << fullname.toStdString() << " actionDestroyed \n";
    }
    p_action   = NULL;
    p_parent   = NULL;
    p_window   = NULL;
    p_toolbar  = NULL;
}

XL::Tree * CleanMenuInfo::Do(XL::Tree *what)
// ----------------------------------------------------------------------------
//   Purge all menu infos
// ----------------------------------------------------------------------------
{
    if (what)
        what->Purge<MenuInfo>();
    return what;
}

TAO_END
