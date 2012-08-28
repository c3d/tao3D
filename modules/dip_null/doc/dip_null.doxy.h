/**
 * @~english
 * @taomoduledescription{DIP_null, DIP no data protocol}
 *
 * Module for DIP interaction that does not use data.
 *
 * @endtaomoduledescription{DIP_null}
 *
 * @~french
 * @taomoduledescription{DIP_null, Protocol DIP sans donnée.}
 *
 * Module pour interaction sans donnée avec le DIP. 
 *
 * @endtaomoduledescription{DIP_null}
 *
 * @~
 * @{
 */

/**
 * @~english
 * Begin a null protocol feature.
 *
 * Begin a null protocol if license "DIP_null 1.0" is available.
 *
 * @param T is the feature type
 * @param F is the feature UUID
 *
 * @~french
 * Démarre une interaction utilisant le protocol "null".
 *
 * Le démarrage de la fonctionnalité est soumis à la presence de la licence "DIP_null 1.0".
 *
 * @param T est le type de fonctionnalité.
 * @param F est l'identifiant unique (UUID) de cette interaction.
 *
 * @~
 * 
 */
dip_null_start( T:text, F:text )

/**
 * @~english
 * End a null protocol feature.
 *
 * End a null protocol.
 *
 * @param T is the feature type
 * @param F is the feature UUID
 *
 * @~french
 * Arrête une interaction utilisant le protocol "null".
 *
 * 
 * @param T est le type de fonctionnalité.
 * @param F est l'identifiant unique (UUID) de cette interaction.
 *
 * @~
 * 
 */
dip_null_end( T:text, F:text )
/**
 * @}
 */
