/**
 * @~english
 * @addtogroup Page Pages
 * @ingroup TaoBuiltins
 * This group of primitives deals with the page attributes.
 * 
 * A document is made of one or more pages. Pages are primarily identified by a name.
 * Navigation between pages is not necessarily linear, it may be built by the
 * document's author. A default linear navigation is automaticaly build.
 *
@code
// Define a page holding an image
new_page Filename:text -> 
    page Filename, do
        color "white"
        image 0, 0, filename

// Define a single thumbnail of a page. 
// The page_thumbnail is only creating a texture. It must be applied on a shape to show it. This is why we have a rectangle.
// The active rectangle allows the user to click on the thumbnail to go to the real page.
// x, y: position (-0.5 to 0.5, relative to window size)
// s: size (relative to window size)
thumb X:real, Y:real, S:real, N:text ->
    color "white"
    page_thumbnail S, 0.02, N
    active_widget
        rectangle window_width * X, window_height * Y, window_width * S, window_height * S
        on_click 
            goto_page N

// Create a page with 4 thumbnails
page "Images", do
    thumb -0.25,  0.25, 0.4, "tortue.jpg"
    thumb  0.25,  0.25, 0.4, "papillon2.jpg"
    thumb -0.25, -0.25, 0.4, "coccinelle.jpg"
    thumb  0.25, -0.25, 0.4, "perroquet.jpg"

// Create one page per image
new_page with files "*.jpg"

@endcode
 * 
 * @~french
 * @addtogroup Page Pages
 * @ingroup TaoBuiltins
 * Regroupe les primitives de création et de manipulation de pages.
 *
 * Un document est généralement composé d'une ou plusieurs pages. Chaque page
 * est identifiée par un nom. Bien que les touches Page précédente et Page
 * suivante ainsi que Flèche en haut et Flèche en bas permettent par défaut de
 * parcourir les pages dans l'ordre de leur déclaration dans le document,
 * l'auteur peut choisir un ordre de parcours différent (cf. @ref page_link,
 * @ref on "on \"key\"", @ref refKeyDetails, @ref goto_page).
 *
 * Pour l'impression, la page est par défaut au format A4 portrait.
 *
@code
// Fonction pour créer une page qui contient une image
// Le nom de la page est le nom du fichier
new_page Filename:text ->
    page Filename, do
        color "white"
        image 0, 0, filename

// Création d'une page par image .jpg dans le dossier du document
new_page with files "*.jpg"

// Affiche un aperçu d'une page donnée
// N est le nom de la page, X et Y l'endroit où afficher le rectangle d'aperçu,
// S est le facteur de réduction de la taille de la page
// Chaque aperçu est "cliquable" pour aller à la page séléectionnée
thumb X:real, Y:real, S:real, N:text ->
    color "white"
    page_thumbnail S, 0.02, N
    active_widget
        rectangle window_width * X, window_height * Y, window_width * S, window_height * S
        on_click
            goto_page N

// Création d'une page avec 4 aperçus
page "Images", do
    thumb -0.25,  0.25, 0.4, "tortue.jpg"
    thumb  0.25,  0.25, 0.4, "papillon2.jpg"
    thumb -0.25, -0.25, 0.4, "coccinelle.jpg"
    thumb  0.25, -0.25, 0.4, "perroquet.jpg"
@endcode
 *
 * @~
 * @{
 */

/**
 * @~english
 * Creates a new page or make page current.
 *
 * Creates a new page in the document with the given @p name.
 * The body contents will be evaluated
 * only when the page is being displayed. The order of pages in the document
 * is the order of page creation. If a page with the same name already
 * an error is displayed.
 *
 * @returns The name of the previous page.
 *
 * @~french
 * Crée une nouvelle page ou complète une page existante.
 *
 * Crée une nouvelle page nommée @p name dans le document.
 * Le contenu de la page (@p body) n'est
 * évalué que lorsque la page est affichée. L'ordre d'affichage des pages
 * est l'ordre de leur déclaration. Si une page de même nom existe déjà,
 * une erreur est affichée.
 *
 * @returns Le nom de la page précédente.
 */
text page (name:text, body:tree);

/**
 * @~english
 * Goes to the given page.
 *
 * The page is identified by its name.
 *
 * @returns The name of the previously shown page.
 *
 * @~french
 * Affiche une page d'après son nom.
 *
 * Affiche la page dont le nom est @p p.
 *
 * @returns Le nom de la page précédemment affichée.
 */
text goto_page (p:text);

