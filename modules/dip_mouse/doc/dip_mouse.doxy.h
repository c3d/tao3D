/**
 * @~english
 * @taomoduledescription{DipMouse, DIP protocol for mouse}
 *
 * Protocol for mouse data exchange with the DIP.
 *
 * To start to use such a protocol, first call @ref dip_mouse_begin,
 * then you can either use the @ref dip_mouse function with 2 arguments that
 * stores information into following arrays
 *    - @ref USER_UUID,
 *    - @ref MOUSE_BUTTON,
 *    - @ref MOUSE_X,
 *    - @ref MOUSE_Y.
 *
 * The number of stored information is in @ref USER_COUNT.
 *
 * Or you can use
 * the @ref dip_mouse with 3 arguments, and the third argument is a
 * function called on each 4-tuple of data. This function must have the
 * same signature as @ref dip_mouse_default_callback.
 *
 * For debugging purpose, the @ref dip_mouse_display_all function prints on
 * stdout data stored in arrays.
 *
 * @endtaomoduledescription{DipMouse}
 *
 * @~french
 * @taomoduledescription{DipMouse, Protocol du DIP pour la souris}
 *
 * Protocol d'interaction avec le DIP faisant transiter les données d'une souris.
 *
 * Pour utiliser un tel protocol, premièrement appeler la fonction @ref dip_mouse_begin.
 * Ensuite vous avez le choix entre utiliser la fonction @ref dip_mouse à deux arguments
 * qui va stocker les données dans les tableaux suivants :
 *    - @ref USER_UUID,
 *    - @ref MOUSE_BUTTON,
 *    - @ref MOUSE_X,
 *    - @ref MOUSE_Y.
 *
 * Le nombre total d'élements de ces tableaux étant dans la variable @ref USER_COUNT.
 *
 * Ou vous pouvez utiliser la fonction @ref dip_mouse à trois arguments, le troisième
 * argument étant le nom d'une fontion qui sera appelée pour chaque quadruplé et qui
 * doit avoir la même signature que la fonction @ref dip_mouse_default_callback.
 *
 * Pour les besoins de mise au point la fonction @ref dip_mouse_display_all affiche le
 * contenu des 4 tables sus-citées.
 *
 * @endtaomoduledescription{DipMouse}
 *
 * @~
 *
 * @{
 */

/**
 * @~english
 * Begin a mouse protocol feature.
 *
 * Begin a mouse feature, given a feature UUID if license "DIP_mouse 1.0"
 * is available.
 *
 * @param FeatureType is the feature type
 * @param Fuuid is the feature UUID
 *
 * @~french
 * Démarre une interaction utilisant le protocol "mouse".
 *
 * Le démarrage de la fonctionnalité est soumis à la présence de la licence
 * "DIP_mouse 1.0".
 *
 * @param FeatureType est le type de fonctionnalité.
 * @param Fuuid est l'identifiant unique (UUID) de cette interaction.
 *
 */
dip_mouse_begin (FeatureType:text, Fuuid:text);


/**
 * @~english
 * End a mouse protocol feature.
 *
 * End a mouse protocol.
 *
 * @param FeatureType is the feature type
 * @param Fuuid is the feature UUID
 *
 * @~french
 * Arrête une interaction utilisant le protocol "mouse".
 *
 *
 * @param FeatureType est le type de fonctionnalité.
 * @param Fuuid est l'identifiant unique (UUID) de cette interaction.
 *
 */
dip_mouse_end (FeatureType:text, Fuuid:text);

/**
 * @~english
 * Deals with data.
 *
 * Deals with data for that feature UUID. It is called once after the
 * @ref dip_mouse_begin and the named function @p FunctionName is then
 * applied to each set of data.
 *
 * @param FeatureType is the feature type
 * @param Fuuid is the feature UUID
 * @param FunctionName is the name of the function that will be called
 * to handle mouse data. The callback signature must be the same as
 * @ref dip_mouse_default_callback.
 *
 * @~french
 * Gère les données.
 *
 * Gère les données pour cette fonctionnalité. Cette fonction est appelée
 * une seule fois après @ref dip_mouse_begin, puis la callback nommée
 * @p FunctionName est ensuite appliquée à chacun des jeux de données.
 *
 * @param FeatureType est le type de fonctionnalité.
 * @param Fuuid est l'identifiant unique (UUID) de cette interaction.
 * @param FunctionName est le nom de la fonction qui sera appelée pour
 * gérer les données de la souris. La signature de cette fonction doit
 * être identique à celle de @ref dip_mouse_default_callback.
 *
 */
