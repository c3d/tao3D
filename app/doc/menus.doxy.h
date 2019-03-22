// *****************************************************************************
// menus.doxy.h                                                    Tao3D project
// *****************************************************************************
//
// File description:
//
//
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2011, Catherine Burvelle <catherine@taodyne.com>
// (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011-2012, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************
/**
 * @~english
 * @addtogroup TaoGui Application User Interface
 * @ingroup TaoBuiltins
 *
 * User Interface manipulation of Tao Applications.
 * @~french
 * @addtogroup TaoGui Interface Graphique
 * @ingroup TaoBuiltins
 *
 * Fonctions permettant de manipuler l'interface graphique de Tao
 * Presentations.
 */
/**
 * @~english
 * @addtogroup TaoMenus Application menus
 * @ingroup TaoGui
 *
 * Creates and inserts new menus into the Tao application window.
 *
 * With the primitives defined in this group, a document can create and insert
 * new menus in the menu bar of the Tao3D application. Specific
 * code can be executed when the menu is triggered. In addition, stateful menus
 * (with a check mark: enable or disabled) are supported.
 *
 * Menus are structured as follows:
 *   - A top level element that is either
 *       - the application menu bar (made current by @ref menubar)<BR>
 *         or
 *       - a created tool bar (made current by @ref toolbar).<BR>
 *         or
 *       - a contextual menu
 *
 *   - A top-level menu may contain menu items (created or made current by
 *       @ref menu_item) and/or sub-menus (create or made current by
 *       @ref submenu). The top-level menu is either added to the menubar,
 *       a toolbar or registered as a contextual menu.
 *   - A sub-menu, like top-level menus, may contain sub-menus and menu
 *       items.
 *
 * Example (<a href="examples/menus.ddd">menus.ddd</a>):
 * @include menus.ddd
 *
 * The screen shot below shows the result. The menu "Select me" was clicked once,
 * then "Toggle me" was activated, and shown again.
 * @image html menus.png "Creating application menus from a document"
 *
 * @~french
 * @addtogroup TaoMenus Menus applicatifs
 * @ingroup TaoGui
 *
 * Crée des menus et les insère dans la fenêtre de l'application Tao.
 *
 * Les primitives décrites sur cette page permettent de créer des menus et de
 * les insérer dans la barre de menus de l'application Tao3D.
 * Lorsque le menu est sélectionné, le code Tao associé est exécuté.
 * De plus, il est possible de créer des menus avec une marque de sélection
 * (fonction activée / désactivée).
 *
 * Les menus sont structurés comme suit :
 *   - Il existe un menu racine, qui est soit :
 *       - la barre de menu de l'application (sélectionnée par
 *         @ref menubar)<BR>
 *         ou
 *       - une barre d'outil (sélectionnée par @ref toolbar)<BR>
 *         ou
 *       - un menu contextuel.
 *   - Un menu racine peut contenir des éléments de menu (créés ou
 *     sélectionnés par @ref menu_item) et/ou des sous-menus (créés ou
 *     sélectionnés par @ref submenu).
 *   - Un sous-menu peut à son tour contenir d'autres sous-menus et des
 *     éléments de menu (comme les menus racine).
 *
 * Voici un exemple : (<a href="examples/menus.ddd">menus.ddd</a>):
 * @include menus.ddd
 *
 * La capture d'écran qui suit montre le résultat. Le menu <em>Select me</em>
 * a été cliqué une fois, puis <em>Toggle me</em> a été activé, et enfin le
 * menu a été de nouveau déroulé.
 * @image html menus.png "Création de menus depuis un document"
 *
 * @~
 * @{
 */

/**
 * @~english
 * Make the named menu current.
 *
 * This is a simplified form of menu(name:text, label:text, icon:text)
 * when you just want to make an existing menu current.
 *
 * @see menubar to set the current %menubar \n
 *      menu(name:text, label:text, icon:text) to create a new menu
 *
 * @~french
 * Fait du menu spécifié le menu en cours.
 *
 * C'est une forme simplifiée de menu(name:text, label:text, icon:text)
 * utile lorsque vous voulez simplement sélectionner un menu pour y ajouter des
 * éléments, par exemple.
 *
 * @see menubar pour sélectionner la barre de %menu courante \n
 *      menu(name:text, label:text, icon:text) pour créer un nouveau %menu
 */
