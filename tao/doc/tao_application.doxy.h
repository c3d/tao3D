/**
 * @~english
 * @addtogroup TaoGuiInter Tao Interface Interaction
 * @ingroup TaoGui
 * Tao3D interface.
 * Tao Presentation graphical interface can be modified by an XL document.
 *
 * @~french
 * @addtogroup TaoGuiInter Configuration de l'interface Tao
 * @ingroup TaoGui
 * L'interface graphique de Tao3D peut être modifiée par le
 * document.
 *
 * @~
 * @{
 */


/**
 * @~english
 * Set the message in the status bar.
 * Show text @p t until next message.
 *
 * @~french
 * Affiche un message dans la barre d'état.
 * Le message persiste jusqu'au prochain appel, ou jusqu'à ce que Tao
 * Presentations affiche un autre message.
 */
status(t:text);

/**
 * @~english
 * Set the message in the status bar.
 * Show text @p t until next message or for @p d seconds. If no other
 * message is displayed after @p d seconds, the message is cleared.
 *
 * @~french
 * Affiche un message dans la barre d'état.
 * Le message persiste jusqu'au prochain appel, ou jusqu'à ce que Tao
 * Presentations affiche un autre message, ou pendant @p d secondes
 * au maximum. Si aucun autre message n'est affiché après @p d secondes,
 * le texte s'efface automatiquement.
 */
status(t:text, d:real);

/**
 * @~english
 * Controls source code panel.
 * @p src true show the panel, @p src false hide it.
 *
 * @~french
 * Affiche ou cache la fenêtre de code source.
 * Affiche la fenêtre si @p src vaut @c true, la cache si @p src vaut
 * @c false.
 */
show_source(src:boolean);


/**
 * @~english
 * Controls error panel.
 * @p err true show the panel, @p err false hide it.
 *
 * @~french
 * Affiche ou cache la fenêtre d'erreur.
 * Affiche la fenêtre si @p err vaut @c true, la cache si @p src vaut
 * @c false.
 */
show_error(err:boolean);


/**
 * @~english
 * Controls time dependant animations.
 * @p animations true run animations, @p animations false stop them.
 * @returns the previous value.
 *
 * @~french
 * Active ou désactive les animations.
 * Lorsque @p animations vaut @c false, le temps est virtuellement figé.
 * @returns la valeur précédente.
 */
boolean enable_animations(animations:boolean);


/**
 * @~english
 * Controls the display of rendering statistics.
 *
 * Shows or hides fps statistics.
 * @returns the previous value.
 *
 * @~french
 * Active ou désactive les statistiques d'affichage.
 * @returns la valeur précédente.
 */
boolean show_statistics(flag:boolean);


/**
 * @~english
 * Reset document view to default parameters.
 * The related primitives are :
 *
 * @~french
 * Réinitialise les paramètres d'affichage.
 * Les primitives concernées sont :
 * @~
 * - @ref camera_position
 * - @ref camera_target
 * - @ref camera_up_vector
 * - @ref zoom
 */
reset_view();


/**
 * @~english
 * Returns the current zoom factor.
 * @~french
 * Renvoie le facteur de zoom courant.
 */
real zoom();


/**
 * @~english
 * Sets the zoom factor.
 * @~french
 * Définit le facteur de zoom.
 */
zoom( factor:real);


/**
 * @~english
 * Zooms out.
 * The zoom factor is multiplied by 0.25.
 * @~french
 * Réduit le zoom.
 * Le facteur de zoom est multiplié par 0.25.
 */
zoom_out();


/**
 * @~english
 * Zooms in.
 * The zoom factor is divided by 0.25.
 * @~french
 * Augmente le zoom.
 * Le facteur de zoom est divisé par 0.25.
 */
zoom_in();


/**
 * @~english
 * Toggle the cursor.
 * Toggle between hand (panning) and arrow (selection) cursors.
 * @~french
 * Change le curseur de souris.
 * Passe du curseur de sélection, en forme de flèche, au curseur de
 * déplacement, en forme de main.
 */
toggle_hand_cursor();


/**
 * @~english
 * The current value of the frame counter.
 * Returns the number of frames (images) that have been displayed so far for the
 * current document. You may use this value for instance to compute
 * frame-per-second statistics, as in the following code:
 * @~french
 * La valeur actuelle du compteur de trames.
 * Renvoie le nombre d'images qui ont été affichées depuis l'ouverture du
 * document courant. Cette primitive permet par exemple de calculer la
 * vitesse d'affichage en images par seconde, comme dans l'exemple suivant.
 * @~
@code
page "p1",
    goto_page "p2"

T0 := 0.0
C0 := 0
Message := "---"
FPS := 0.0
page "p2",
    locally
        if T0 = 0 then
            T0 := page_time
            C0 := frame_count

        locally
            rotatez 10*page_time
            color "blue"
            rectangle 0, 0, 200, 100

        text_box 0, -window_height/2 + 50, 300, 100,
            font "Arial", 30
            align_center
            text Message

        refresh 0.0

    locally
        if page_time > 2 then
            FPS := (frame_count-C0)/(page_time-T0)
            Message := "FPS = " & text FPS
            T0 := 0

        no_refresh_on TimerEvent
        refresh 2.0
@endcode
 */
integer frame_count();

/**
 * @~english
 * Opens the specified URL with an external application.
 * For example:
 * @~french
 * Ouvre l'URL avec une application externe.
 * Par exemple:
 * @~
@code
enable_selection_rectangle false
Clicked -> 0.0
page "URL",
    active_widget
        if page_time < Clicked + 0.1 then
            color "gray"
        else
            color "darkblue"
        rounded_rectangle 0, 0, 300, 100, 20
        URL -> "http://google.com"
        text_box 0, 0, 300, 100,
            vertical_align_center
            align_center
            color "white"
            font "Arial", 30
            text URL
        on_click
            open_url URL
            Clicked := page_time
@endcode
 */
boolean open_url(url:text);

/** @} */
