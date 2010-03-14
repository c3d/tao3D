#include "tao.h"
#include "menuinfo.h"
#include <iostream>

TAO_BEGIN

MenuInfo::MenuInfo(QMenu *menu, std::string name)
    : menu(menu), action(NULL), menubar(NULL), fullName(name)
{
    std::cout<<"MenuInfo : "<< name<< " menuItem cree, "
            << menu->children().size()
            << " children present in the menu\n";
}

MenuInfo::MenuInfo(QMenuBar *menubar, QMenu *menu, std::string name)
    : menubar(menubar), action(NULL), menu(menu),fullName(name)
{
    std::cout<<"MenuInfo : "<< name<< " menu cree, "
            << menubar->children().size()
            << " children present in the menubar\n";
}


MenuInfo::~MenuInfo()
{
    std::cout << "Dest MenuInfo " << fullName << "\n";
}

TAO_END