menu(name:text);

/**
 * @~english
 * Adds a top-level menu to the current menu bar and make it the current menu.
 *
 * This is a simplified form of menu(name:text, label:text, icon:text)
 * when you want a menu with no icon.
 *
 * @see menubar to set the current %menubar \n
 *      menu(name:text, label:text, icon:text) to provide an icon
 *
 * @~french
 * Ajoute un menu racine à la barre de menu courante et en fait le menu en cours.
 *
 * C'est une forme simplifiée de menu(name:text, label:text, icon:text)
 * utile lorsque vous voulez créer un menu sans icône.
 *
 * @see menubar pour sélectionner la barre de %menu courante \n
 *      menu(name:text, label:text, icon:text) pour préciser une icône
 */
menu(name:text, label:text);

/**
 * @~english
 * Adds a top-level menu to the current menu bar and make it the current menu.
 *
 * @p name is a text string that should uniquely identify this menu in the
 * Tao application. @p label is the visible menu string.
 * If the menu already exists, it is simply updated. If you want to
 * associate an icon to the menu, give the path to the icon file using the
 * @p icon parameter; otherwise supply an emtpy string. The icon path may be
 * absolute or relative to the current document.
 *
 * When this primitive returns, the menu becomes current for @ref submenu
 * and @ref separator. Call @ref menu or @ref submenu again to make another
 * menu current.
 *
 * To use contextual menus, make them current using their names :
 *    - "TAO_CONTEXT_MENU"
 *    - "TAO_CONTEXT_MENU_SHIFT"
 *    - "TAO_CONTEXT_MENU_CONTROL"
 *    - "TAO_CONTEXT_MENU_ALT"
 *    - "TAO_CONTEXT_MENU_META"
 *
 * @see menubar to set the current %menubar
 *
 * @~french
 * Ajoute un menu racine à la barre de menu courante et en fait le menu en cours.
 *
 * @p name est une chaîne de caractère qui doit identifier le menu de manière
 * unique dans l'application Tao. @p label est le texte visible dans le menu.
 * Si le menu existe déjà, il est mis à jour. Si vous souhaitez associer une
 * icône au menu, donnez le chemin vers le fichier icône grâce au paramètre
 * @p icon, sinon passez une chaîne vide. Le chemin vers le fichier icône
 * peut être absolu, ou relatif au document courant.
 *
 * Après l'appel à cette primitive, le menu devient le menu courant pour toute
 * action @ref submenu et @ref separator. Appelez @ref menu ou @ref submenu
 * à nouveau pour faire d'un autre menu le menu courant.
 *
 * Pour utiliser les menus contextuels, utilisez les noms suivants :
 *    - "TAO_CONTEXT_MENU"
 *    - "TAO_CONTEXT_MENU_SHIFT"
 *    - "TAO_CONTEXT_MENU_CONTROL"
 *    - "TAO_CONTEXT_MENU_ALT"
 *    - "TAO_CONTEXT_MENU_META"
 *
 * @see menubar pour sélectionner la barre de %menu courante
 */
menu(name:text, label:text, icon:text);

/**
 * @~english
 * Make the named submenu current.
 *
 * This is a simplified form of submenu(name:text, label:text, icon:text)
 * when you just want to make an existing submenu current.
 *
 * @see submenu(name:text, label:text, icon:text) to create a new submenu
 *
 * @~french
 * Fait du sous-menu spécifié le menu en cours.
 *
 * C'est une forme simplifiée de submenu(name:text, label:text, icon:text)
 * utile lorsque vous voulez simplement sélectionner un sous-menu pour y
 * ajouter des éléments, par exemple.
 *
 * @see submenu(name:text, label:text, icon:text) pour créer un nouveau
 * %submenu
 */
submenu(name:text);

/**
 * @~english
 * Adds a sub-menu to the current menu or sub-menu and make it the current
 * menu.
 *
 * This is a simplified form of submenu(name:text, label:text, icon:text)
 * when you want a sub-menu with no icon.
 *
 * @see menu to set the current top-level %menu \n
 *      submenu(name:text, label:text, icon:text) to provide an icon
 *
 * @~french
 * Ajoute un sous-menu au menu ou sous-menu courant et en fait le menu en cours.
 *
 * C'est une forme simplifiée de submenu(name:text, label:text, icon:text)
 * utile lorsque vous voulez créer un sous-menu sans icône.
 */
