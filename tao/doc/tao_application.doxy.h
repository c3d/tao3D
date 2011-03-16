/**
 * @addtogroup TaoGuiInter Tao Interface Interaction
 * @ingroup TaoGui
 * Tao Presentations interface.
 * Tao Presentation graphical interface can be modified by an XL document.
 * @{
 */


/**
 * Set the message in the status bar.
 *
 * Show text @p t until next message.
 */
status(t:text);

/**
 * Controls source code panel.
 *
 * @p src true show the panel, @p src false hide it.
 */
show_source(src:boolean);


/**
 * Controls error panel.
 *
 * @p err true show the panel, @p err false hide it.
 */
show_error(err:boolean);


/**
 * Controls time dependant animations.
 *
 * @p animations true run animations, @p animations false stop them.
 */
enable_animations(animations:boolean);


/**
 * Controls slide-show mode.
 *
 * Activate or deactivate slide-show mode, depending on 
 * the value of the flag. When activated, the document is
 * displayed full-screen, the cursor hides automatically
 * and the screensaver is deactivated.
 * @returns [boolean] A flag indicating the previous value of the slide-show mode.
 */
slide_show(show:boolean);


/**
 * Controls the display of rendering statistics.
 *
 * Shows or hides fps statistics.
 */
show_statistics(flag:boolean);


/**
 * Reset document view to default parameters.
 *
 * @todo Which parameters are reset, modified by which primitives ?
 */
reset_view();


/**
 * Get zoom factor.
 * @return [real] the current zoom factor.
 */
zoom();


/**
 * Zoom the view.
 *
 * Zoom is not scaling. This primitive zoom the view by the @p factor.
 *
 */
zoom( factor:real);


/**
 * Zoom out.
 *
 * Zoom out by a factor of 0.25
 */
zoom_out();


/**
 * Zoom in.
 *
 * Zoom in by a factor of 0.25
 */
zoom_in();


/**
 * Toggle the cursor.
 *
 * Toggle between hand (panning) and arrow (moving) cursors.
 */
toggle_hand_cursor();


/**
 * Show or hide elements.
 *
 * Gradualy show or hide the current layout. @p amount of global transparency.
 */
show(amount:real);

/** @} */


/**
 * @addtogroup stereoscopy Stereoscopie
 * @ingroup TaoGui
 * Deals with stereoscopy capabilities of Tao Presentations.
 *
 * @{
 */
/**
enable_stereoscopy
eye_distance
stereoscopy_settings
*/
/** @} */
