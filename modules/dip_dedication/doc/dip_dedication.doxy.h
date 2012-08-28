/**
 * @~english
 * @taomoduledescription{DIP_dedication, DIP protocol for dedication}
 *
 * Module that deals with dedication protocol between DIP and CWG
 *
 * To start to use such a protocol, first call @ref dip_dedication_begin,
 * then you can either use the @ref dip_dedication function with 2 arguments
 * that stores information into following arrays
 *    - @ref FOR_NAME
 *    - @ref FROM_NAME
 *    - @ref DEDICATION_TEXT
 *    - @ref DEDICATION_EID
 *
 * The number of stored information is in @ref DEDICATION_COUNT.
 *
 * Or you can use
 * the @ref dip_dedication with 3 arguments, and the third argument is a
 * function called on each 4-tuple of data. This function must have the
 * same signature as @ref dip_dedication_default_callback.
 *
 * For debugging purpose, the @ref dip_dedication_display_all function prints on
 * stdout data stored in arrays.
 *
 * @endtaomoduledescription{DIP_dedication}
 *
 * @~french
 * @taomoduledescription{DIP_dedication, Module DIP_dedication}
 *
 * Module qui gère le protocol de dédicaces entre le DIP et le CWG
 *
 * Pour utiliser un tel protocol, premièrement appeler la fonction
 * @ref dip_dedication_begin. Ensuite vous avez le choix entre utiliser la
 * fonction @ref dip_dedication à deux arguments qui va stocker les données
 * dans les tableaux suivants :
 *    - @ref FOR_NAME
 *    - @ref FROM_NAME
 *    - @ref DEDICATION_TEXT
 *    - @ref DEDICATION_EID
 *
 * Le nombre total d'élements de ces tableaux étant dans la
 * variable @ref DEDICATION_COUNT.
 *
 * Ou vous pouvez utiliser la fonction @ref dip_dedication à trois arguments,
 * le troisième argument étant le nom d'une fontion qui sera appelée pour
 * chaque quadruplé et qui doit avoir la même signature que la fonction
 * @ref dip_dedication_default_callback.
 *
 * Pour les besoins de mise au point la fonction
 * @ref dip_dedication_display_all affiche le
 * contenu des 4 tables sus-citées.
 *
 * @endtaomoduledescription{DIP_dedication}
 *
 * @~
 * @{
 */

/**
 * @~english
 * Begin a dedication protocol feature.
 *
 * Begin a dedication feature, given a feature UUID if license
 * "DIP_dedication 1.0" is available.
 *
 * @param FeatureType is the feature type
 * @param Fuuid is the feature UUID
 *
 * @~french
 * Démarre une interaction utilisant le protocol "dedication".
 *
 * Le démarrage de la fonctionnalité est soumis à la présence de la licence
 * "DIP_dedication 1.0".
 *
 * @param FeatureType est le type de fonctionnalité.
 * @param Fuuid est l'identifiant unique (UUID) de cette interaction.
 *
 */
dip_dedication_begin(FeatureType:text, Fuuid:text);


/**
 * @~english
 * End a dedication protocol feature.
 *
 * End a dedication protocol.
 *
 * @param FeatureType is the feature type
 * @param Fuuid is the feature UUID
 *
 * @~french
 * Arrête une interaction utilisant le protocol "dedication".
 *
 * @param FeatureType est le type de fonctionnalité.
 * @param Fuuid est l'identifiant unique (UUID) de cette interaction.
 *
 */
dip_dedication_end (FeatureType:text, Fuuid:text);


/**
 * @~english
 * Deals with data.
 *
 * Deals with data for that feature UUID. It is called once after the
 * @ref dip_dedication_begin and the named function @p FunctionName is then
 * applied to each set of data.
 *
 * @param FeatureType is the feature type
 * @param Fuuid is the feature UUID
 * @param FunctionName is the name of the function that will be called
 * to handle dedication data. The callback signature must be the same as
 * @ref dip_dedication_default_callback.
 *
 * @~french
 * Gère les données.
 *
 * Gère les données pour cette fonctionnalité. Cette fonction est appelée
 * une seule fois après @ref dip_dedication_begin, puis la callback nommée
 * @p FunctionName est ensuite appliquée à chacun des jeux de données.
 *
 * @param FeatureType est le type de fonctionnalité.
 * @param Fuuid est l'identifiant unique (UUID) de cette interaction.
 * @param FunctionName est le nom de la fonction qui sera appelée pour
 * gérer les données de la dédicace. La signature de cette fonction doit
 * être identique à celle de @ref dip_dedication_default_callback.
 *
 */
