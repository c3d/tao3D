#ifndef MENUINFO_H
#define MENUINFO_H

#include <string>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include "tree.h"

TAO_BEGIN

struct MenuInfo : public XL::Info
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


TAO_END

#endif // MENUINFO_H
