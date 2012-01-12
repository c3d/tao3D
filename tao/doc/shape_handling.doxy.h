/**
 * @~english
 * @defgroup shape_handling Shape handling
 * @ingroup Graphics
 *
 * Tools to group, play with forward / backward, etc...
 * 
 * @~french
 * @defgroup shape_handling Manipulation interactive des formes géométriques
 * @ingroup Graphics
 *
 * Groupage, mise au premier plan ou à arrière-plan, etc.
 *
 * @~
 * @{
 */

/**
 * @~english
 * Creates a logical group around shapes.
 * The group can then be transformed as one object.
 * @~french
 * Crée un groupe de plusieurs blocs shape
 * Le groupe peut ensuite être manipulé comme un objet unique.
 */
group(shapes:tree);

/**
 * @~english
 * Groups the selection.
 * The selected shapes are placed in a @ref group .
 * This function is to be used once from a graphical interface and not aimed to be placed in a document.
 *
 * @~french
 * Groupe la selection.
 * Les formes géométriques qui sont dans des blocs @ref shape sont placés dans
 * un @ref group.
 * Cette fonction est prévue pour être utilisée depuis l'interface graphique,
 * et non pour être placée dans le document.
 */
group_selection();

/**
 * @~english
 * Ungroups the selection.
 * The selected group is ungrouped.
 * This function is to be used once from a graphical interface and not aimed to be placed in a document.
 *
 * @~french
 * Dégroupe la selection.
 * Cette fonction est prévue pour être utilisée depuis l'interface graphique,
 * et non pour être placée dans le document.
 */
ungroup_selection();


/**
 * @~english
 * Bring forward.
 * Bring the selected shape one place forward.
 * This function is to be used once from a graphical interface and not aimed to be placed in a document.
 *
 * @~french
 * Avance la forme géométrique.
 * Déplace la forme sélectionnée en avant dans la liste des formes.
 * Cette fonction est prévue pour être utilisée depuis l'interface graphique,
 * et non pour être placée dans le document.
 */
bring_forward();

/**
 * @~english
 * Bring to front.
 * Bring the selected shape to front place.
 * This function is to be used once from a graphical interface and not aimed to be placed in a document.
 *
 * @~french
 * Met la forme géométrique au premier plan.
 * Cette fonction est prévue pour être utilisée depuis l'interface graphique,
 * et non pour être placée dans le document.
 */
bring_to_front();

/**
 * @~english
 * Send backward.
 * Send the selected shape one place backward.
 * This function is to be used once from a graphical interface and not aimed to be placed in a document.
 *
 * @~french
 * Recule la forme géométrique.
 * Déplace la forme sélectionnée en arrière dans la liste des formes.
 * Cette fonction est prévue pour être utilisée depuis l'interface graphique,
 * et non pour être placée dans le document.
 */
send_backward();

/**
 * @~english
 * Send to back.
 * Send the selected shape to the last place.
 * This function is to be used once from a graphical interface and not aimed to be placed in a document.
 *
 * @~french
 * Met la forme géométrique en arrière plan.
 * Cette fonction est prévue pour être utilisée depuis l'interface graphique,
 * et non pour être placée dans le document.
 */
send_to_back();

/**
 * @~english
 * Set the polygon offset factors.
 * @param f0 factor base
 * @param f1, factor increment
 * @param u0, unit base
 * @param u1, unit increment
 * @return [integer] The current polygon offset.
 *
 * @~french
 * Contrôle les paramètres de polygon offset.
 */
polygon_offset(f0:real, f1:real, u0:real, u1:real);


/**
 * @~english
 * Set an attribute value.
 * It set or update an attribute of a shape or of a group. 
 * The @p attribute identifies the attribute to add or modify and @p value is 
 * the code that represent the attribute value.
 * This function is to be used once from a graphical interface and not aimed to be placed in a document.
 *
 * It is used by color chooser action to set the color (or line_color) of shapes.
 *
 * @~french
 * Définit la valeur d'un attribut pour un bloc shape ou un groupe.
 * Le paramètre @p attribute est le nom de l'attribut à ajouter ou modifier,
 * et @p value est sa valeur.
 * Cette fonction est prévue pour être utilisée depuis l'interface graphique,
 * et non pour être placée dans le document.
 * Elle est utilisée par l'action de sélection de couleur pour définir la
 * couleur de remplissage ou la couleur de ligne d'un bloc @ref shape.
 */
set_attribute(attribute:text, value:tree);

/**
 * @~english
 * Delete selection.
 * If a text is selected, replace the current selected text by the @p key value.
 * If it is a non text element that is selected, just suppresses the element.
 *
 * @~french
 * Supprime la sélection.
 * Si un texte est sélectionné, remplace ce texte par @p key.
 */
delete_selection(key:text);
/**
 * @}
 */
