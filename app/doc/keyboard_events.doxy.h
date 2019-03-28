// *****************************************************************************
// keyboard_events.doxy.h                                          Tao3D project
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
// (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2013, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
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
 * @page refKeyboardEvents Keyboard events
 *
 * Tao3D offers two mechanisms to execute code when a key is
 * pressed or released.
 * - One is global, and consists in defining one or several forms similar
 * to: <tt>key "X" -> ...</tt>. Being in the global scope, these definitions
 * apply to all pages.
 * - The other one is local, and consists in calling the @ref on primitive
 * with event names "key",
 * "keydown", "keyup" or their alternatives with a regular expression.
 * By using these forms, it is possible to associate different actions to the
 * same event in different places in the document. These forms are also
 * more powerful in that you may link some code to several events in one
 * line, thanks to regular expressions.
 *
 * The two mechanisms are detailed below.
 *
 * When a keyboard event occurs (key pressed or key released):
 * 1. Tao3D triggers the execution of any block of code that depends
 * on that
 * kind of keyboard event. For instance, the @ref locally blocks in the
 * following example would be re-evaluated on each key press, but not on key
 * release, because
 * @ref on "on \"keydown\"" implies @ref refresh_on KeyPressEvent. Then,
 * @ref on "on \"keydown:Return\"" would execute its body only when the key is
 * the @c Return key.
 * @include key_event_refresh.ddd
 * 2. The event is further processed internally to
 * trigger built-in behaviors (for instance, Ctrl-C [Cmd-C on Mac] will
 * take a copy of the screen or copy a shape).
 * 3. If a global definition of the form <tt>key "X" -> ...</tt> exists
 * (@c X being the name of the event as returned by @ref key_event), it is
 * evaluated.
 *
 * @section refKeyDetails key "X" -> ...
 * You may associate global actions to key events simply by defining a form
 * called @c key followed by the name of the key event you want to process.
 * The name of a key event is the one returned by @ref key_event.
 * The following code will show which name is returned for which key event:
@code
Name -> ""
on "key",
    Name := key_event
text_box 0, 0, window_width, window_height,
    align_center
    vertical_align_center
    font "Arial", 40
    text Name
@endcode
 *
 * Some shortcuts are defined by default in @c tao.xl (located
 * in the application's installation directory). Here are some definitions
 * that you will find there:
 *
@code
key "Control-L"                         -> toggle_slide_show
key "Escape"                            -> main_chooser
key "Control-+"                         -> zoom_in
key "Control--"                         -> zoom_out
key "b"                                 -> toggle_blank_screen
key "Space"                             -> next_page
@endcode
 *
 * You may override any of these definitions in your document. For instance,
 * if you don't want "b" to be  a shortcut for "blank screen", re-define
 * the global shortcut so that it does nothing:
@code
key "b" -> {}
@endcode
 *
 * @addindex key (on -)
 * @addindex keydown (on -)
 * @addindex keyup (on -)
 *
 * @section refOnKeyDetails on "key" / "keydown" / "keyup"
 * The @ref on "on \"key\"/\"keydown\"/\"keyup\"" family of functions is meant
 * to execute some code when a specific keyboard event occurs.
 * Contrary to the <tt>key "X" -> ...</tt> syntax, <tt>on "key*"</tt> allows
 * to perform different actions in different contexts. It will automatically
 * cause a refresh (re-evaluation) of the enclosing layout when the event
 * occurs. @n
 * For instance, the
 * following code associates the left and right arrow keys with a
 * rotation action on page one, and with a translation on page two.
 * @subsection on_keydown_example_leftright on "keydown" (simple example)
@code
Rz -> 0.0
page "Rotation",
    on "keydown:Left",
        Rz := Rz + 10
    on "keydown:Right",
        Rz := Rz - 10
    rotatez Rz
    color "purple"
    rectangle 0, 0, 200, 100

Tx -> 0.0
page "Translation",
    on "keydown:Left",
        Tx := Tx - 10
    on "keydown:Right",
        Tx := Tx + 10
    translatex Tx
    color "orange"
    circle 0, 0, 100
@endcode
 * @subsection on_keydown_example_cmd on "keydown" (command prompt example)
 * The following example relies on the @ref on "on \"keydown:<regexp>\"" syntax
 * to implement keyboard shortcuts, as well as a simple command processing
 * page.
 * @include on_keydown.ddd
 * @~french
 * @page refKeyboardEvents Événements clavier
 * Tao3D propose deux mécanismes distincts pour exécuter du code
 * lors de l'appui ou du relâchement d'une touche.
 * - Le premier est global : il s'agit de définir une ou plusieurs fonctions
 * de la forme <tt>key "X" -> ...</tt>. Ces définitions s'appliquent à
 * toutes les page.
 * - Le second est local : il s'agit d'appeler la primitive @ref on avec les
 * noms d'événements : "key", "keydown", "keyup", ou leurs variantes contenant
 * une expression régulière. Celà permet d'associer des actions différentes
 * à un même événement à des endroits différents du document. Ces formes sont
 * également plus puissantes dans le sens où il est possible d'associer le
 * même code à plusieurs événements en use seule fois, grâce à des
 * expressions régulières.
 *
 * Les deux mécanismes sont détaillés plus bas.
 *
 * Lorsqu'un événement clavier survient (appui ou relâchement d'une touche) :
 * 1. Tao3D déclenche l'exécution de tout bloc de code qui dépend
 * de cet événement. Par exemple, le bloc @ref locally de l'exemple suivant
 * sera évalué à nouveau à chaque appui sur une touche, mais pas lors du
 * relâchement de la touche, car
 * @ref on "on \"keydown\"" implique @ref refresh_on KeyPressEvent. Ensuite,
 * @ref on "on \"keydown:Return\"" exécutera son contenu lors de l'appui sur
 * la touche @c Return.
 * @include key_event_refresh_fr.ddd
 * 2. L'événement est ensuite traité pour déclencher certains comportements
 * comme Ctrl-C (Cmd-C sur Mac) qui copie l'écran ou la forme sélectionnée
 * 3. Enfin, si une définition globale de la forme <tt>key "X" -> ...</tt>
 * existe (@c X étant le nom de l'événement tel que renvoyé par
 * @ref key_event), elle est évaluée.
 *
 * @section refKeyDetails key "X" -> ...
 * Il est possible d'associer des actions globales à des événements clavier en
 * définissant simplement en définissant une forme : le mot @c key suivi
 * du nom de l'événement à traiter. Ce nom est tel que retourné par
 * @ref key_event. Le code qui suit affiche le nom de chaque événement
 * clavier :
