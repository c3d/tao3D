/**
 * @~english
 * @addtogroup TaoModules Module management primitives
 * @ingroup TaoBuiltins
 *
 * Use code and resources from external modules.
 *
 * @todo Add a link to the module SDK documentation
 *
 * @~french
 * @addtogroup TaoModules Chargement d'un module
 * @ingroup TaoBuiltins
 *
 * Utiliser du code et des fichiers provenant de modules externes.
 *
 * @~
 * @{
 */

/**
 * @~english
 * Loads a Tao module.
 * The syntax is:
@code
import MyModule 1.0
@endcode
 * Note that <tt>MyModule</tt> is not enclosed in double quotes.
 *
 * When Tao encounters the import statement, it looks up ModuleName in the
 * list of currently known (and enabled) modules, checks the version
 * compatibility, and then makes the module definitions available to the Tao
 * document.
 *
 * Version numbers can have one of three forms:
 *  - An integer value, e.g. 1, which is the same as 1.0
 *  - A real value, e.g., 1.023
 *  - A text value, e.g., "1.023"
 *
 * Version matching is a major/minor match. For a module to load, the module
 * major must be equal to the requested major (the integer part of the
 * version), and the module minor must be
 * greater or equal to the requested minor (the fractional part of the
 * version).
 *
 * If the version is not specified, it is assumed to be 1.0.
 *
 * Without explicit import, no definition from the module's main XL file are
 * reachable from the document.
 *
 * @param modulename The export name of the module, as defined in the module
 *        documentation (this is also the value of import_name of the
 *        module_description block in the module's main XL file)
 * @param version The minimum version of the module known to work with the
 *        current document.
 *
 * @~french
 * Charge un module Tao Presentations.
 * La syntaxe est :
@code
import MonModule 1.0
@endcode
 * Notez que <tt>MonModule</tt> n'apparaît pas entre guillemets.
 *
 * Quand Tao Presentations rencontre le mot-clé import, il recherche MonModule
 * dans la liste des modules connus (et actifs), vérifie que la version
 * demandée est compatible, et rend les définitions du module visible pour
 * le document.
 *
 * Les numéros de version peuvent prendre trois formes :
 *  - Une valeur entière, par exemple 1, qui est identique à 1.0
 *  - Une valeur réelle, par exemple 1.023
 *  - Un texte, par exemple "1.023"
 *
 * La vérification des versions se fait par comparaison des valeurs majeure/
 * mineure. Pour qu'un module puisse se charger, la version majeure du
 * module doit être égale à la version majeure demandée (la partie entière
 * de la version), et la version mineure doit être supérieure ou égale à la
 * version mineure demandée (la partie fractionaire de la version).
 *
 * Si la version n'est pas précisée, elle est égale à 1.0.
 *
 * Sans import, les définitions contenus dans le fichier @c .xl principal d'un
 * module ne peuvent pas être utilisées dans le document.
 *
 * @param modulename Le nom d'import du module, tel que défini dans la
 *        documentation du module. C'est aussi la valeur de l'attribut
 *        import_name du bloc module_description situé dans le fichier
 *        XL principal du module.
 * @param version La version minimale du module dont le document courant
 *        a besoin pour fonctionner. Entier, réel ou texte.
 */
import(modulename:name, version);

/**
 * @~english
 * Loads a Tao module.
 * The requested version is 1.0. Therefore, <tt>import MyModule</tt>
 * is equivalent to <tt>import MyModule 1.0</tt>.
 * Note that <tt>ModuleName</tt> is not enclosed in double quotes.
 * @~french
 * Charge un module Tao.
 * La version demandée est 1.0. Par conséquent, <tt>import MonModule</tt>
 * est équivalent à <tt>import MonModule 1.0</tt>.
 * Notez que <tt>MonModule</tt> n'apparaît pas entre guillemets.
 */
import(modulename:name);

/**
 * @~english
 * Loads a source file and makes its definitions available to the main program.
 * For example:
@code
import "file.xl"
import "file.ddd"
@endcode
 * @~french
 * Charge un fichier source et rend ses définitions disponibles au programme
 * principal. Par exemple:
@code
import "fichier.xl"
import "fichier.ddd"
@endcode
 */
import(file:text);

/**
 * @}
 */
