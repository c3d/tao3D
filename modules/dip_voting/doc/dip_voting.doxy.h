/**
 * @~english
 * @taomoduledescription{DipVoting, DIP protocol for voting}
 *
 * DIP protocol that deals with voting interaction data
 *
 * To start to use such a protocol, first call @ref dip_voting_begin,
 * then you can either use the @ref dip_voting function with 2 arguments that
 * stores information into following arrays
 *    - @ref VOTE_VALUES
 *    - @ref ITEM_TYPES
 *    - @ref ITEM_UUIDS
 *
 * The number of stored information is in @ref ROW_COUNT.
 *
 * Or you can use
 * the @ref dip_voting with 3 arguments, and the third argument is a
 * function called on each 3-tuple of data. This function must have the
 * same signature as @ref dip_voting_default_callback.
 *
 * For debugging purpose, the @ref dip_voting_display_all function prints on
 * stdout data stored in arrays.
 *
 * @endtaomoduledescription{DipVoting}
 *
 * @~french
 * @taomoduledescription{DipVoting, Protocol DIP pour le vote}
 *
 * Protocol pour les interactions avec le DIP de type vote.
 *
 * Pour utiliser un tel protocol, premièrement appeler la fonction
 * @ref dip_voting_begin.
 * Ensuite vous avez le choix entre utiliser la fonction @ref dip_voting à
 * deux arguments qui va stocker les données dans les tableaux suivants :
 *    - @ref VOTE_VALUES
 *    - @ref ITEM_TYPES
 *    - @ref ITEM_UUIDS
 *
 * Le nombre total d'élements de ces tableaux étant dans la variable
 * @ref USER_COUNT.
 *
 * Ou vous pouvez utiliser la fonction @ref dip_voting à trois arguments,
 * le troisième argument étant le nom d'une fontion qui sera appelée pour
 * chaque triplé et qui doit avoir la même signature que la fonction
 * @ref dip_voting_default_callback.
 *
 * Pour les besoins de mise au point la fonction @ref dip_voting_display_all
 * affiche le contenu des 3 tables sus-citées.
 *
 * @endtaomoduledescription{DipVoting}
 *
 * @~
 * @{
 */

/**
 * @~english
 * Begin a voting protocol feature.
 *
 * Begin a voting feature, given a feature UUID if license "DIP_voting 1.0"
 * is available.
 *
 * @param FeatureType is the feature type
 * @param Fuuid is the feature UUID
 *
 * @~french
 * Démarre une interaction utilisant le protocol "voting".
 *
 * Le démarrage de la fonctionnalité est soumis à la présence de la licence
 * "DIP_voting 1.0".
 *
 * @param FeatureType est le type de fonctionnalité.
 * @param Fuuid est l'identifiant unique (UUID) de cette interaction.
 *
 */
dip_voting_begin (FeatureType:text, Fuuid:text);

/**
 * @~english
 * End a voting protocol feature.
 *
 * End a voting protocol.
 *
 * @param FeatureType is the feature type
 * @param Fuuid is the feature UUID
 *
 * @~french
 * Arrête une interaction utilisant le protocol "voting".
 *
 *
 * @param FeatureType est le type de fonctionnalité.
 * @param Fuuid est l'identifiant unique (UUID) de cette interaction.
 *
 */
dip_voting_end (FeatureType:text, Fuuid:text);


/**
 * @~english
 * Deals with data.
 *
 * Deals with data for that feature UUID. It is called once after the
 * @ref dip_voting_begin and the named function @p FunctionName is then
 * applied to each set of data.
 *
 * @param FeatureType is the feature type
 * @param Fuuid is the feature UUID
 * @param FunctionName is the name of the function that will be called
 * to handle voting data. The callback signature must be the same as
 * @ref dip_voting_default_callback.
 *
 * @~french
 * Gère les données.
 *
 * Gère les données pour cette fonctionnalité. Cette fonction est appelée
 * une seule fois après @ref dip_voting_begin, puis la callback nommée
 * @p FunctionName est ensuite appliquée à chacun des jeux de données.
 *
 * @param FeatureType est le type de fonctionnalité.
 * @param Fuuid est l'identifiant unique (UUID) de cette interaction.
 * @param FunctionName est le nom de la fonction qui sera appelée pour
 * gérer les données du vote. La signature de cette fonction doit
 * être identique à celle de @ref dip_voting_default_callback.
 *
 */
