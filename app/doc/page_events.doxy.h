// *****************************************************************************
// page_events.doxy.h                                              Tao3D project
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
// (C) 2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2013, Jérôme Forissier <jerome@taodyne.com>
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
 * @page refPageEvents Page events
 * This page explains how one can use @ref on "on \"pageentry\"" and
 * @ref on "on \"pageexit\"" to perform some actions when a page becomes
 * visible or when it stops from being shown. @n
 * The body of <tt>on "pageentry"</tt> and <tt>on "pageexit"</tt> is
 * normally not executed when a page is evaluated, unless it is the
 * first evaluation of the page (@c pageentry) or another page is about
 * to be evaluated (@c pageexit). Note that when the conditions for
 * page entry or page exit are met, the whole page is executed. If multiple
 * occurrences of <tt>on "pageentry"</tt> or <tt>on "pageexit"</tt> exist,
 * they are run in the order they appear. Therefore, a @c pageentry block
 * is usually best inserted at the begining of the page (so that the remaining
 * code gets executed once the initialization is done). Similarly, a
 * @c pageexit block normally belongs to the end of the page.
 * @ref refPageExitExample is a case where this is critically important.
 *
 * If page transitions are in effect (@ref transition or the Transitions module),
 * the @c pageentry event for the next page occurs before tne @c pageexit
 * event for the current page.
 *
 * When a page is restarted, that is, when the current page is selected from
 * the page chooser menu:
 * - the @c pageexit event is fired, immediately followed by @c pageentry.
 * - the execution of the page starts again with @ref page_time = 0
 *
 * @section refPageEntryExample on "pageentry" example
 * In the following example, the circle on page 1 can be moved using the left
 * and right arrow keys. It will take back its initial position if you change
 * page (press Page down or the down arrow key), then come back to page 1
 * (Page up or up arrow key).
@code
Cx -> 0
page "Test",
    on "pageentry",     { Cx := 100 }
    on "keydown:Left",  { Cx := Cx - 10 }
    on "keydown:Right", { Cx := Cx + 10 }
    translatex Cx
    color_hsv (20*page_time) mod 360, 1, 1, 1
    circle 0, 0, 100

page "The End", {}
@endcode
 *
 * @section refPageExitExample on "pageexit" example
 * The following code will play a YouTube video, and stop it automatically
 * when the user leaves the page.
@code
import VLCAudioVideo

page "Him at the Zoo",
    movie "http://www.youtube.com/watch?v=jNQXAC9IVRw"
    on "pageexit", { movie_only "" }

page "The End", {}
@endcode
 * @~french
 * @page refPageEvents Événements de page
 * Cette page explique comment utiliser @ref on "on \"pageentry\"" et
 * @ref on "on \"pageexit\"" pour effectuer des actions lorsqu'une page devient
 * visible ou n'est plus affichée. @n
 * Le corps de <tt>on "pageentry"</tt> et <tt>on "pageexit"</tt> n'est
 * normalement pas exécuté lorsqu'une page est évaluée, excepté quand il s'agit
 * de la première évaluation de la page (@c pageentry) ou lorsqu'une autre page
 * est sur le point d'être évaluée (@c pageexit). Lorsque les conditions
 * d'exécution de @c pageentry ou @c pageexit sont réunies, la page entière est
 * évaluée. Si plusieurs occurrences de @c pageentry ou de @c pageexit sont
 * présentes, elles sont exécutées dans l'ordre dans lequel elles apparaîssent.
 * De ce fait, on place généralement les blocs @c pageentry au début de la
 * page, de sorte que le reste de la page puisse bénéficier des initialisations
 * effectuées pas le bloc. De la même manière. un bloc @c pageexit sera souvent
 * positionné en fin de page. @ref refPageExitExample est un cas où le
 * placement en fin de page est très important.
 *
 * Si des effets de transition sont activés (@ref transition, ou par
 * l'intermédiaire du module Transitions), l'événement @c pageentry pour la
 * page suivante est déclenché avant l'événement @c pageexit pour la page
 * précédente.
 *
 * Lorsqu'une page est redémarrée, c'est-à-dire lorsque la page en cours est
 * sélectionnée dans le menu de sélection des pages (Ctrl-G, Cmd-G sur Mac) :
 * - l'événements @c pageexit est généré, immédiatement suivi de @c pageentry
 * - l'exécution de la page reprend à @ref page_time = 0
 *
 * @section refPageEntry on "pageentry" : exemple
 * Dans l'exemple suivant, le cercle affiché sur la page 1 peut être déplacé
 * à l'aide des touches fléchées (gauche et droite) du clavier. Il reprend sa
 * position initiale si vous changez de page puis revenez à la première page.
@code
Cx -> 0
page "Test",
    on "pageentry",     { Cx := 100 }
    on "keydown:Left",  { Cx := Cx - 10 }
    on "keydown:Right", { Cx := Cx + 10 }
    translatex Cx
    color_hsv (20*page_time) mod 360, 1, 1, 1
    circle 0, 0, 100

page "Fin", {}
@endcode
 *
 * @section refPageExitExample on "pageexit" : exemple
 * Le code qui suit joue une vidéo YouTube, et l'arrête automatiquement lors
 * d'un changement de page.
@code
import VLCAudioVideo

page "Lui au zoo",
    movie "http://www.youtube.com/watch?v=jNQXAC9IVRw"
    on "pageexit", { movie_only "" }

page "Fin", {}
@endcode
 */
