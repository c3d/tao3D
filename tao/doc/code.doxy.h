/**
 * @addtogroup code
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
 * @fn x ( mm)
 * @brief Convert from mm to pixels
 *
 *  Convert from mm to pixels
 *
 * @param x [real] The value in mm
 * @returns The value in pixel. 
 */
x ( mm);

/**
 * @fn x ( cm)
 * @brief Convert from cm to pixels
 *
 *  Convert from cm to pixels
 *
 * @param x [real] The value in cm
 * @returns The value in pixel. 
 */
x ( cm);

/**
 * @fn x ( px)
 * @brief Convert from pixel
 *
 *  Convert from pixel to pixel. Currently returning a copy of the given value @c x.
 *
 * @param x [real] The value in pixel
 * @returns The value in pixel. 
 */
x ( px);

/**
 * @fn x ( pt)
 * @brief Convert from pt to pixels
 *
 *  Convert from pt to pixels
 *
 * @param x [real] The value in point
 * @returns The value in pixel. 
 */
x ( pt);

/**
 * @fn x ( inch)
 * @brief Convert from inch to pixels
 *
 *  Convert from inch to pixels
 *
 * @param x [real] The value in inch
 * @returns The value in pixel. 
 */
x ( inch);


/**
 * @fn constant ( t)
 * @brief a constant tree
 *
 *  Marks the tree that it cannot be modified afterward.
 *
 * @param t [tree] the tree that cannot be modify.
 * @returns the tree itself.
 */
constant ( t);

/**
 * @fn refresh ( interval)
 * @brief Set document refresh time
 *
 *  Refresh the document after the given time interval.
 *
 * @param interval [real] Interval in seconds
 */
refresh ( interval);


/**
 * @fn exit ( n)
 * @brief Exit from Tao
 *
 *  Exit from Tao with the given value
 * @bug It is not closing and cleanning the Qt environment on quitting.
 *
 * @param n [integer] exit parameter
 */
exit ( n);


/**
 * @fn locally ( t)
 * @brief Make a local context
 *
 *  Evaluate the child tree while preserving the current state. 
 * Any state modification like color or translation done in the 
 * locally block are only avalable inside the locally block.
 *
 * @param t [tree] The code to execute in a local context
 * @returns The result of the child execution.
 */
locally ( t);

/**
 * @fn current_repository ()
 * @brief Return true if we use a git repository with the current document
 *
 *  Return true if the current document is stored in a repository, 
 * Git functionality is compiled in, and not disabled on the command line.

 * @returns True if a git repository is used and usable for this document, false otherwise.
 */
current_repository ();


/**
 * @fn checkout ( t)
 * @brief checkout from repository
 *
 *  Checkout a branch or a commit from the git repository if available @see current_repository.
 *
 * @param t [text] The name of the branch or the commit ID to checkout.

 * @returns True if the checkout succeded, false otherwise.
 */
checkout ( t);


/**
 * @fn is_available ( name)
 * @brief Check if a functionality is available
 *
 *  Check if a specific Tao feature is available with the current version of the program.
 * Known features are @c git and @c stereoscopy .
 *
 * @param name [symbol] The name of the feature to test.
 * @returns False if the feature is not compiled in, true otherwise.
 */
is_available ( name);

/**
 * @fn offline_rendering ()
 * @brief Check if we are currently rendering to file
 * 
 * Return true if we are currently rendering to file.
 *  
 * @returns Return the offline rendering status
 */
offline_rendering ();

/**
 * @fn disable_depth_test ()
 * @brief Disable OpenGL depth test
 *
 *  Deactivate the OpenGL depth test for polygons.
 * @returns The previous state of depth testing
 */
disable_depth_test ();

/**
 * @fn enable_depth_test ()
 * @brief Enable OpenGL depth test
 *
 *  Activate the OpenGL depth test for polygons using the depth buffer.
 * @returns The previous state of depth testing
 */
enable_depth_test ();



/** @} */
