/**
 * @addtogroup code
 * @ingroup TaoBuiltins
 * This group deals with some helper functionnalities.
 * Some of the following functions are postfix ones and are to be used like units :
 * - mm
 * - cm
 * - px
 * - pt
 * - inch
 *
 * If you want to specify a translation (which takes pixels) along X-axis of 2 inches, just use
 * @code translatex 2 inch @endcode
 * @note The size on the screen may not be exactly the one requested, but it is when printing.
 *
 * @{
 */
/**
 * Converts from millimeters to pixels.
 *
 * @param x [real] The value in mm
 * @returns The value in pixel. 
 */
x ( mm);

/**
 * Converts from centimeters to pixels.
 *
 * @param x [real] The value in cm
 * @returns The value in pixel. 
 */
x ( cm);

/**
 * Converts from pixels to pixels.
 *
 * This function simply returns a copy of @p x.
 *
 * @param x [real] The value in pixel
 * @returns The value in pixel. 
 */
x ( px);

/**
 * Convert from points (pt) to pixels.
 *
 * @p x is a value in PostScript points (1/72 inch).
 * @param x [real] The value in point
 * @returns The value in pixel. 
 */
x ( pt);

/**
 * Converts from inches to pixels.
 *
 * @param x [real] The value in inch
 * @returns The value in pixel. 
 */
x ( inch);


/**
 * Marks the tree that it cannot be modified afterward.
 *
 * @param t [tree] the tree that cannot be modify.
 * @returns the tree itself.
 */
constant (t:tree);

/**
 * Sets the refresh time for the current loayout.
 *
 * The current layout (that is, the enclosing @ref locally or @p shape) will
 * be re-executed every @p interval seconds.
 */
refresh (interval:real);


/**
 * Exits the Tao application.
 *
 * @p n is the exit status of the process.
 * @bug #771 No cleanup is performed; the size and positions of Tao windows
 * are not saved.
 */
exit (n:integer);


/**
 * Creates a local context.
 *
 * Evaluate the child tree while preserving the current graphical state.
 * Any state modification like color or translation done in the 
 * locally block only affect the block.
 */
locally (t:tree);

/**
 * Returns true if the current document is stored in a repository.
 *
 * Returns true if the current document is stored in a repository,
 * Git functionality is compiled in, and not disabled on the command line.
 */
current_repository ();


/**
 * Checks out a version of the current document.
 *
 * Checkout a branch or a commit from the git repository, if available.
 * @p t must be a valid version identifier: branch name, tag name or commit ID.
 * @see current_repository.
 * @returns True if the checkout succeded, false otherwise.
 */
checkout (t:text);


/**
 * Checks if a Tao feature is available with the current Tao version.
 *
 * @p name is the name of the feature to test. Valid feature names are:
 *   - @b git document versioning with Git
 *   - @b stereoscopy support for stereoscopic 3D displays (active or passive
 *       glasses, for instance)
 *
 * @returns true if the feature is compiled in, false otherwise.
 */
is_available (name:text);

/**
 * Checks if we are currently rendering to file.
 */
offline_rendering ();

/**
 * Disables the OpenGL depth test for polygons.
 * @returns The previous state of depth testing
 */
disable_depth_test ();

/**
 * Enables the OpenGL depth test for polygons.
 * @returns The previous state of depth testing
 */
enable_depth_test ();

/**
 * Send a key to text edition.
 * Send a key event to any available text selection action.
 */
key(keyname:text);

/**
 * Return the current keyboard modifiers.
 * The keyboard modifiers are a bitwise or of the following values
 *  - 0x02000000	One of the Shift keys
 *  - 0x04000000	One of the Ctrl / Control keys
 *  - 0x08000000	One of the Alt keys
 *  - 0x10000000	One of the Meta keys
 *  - 0x20000000	A keypad button
 *  - 0x40000000	Mode_switch key (X11 only)
 *  
 * @note On Mac OS X, the Control modifier corresponds to
 * the Command keys on the Macintosh keyboard, and the
 * Meta modifier corresponds to the Control keys.
 * The Keypad modifier will also be set when an arrow key
 * is pressed as the arrow keys are considered part of
 * the keypad.
 *
 * @note On Windows keyboards, the Meta modifier is mapped
 *  to the optional Windows key.
 */
keyboard_modifiers();

/**
 * get control modifier
@code
control_modifier  -> (keyboard_modifiers and 16#04000000) <> 0
@endcode
 */
control_modifier();

/**
 * get shift modifier
@code
shift_modifier    -> (keyboard_modifiers and 16#02000000) <> 0
@endcode
 */
shift_modifier();

/**
 * get alt modifier
@code
alt_modifier      -> (keyboard_modifiers and 16#08000000) <> 0
@endcode
 */
alt_modifier();

/**
 * get meta modifier
@code
meta_modifier     -> (keyboard_modifiers and 16#10000000) <> 0
@endcode
 */
meta_modifier();

/** @} */
