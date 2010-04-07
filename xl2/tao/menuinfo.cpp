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

MenuInfo::MenuInfo(QMenu *menu, std::string name)
// ----------------------------------------------------------------------------
//   Associated a menu entry to a tree
// ----------------------------------------------------------------------------
    : fullName(name), menu(menu), menubar(NULL), action(NULL)
{
    IFTRACE(menus)
        std::cout<<"MenuInfo : "<< name << " menuItem created, "
                 << menu->children().size()
                 << " children present in the menu\n";
}

MenuInfo::MenuInfo(QMenuBar *menubar, QMenu *menu, std::string name)
// ----------------------------------------------------------------------------
//    Associate a menu bar entry to a tree
// ----------------------------------------------------------------------------
    : fullName(name), menu(menu), menubar(menubar), action(NULL)
{
    IFTRACE(menus)
        std::cout<< "MenuInfo : " << name<< " menu created\n";

}


MenuInfo::~MenuInfo()
// ----------------------------------------------------------------------------
//   Delete a menu entry
// ----------------------------------------------------------------------------
{
    IFTRACE(menus)
        std::cout << "Delete MenuInfo for " << fullName << "\n";
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