submenu(name:text, label:text);

/**
 * @~english
 * Adds a sub-menu to the current menu or sub-menu and make it the current
 * menu.
 *
 * @p name is a text string that should uniquely identify this menu in the
 * Tao application. @p label is the visible menu string.
 * If the menu already exists, it is simply updated. If you want to
 * associate an icon to the menu, give the path to the icon file using the
 * @p icon parameter; otherwise supply an emtpy string. The icon path may be
 * absolute or relative to the current document.
 *
 * When this primitive returns, the sub-menu becomes current for @ref submenu
 * and @ref separator. Call @ref menu or @ref submenu again to make another
 * menu current.
 *
 * @see menu to set the current top-level %menu
 *
 * @~french
 * Ajoute un sous-menu au menu ou sous-menu courant et en fait le menu en
 * cours.
 *
 * @p name est une chaîne de caractère qui doit identifier le menu de manière
 * unique dans l'application Tao. @p label est le texte visible dans le menu.
 * Si le menu existe déjà, il est mis à jour. Si vous souhaitez associer une
 * icône au menu, donnez le chemin vers le fichier icône grâce au paramètre
 * @p icon, sinon passez une chaîne vide. Le chemin vers le fichier icône
 * peut être absolu, ou relatif au document courant.
 *
 * Après l'appel à cette primitive, le menu devient le menu courant pour toute
 * action @ref submenu et @ref separator. Appelez @ref menu ou @ref submenu
 * à nouveau pour faire d'un autre menu le menu courant.
 *
 * @see menu pour sélectionner le %menu racine
 */
submenu(name:text, label:text, icon:text);

/**
 * @~english
 * Adds a menu item with an associated action to the current menu.
 *
 * This is a simplified version of
 * menu_item(name:text, label:text, icon:text, checkable:boolean, checked:text ,action:tree).
 *
 * @see menu, submenu to set the current %menu \n
 *      menu_item(name:text, label:text, icon:text, checkable:boolean, checked:text, action:tree)
 *
 * @~french
 * Ajoute une entrée de menu avec une action au menu en cours.
 *
 * C'est une forme simplifiée de
 * menu_item(name:text, label:text, icon:text, checkable:boolean, checked:text, action:tree).
 *
 * @see menu, submenu pour sélectionner le %menu courant \n
 *      menu_item(name:text, label:text, icon:text, checkable:boolean, checked:text, action:tree)
 */
menu_item(name:text, label:text, action:tree);

/**
 * @~english
 * Adds a menu item with an associated action to the current menu.
 *
 * This is a simplified version of
 * menu_item(name:text, label:text, icon:text, checkable:boolean, checked:text, action:tree).
 *
 * @see menu, submenu to set the current %menu \n
 *      menu_item(name:text, label:text, icon:text, checkable:boolean, checked:text, action:tree)
 *
 * @~french
 * Ajoute une entrée de menu avec une action au menu en cours.
 *
 * C'est une forme simplifiée de
 * menu_item(name:text, label:text, icon:text, checkable:boolean, checked:text, action:tree).
 *
 * @see menu, submenu pour sélectionner le %menu courant \n
 *      menu_item(name:text, label:text, icon:text, checkable:boolean, checked:text, action:tree)
 */
menu_item(name:text, label:text, icon:text, action:tree);

/**
 * @~english
 * Adds a menu item with an associated action to the current menu.
 *
 * This is a simplified version of
 * menu_item(name:text, label:text, icon:text, checkable:boolean, checked:text, action:tree).
 *
 * @see menu, submenu to set the current %menu \n
 *      menu_item(name:text, label:text, icon:text, checkable:boolean, checked:text, action:tree)
 *
 * @~french
 * Ajoute une entrée de menu avec une action au menu en cours.
 *
 * C'est une forme simplifiée de
 * menu_item(name:text, label:text, icon:text, checkable:boolean, checked:text, action:tree).
 *
 * @see menu, submenu pour sélectionner le %menu courant \n
 *      menu_item(name:text, label:text, icon:text, checkable:boolean, checked:text, action:tree)
 */
