/**
 * @addtogroup TaoModules Module management primitives
 * @ingroup TaoBuiltins
 *
 * @brief Use code and resources from external modules.
 *
 * @{
 */

/**
 * Loads a Tao module.
 * The syntax is:
@verbatim
import MyModule "1.0"
@endverbatim
 *
 * When Tao encounters the import statement, it looks up ModuleName in the
 * list of currently known (and enabled) modules, checks the version
 * compatibility, and then makes the module definitions available to the Tao
 * document.
 *
 * Version matching is a major/minor match. For a module to load, module.major
 * must be equal to requested.major and module.minor must be greater or equal
 * to requested.minor.
 *   @li major is the part before the first dot
 *   @li minor is the remaining part
 *
 * For instance in 1.0.2, major is 1 and minor is 0.2. Comparison is performed
 * after converting to integers.
 *
 * Without explicit import, no definition from the module's module.xl file are
 * reachable from the document.
 *
 * @param modulename The export name of the module, as defined in the module
 *        documentation (this is also the value of import_name of the
 *        module_description block in module.xl)
 * @param version The minimum version of the module known to work with the
 *        current document.
 */
import(name modulename, text version);

/**
 * @}
 */