dip_voting (FeatureType:text, Fuuid:text, FunctionName:text);

/**
 * @~english
 * Stores data.
 *
 * Deals with data for that feature UUID.
 *
 * It is a shortcut to
 * @code
 dip_voting FeatureType, Fuuid, "dip_voting_default_callback"
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
 dip_voting FeatureType, Fuuid, "dip_voting_default_callback"
 * @endcode
 * La fonction @ref dip_voting_default_callback enregistre les données dans
 * des tableaux.
 *
 * @param FeatureType est le type de fonctionnalité.
 * @param Fuuid est l'identifiant unique (UUID) de cette interaction.
 *
 *@~
 *@see dip_voting_default_callback
 */
dip_voting ( FeatureType:text, Fuuid:text);

/**
 * @~english
 * Stores one piece of data.
 *
 * Callback called for each voting.
 * It is storing data into following arrays
 *    - @ref VOTE_VALUES
 *    - @ref ITEM_TYPES
 *    - @ref ITEM_UUIDS
 *
 * And it increases the @ref ROW_COUNT by one.
 *
 * @param VoteValue is the value choosen by the user.
 * @param ItemType is a kind of item to display.
 * @param ItemUuid is the UUID of item to display.
 *
 *
 * @~french
 * Enregistre un élément.
 *
 * Callback appelée pour chaque vote. Elle enregistre les données
 * dans les tables suivantes :
 *    - @ref VOTE_VALUES
 *    - @ref ITEM_TYPES
 *    - @ref ITEM_UUIDS
 *
 * Et elle augmente de un la valeur de @ref ROW_COUNT.
 *
 * @param VoteValue est la valeur choisie par l'utilisateur.
 * @param ItemType est le type de l'élément à afficher.
 * @param ItemUuid est l'identifiant de l'élément à afficher.
 *
 */
dip_voting_default_callback( VoteValue:integer, ItemType:text, ItemUuid:text);

/**
 * @~english
 * Display all data.
 *
 * Display the content of the three arrays.
 * It uses @ref dip_voting_display as follow :
 *
 * @~french
 * Affiche toutes les données.
 *
 * Affiche le contenu des trois tableaux.
 * La fonction utilise @ref dip_voting_display de la manière suivante :
 *
 * @~
 * @code
     dip_voting_display with 0 .. ROW_COUNT
 * @endcode
 *
 */
dip_voting_display_all ();

/**
 * @~english
 * Display one piece of data.
 *
 * Display the content of row @p Row of the three arrays. It prints on stdout
 * one line with the following format:
 *
 * @~french
 * Affiche une ligne de données.
 *
 * Affiche le contenu des trois tableaux pour la ligne @p Row. La fonction
 * imprime sur la sortie standard une ligne avec le format suivant :
 *
 * @~
 * @code
 <row number> - <vote_value> <item_type> <item_uuid>
 * @endcode
 *
 */
dip_voting_display (Row:integer );

/**
 * @~english
 * The number of vote.
 *
 * The number of items stored in each table.
 *
 * @~french
 * Le nombre de vote.
 *
 * Le nombre de données stockées dans chacune des tables.
 *
 */
integer ROW_COUNT;

/**
 * @~english
 * Array storing the vote values.
 *
 * @~french
 * Table contenant les votes.
 */
integer[] VOTE_VALUES;


/**
 * @~english
 * Type of data to display.
 *
 * It is a 3-letter string looking like file extension.
 *
 * @~french
 * Type des données.
 *
 * C'est une chaîne de 3 lettres tel les extensions de fichiers.
 *
 */
text[] ITEM_TYPES;


/**
 * @~english
 * Item UUID.
 *
 * @~french
 * Identifiant de l'élément.
 *
 */
text[] ITEM_UUIDS;


/**
 * @}
 */