menu_item(name:text, label:text, checkable:boolean, checked:text, action:tree);

/**
 * @~english
 * Adds a menu item with an associated action to the current menu.
 *
 * @p name is a text string that should uniquely identify this menu item in the
 * Tao application. @p label is the visible menu item string.
 * If the menu item already exists, it is simply updated. If you want to
 * associate an icon to the menu item,  give the path to the icon file using the
 * @p icon parameter; otherwise supply an emtpy string. The icon path may be
 * absolute or relative to the current document.
 *
 * The action to execute when the menu is clicked should be given as an indented
 * block of code (see the example in this group's description).
 *
 * A menu item can have a check box to indicate it has a state and may be
 * changed from checked to unchecked and vice-versa. To create a checkable item,
 * set @p checkable to true. In this case, set the @p checked parameter to "true"
 * to have a check mark be displayed initially.
 *
 * To get a selection sensitive item, the item @p name must start with @c menu:select:
 *
 * @see menu, submenu to set the current %menu
 *
 * @~french
 * Ajoute une entrée de menu avec une action au menu en cours.
 *
 * @p name est une chaîne de caractère qui doit identifier le menu de manière
 * unique dans l'application Tao. @p label est le texte visible dans le menu.
 * Si le menu existe déjà, il est mis à jour. Si vous souhaitez associer une
 * icône au menu, donnez le chemin vers le fichier icône grâce au paramètre
 * @p icon, sinon passez une chaîne vide. Le chemin vers le fichier icône
 * peut être absolu, ou relatif au document courant.
 *
 * L'action à exécuter lorsqu'on clique sur le menu est donnée sous forme d'un
 * bloc de code indenté (cf. l'exemple dans l'introduction de cette page).
 *
 * Un élément de menu peut avoir une case à cocher pour indiquer qu'il a un
 * état et qu'il peut être activé et désactivé. Pour créer un tel menu,
 * donnez la valeur @c true au paramètre @p checkable. Dans ce cas, donnez la
 * valeur @c true à @p checked pour que le menu soit initialement sélectionné.
 * @p checked est modifié lorsque le menu est cliqué.
 *
 * @see menu, submenu pour sélectionner le %menu courant
 */
menu_item(name:text, label:text, icon:text, checkable:boolean, checked:text, action:tree);

/**
 * @~english
 * Sets the menu bar attached to the current window as the current menu bar.
 *
 * @~french
 * Fait de la barre de menu de la fenêtre en cours le menu courant.
 */
menubar();

/**
 * @~english
 * Adds a toolbar to the current widget at the specified location.
 * @p name is a text string that should uniquely identify this tool bar in the
 * application. @p label is the user-visible name of the toolbar, as it will
 * appear in the View menu. @p location specifies where the toolbar should
 * appear, relative to the current widget.
 * The possible values for this parameter are:
 *   - @c "n" or @c "N" (north): the widget's top toolbar area
 *   - @c "s" or @c "S" (south): the widget's bottom toolbar area
 *   - @c "w" or @c "W" (west): the widget's left toolbar area
 *   - @c "e" or @c "E" (east): the widget's right toolbar area
 *
 * When this primitive returns, the tool bar becomes current.
 *
 * @~french
 * Ajoute une barre d'outil au widget en cours à l'endroit spécifié.
 * @p name est une chaîne de texte qui doit identifier la barre d'outil de
 * manière unique dans l'application. @p label est le nom de la barre d'outil
 * visible par l'utilisateur, tel qu'il apparaîtra dans le menu @c Affichage.
 * @p location permet de préciser où la barre d'outil doit apparaître. Les
 * valeurs possibles sont :
 *   - @c "n" ou @c "N" : en haut
 *   - @c "s" ou @c "S" : en bas
 *   - @c "w" ou @c "W" : à gauche
 *   - @c "e" ou @c "E" : à droite
 *
 * Après l'appel à cette primitive, la barre d'outil devient le menu courant.
 */
toolbar(name:text, label:text, location:text);

/**
 * @~english
 * Adds a visual separator to the current menu, sub-menu, menu bar or tool bar.
 * @~french
 * Ajoute un séparateur visuel.
 * Ajoute un séparateur au menu, au sous-menu, à la barre de menu ou à la barre
 * d'outil en cours.
 */
separator();

/**
 * @}
 */
