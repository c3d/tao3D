/**
 * @addtogroup mouse
 * @ingroup TaoBuiltins
 * This group deals with mouse location and mouse events.
 *
 * Mouse location is expressed in pixel in the window. 
 * The coordinate (0,0) is the center of the window. 
 * - x > 0 goes toward the right
 * - y > 0 goes toward the top 
 *
 * The mouse has always z-coordinate equal to 0.
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
 * Returns the current X position of the mouse.
 */
mouse_x ();

/**
 * Returns the current Y position of the mouse.
 */
mouse_y ();

/**
 * Mouse wheel handling.
 * With control key down, zoum in or out. Otherwise call @c pan
 */
wheel_event (x, y);

/** @} */
