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
 * Menus are structured as follows:
 *   @li The menu bar (made current by @ref menubar) contains top-level menus
 *       (created or made current by @ref menu)
 *   @li A top-level menu may contain menu items (created or made current by
 *       @ref menu_item) and/or sub-menus (create or made current by
 *       @ref submenu).
 *   @li A sub-menu, like top-level menus, may contain sub-menus and menu
 *       items.
 *
 * Example (<a href="examples/menus.ddd">menus.ddd</a>):
 * @include menus.ddd
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
 * @see menubar to set the current %menubar
 */
tree menu(text name, text label, text icon);

/**
 * Adds a menu to the current menu or sub-menu and make it the current menu.
 *
 * This is a simplified form of submenu(text name, text label, text icon)
 * when you want a menu with no icon.
 *
 * @see menu to set the current top-level %menu \n
 *      submenu(text name, text label, text icon) to provide an icon
 */
tree submenu(text name, text label);

/**
 * Adds a menu to the current menu or sub-menu and make it the current menu.
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
 * @see menu, submenu to set the current %menu
 * @bug checked should be a boolean
 */
tree menu_item(text name, text label, text icon, boolean checkable, text checked, code action);

/**
 * Sets the menu bar attached to the current window as the current menu bar.
 *
 * When this primitive returns, the menu bar becomes current for
 * @ref separator.
 */
tree menubar();

/**
 * Adds a toolbar to the current widget at the specified location.
 * @p name is a text string that should uniquely identify this tool bar in the
 * application. @p label is the user-visible name of the toolbar, as it will
 * appear in the View menu. @p location specifies where the toolbar should
 * appear, relative to the current widget.
 * The possible values for this parameter are:
 *   @li @c "n" or @c "N" (north): the widget's top toolbar area
 *   @li @c "s" or @c "S" (south): the widget's bottom toolbar area
 *   @li @c "w" or @c "W" (west): the widget's left toolbar area
 *   @li @c "e" or @c "E" (east): the widget's right toolbar area
 *
 * When this primitive returns, the tool bar becomes current for
 * @ref separator.
 *
 * @todo Does @p label really appear in the View menu?
 * @todo Accept "top", "bottom", "left" and "right" for @p location?
 */
tree toolbar(text name, text label, text location);

/**
 * Adds a separator to the current menu or sub-menu or menu bar or tool bar.
 */
tree separator();

/**
 * @}
 */