/**
 * @~english
 * Sets the page time to use when printing.
 *
 * As pages can change with the time, the user can set the time used when
 * printing this page. The default value is 0.0.
 *
 * @returns Previous time used for printing
 *
 * @~french
 * Définit le temps à utiliser lors de l'impression.
 *
 * Puisque le contenu d'une page peut changer au cours du temps (par exemple,
 * lorsque la page contient des animations), il est utile de pouvoir choisir
 * un temps précis à utiliser pour l'impression. La page sera imprimée telle
 * qu'affichée @p t secondes après son affichage initial.
 * La valeur par défaut est 0.0.
 *
 * @returns La valeur précédente du temps d'impression.
 */
real page_print_time (t:real);


/**
 * @~english
 * Links another page to the current page.
 *
 * Add a key binding to go from the current page to the specified page.
 * The page change is triggered when @p key is pressed.
 *
 * @returns The name of the page previously linked to this key.
 *
 * @~french
 * Relie une page à la page courante.
 *
 * Après l'appel, lorsque la page courante est affichée et que l'utilisateur
 * enfonce la touche @p key, c'est la page @p name qui est affichée.
 *
 * @returns Le nom de la page précédemment associée à la touche @p key.
 */
text page_link (key:text, name:text);

/**
 * @~english
 * Returns the name of the nth page.
 *
 * @p n is the page index. First page has index 1. If the page does not exist
 * an empty string is returned.
 *
 * @~french
 * Renvoie le nom de la n-ième page.
 *
 * @p n est l'index de la page. La première page déclarée dans le document
 * a l'index 1. Si la page n'existe pas, une chaîne vide est renvoyée.
 */
text page_name (n:integer);

/**
 * @~english
 * Generates a page thumbnail as a texture.
 *
 * @p p is the name of the page to render. The mini page is scaled by a
 * factor @p s. It is refreshed every @p i seconds.
 *
 * @~french
 * Crée une texture contenant l'aperçu de la page spécifiée.
 *
 * @p p est le nom de la page dont on veut créer un aperçu. La taille de la
 * texture est égale à la taille de la fenêtre d'affichage, multipliée par le
 * facteur @p s. Si la page @p pp contient des animations, l'aperçu sera
 * également animé. Dans ce cas, il faut noter que le rafraîchissment de
 * l'affichage a lieu toutes les @p i secondes.
 */
page_thumbnail (s:real, i:real, p:text);

/**
 * @~english
 * Save a thumbnail of the specified page to file.
 * @p w and @p h are the width and height of the picture, in pixels. @p pnum is
 * the page number, -1 for the current page. @p path is the path to the image
 * file that will be saved. Intermediate directories are created as needed. The
 * file format is automatically chosen from the file extension. If the path is
 * not absolute, it is relative to the document directory. @p ptime is
 * the value of @ref page_time to use to take the snapshot of the page
 * (default value: 0.0).
 *
 * @~french
 * Crée une miniature d'une page et l'enregistre dans un fichier.
 * @p w et @p h sont la largeur et la hauteur de l'image, en pixels. @p pnum
 * est le numéro de la page, -1 signifie la page courante. @p path est le chemin
 * sur le fichier image. Les répertoires intermédiaires sont créés
 * automatiquement si nécessaire. Le format d'image est déterminé d'après
 * l'extension du fichier. Si le chemin n'est pas absolu, il est relatif au
 * répertoire du document. @p time est la valeur de @ref page_time utilisée
 * pour faire la capture de la page (valeur par défaut : 0.0).
 */
save_page_thumbnail(w:real, h:real, pnum:text, path:text, ptime:real);

/**
 * @~english
 * Returns the number of pages in the current document.
 * @~french
 * Renvoie le nombre de pages dans le document.
 */
integer page_count ();

#if 0
/* What's the use case for page_height and page_width? */
/**
 * Returns the height of the current page.
 *
 * The page height is in pixels.
 * @todo check unit (pixels)
 */
page_height ();

/**
 * Returns the width of the current page.
 *
 * The page width is in pixels.
 * @todo check unit (pixels)
 */
page_width ();
#endif

/**
 * @~english
 * Returns the name of the current page.
 * @~french
 * Renvoie le nom de la page courante.
 * @~
 * @since 1.44
 */
text page_name();

/**
 * @~english
 * @deprecated Use @ref page_name instead.
 * @~french
 * @deprecated Utilisez @ref page_name.
 */
text page_label();

/**
 * @~english
 * Returns the number of the current page.
 * @~french
 * Renvoie le numéro (index) de la page courante.
 */
