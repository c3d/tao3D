/**
 * @addtogroup TaoModules Module management primitives
 * @ingroup TaoBuiltins
 *
 * Use code and resources from external modules.
 *
 *@todo Add a link to the module SDK documentation
 *
 * @{
 */

/**
 * Loads a Tao module.
 * The syntax is:
@code
import MyModule 1.0
@endcode
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
 */
import(modulename:name, version:text);

/**
 * @}
 */
