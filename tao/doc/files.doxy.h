/**
 * @~english
 * @addtogroup TaoFiles File utilities
 * @ingroup TaoBuiltins
 *
 * Some file related builtins: list files present on the computer, load data
 * from a file.
 *
 * @~french
 * @addtogroup TaoFiles Manipulations de fichiers
 * @ingroup TaoBuiltins
 *
 * Fonctions liées à la manipulation de fichiers : lister les fichiers présents
 * sur l'ordinateur, charger des données depuis un fichier.
 *
 * @~
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
 * If @p pattern does not start with a slash or a Windows drive specification
 * (that is, if it is not an
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
// Access a USB stick or an external drive:
//   (Windows)
print_path with files "E:/images/*.jpg"
//   (MacOSX)
print_path with files "/Volumes/NAME/*.jpg"
//    (Linux)
print_path with files "/media/NAME/*.jpg"
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
 * Seuls les fichiers normaux sont pris en compte. Les répertoires et les liens
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
 * Si @p pattern ne commence pas par un slash ou une lettre de lecteur
 * Windows (c'est-à-dire, si ce n'est pas
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
// Accéder �  une clé USB ou un disque externe :
//   (Windows)
affiche with files "E:/images/*.jpg"
//   (MacOSX)
affiche with files "/Volumes/NAME/*.jpg"
//    (Linux)
affiche with files "/media/NAME/*.jpg"
 @endcode
 */
tree files(patterns:tree);

/**
 * @~english
 * Loads text from a file.
 * Returns a text string that is the contents of the file.
 * @~french
 * Charge du texte à  partir d'un fichier.
 * Renvoie le contenu du fichier sous forme de chaîne de caractère.
 */
text load_text(path:text);


/**
 * @~english
 * Check if a file really exists.
 * Allow to check if the file @p filename exists or not.
 * @~french
 * Vérifie si un fichier existe ou non.
 * Permet de vérifier si le fichier @p filename existe ou non.
 */
bool file_exists(filename:text);

/**
 * @~english
 * Get base name of a file.
 * Returns the base name of a file without the path.
 * @~french
 * Récupère le nom de base d'un fichier
 * Renvoie le nom de base du fichier spécifié sans son chemin.
 */
text base_name(filename:text);

/**
 * @~english
 * Get path of a file.
 * Returns the path of the specified file.
 * @~french
 * Récupère le chemin d'un fichier.
 * Renvoie le chemin absolu ou relatif du fichier specifié.
 */
text dir_name(filename:text);

/**
 * @~english
 * Loads data from a file in comma-separated values (CSV) format.
 * The field separator is semicolon (@c ;) or comma (@c ,). Numerical
 * values are read according to the current locale. @n
 * For each line in the file, @ref load_csv calls the function
 * specified by @p prefix. @n
 * The following example computes the sum of all lines that have three
 * numerical values:
 * @code
// -- test.csv:
// First value; Second value; Third value
// 1;12;40
// 3;6.5;-1.7
// -- Output:
// 60.8

Total -> 0.0
Total := 0.0
load_csv "test.csv", "process_line"
writeln Total

process_line A:real, B:real, C:real -> Total := Total + A + B + C
process_line AnythingElse -> false
 * @endcode
 * @~french
 * Lit des données depuis un fichier au format CSV.
 * Le séparateur de champs est le point-virgule (@c ;) ou la virgule
 * (@c ,). Les valeurs numériques sont interprétées en fonction des
 * préférences système (<em>locale</em>).
 * @n
 * La fonction dont le nom est précisé par @p prefix est appelée pour
 * chaque ligne lue. Chaque valeur lue est un paramètre de la fonction.
 * @n
 * L'exemple qui suit fait la somme de toutes les lignes qui contiennent
 * trois valeurs numériques :
 * @code
// -- test.csv :
// Valeur 1; Valeur 2; Valeur 3
// 1;12;40
// 3;6.5;-1.7
// -- Résultat :
// 60.8

Total -> 0.0
Total := 0.0
load_csv "test.csv", "traitement_ligne"
writeln Total

traitement_ligne A:real, B:real, C:real -> Total := Total + A + B + C
traitement_ligne Autre -> false
 * @endcode
 */