integer page_number ();

/**
 * @~english
 * Returns the name of the previously displayed page.
 * @~french
 * Renvoie le nom de la page affichée précédemment.
 * @~
 * @since 1.44
 */
text prev_page_name ();


/**
 * @~english
 * @deprecated Use @ref prev_page_name instead.
 * @~french
 * @deprecated Utilisez @ref prev_page_name.
 */
text prev_page_label ();

/**
 * @~english
 * Returns the number of the previously displayed page.
 * @~french
 * Renvoie le numéro (index) de la page affichée précédemment.
 */
integer prev_page_number ();

/**
 * @~english
 * Defines a transition period between pages.
 * This primitive executes some code during a given duration when changing
 * page. It may be used to implement transition effects. @n
 * @p Duration is the total duration of the transition in seconds.
 * @p Body is the code that will be executed during the transition. Some
 * specific primitives may be used in this code:
 * - @ref transition_current_page and @ref transition_next_page are the
 * contents of the current and the target page, respectively;
 * - @ref transition_time is the elapsed time from the start of the transition,
 * in seconds;
 * - @ref transition_duration is the total duration of the transition in
 * seconds (equal to the value of the @p Duration parameter)
 * - @ref transition_ratio indicates the amount of progress, between 0.0 and
 * 1.0.
 *
 * The target page starts as soon as the transition begins, that is: the
 * value returned by @ref page_time in the target page is 0.0 when the
 * transition begins, and @p Duration when it completes. Naturally it keeps
 * increasing as the target page is still being displayed. @n
 * The target page is not necessarily the next one in the sense of page order.
 * It may be any page when the page selection menu is used or when the
 * @ref goto_page primitive is used. @n
 * Here is an example that implements a simple dissolve effect, applied to
 * all pages:
 *
 * @~french
 * Définit une période de transition entre pages.
 * Cette primitive permet d'exécuter un code spécifique pendant une
 * certaine durée lorsqu'on quitte la page en cours. Elle permet de réaliser
 * simplement des effets de transition. @n
 * @p Duration indique la durée totale de la transition en secondes.
 * @p Body est un bloc de code qui sera exécuté pendant la transition.
 * Certaines primitives spécifique peuvent être utilisées dans ce contexte :
 * - @ref transition_current_page et @ref transition_next_page désignent
 * respectivement le contenu de la page actuellement affichée et de la page
 * qui va la remplacer;
 * - @ref transition_time est le temps écoulé depuis le début de la
 * transition;
 * - @ref transition_duration est la durée totale de la transition (égale
 * à la valeur du paramère @p Duration);
 * - @ref transition_ratio indique la progression de la transition,
 * entre 0.0 et 1.0.
 *
 * La page suivante commence dès le début de la transition, c'est-à-dire
 * que la valeur de @ref page_time dans cette page vaut 0.0 au début de
 * la transition, et vaut @p Duration à la fin de celle-ci. Naturellement,
 * elle continue à augmenter tant que la nouvelle page reste affichée. @n
 * La page suivante au sens de la transition n'est pas nécessairement celle
 * qui suit dans l'ordre des page ; c'est celle qui sera affichée après
 * la page en cours. Ce peut donc être n'importe quelle page si on
 * utilise le menu de navigation ou la primitive @ref goto_page. @n
 * Voici un exemple qui réalise un simple fondu-enchaîné pour toutes
 * les pages (la page courante disparait progressivement pendant que la
 * nouvelle apparaît) :
 *
 * @~
@code
transition 0.4,
    locally
        show 1 - transition_ratio
        transition_current_page
    locally
        show transition_ratio
        transition_next_page

page "Rectangle",
    color "blue"
    rectangle 0, 0, 200, 100

page "Circle",
    color "red"
    circle 0, 0, 100

page "Star",
    color "green"
    rotatez 5*page_time
    star 0, 0, 200, 200, 5, 0.4
@endcode
 *
 * @~french
 * Lorsque @ref transition est utilisée en dehors de toute page, elle
 * s'applique à toutes les pages. Pour définir des transitions différentes
 * pour chaque page, il suffit d'appeler @ref transition à l'intérieur de
 * @ref page. Par exemple :
 *
 * @~english
 * When @ref transition is used outside of any page, it applies to all the
 * pages. In order to define specific transitions for each page, you need to
 * call @ref transition inside the page code, as follows:
 *
 * @~
@code
transition_dissolve ->
    transition 0.4,
        locally
            show 1 - transition_ratio
            transition_current_page
        locally
            show transition_ratio
            transition_next_page

transition_zoom ->
    transition 0.4,
        if transition_ratio < 0.5 then
            S := 1 - 2 * transition_ratio
            scale S, S, S
            transition_current_page
        else
            S := 2*(transition_ratio - 0.5)
            scale S, S, S
            transition_next_page

page "Rectangle",
    transition_dissolve
    color "blue"
    rectangle 0, 0, 200, 100

page "Circle",
    color "red"
    circle 0, 0, 100

page "Star",
    transition_zoom
    color "green"
    rotatez 5*page_time
    star 0, 0, 200, 200, 5, 0.4
@endcode
 *
 * @~french
 * Dans l'exemple ci-dessus, on a une transition en fondu-enchaîné lorsqu'on
 * quitte la première page (le rectangle) et un effet de zoom en quittant la
 * dernière page (l'étoile). La deuxième page (le cercle) ne contient pas
 * d'appel à @ref transition, il n'y a donc aucun effet particulier lorsqu'on
 * quitte cette page (la suivante est affichée immédiatement).
 * @~english
 * In the above example, a "dissolve" transition is applied when leaving
 * the first page. The third page triggers a "zoom" transition on exit.
 * The second page, however, executes no transition because it does not call
 * @ref transition. Therefore the next page is shown immediately.
 */
