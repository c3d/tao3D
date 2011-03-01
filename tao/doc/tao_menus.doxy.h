/**
 * @addtogroup TaoMenus Application menus
 * @ingroup TaoBuiltins
 *
 * Creates and inserts new menus into the Tao application window.
 *
 * With the primitives defined in this group, a document can create and insert
 * new menus in the menu bar of the Tao Presentations application. Specific
 * code can be executed when the menu is triggered. In addition, stateful menus
 * (with a check mark: enable or disabled) are supported.
 *
 * Example:
 @verbatim
// menus.ddd

menubar
menu "mymenu:1", "My Menu"
menu_item "mymenu:1.1", "Select me",
    writeln 'Menu "Select me" was selected'
submenu "menu:1.2", "Submenu"
menu_item "menu:1.2.1", "Toggle me", "", true, "false",
    writeln 'Menu "Toggle me" selected'
 @endverbatim
 *
 * The screen shot below shows the result. The menu "Select me" was clicked once,
 * then "Toggle me" was activated, and shown again.
 * @image html menus.png "Creating application menus from a document"
 *
 * @{
 */

/**
 * Adds a top-level menu to the current menu bar and make it the current menu.
 *
 * This is a simplified form of menu(text name, text label, text icon)
 * when you want a menu with no icon.
 *
 * @see menubar to set the current %menubar \n
 *      menu(text name, text label, text icon) to provide an icon
 */
tree menu(text name, text label);

/**
 * Adds a top-level menu to the current menu bar and make it the current menu.
 *
 * @a name is a text string that should uniquely identify this menu in the
 * Tao application. @a label is the visible menu string.
 * If the menu already exists, it is simply updated. If you want to
 * associate an icon to the menu, give the path to the icon file using the
 * @a icon parameter; otherwise supply an emtpy string. The icon path may be
 * absolute or relative to the current document.
 *
 * @see menubar to set the current %menubar
 */
tree menu(text name, text label, text icon);

/**
 * Adds a menu to the current menu and make it the current menu.
 *
 * This is a simplified form of submenu(text name, text label, text icon)
 * when you want a menu with no icon.
 *
 * @see menu to set the current top-level %menu \n
 *      submenu(text name, text label, text icon) to provide an icon
 */
tree submenu(text name, text label);

/**
 * Adds a menu to the current menu and make it the current menu.
 *
 * @a name is a text string that should uniquely identify this menu in the
 * Tao application. @a label is the visible menu string.
 * If the menu already exists, it is simply updated. If you want to
 * associate an icon to the menu, give the path to the icon file using the
 * @a icon parameter; otherwise supply an emtpy string. The icon path may be
 * absolute or relative to the current document.
 *
 * @see menu to set the current top-level %menu
 */
tree submenu(text name, text label, text icon);

/**
 * Adds a menu item with an associated action to the current menu.
 *
 * This is a simplified version of
 * menu_item(text name, text label, text icon, boolean checkable, text checked, code action).
 *
 * @see menu, submenu to set the current %menu \n
 *      menu_item(text name, text label, text icon, boolean checkable, text checked, code action)
 */
tree menu_item(text name, text label, code action);

/**
 * Adds a menu item with an associated action to the current menu.
 *
 * @a name is a text string that should uniquely identify this menu item in the
 * Tao application. @a label is the visible menu item string.
 * If the menu item already exists, it is simply updated. If you want to
 * associate an icon to the menu item,  give the path to the icon file using the
 * @a icon parameter; otherwise supply an emtpy string. The icon path may be
 * absolute or relative to the current document.
 *
 * The action to execute when the menu is clicked should be given as an indented
 * block of code (see the example in this group's description).
 *
 * A menu item can have a check box to indicate it has a state and may be
 * changed from checked to unchecked and vice-versa. To create a checkable item,
 * set @a checkable to true. In this case, set the @a checked parameter to "true"
 * to have a check mark be displayed initially.
 *
 * @see menu, submenu to set the current %menu
 * @bug checked should be a boolean
 */
tree menu_item(text name, text label, text icon, boolean checkable, text checked, code action);

/**
 * Sets the menu bar attached to the current window as the current menu bar.
 */
tree menubar();

/**
 * @}
 */