tree load_csv(filename:text, prefix:text);


/**
 * @~english
 * Loads data from a file in tab-separated values (TSV) format.
 * Similar to @ref load_csv except that the field separator is the
 * TAB character.
 * @~french
 * Lit des données depuis un fichier au format TSV.
 * Similaire à @ref load_csv mais le séparateur de champs est la
 * tabulation.
 */
tree load_tsv(filename:text, prefix:text);


/**
 * @~english
 * Loads data from a file.
 * This is a general version of @ref load_csv or @ref load_tsv.
 * @p fs and @p rs allow
 * to specify the characters that may be used as field and record
 * separators, respectively.
 * @~french
 * Lit des données depuis un fichier.
 * Cette fonction est une forme plus générale de @ref load_csv et @ref load_tsv.
 * @p fs et @p rs permettent de préciser quels caractères utiliser comme
 * séparateurs de champs et d'enregistrements, respectivement.
 */
tree load_data(filename:text, prefix:text, fs:text, rs:text);


/**
 * @~english
 * Saves a screen capture to a file.
 * Captures the contents of the graphical window, without the borders,
 * and saves it into the file specified by @p fileName. If the file path is
 * not absolute, it is relative to the document directory (where the main
 * <tt>.ddd</tt> file is located). If the destination directory does not
 * exist, the file is not created. @n
 * The file format is derived from the
 * extension (such as <tt>.png</tt> or <tt>.jpg</tt>). @n
 * The graphical area is captured once the drawing is complete, and only one
 * capture may be active at any given time. Therefore, the following example
 * will create file <tt>capture2.png</tt> with one circle and one rectangle:
 * @code
rectangle -150, 0, 200, 100
screenshot "capture1.png"  // Ignored
circle 100, 0, 50
screenshot "capture2.png"
 * @endcode
 * The capture saves the exact content of the application's drawing area,
 * so if a 3D display mode is active (such as anaglyph, interleaved or side
 * by side), the resulting snapshot will be encoded for this mode. @n
 * If @p withAlpha is @c false, the file will not contain any transparency
 * information.
 * @~french
 * Enregistre une capture d'écran dans un fichier.
 * Capture le contenu de la fenêtre graphique, sans les bordures, et
 * l'enregistre dans le fichier @p fileName. Si le chemin n'est pas absolu,
 * il est relatif au répertoire du document principal (<tt>.ddd</tt>). Si le
 * répertoire de destination n'existe pas, le fichier n'est pas créé. @n
 * Le format du fichier est déduit de l'extension (par exemple, <tt>.png</tt>
 * ou <tt>.jpg</tt>). @n
 * La zone graphique est capturé après que la phase de dessin est terminée, et
 * une seule capture peut être active pour une trame. Ainsi, l'exemple suivant
 * crée un seul fichier, <tt>capture2.png</tt>, qui contient un rectangle et
 * un cercle.
 * @code
rectangle -150, 0, 200, 100
screenshot "capture1.png"  // Ignoré
circle 100, 0, 50
screenshot "capture2.png"
 * @endcode
 * Lorsque @p withAlpha est @c false, le fichier ne comporte pas d'information
 * de transparence.
 */
boolean screenshot(fileName:text, withAlpha:boolean);


/**
 * @~english
 * Saves a screen capture to a file.
 * Same <tt>screenshot fileName, false</tt>.
 * @~french
 * Enregistre une capture d'écran dans un fichier.
 * Équivalent à : <tt>screenshot fileName, false</tt>.
 */
boolean screenshot(fileName:text);


/**
 * @~english
 * Get the main document folder.
 * @return main document folder.
 * @~french
 * Récupère le chemin vers le dossier du document principal.
 * @return chemin vers le dossier du document principal.
 */
text document_dir();

/**
 * @}
 */