dip_dedication (FeatureType:text, Fuuid:text, FunctionName:text);

/**
 * @~english
 * Stores data.
 *
 * Deals with data for that feature UUID.
 *
 * It is a shortcut to
 * @code
 dip_dedication FeatureType, Fuuid, "dip_dedication_default_callback"
 * @endcode
 * The default callback stores data in arrays.
 *
 * @param FeatureType is the feature type
 * @param Fuuid is the feature UUID
 *
 *
 * @~french
 * Enregistre les données.
 *
 * Gère les données pour cette fonctionnalité.
 *
 * C'est un racourcis pour
 * @code
 dip_dedication FeatureType, Fuuid, "dip_dedication_default_callback"
 * @endcode
 * La fonction @ref dip_dedication_default_callback enregistre les données dans
 * des tableaux.
 *
 * @param FeatureType est le type de fonctionnalité.
 * @param Fuuid est l'identifiant unique (UUID) de cette interaction.
 *
 *@~
 *@see dip_dedication_default_callback
 */
dip_dedication  (FeatureType:text, Fuuid:text);


/**
 * @~english
 * Stores one piece of data.
 *
 * Callback called for each dedication.
 * It is storing data into following arrays
 *    - @ref FOR_NAME
 *    - @ref FROM_NAME
 *    - @ref DEDICATION_TEXT
 *    - @ref DEDICATION_EID
 *
 * And it increases the @ref DEDICATION_COUNT by one.
 *
 * @param ForName is the text to display as the name of the person FOR which
 * the dedication is.
 * @param FromName is the text to display as the name of the person FROM which
 * the dedication is.
 * @param DedicationText is the text of the dedication to display.
 * @param DedicationEID is the element identifier that identifies the
 * dedication style.
 *
 *
 * @~french
 * Enregistre un élément.
 *
 * Callback appelée pour chaque dédicace. Elle enregistre les données
 * dans les tables suivantes :
 *    - @ref FOR_NAME
 *    - @ref FROM_NAME
 *    - @ref DEDICATION_TEXT
 *    - @ref DEDICATION_EID
 *
 * Et elle augmente de un la valeur de @ref DEDICATION_COUNT.
 *
 * @param ForName est le texte à afficher en tant que nom de la personne POUR
 * qui est la dédicace.
 * @param FromName est le texte à afficher en tant que nom de la personne DE
 * qui est la dédicace.
 * @param DedicationText est le texte de la dédicace.
 * @param DedicationEID est l'identifiant du style de dédicace.
 *
 */
dip_dedication_default_callback (ForName:text, FromName:text, DedicationText:text, DedicationEID:text);

/**
 * @~english
 * Display all data.
 *
 * Display the content of the four arrays.
 * It uses @ref dip_dedication_display as follow :
 *
 * @~french
 * Affiche toutes les données.
 *
 * Affiche le contenu des quatre tableaux.
 * La fonction utilise @ref dip_dedication_display de la manière suivante :
 *
 * @~
 * @code
     dip_dedication_display with 0 .. DEDICATION_COUNT
 * @endcode
 *
 */
dip_dedication_display_all();

/**
 * @~english
 * Display one piece of data.
 *
 * Display the content of row @p Row of the four arrays. It prints on stdout
 * one line with the following format:
 *
 * @~french
 * Affiche une ligne de données.
 *
 * Affiche le contenu des quatre tableaux pour la ligne @p Row. La fonction
 * imprime sur la sortie standard une ligne avec le format suivant :
 *
 * @~
 * @code
 <row number> - <for_name> <from_name> <dedication_text> <dedication_eid>
 * @endcode
 *
 */
dip_dedication_display (Row:integer);

/**
 * @~english
 * The number of dedication.
 *
 * The number of items stored in each table.
 *
 * @~french
 * Le nombre de dédicaces.
 *
 * Le nombre de données stockées dans chacune des tables.
 *
 */
integer DEDICATION_COUNT;


/**
 * @~english
 * Name of the dedication's beneficiary.
 *
 * @~french
 * Nom du destinataire de la dédicace.
 */
text[] FOR_NAME;


/**
 * @~english
 * Name of the dedication's originary.
 *
 * @~french
 * Nom du signataire de la dédicace.
 */
text[] FROM_NAME;


/**
 * @~english
 * Dedication's text.
 *
 * @~french
 * Texte de la dédicace.
 */
text[] DEDICATION_TEXT;


/**
 * @~english
 * Dedication's style identifier.
 *
 * @~french
 * Identifiant du style de la dédicace.
 */
text[] DEDICATION_EID;


/**
 * @}
 */