dip_mouse (FeatureType:text, Fuuid:text, FunctionName:text);


/**
 * @~english
 * Stores data.
 *
 * Deals with data for that feature UUID.
 *
 * It is a shortcut to
 * @code
 dip_mouse FeatureType, Fuuid, "dip_mouse_default_callback"
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
 dip_mouse FeatureType, Fuuid, "dip_mouse_default_callback"
 * @endcode
 * La fonction @ref dip_mouse_default_callback enregistre les données dans
 * des tableaux.
 *
 * @param FeatureType est le type de fonctionnalité.
 * @param Fuuid est l'identifiant unique (UUID) de cette interaction.
 *
 *@~
 *@see dip_mouse_default_callback
 */
dip_mouse (FeatureType:text, Fuuid:text);


/**
 * @~english
 * Stores one piece of data.
 *
 * Callback called for each mouse click.
 * It is storing data into following arrays
 *    - @ref USER_UUID
 *    - @ref MOUSE_BUTTON
 *    - @ref MOUSE_X
 *    - @ref MOUSE_Y
 *
 * And it increases the @ref USER_COUNT by one.
 *
 * @param UserUUID is the identifier that uniquely identify that user
 * @param MouseButton is a mask of pushed buttons
 * @param MouseX is the X-coordinate of the mouse between 0.0 and 1.0.
 * @param MouseY is the Y-coordinate of the mouse between 0.0 and 1.0.
 *
 *
 * @~french
 * Enregistre un élément.
 *
 * Callback appelée pour chaque clic de souris. Elle enregistre les données
 * dans les tables suivantes :
 *    - @ref USER_UUID
 *    - @ref MOUSE_BUTTON
 *    - @ref MOUSE_X
 *    - @ref MOUSE_Y
 *
 * Et elle augmente de un la valeur de @ref USER_COUNT.
 *
 * @param UserUUID l'identifieur unique de l'utilisateur
 * @param MouseButton est un masque des boutons appuyés
 * @param MouseX est l'abscisse de la souris compris entre 0.0 et 1.0
 * @param MouseY est l'ordonnée de la souris compris entre 0.0 et 1.0
 *
 */
dip_mouse_default_callback( UserUUID:text, MouseButton:integer, MouseX:real, MouseY:real );


/**
 * @~english
 * Display all data.
 *
 * Display the content of the four arrays.
 * It uses @ref dip_mouse_display as follow :
 *
 * @~french
 * Affiche toutes les données.
 *
 * Affiche le contenu des quatre tableaux.
 * La fonction utilise @ref dip_mouse_display de la manière suivante :
 *
 * @~
 * @code
     dip_mouse_display with 0 .. USER_COUNT
 * @endcode
 *
 */
dip_mouse_display_all();

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
 <row number> - <user_uuid> <mouse_button> <mouse_x> <mouse_y>
 * @endcode
 *
 */
dip_mouse_display (Row:integer);

/**
 * @~english
 * UUID representing the user.
 *
 * @~french
 * UUID représentant l'acteur.
 */
text[] USER_UUID;

/**
 * @~english
 * Array storing the used buttons for an action.
 *
 * It is a mask of all pressed buttons.
 *
 * @~french
 * Table contenant les boutons utilisés.
 *
 * C'est un masque de tous les boutons enfoncés.
 */
integer[] MOUSE_BUTTON;

/**
 * @~english
 * Array storing the X-coordinate of the action.
 *
 * @~french
 * Table contenant l'abscisse de l'action.
 */
real[] MOUSE_X;

/**
 * @~english
 * Array storing the Y-coordinate of the action.
 *
 * @~french
 * Table contenant l'ordonnée de l'action.
 */
real[] MOUSE_Y;

/**
 * @~english
 * The number of actor.
 *
 * The number of items stored in each table.
 *
 * @~french
 * Le nombre d'acteur.
 *
 * Le nombre de données stockées dans chacune des tables.
 *
 */
integer USER_COUNT;


/**
 * @}
 */
