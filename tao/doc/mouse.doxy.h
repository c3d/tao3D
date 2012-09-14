/**
 * @~english
 * @addtogroup mouse Mouse and selection
 * @ingroup TaoBuiltins
 * This group deals with mouse location, mouse events and item
 * selection.
 *
 * @~french
 * @addtogroup mouse Souris et sélection
 * @ingroup TaoBuiltins
 * Cette page regroupe les primitives relatives à la position de la souris et
 * aux événements souris, et à la sélection de formes géométriques.
 *
 * @~
 * @{
 */
/**
 * @~english
 * Associates the specified code to the named event.
 *
 * Associate the specified code @p t and the action named @p n. This code is
 * executed when the action occurs on the current @c shape or @c active_widget.
 * Currently known event name are
 * - @c click
 * - @c mouseover
 *
 * @~french
 * Associe du code à un événement.
 *
 * Associe le code @p t à l'événement done le nom est @p n. Ce code est exécuté
 * quand l'événement se produit sur le @a layout englobant cet appel (il doit
 * s'agir d'un bloc @c shape ou @c active_widget).
 * Les noms valides sont :
 * - @c click : clic de souris
 * - @c mouseover : passage du pointeur de souris (sans clic)
 */
on (n:text, t:tree);

/**
 * @~english
 * Associates code with click.
 * Associates the specified code to the click.
 * It is equivalent to <tt> on "click", t</tt>.
 * Here is an example:
 * @code
enable_selection_rectangle false
Clicked -> 0.0
active_widget
    locally
        if (time < Clicked + 0.1) then
            color "blue"
        else
            color "darkblue"
        rounded_rectangle 0, 0, 150, 60, 20
        text_box 0, 0, 150, 60,
            vertical_align_center
            align_center
            font "Arial", 28
            color "white"
            text "Click me!"
    on_click
        writeln "Click!"
        Clicked := time
 * @endcode
 *
 * @~french
 * Associe du code au clic de souris.
 * Associe le code @p t au clic de souris.
 * Équivalent à <tt> on "click", t</tt>.
 * Voici un exemple :
 * @code
enable_selection_rectangle false
Cliqué -> 0.0
active_widget
    locally
        if (time < Cliqué + 0.1) then
            color "blue"
        else
            color "darkblue"
        rounded_rectangle 0, 0, 160, 60, 20
        text_box 0, 0, 160, 60,
            vertical_align_center
            align_center
            font "Arial", 24
            color "white"
            text "Cliquez-moi !"
    on_click
        writeln "Clic!"
        Cliqué := time
 * @endcode
 */
on_click (t:tree);

/**
 * @~english
 * Associates code with mouse over.
 * Associates the specified code to the mouse over.
 * It is equivalent to <tt> on "mouseover", t</tt>.
 *
 * @~french
 * Associe du code au passage de la souris.
 * Associe le code @p t au passage de la souris.
 * Équivalent à <tt> on "mouseover", t</tt>.
 */
on_mouseover (t:tree);

/**
 * @~english
 * Returns the buttons of the last mouse event.
 *
 * It allows to access to the last mouse button event. It is an OR combination of following values:<BR>
 * <TABLE>
 *<TR><TH> Qt name</TH> <TH>Hexadecimal value</TH><TH>Description</TH> </TR>
 *<TR><TD> Qt::NoButton</TD><TD> 0x00000000</TD><TD>The button state does not refer to any button (see QMouseEvent::button()).</TD></TR>
 *<TR><TD> Qt::LeftButton</TD><TD> 0x00000001</TD><TD>The left button is pressed, or an event refers to the left button. (The left button may be the right button on left-handed mice.)</TD></TR>
 *<TR><TD> Qt::RightButton</TD><TD> 0x00000002</TD><TD>The right button.</TD></TR>
 *<TR><TD> Qt::MidButton</TD><TD> 0x00000004</TD><TD>The middle button.</TD></TR>
 * </TABLE>
 *
 * @~french
 * Renvoie les boutons utilisés lors du dernier événement souris, soit une combinaison des valeurs
 * suivantes:<BR>
 * <TABLE>
 *<TR><TH> Nom Qt</TH> <TH>Valeur</TH><TH>Description</TH> </TR>
 *<TR><TD> Qt::NoButton</TD><TD> 0x00000000</TD><TD>Aucun bouton.</TD></TR>
 *<TR><TD> Qt::LeftButton</TD><TD> 0x00000001</TD><TD>Le bouton gauche (celà peut être le bouton droit sur une souris pour gaucher)</TD></TR>
 *<TR><TD> Qt::RightButton</TD><TD> 0x00000002</TD><TD>Le bouton droit.</TD></TR>
 *<TR><TD> Qt::MidButton</TD><TD> 0x00000004</TD><TD>Le bouton du milieu.</TD></TR>
 * </TABLE>
 */
