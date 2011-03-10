/**
 * @addtogroup TaoFiles Listing files
 * @ingroup TaoBuiltins
 *
 * Lists file present on the computer.
 * @{
 */


/**
 * List full paths of files that match the given pattern.
 *
 * @p pattern is interpreted as a so-called "glob pattern". The forward
 * slash characters (@c /) should be used to denote subdirectories.
 * Only regular files are considered: directories and symbolic links that
 * match the pattern will not be retained by the primitive. The paths
 * returned by this primitive are always absolute paths.
 *
 * Glob expressions use special characters:
 *   @li The exclamation mark @c ? matches one unknown character
 *   @li The asterisk @c * matches any number of unknown characters
 *   @li The square brackets <tt> [ ]</tt> can be used to match one
 *       character as part of a group of characters
 *
 * For example, <tt>images/1*.[jJ][pP][gG]</tt> will match
 * <tt>images/1.jpg</tt>, <tt>images/1a.jpg</tt> ands
 * <tt>images/10b.JPG</tt>, but not <tt>images/a.jpg</tt>.
 *
 * If @p pattern does not start with a slash (that is, if it is not an
 * absolute path), then pattern will be matched relative to the directory
 * of the current file.
 *
 * @return a comma-separated list of strings, each one being the absolute
 * path to a regular file that matches the given pattern.
 *
 * @bug Not sure if relative paths are matched relative to the path of the
 * file that calls @ref files, or the path of the main document.
 * @bug What about symbolic links?
 */
tree files(text pattern);

/**
 * @deprecated Same as @ref files.
 */
tree list_files(text pattern);

/**
 * Loads text from a file.
 * Returns a text string that is the contents of the file.
 */
text load_text(text path);

/**
 * @}
 */
