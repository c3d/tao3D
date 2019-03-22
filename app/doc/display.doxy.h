// *****************************************************************************
// display.doxy.h                                                  Tao3D project
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
// (C) 2012, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2012, Catherine Burvelle <catherine@taodyne.com>
// (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
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
 * @defgroup Display Display and stereoscopy controls
 * @ingroup TaoBuiltins
 *
 * Select display mode and stereoscopic 3D settings.
 * Tao3D can take advantage of several display technologies
 * to show 3D contents. Depending on the display hardware at your
 * disposal, you may select one of the display modes supported by
 * the application. If you have a 3D-capable screen or projector,
 * chances are that it can be adequately used with one of the display
 * modes listed on this page.
 *
 * Here is an illustration of the modes currently supported by Tao
 * Presentations (see @ref display_mode):
 * @image html 2D_small.png "Default (\"default\",  \"2D\")"
 * @image html anaglyph_small.png "Red/blue anaglyph (\"anaglyph\")"
 * @image html anaglyph_detail.png "Red/blue anaglyph (detail)"
 * @image html hsplitstereo_small.png "Side-by-side (\"hsplitstereo\", \"sidebysidestereo\")"
 * @image html vsplitstereo_small.png "Over/under (\"vsplitstereo\",  \"overunderstereo\")"
 * @image html hintstereo_small.png "Interlaced (\"hintstereo\")"
 * @image html hintstereo_detail.png "Interlaced (detail)"
 * @image html checkerboard_small.png "Checkerboard (\"checkerboard\")"
 * @image html checkerboard_detail.png "Checkerboard (detail)"
 * @image html hframepacking_small.png "Horizontal frame packing (\"hframepacking\")"
 * @image html vframepacking_small.png "vertical frame packing (\"vframepacking\")"
 * @image html alioscopy_small.png "Alioscopy® (\"alioscopy\")"
 * @image html alioscopy_detail.png "Alioscopy® (detail)"
 *
 * @~french
 * @defgroup Display Affichage et contrôle de la stéréoscopie
 * @ingroup TaoBuiltins
 *
 * Permet de sélectionner le mode d'affichage et les paramètres de 3D
 * stéréoscopique.
 * Tao3D peut tirer parti de différentes technologies pour afficher
 * du contenu 3D. En fonction du matériel dont vous disposez, vous pouvez
 * sélectionner l'un des modes d'affichage disponibles dans l'application.
 * Si vous avez un écran ou un projecteur capable d'afficher en 3D, il y
 * a de fortes chances que Tao3D vous permette de l'exploiter au
 * mieux.
 *
 * Voici une illustration des modes actuellement supportés par Tao
 * Presentations (cf. @ref display_mode):
 * @image html 2D_small.png "Défaut (\"default\",  \"2D\")"
 * @image html anaglyph_small.png "Anaglyph rouge/bleu (\"anaglyph\")"
 * @image html anaglyph_detail.png "Anaglyph rouge/bleu (détail)"
 * @image html hsplitstereo_small.png "Côte-à-côte (\"hsplitstereo\", \"sidebysidestereo\")"
 * @image html vsplitstereo_small.png "Dessus/dessous (\"vsplitstereo\",  \"overunderstereo\")"
 * @image html hintstereo_small.png "Entrelacé (\"hintstereo\")"
 * @image html hintstereo_detail.png "Entrelacé (détail)"
 * @image html checkerboard_small.png "Damier (\"checkerboard\")"
 * @image html checkerboard_detail.png "Damier (détail)"
 * @image html hframepacking_small.png "Frame packing horizontal (\"hframepacking\")"
 * @image html vframepacking_small.png "Frame packing vertical (\"vframepacking\")"
 * @image html alioscopy_small.png "Alioscopy® (\"alioscopy\")"
 * @image html alioscopy_detail.png "Alioscopy® (détail)"
 *
 * @~
 * @{
 */

/**
 * @~english
 * Controls stereoscopy.
 * When @p mode is @c false, stereoscopic output is disabled, and
 * the 2D output mode is selected.
 * When @p mode is @c true, the quad-buffer stereoscopic output
 * is enabled, if supported by the hardware. If this mode is not
 * supported, the primitive does nothing.
 * @param mode is @c true or @c false.
 *
 * @~french
 * Active ou désactive la stéréoscopie.
 * Lorsque @p mode est @c false, l'affichage stéréoscopique est désactivé,
 * et le mode 2D est actif.
 * Lorsque @p mode est @c true, l'affichage stéréoscopique quad-buffer
 * est activé, si le matériel le supporte. Si ce n'est pas le cas, il ne se
 * passe rien.
 * @param mode est @c true ou @c false.
 */
enable_stereoscopy(mode:name);

/**
 * @~english
 * Select the display mode.
 * @param mode The name of the display mode. Currently supported
 * modes are:
 * - "default", "2D": two-dimensional display.
 * - "quadstereo": OpenGL quad-buffered stereoscopic 3D output.
 *   Exact video output format depends on the OpenGL driver.
 * - "anaglyph": stereoscopic 3D on a regular screen with red/blue
 *   glasses.
 * - "hsplitstereo", "sidebysidestereo": stereoscopic 3D with horizontal
 *   split enconding. The left half of the picture is for the left eye,
 *   the right half is for the right eye. Each half represents the full
 *   scene and is compressed horizontally. For instance, when rendering to
 *   a 1920x1080 (HDTV) screen you will get two 960x1080 images side-by-side.
 *   This format is typically supported by many consumer-grade 3D HDTV.
 * - "vsplitstereo", "overunderstereo": stereoscopic 3D with vertical
 *   split encoding. Similar to horizontal split (above), but vertically.
 * - "hintstereo": stereoscopic 3D with horizontal interlace encoding.
 *   Some 3D capable laptops (with passive glasses) expect this format.
 * - "checkerboard": stereoscopic 3D with checkerboard encoding.
 * - "hframepacking": stereoscopic 3D output in horizontal frame packing
 *   mode. This is similar to the horizontal side-by-side
 *   disposition ("hsplitstereo"), but without horizontal compression.
 * - "vframepacking": stereoscopic 3D output in verical frame packing
 *   mode. Like the vertical over/under disposition, but without vertical
 *   compression.
 * - "alioscopy": suitable encoding for
 *   <a href="http://www.alioscopy.com">Alioscopy® glass-free 3D
 *   displays</a>.
 * - "tridelity": suitable encoding for
 *   <a href="http://www.tridelity.com">Tridelity® glass-free 3D Multi-View
 *   displays</a>.
 * @note Display modes are implemented as modules. Some modes may
 * or may not be available, depending on your version of Tao3D.
 * You can obtain a list of all
 * valid mode strings for your configuration by running Tao3D
 * with the @c -tdisplaymode command-line parameter. Look for output
 * similar to:
 * @~french
 * Selectionne le mode d'affichage.
 * @param mode Le nom du mode d'affichage. Les valeurs possibles sont :
 * - "default", "2D" : affichage en deux dimensions.
 * - "quadstereo" : 3D stéréoscopique basé sur le quad-buffer OpenGL.
 *   Le format de sortie exact dépend du pilote OpenGL et de ses réglages.
 * - "anaglyph" : 3D stéréoscopique sur écran classique avec des
 *   lunettes rouge et bleu.
 * - "hsplitstereo", "sidebysidestereo" : 3D stéréoscopique en mode de
 *   partage horizontal. La moitié gauche de l'écran est pour l'œil gauche,
 *   la moitié droite et pour l'œil droit. Chaque moitié représente la scène
 *   complète et est compressée horizontalement. Par exemple, lorsque la
 *   résolution d'affichage est 1920x1080, on obtient deux images de 960x1080
 *   côte-à-côte. Ce format convient en général à la plupart des téléviseurs
 *   3D grand public.
 * - "vsplitstereo", "overunderstereo" : 3D stéréoscopique en mode de
 *   partage vertical. Similaire au mode partage horizontal (ci-dessus),
 *   mais verticalement.
 * - "hintstereo" : 3D stéréoscopique en mode entrelacé. Certains ordinateurs
 *   portables avec lunettes passives demandent ce format.
 * - "checkerboard" : 3D stéréoscopique en mode damier.
 * - "hframepacking" : 3D stéréoscopique en mode frame packing horizontal.
 *   Ce mode est similaire au partage horizontal ("hsplitstereo"), mais sans
 *   compression horizontale.
 * - "vframepacking" : 3D stéréoscopique en mode frame packing vertical.
 *   Ce mode est similaire au partage vertical ("vsplitstereo"), mais sans
 *   compression verticale.
 * - "alioscopy": un encodage adapté aux
 *   <a href="http://www.alioscopy.com">écrans 3D Alioscopy® sans lunettes</a>.
 * - "tridelity": un encodage adapté aux
 *   <a href="http://www.tridelity.com">écrans Multi-Vue 3D Tridelity® sans lunettes</a>.
 * @note Les modes d'affichage sont implémentés sous forme de modules. Certains
 * peuvent ne pas être disponibles, selon votre version de Tao3D.
 * Vous pouvez obtenir une liste de tous les modes valides dans votre
 * configuration en démarrant Tao3D avec l'option
 * @c -tdisplaymode :
 * @~
@code
[Display Driver] Registering display function: checkerboard@0x1b4148c0
[Display Driver] Registering display function: hsplitstereo@0x1c76c110
[Display Driver] Registering alias: sidebysidestereo => hsplitstereo@0x1c76c110
@endcode
 * @~english
 * Display modes can also be selected with the command chooser (Escape
 * key), or with the View/Display mode menu.
 * @return @c true on success, @c false otherwise.
 * @~french
 * Les modes d'affichage peuvent également être sélectionnés par le menu
 * interactif (touche Échap), ou par le menu Affichage/Mode d'affichage.
 * @return @c true en cas de succès, sinon @c false.
 */
boolean display_mode(mode:text);

/**
 * @~english
 * The name of the current display mode.
 * @~french
 * Le nom du mode d'affichage actuel.
 */
text display_mode();

/**
 * @~english
 * Test if a display mode is available.
 * @~french
 * Teste si un mode d'affichage est disponible.
 * @see display_mode, display_mode(mode:text)
 */
boolean has_display_mode(mode:text);

/**
 * @~english
 * Get eye distance.
 * @return [real] eye distance.
 * @~french
 * Renvoie la distance inter-occulaire.
 * @return [réel] la distance inter-occulaire.
 */
real eye_distance();

/**
 * @~english
 * Set eye distance.
 * @~french
 * Définit la distance inter-occulaire.
 */
eye_distance(e:real);

/**
 * @~english
 * Enter or leave presentation mode.
 * When @p on is @c true:
 * - the current document occupies the whole screen area (see @ref full_screen),
 * - the screen saver is disabled,
 * - the mouse pointer hides automatically after a couple of seconds, and becomes
 * visible again if the mouse is moved (see @ref auto_hide_cursor).
 *
 * When @p on is @c false, the document is shown in the default windowed
 * mode. When leaving the presentation mode, the previous
 * dimensions and position of the window are restored, the screen saver is not
 * disabled anymore, and the mouse pointer does no longer hide automatically.
 * @returns @c true if the document was in presentation mode before the call,
 * @c false otherwise.
 *
 * @~french
 * Active ou désactive le mode présentation.
 * Si @p on vaut @c true :
 * - le document courant est affiché en mode « plein écran » (cf.
 * @ref full_screen)
 * - l'économiseur d'écran est désactivé,
 * - le pointeur de la souris disparaît automatiquement après quelques secondes
 * de non-utilisation (il s'affiche de nouveau si l'on bouge la souris).
 *
 * Si @p on vaut @c false, le document s'affiche en mode fenêtré. Les dimensions
 * ainsi que la position de la fenêtre principale reprennent leur valeurs
 * précédentes, l'économiseur d'écran n'est plus désactivé et le pointeur de
 * la souris s'affiche normalement.
 * @returns @c true si le mode présentation était actif avant l'appel, @c false
 * sinon.
 *
 * @~
 * @see toggle_slide_show, full_screen, toggle_full_screen, auto_hide_cursor,
 * toggle_auto_hide_cursor
 */
boolean slide_show(on:boolean);

/**
 * @~english
 * Switch between windowed and slideshow modes.
 * @returns @c true if the previous state was presentation mode,
 * @c false otherwise.
 *
 * @~french
 * Passe du mode fenêtré au mode présentation et réciproquement.
 * @returns @c true si le mode présentation était actif avant l'appel, @c false
 * sinon.
 * @~
 * @see slide_show, full_screen, toggle_full_screen
 */
boolean toggle_slide_show();

/**
 * @~english
 * Enter or leave full screen mode.
 * When @p on is @c true, the application uses the whole screen to display the
 * current document. The screen to which the display is maximized is the one
 * where the window was previously located. When @p on is @c false, the normal
 * windowed mode is activated. When leaving the full screen mode, the previous
 * dimensions and position of the window are restored.
 * @returns @c true if the previous state was full screen, @c false otherwise.
 * @~french
 * Active ou désactive le mode plein écran.
 * Lorsque @p on vaut @c true, la zone d'affichage couvre entièrement la surface
 * de l'écran sur lequel se trouvait précédemment la fenêtre principale de
 * l'application. Lorsque @p on vaut @c false, le document s'affiche dans
 * une fenêtre. Lors de la sortie du mode plein écran, la fenêtre retrouve
 * ses dimensions et sa position précédentes.
 * @returns @c true si le mode plein écran était actif avant l'appel, @c false
 * sinon.
 * @~
 * @see toggle_full_screen, slide_show, toggle_slide_show
 */
boolean full_screen(on:boolean);

/**
 * @~english
 * Switch between windowed and full screen modes.
 * @returns @c true if the previous state was full screen, @c false otherwise.
 * @~french
 * Passe du mode fenêtré au mode plein écran et réciproquement.
 * @returns @c true si le mode plein écran était actif avant l'appel, @c false
 * sinon.
 * @~
 * @see full_screen, slide_show, toggle_slide_show
 */
boolean toggle_full_screen();

/**
 * @~english
 * Enables or disables mouse pointer hiding.
 * @returns the state of the setting before the call.
 * @~french
 * Active ou désactive la disparition automatique du pointeur de souris.
 * Lorsque ce paramètre est activé, le pointeur de souris disparaît après
 * deux secondes d'inactivité. Il suffit de déplacer à nouveau la souris
 * pour que le pointeur réapparaisse.
 * @returns l'état avant l'appel.
 * @~
 * @see toggle_auto_hide_cursor, slide_show, toggle_slide_show
 */
boolean auto_hide_cursor(on:boolean);

/**
 * @~english
 * Switch mouse pointer between auto-hide and normal modes.
 * When this setting
 * @returns the state of the setting before the call.
 * @~french
 * Change le mode d'affichage du pointeur de souris.
 * @returns l'état avant l'appel.
 * @~
 * @see auto_hide_cursor, slide_show, toggle_slide_show
 */
boolean toggle_auto_hide_cursor();

/**
 * @}
 */
