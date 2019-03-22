// *****************************************************************************
// mouse.doxy.h                                                    Tao3D project
// *****************************************************************************
//
// File description:
//
//
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2011-2012, Catherine Burvelle <catherine@taodyne.com>
// (C) 2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011-2013, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************
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
 * Currently known event names are:
 * @addindex click (on -)
 * - @c click
 * @addindex mouseover (on -)
 * - @c mouseover
 *
 * In addition, the following event names are available which are not related
 * to a shape:
 * @addindex pagechange (on -)
 * - @c pagechange (since 1.18)@n
 * The code @c t associated with this event is executed when a page change
 * has just occurred and @ref page_name or @ref page_number point to the new
 * page (@ref prev_page_name and @ref prev_page_number are available to
 * get information about the page that was displayed previously).
 * If the previous page has a @ref transition, the code is excuted
 * after the transition is done.
 * @addindex key (on -)
 * - @c key, @c key:\<regexp\> (since 1.44) @n
 * The code is executed when the event that caused the current evaluation of
 * the program is a keyboard event (key press or key release).
 * If  @em :\<regexp\> is given, the code is executed only when the value of
 * @ref key_event, @ref key_name or @ref key_text matches the given regular
 * expression. The syntax is the one used by
 * <a href="http://qt-project.org/doc/qt-4.8/qregexp.html#details">QRegExp</a>.
 * @addindex keydown (on -)
 * - @c keydown, @c keydown:\<regexp\> (since 1.44) @n
 * Similar to @c key or @c key:\<regexp\>, but only applies to key press
 * events.
 * @addindex keyup (on -)
 * - @c keyup, @c keyup:\<regexp\> (since 1.44) @n
 * Similar to @c key or @c key:\<regexp\>, but only applies to key release
 * events.
 * @addindex pageentry (on -)
 * - @c pageentry (since 1.44) @n
 * The code is executed if the page is being evaluated for the first time,
 * or is being evaluated again after another page has been shown.
 * @addindex pageexit (on -)
 * - @c pageexit (since 1.44) @n
 * The code is executed immediately before a page change.
 * @~french
 * Associe du code à un événement.
 *
 * Associe le code @p t à l'événement done le nom est @p n. Ce code est exécuté
 * quand l'événement se produit sur le @a layout englobant cet appel (il doit
 * s'agir d'un bloc @c shape ou @c active_widget).
 * Les noms valides sont :
 * @addindex click (on -)
 * - @c click : clic de souris
 * @addindex mouseover (on -)
 * - @c mouseover : passage du pointeur de souris (sans clic)
 *
 * D'autre part, les noms d'évènement suivants, indépendant de @c shape ou
 * @c active_widget, peuvent être utilisés :
 * @addindex pagechange (on -)
 * - @c pagechange  (depuis 1.18)@n
 * Cet événement correspond à un changement de page. Lors le code @c t
 * s'exécute, le changement de page vient de se produire (et par conséquent,
 * @ref page_name ou @ref page_number désignent la nouvelle page ; les
 * primitives @ref prev_page_name et @ref prev_page_number permettent
 * d'obtenir des informations sur la page affichée précédemment).
 * Si une transition (cf. @ref transition) est associée à la page précédente,
 * le code @c t est exécuté lorsque la transition s'est terminée.
 * @addindex key (on -)
 * - @c key, @c key:\<regexp\> (depuis 1.44) @n
 * Le code est exécuté si l'évènement qui a provoqué l'évaluation du programme
 * est un évènement clavier. Si @em :\<regexp\> est précisée, le code n'est
 * exécuté que si la valeur de @ref key_event, @ref key_name ou @ref key_text
 * correspond à l'expression régulière donnée. La syntaxe est celle de
 * <a href="http://qt-project.org/doc/qt-4.8/qregexp.html#details">QRegExp</a>.
 * @addindex keydown (on -)
 * - @c keydown, @c keydown:\<regexp\> (depuis 1.44) @n
 * Similaire à @c key ou @c key:\<regexp\>, mais ne prend en compte que les
 * évènement correspondant à l'appui sur une touche.
 * @addindex keyup (on -)
 * - @c keyup, @c keyup:\<regexp\> (depuis 1.44) @n
 * Similaire à @c key ou @c key:\<regexp\>, mais ne prend en compte que les
 * évènement correspondant au relâchement d'une touche.
 * @addindex pageentry (on -)
 * - @c pageentry (depuis 1.44) @n
 * Le code est exécuté si la page est évaluée pour la première fois, ou est
 * évaluée de nouveau après qu'un autre page a été affichée.
 * @addindex pageexit (on -)
 * - @c pageexit (depuis 1.44) @n
 * Le code est exécuté immédiatement avant un changement de page.
 * @~
 * @see @ref refKeyboardEvents, @ref refPageEvents
 */
on (n:text, t:tree);

/**
 * @~english
 * Associates code with click.
 * Associates the specified code to the click.
 * It is equivalent to <tt> on "click", t</tt>.
 * Here is an example:
@code
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
@endcode
 *
 * @~french
 * Associe du code au clic de souris.
 * Associe le code @p t au clic de souris.
 * Équivalent à <tt> on "click", t</tt>.
 * Voici un exemple :
@code
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
@endcode
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
integer mouse_buttons ();

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
real mouse_x ();

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
real mouse_y ();

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
integer screen_mouse_x ();

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
integer screen_mouse_y ();


/**
 * @~english
 * Mouse wheel handling callback.
 * Called when the mouse wheel is moved.
 * @p y is the distance the wheel is rotated vertically, in eighths of a
 * degree. A positive value indicates that the wheel was rotated forward
 * (away from the user), while a negative value indicates that the wheel
 * was rotated backwards (towards the user).
 * @p y is the distance the wheel is rotated horizontally, in eighth of a
 * degree.@n
 * Most mice have a wheel that rotates in the vertical direction and
 * therefore @p x is usually zero. They often work in steps of 15 degrees,
 * so that @p x or @ y are usually 120 or -120. Some mice may have a
 * finer resolution, and give smaller values.
 *
 * @~french
 * Traite un événement roue de souris.
 * Cette forme est exécutée lorsque la roue de la souris est déplacée.
 * @p y est la distance parcourue par la roue verticalement, en huitièmes
 * de degrés. Une valeur positive indique que la roue a tourné en avant
 * (en s'éloignant de l'utilisateur), alors qu'une valeur négative indique
 * que la roue a tourné en arrière (vers l'utilisateur).
 * @p y est la distance parcourue par la roue horizontalement, en huitièmes
 * de degrés.@n
 * La plupart des souris ont une roue verticale, et donc @p x est en général
 * nul. Elles fonctionnent en général par pas de 15 degrés, de sorte que
 * la valeur de @p x ou @p y est souvent 120 ou -120. Mais certaines souris
 * peuvent utiliser des valeurs plus fines.
 */
wheel_event (x, y);


/**
 * @~english
 * Enable or disable the selection rectangle.
 * This primitive controls whether a selection rectangle is drawn as the left
 * mouse button is pressed and the mouse is dragged.
 * @return the previous state
 *
 * @~french
 * Active ou désactive le rectangle de sélection.
 * Cette primitive permet de contrôler si un rectangle de sélection est tracé
 * lorsque le bouton gauche est enfoncé et la souris est déplacée.
 * @return l'état précédent
 */
boolean enable_selection_rectangle (on:boolean);

/** @} */
