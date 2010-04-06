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
#include <QMenuBar>
#include <QAction>
#include "tree.h"

TAO_BEGIN

struct MenuInfo : public XL::Info
// ----------------------------------------------------------------------------
//  Qt menu data associated to an XL tree
// ----------------------------------------------------------------------------
{
public:
    MenuInfo(QMenu *menu, std::string name);
    MenuInfo(QMenuBar *menubar, QMenu *menu, std::string name);
    ~MenuInfo();

public:
    // Represent the name of the menu with its path separated with '|'
    std::string    fullName;
    QMenu        * menu; // This is the menu
    QMenuBar     * menubar;// This is the menubar
    QAction      * action;

};

struct CleanMenuInfo : XL::SimpleAction
{
    XL::Tree * Do(XL::Tree *what);

};

struct GroupInfo : public XL::Info, public QButtonGroup
// ----------------------------------------------------------------------------
// QGroupButton associated to an XL tree
// ----------------------------------------------------------------------------
{
    GroupInfo(QWidget * parent) : XL::Info(), QButtonGroup(parent){}
};

TAO_END

#endif // MENUINFO_H