mouse_buttons ();

/**
 * @~english
 * Returns the current X position of the mouse in the local frame.
 * The returned value is the X coordinate of the mouse pointer after
 * projection in the plane Z=0 for the current geometric transformation.
 * In other words, if you draw a point at coordinates (mouse_x, mouse_y)
 * it will always follow the mouse. But the value you get is usually not
 * the position of the mouse in screen coordinates.
 *
 * Note however that the default zoom level and camera position are such
 * that mouse_x corresponds to pixels with (X=0, Y=0) being the center of
 * the screen, the positive X-axis pointing right, and the positive Y-axis
 * pointing up.
 * @see screen_mouse_x
 *
 * @~french
 * Renvoie l'abscisse actuelle de la souris dans le repère local.
 * La valeur renvoyée par cette primitive est la coordonnée X du pointeur de
 * souris après projection dans le plan Z = 0 de la transformation
 * géométrique courante.
 * Autrement dit, si vous tracez un point en (mouse_x, mouse_y) il suivra
 * toujours la souris. Mais la valeur que vous obtenez n'est en général pas
 * la position de la souris en coordonnées écran.
 *
 * Le zoom par défaut et la position de la caméra sont tels que la valeur
 * mouse_x correspond à des pixels, (X=0, Y=0) étant le centre de l'écran,
 * l'axe X étant vers la droite, l'axe Y étant vers le haut.
 */
mouse_x ();

/**
 * @~english
 * Returns the current Y position of the mouse in the local frame.
 * The returned value is the Y coordinate of the mouse pointer after
 * projection in the plane Z=0 for the current geometric transformation.
 * In other words, if you draw a point at coordinates (mouse_x, mouse_y)
 * it will always follow the mouse. But the value you get is usually not
 * the position of the mouse in screen coordinates.
 *
 * Note however that the default zoom level and camera position are such
 * that mouse_y corresponds to pixels with (X=0, Y=0) being the center of
 * the screen, the positive X-axis pointing right, and the positive Y-axis
 * pointing up.
 * @see screen_mouse_y
 *
 * @~french
 * Renvoie l'ordonnée actuelle de la souris dans le repère local.
 * La valeur renvoyée par cette primitive est la coordonnée Y du pointeur de
 * souris après projection dans le plan Z = 0 de la transformation
 * géométrique courante.
 * Autrement dit, si vous tracez un point en (mouse_x, mouse_y) il suivra
 * toujours la souris. Mais la valeur que vous obtenez n'est en général pas
 * la position de la souris en coordonnées écran.
 *
 * Le zoom par défaut et la position de la caméra sont tels que la valeur
 * mouse_y correspond à des pixels, (X=0, Y=0) étant le centre de l'écran,
 * l'axe X étant vers la droite, l'axe Y étant vers le haut.
 */
mouse_y ();

/**
 * @~english
 * Returns the current X position of the mouse in screen coordinates.
 * Coordinates are in pixels. The left of the display area is at X coordinate
 * 0, while the right is at X = @ref window_width.
 *
 * @~french
 * Renvoie l'abscisse de la souris en coordonnées écran.
 * Les coordonnées sont en pixels. Le bord gauche de la zone d'affichage est
 * à l'abscisse X = 0, tandis que le bord droit est en
 * X = @ref window_width.
 */
screen_mouse_x ();

/**
 * @~english
 * Returns the current Y position of the mouse in screen coordinates.
 * Coordinates are in pixels. The bottom of the display area is at Y coordinate
 * 0, while the top is at Y = @ref window_height.
 *
 * @~french
 * Renvoie l'ordonnée de la souris en coordonnées écran.
 * Les coordonnées sont en pixels. Le bas de la zone d'affichage est
 * à l'ordonnée Y = 0, tandis que le haut est en
 * Y = @ref window_height.
 */
screen_mouse_y ();


/**
 * @~english
 * Mouse wheel handling callback.
 * Called when the mouse wheel is moved.
 * By default, a mouse wheel event with control key down zooms in or out.
 * With control key up, it moves the viewpoint.
 *
 * @~french
 * Traite un événement roue de souris.
 * Cette forme est exécutée lorsque la roue de la souris est déplacée.
 * Par défaut, la roue de la souris permet de zoomer lorsque la touche
 * contrôle est simultanément enfoncée, et permet de déplacer le point de vue
 * dans le cas contraire.
 */
wheel_event (x, y);


/**
 * @~english
 * Enable or disable the selection rectangle.
 * This primitive controls whether a selection rectangle is drawn as the left
 * mouse button is pressed and the mouse is dragged.
 *
 * @~french
 * Active ou désactive le rectangle de sélection.
 * Cette primitive permet de contrôler si un rectangle de sélection est tracé
 * lorsque le bouton gauche est enfoncé et la souris est déplacée.
 */
enable_selection_rectangle (on:boolean);

/** @} */