transition (Duration:real, Body:code);

/**
 * @~english
 * Returns the elapsed time since the beginning of a transition.
 * When executed in a @ref transition block, it returns the elapsed time
 * since the beginning of a transition in seconds. Otherwise it returns 0.
 * @~french
 * Renvoie le temps écoulé depuis le début d'une transition.
 * Lorsque cette primitive est exécutée dans un bloc @ref transition, elle
 * renvoie le temps écoulé depuis le début de la transition, en secondes.
 * En cas contraire elle renvoie 0.
 */
real transition_time();

/**
 * @~english
 * Returns the percentage of completion of a transition.
 * When executed in a @ref transition block, it returns
 * (@ref transition_time / @ref transition_duration). Otherwise it
 * returns 0.
 * @~french
 * Renvoie le pourcentage de complétion d'une transition.
 * Lorsque cette primitive est exécutée dans un bloc @ref transition, elle
 * renvoie (@ref transition_time / @ref transition_duration). En cas
 * contraire elle renvoie 0.
 */
real transition_ratio();

/**
 * @~english
 * Returns the total duration of a transition.
 * When executed in a @ref transition block, it returns the value of the
 * @p Duration parameter.
 * @~french
 * Renvoie la duréee totale d'une transition.
 * Lorsque cette primitive est exécutée dans un bloc @ref transition, elle
 * renvoie la valeur du parametre @p Duration. En cas contraire elle renvoie
 * 0.
 */
real transition_duration();

/**
 * @~english
 * The contents of the current page during a transition.
 * When executed in a @ref transition block, it evaluates to the contents of
 * the current page. Otherwise, it evaluates to @c false.
 * @~french
 * Le contenu de la page courante lors d'une transition.
 * Lorsque cette primitive est exécutée dans un bloc @ref transition, elle
 * représente le contenu de la page en cours. En cas contraire elle équivaut
 * à @c false.
 */
transition_current_page();


/**
 * @~english
 * The contents of the target page during a transition.
 * When executed in a @ref transition block, it evaluates to the contents of
 * the page that will replace the current one. Otherwise, it evaluates to
 * @c false.
 * @~french
 * Le contenu de la page cible lors d'une transition.
 * Lorsque cette primitive est exécutée dans un bloc @ref transition, elle
 * représente le contenu de la page qui remplacera la page en cours.
 * En cas contraire elle équivaut à @c false.
 */
transition_next_page();

/**
 * @~english
 * Skip to the next page.
 * This function is implemented as follows:
 * @~french
 * Affiche la page suivante.
 * Cette fonction est implémentée de la façon suivante:
 * @~
@code
next_page ->
    if page_number < page_count then
        goto_page page_name(page_number+1)
        refresh 0.0
@endcode
 */
next_page();

/**
 * @~english
 * Skip to the previous page.
 * This function is implemented as follows:
 * @~french
 * Affiche la page précédente.
 * Cette fonction est implémentée de la façon suivante:
 * @~
@code
previous_page ->
    if page_number > 1 then
        goto_page page_name(page_number-1)
        refresh 0.0
@endcode
 */
previous_page();

/** @} */