@code
Name -> ""
on "key",
    Name := key_event
text_box 0, 0, window_width, window_height,
    align_center
    vertical_align_center
    font "Arial", 40
    text Name
@endcode
 *
 * Certains raccourcis sont définis par défaut dans @c tao.xl (qui se trouve
 * dans le répertoire d'installation de l'application). Voici quelques-unes
 * de ces définitions :
@code
key "Control-L"                         -> toggle_slide_show
key "Escape"                            -> main_chooser
key "Control-+"                         -> zoom_in
key "Control--"                         -> zoom_out
key "b"                                 -> toggle_blank_screen
key "Space"                             -> next_page
@endcode
 *
 * Vous pouvez surcharger n'importe laquelle de ces définitions dans votre
 * document. Par exemple si vous ne souhaitez pas que la touche "b"
 * déclenche la fonction <em>blank screen</em> (écran noir), il vous suffit
 * de redéfinir la forme de manière à ce qu'elle ne fasse rien :
@code
key "b" -> {}
@endcode
 *
 * @addindex key (on -)
 * @addindex keydown (on -)
 * @addindex keyup (on -)
 *
 * @section refOnKeyDetails on "key" / on "keydown" / on "keyup"
 * Les fonctions @ref on "on \"key\"/\"keydown\"/\"keyup\"" permettent
 * d'exécuter du code
 * lors de l'appui ou du relâchement d'une touche. Contrairement à la syntaxe
 * <tt>key "X" -> ...</tt>, <tt>on "key*"</tt> permet d'associer des actions
 * différentes à un même événement dans des contextes diférents. Par exemple,
 * pour associer les touches flèche à gauche et flèche à droite à un effet de
 * rotation sur une première page, puis à une translation sur une autre page,
 * il suffit d'écrire le code suivant.
 * @subsection on_keydown_example_leftright on "keydown" : exemple simple
@code
Rz -> 0.0
page "Rotation",
    on "keydown:Left",
        Rz := Rz + 10
    on "keydown:Right",
        Rz := Rz - 10
    rotatez Rz
    color "purple"
    rectangle 0, 0, 200, 100

Tx -> 0.0
page "Translation",
    on "keydown:Left",
        Tx := Tx - 10
    on "keydown:Right",
        Tx := Tx + 10
    translatex Tx
    color "orange"
    circle 0, 0, 100
@endcode
 * @subsection on_keydown_example_cmd on "keydown" : exemple d'interpréteur de commandes
 * L'exemple suivant utilise la syntaxe @ref on "on \"keydown:<regexp>\"" pour
 * implémenter des raccourcis clavier, ainsi qu'un interpréteur de commandes
 * simple.
 * @include on_keydown_fr.ddd
 */
