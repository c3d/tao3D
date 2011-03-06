/**
 * @addtogroup misc Miscellaneous
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
 * This function simply returns a copy of @a x.
 *
 * @param x [real] The value in pixel
 * @returns The value in pixel. 
 */
x ( px);

/**
 * Convert from points (pt) to pixels.
 *
 * @a x is a value in PostScript points (1/72 inch).
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
 * The current layout (that is, the enclosing @ref locally or @a shape) will
 * be re-executed every @a interval seconds.
 */
refresh (interval:real);


/**
 * Exits the Tao application.
 *
 * @a n is the exit status of the process.
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
 * @a t must be a valid version identifier: branch name, tag name or commit ID.
 * @see current_repository.
 * @returns True if the checkout succeded, false otherwise.
 */
checkout (t:text);


/**
 * Checks if a Tao feature is available with the current Tao version.
 *
 * @a name is the name of the feature to test. Valid feature names are:
 *   @li @b git document versioning with Git
 *   @li @b stereoscopy support for stereoscopic 3D displays (active or passive
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



/** @} */
