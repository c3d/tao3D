/**
 * @addtogroup TaoFiles Listing files
 * @ingroup TaoBuiltins
 *
 * List files present on the computer.
 * @{
 */


/**
 * List full paths of files that match the given pattern(s).
 *
 * @p patterns is one or several text strings separated by commas,
 * semicolons or carriage returns.
 * Each pattern is interpreted as a so-called "glob pattern". The forward
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
 * For example:
 *
 @code
print_path T:text -> writeln "Path: ", T

print_path with files "images/*.jpg", "images/*.png"
// The following syntax is valid, too
print_path with files
    "images/*.jpg"
    "images/*.png"
 @endcode
 *
 * @todo Not sure if relative paths are matched relatively to the path of the
 * file that calls @ref files, or to the path of the main document.
 * @todo What about symbolic links?
 */
tree files(patterns:tree);

/**
 * Loads text from a file.
 * Returns a text string that is the contents of the file.
 */
text load_text(path:text);

/**
 * @}
 */
