/**
 * @defgroup shape_handling Shape handling
 * @ingroup Graphics
 *
 * Tools to group, play with forward / backward, etc...
 * 
 * @{
 */

/**
 * Creates a logical group around shapes.
 * The group can then be transformed as one object.
 */
group(shapes:tree);

/**
 * Groups the selection.
 * The selected shapes are placed in a @ref group .
 * This function is to be used once from a graphical interface and not aimed to be placed in a document.
 */
group_selection();

/**
 * Ungroups the selection.
 * The selected group is ungrouped.
 * This function is to be used once from a graphical interface and not aimed to be placed in a document.
 */
ungroup_selection();


/**
 * Bring forward.
 * Bring the selected shape one place forward.
 * This function is to be used once from a graphical interface and not aimed to be placed in a document.
 */
bring_forward();

/**
 * Bring to front.
 * Bring the selected shape to front place.
 * This function is to be used once from a graphical interface and not aimed to be placed in a document.
 */
bring_to_front();

/**
 * Send backward.
 * Send the selected shape one place backward.
 * This function is to be used once from a graphical interface and not aimed to be placed in a document.
 */
send_backward();

/**
 * Send to back.
 * Send the selected shape to the last place.
 * This function is to be used once from a graphical interface and not aimed to be placed in a document.
 */
send_to_back();

/**
 * Set zNear.
 */
z_near(zn:real);

/**
 * Get zNear.
 * @return [real] current zNear value.
 */
z_near();

/**
 * Set zFar.
 */
z_far(zf:real);

/**
 * Get zFar.
 * @return [real] current zFar value.
 */
z_far();

/**
 * Set the polygon offset factors.
 * @param f0 factor base
 * @param f1, factor increment
 * @param u0, unit base
 * @param u1, unit increment
 * @return [integer] The current polygon offset.
 */
polygon_offset(f0:real, f1:real, u0:real, u1:real);


/**
 * Set an attribute value.
 * It set or update an attribute of a shape or of a group. 
 * The @p attribute identifies the attribute to add or modify and @p value is 
 * the code that represent the attribute value.
 * This function is to be used once from a graphical interface and not aimed to be placed in a document.
 *
 * It is used by color chooser action to set the color (or line_color) of shapes.
 */
set_attribute(attribute:text, value:tree);

/**
 * Delete selection.
 * If a text is selected, replace the current selected text by the @p key value.
 * If it is a non text element that is selected, just suppresses the element.
 */
delete_selection(key:text);
/**
 * @}
 */
