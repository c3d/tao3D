/**
 * @~english
 * @addtogroup Page Pages
 * @ingroup TaoBuiltins
 * This group of primitives deals with the page attributes.
 * 
 * A document is made of one or more pages. Pages are primarily identified by a name.
 * Navigation between pages is not necessarily linear, it may be built by the document's author. A default linear navigation is automaticaly build.
 *
 * Currently the page is a A4 paper in portrait format.
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
 * Regroupe les primitives de créeation et de manipulation de pages.
 *
 * Un document est généralement composé d'une ou plusieurs pages. Chaque page
 * est identifiée par un nom. Bien que les touches Page précédente et Page
 * suivante ainsi que Flèche en haut et Flèche en bas permettent par défaut de
 * parcourir les pages dans l'ordre de leur déclaration dans le document,
 * l'auteur peut choisir un ordre de parcours différent (cf. @ref page_link,
 * @ref key, @ref goto_page).
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
 * @{
 */

/**
 * @~english
 * Creates a new page or make page current.
 *
 * Creates a new page in the document with the given @p name, or add to the
 * content of a previously created page. The body contents will be evaluated
 * only when the page is being displayed. The order of pages in the document
 * is the order of page creation.
 *
 * @returns The name of the previous page.
 *
 * @~french
 * Crée une nouvelle page ou complète une page existante.
 *
 * Crée une nouvelle page nommée @p name dans le document, ou ajoute du
 * contenu à la page si elle existe déjà. Le contenu de la page (@p body) n'est
 * évalué que lorsque la page est affichée. L'ordre d'affichage des pages
 * est l'ordre de leur déclaration.
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
integer page_name (n:integer);

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
 * Returns the number of pages in the current document.
 * @~french
 * Renvoie le nombre de pages dans le document.
 */
page_count ();

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
 */
page_label ();

/**
 * @~english
 * Returns the number of the current page.
 * @~french
 * Renvoie le numéro (index) de la page courante.
 */
page_number ();

/** @} */
