/**
 * @addtogroup mouse Mouse and selection
 * @ingroup TaoBuiltins
 * This group deals with mouse location, mouse events and item
 * selection.
 *
 * @{
 */
/**
 * Associates the specified code to the named event.
 *
 * Associate the specified code @p t and the action named @p n. This code is
 * executed when the action occurs on the current @c shape or @c active_widget.
 * Currently known event name are
 * - @c click
 * - @c mouseover
 */
on (n:text, t:tree);

/**
 * Associates code with click.
 *
 *  Associate the specified code to the click.
 *  It is equivalent to @code on "click", t @endcode
 */
on_click (t:tree);

/**
 * Associates code with mouse over.
 *
 *  Associate the specified code to the mouse over.
 *  It is equivalent to @code on "mouseover", t @endcode
 */
on_mouseover (t:tree);

/**
 * Return the buttons of the last mouse event.
 *
 *  It allows to access to the last mouse button event. It is an OR combination of following values:<BR>
 * <TABLE>
 *<TR><TH> Qt name</TH> <TH>Hexadecimal value</TH><TH>Description</TH> </TR>
 *<TR><TD> Qt::NoButton</TD><TD> 0x00000000</TD><TD>The button state does not refer to any button (see QMouseEvent::button()).</TD></TR>
 *<TR><TD> Qt::LeftButton</TD><TD> 0x00000001</TD><TD>The left button is pressed, or an event refers to the left button. (The left button may be the right button on left-handed mice.)</TD></TR>
 *<TR><TD> Qt::RightButton</TD><TD> 0x00000002</TD><TD>The right button.</TD></TR>
 *<TR><TD> Qt::MidButton</TD><TD> 0x00000004</TD><TD>The middle button.</TD></TR>
 *<TR><TD> Qt::XButton1</TD><TD> 0x00000008</TD><TD>The first X button.</TD></TR>
 *<TR><TD> Qt::XButton2</TD><TD> 0x00000010</TD><TD>The second X button.</TD></TR>
 * </TABLE>
 * @returns Return the buttons of the last mouse event
 */
mouse_buttons ();

/**
 * Returns the current X position of the mouse in the local frame.
 * The returned value is the X coordinate of the mouse pointer after
 * projection in the plane Z=0 for the current geometric transformation.
 * In other words, if you draw a point at coordinates (mouse_x, mouse_y)
 * it will always follow the mouse. But the value you get is usually not
 * the position of the mouse in screen coordinates.
 *
 * Note however that the default zoom level and camera position are such
 * that mouse_y corresponds to pixels with (X=0, Y=0) being the center of
 * the screen, the positive X-axis pointing right, and the positive Y-axis
 * pointing up.
 * @see screen_mouse_x
 */
mouse_x ();

/**
 * Returns the current Y position of the mouse in the local frame.
 * The returned value is the Y coordinate of the mouse pointer after
 * projection in the plane Z=0 for the current geometric transformation.
 * In other words, if you draw a point at coordinates (mouse_x, mouse_y)
 * it will always follow the mouse. But the value you get is usually not
 * the position of the mouse in screen coordinates.
 *
 * Note however that the default zoom level and camera position are such
 * that mouse_y corresponds to pixels with (X=0, Y=0) being the center of
 * the screen, the positive X-axis pointing right, and the positive Y-axis
 * pointing up.
 * @see screen_mouse_y
 */
mouse_y ();

/**
 * Returns the current X position of the mouse in screen coordinates.
 * Coordinates are in pixels. The left of the display area is at X coordinate
 * 0, while the right is at X = @ref window_width.
 */
screen_mouse_x ();

/**
 * Returns the current Y position of the mouse in screen coordinates.
 * Coordinates are in pixels. The bottom of the display area is at Y coordinate
 * 0, while the top is at X = @ref window_height.
 */
screen_mouse_y ();


/**
 * Mouse wheel handling.
 * With control key down, zoum in or out. Otherwise call @c pan
 */
wheel_event (x, y);


/**
 * Enable or disable the selection rectangle.
 * This primitive controls whether a selection rectangle is drawn as the left
 * mouse button is pressed and the mouse is dragged.
 */
enable_selection_rectangle (on:boolean);

/** @} */
