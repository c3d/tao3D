/**
 * @~english
 * @addtogroup TaoFiles Listing files
 * @ingroup TaoBuiltins
 *
 * List files present on the computer.
 *
 * @~french
 * @addtogroup TaoFiles Lister des fichiers
 * @ingroup TaoBuiltins
 *
 * Liste des fichiers présents sur l'ordinateur.
 * @{
 */


/**
 * @~english
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
 *   @li The interrogation mark @c ? matches one unknown character
 *   @li The asterisk @c * matches any number of unknown characters
 *   @li The square brackets <tt> [ ]</tt> can be used to match one
 *       character as part of a group of characters
 *
 * For example, <tt>images/1*.[jJ][pP][gG]</tt> will match
 * <tt>images/1.jpg</tt>, <tt>images/1a.jpg</tt> and
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
 *
 * @~french
 * Renvoie les chemins absolus des fichiers qui correspondent au(x) modèle(s)
 * donné(s).
 *
 * @p patterns est une ou plusieurs chaînes de texte séparées par des virgules,
 * des point-virgules ou sur plusieurs lignes.
 * Chaque chaîne est interprétée comme un "glob pattern". Le slash (@c /)
 * sert à séparer les répertoires et sous-répertoires.
 * Seuls les fichier normaux sont pris en compte. Les répertoires et les liens
 * symboliques qui correspondent au modèle ne seront pas retenus par la
 * primitive. Les chemins renvoyés sont toujours absolus.
 *
 * Les "glob patterns" utilisent des caractères spéciaux :
 *   @li Le point d'interrogation @c ? remplace un caractère
 *   @li L'astérisque @c * remplace un nombre quelconque de caractères
 *   @li Les crochets <tt> [ ]</tt> remplace un caractère parmi
 *       une liste.
 *
 * Par exmple, <tt>images/1*.[jJ][pP][gG]</tt> pourra donner
 * <tt>images/1.jpg</tt>, <tt>images/1a.jpg</tt> et
 * <tt>images/10b.JPG</tt>, mais pas <tt>images/a.jpg</tt>.
 *
 * Si @p pattern ne commence pas par un slash (c'est-à-dire, si ce n'est pas
 * un chemin absolu), alors les fichiers seront recherchés relativement au
 * répertoire du fichier courant.
 *
 * @return Une liste de chemins séparés par des virgules, chaque chemin étant
 * le chemin absolu vers un fichier qui correspond au modèle donné.
 *
 * Par exemple :
 *
 @code
affiche T:text -> writeln "Chemin: ", T

affiche with files "images/*.jpg", "images/*.png"
// Cette syntaxe est également possible
affiche with files
    "images/*.jpg"
    "images/*.png"
 @endcode
 */
tree files(patterns:tree);

/**
 * @~english
 * Loads text from a file.
 * Returns a text string that is the contents of the file.
 * @~french
 * Charge du texte à partir d'un fichier.
 * Renvoie le contenu du fichier sous forme de chaîne de caractère.
 */
text load_text(path:text);

/**
 * @}
 */
