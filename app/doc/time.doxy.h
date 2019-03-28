// *****************************************************************************
// time.doxy.h                                                     Tao3D project
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
// (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011-2013, Jérôme Forissier <jerome@taodyne.com>
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
 * @addtogroup grpTime Time and time conversions
 * @ingroup TaoBuiltins
 * This group contains time related primitives.
 *
 * Time-related primitives can be grouped as follows:
 * - Primitives that return information about the current time:
 * time, seconds, hours, minutes, day, month, year,
 * week_day, month_day, year_day. These primitives have an effect on the
 * execution of the block of code they belong to. See @ref secExecModel
 * for general information ; the <em>next refresh</em> value is documented
 * for each primitive below.
 * - Conversion primitives: they are variants of the above that take
 * a timestamp parameter.
 *
 * @~french
 * @addtogroup grpTime Temps, heure, date et conversions
 * @ingroup TaoBuiltins
 * Ce groupe contient des primitives liées au temps.
 *
 * Deux types de primitives peuvent être distinguées :
 * - Celles qui fournissent des informations sur l'instant présent :
 * time, seconds, hours, minutes, day, month, year,
 * week_day, month_day, year_day. Ces primitives ont un effet sur l'exécution
 * du bloc de code auquel elles appartiennent. Cf. @ref secExecModel
 * pour une description du mécanisme général. La valeur de rafraîchissement est
 * documentée pour chaque primitive ci-dessous.
 * - Les primitives de conversion : ce sont des variantes des précedentes, qui
 * acceptent un paramètre temps.
 *
 * @~
 * @{
 */

/**
 * @~english
 * Returns current time in seconds.
 *
 * The return value is the current timestamp expressed as seconds since
 * 1970-01-01T00:00:00 Universal Coordinated Time. This is like the POSIX
 * @c time_t variable, but expressed as a real value to represent fractional
 * seconds.
 *
 * If the document is frozen (that is, if animations are stopped), the
 * returned value remains constant and is the value of @ref time when
 * animations were disabled.
 *
 * Execution of this primitive requests a refresh of the current layout after
 * @ref default_refresh seconds.
 *
 * @~french
 * Renvoie la date et l'heure courante en secondes.
 *
 * La valeur de retour est le nombre de secondes depuis
 * 1970-01-01T00:00:00 UTC. C'est donc une valeur équivalente à la définition
 * POSIX @c time_t, mais exprimée sous forme de valeur réelle pour pouvoir
 * représenter les fractions de secondes.
 *
 * Lorsque les animations sont suspendues (cf. @ref enable_animations), la
 * valeur reste constante et correspond à la valeur de @ref time lorsque les
 * animations ont été stoppées.
 *
 * L'exécution de cette primitive provoque une ré-exécution du @a layout après
 * @ref default_refresh secondes.
 */
real time ();

/**
 * @~english
 * Returns the time elapsed since the page was displayed.
 *
 * The return value is in seconds.
 *
 * If the document is frozen (that is, if animations are stopped), the
 * returned value remains constant and is the value of @ref page_time
 * when animations were disabled.
 *
 * Execution of this primitive requests a refresh of the current layout after
 * @ref default_refresh seconds.
 *
 * @~french
 * Renvoie le temps écoulé depuis l'affichage de la page courante.
 *
 * La valeur est en secondes.
 * Lorsque les animations sont suspendues (cf. @ref enable_animations), la
 * valeur reste constante et correspond à la valeur de @ref page_time lorsque
 * les animations ont été stoppées.
 *
 * L'exécution de cette primitive provoque une ré-exécution du @a layout après
 * @ref default_refresh secondes.
 */
real page_time ();


/**
 * @~english
 * Returns the second part (0 to 59) for the current time.
 *
 * Execution of this primitive requests a refresh of the current layout
 * on the next second.
 *
 * @~french
 * Renvoie la seconde (0 à 59) de l'heure courante.
 *
 * L'exécution de cette primitive provoque une ré-exécution du @a layout à la
 * seconde suivante.
 */
integer seconds ();

/**
 * @~english
 * Returns the minute part (0 to 59) for the current time.
 *
 * Execution of this primitive requests a refresh of the current layout
 * on the next minute.
 *
 * @~french
 * Renvoie la minute (0 à 59) de l'heure courante.
 *
 * L'exécution de cette primitive provoque une ré-exécution du @a layout à la
 * minute suivante.
 */
integer minutes ();

/**
 * @~english
 * Returns the hour part (0 to 23) for the current time.
 *
 * Execution of this primitive requests a refresh of the current layout
 * on the next hour.
 *
 * @~french
 * Renvoie l'heure courante (0 à 23).
 *
 * L'exécution de cette primitive provoque une ré-exécution du @a layout à
 * l'heure suivante.
 */
integer hours ();

/**
 * @~english
 * Returns the day of the month (1 to 31) for the current date.
 *
 * Execution of this primitive requests a refresh of the current layout
 * on the next day.
 *
 * @~french
 * Renvoie le jour du mois (1 à 31) de la date courante.
 *
 * L'exécution de cette primitive provoque une ré-exécution du @a layout
 * le jour suivant.
 */
integer day ();

/**
 * @~english
 * Returns the number of the month (1 to 12) for the current date.
 *
 * Execution of this primitive requests a refresh of the current layout
 * on the next day (not the next month).
 *
 * @~french
 * Renvoie le mois (1 à 12) de la date courante.
 *
 * L'exécution de cette primitive provoque une ré-exécution du @a layout
 * le jour suivant (et non le mois suivant).
 */
integer month ();

/**
 * @~english
 * Returns the current year.
 *
 * For instance 2011.
 *
 * Execution of this primitive requests a refresh of the current layout
 * on the next day (not the next year).
 *
 * @~french
 * Renvoie l'année courante.
 *
 * Par exemple, 2011.
 *
 * L'exécution de cette primitive provoque une ré-exécution du @a layout
 * le jour suivant (et non l'année suivante).
 */
integer year ();

/**
 * @~english
 * Returns the weekday (1-7) for the current date.
 *
 * Day 1 is monday.
 *
 * Execution of this primitive requests a refresh of the current layout
 * on the next day.
 *
 * @~french
 * Renvoie le jour de la semaine.
 *
 * Le jour numéro 1 est lundi.
 *
 * L'exécution de cette primitive provoque une ré-exécution du @a layout
 * le jour suivant.
 */
integer week_day ();

/**
 * @~english
 * Returns the day of the year (1 to 365 or 366 on leap years)
 * for the current date.
 *
 * Execution of this primitive requests a refresh of the current layout
 * on the next day.
 *
 * @~french
 * Renvoie le numero du jour dans l'annee courante (1 à 365 ou 366).
 *
 * L'exécution de cette primitive provoque une ré-exécution du @a layout
 * le jour suivant.
 */
integer year_day ();



/**
 * @~english
 * Returns the second part (0 to 59) for the specified timestamp.
 * @~french
 * Renvoie la seconde (0 à 59) pour la date précisée.
 */
integer seconds (t:real);

/**
 * @~english
 * Returns the minute part (0 to 59) for the specified timestamp.
 * @~french
 * Renvoie la minute (0 à 59) pour la date précisée.
 */
integer minutes (t:real);

/**
 * @~english
 * Returns the hour part (0 to 23) for the specified timestamp.
 * @~french
 * Renvoie l'heure (0 à 23) pour la date précisée.
 */
integer hours (t:real);

/**
 * @~english
 * Returns the day of the month (1 to 31) for the specified timestamp.
 * @~french
 * Renvoie le jour du mois (1 à 31) pour la date précisée.
 */
integer day (t:real);

/**
 * @~english
 * Returns the number of the month (1 to 12) for the specified timestamp.
 * @~french
 * Renvoie le mois (1 à 12) pour la date précisée.
 */
integer month (t:real);

/**
 * @~english
 * Returns the year for the specified timestamp.
 * For instance 2011.
 * @~french
 * Renvoie l'année pour la date précisée.
 * Par exemple 2011.
 */
integer year (t:real);

/**
 * @~english
 * Returns the weekday (1 to 7) for the specified timestamp.
 * Day 1 is monday.
 * @~french
 * Renvoie le jour de la semaine (1 à 7) pour la date précisée.
 * Le jour numéro 1 est lundi.
 */
integer week_day (t:real);

/**
 * @~english
 * Returns the day of the year (1 to 365 or 366 on leap years)
 * for the specified timestamp.
 * @~french
 * Renvoie le numéro du jour (1 à 365 ou 366) pour la date précisée.
 */
integer year_day (t:real);



/**
 * @~english
 * Enables a block of code after a specified interval.
 *
 * The code specified by @p body is not executed until @p delay
 * has expired. Until that time, the @ref after instruction is
 * equivalent to: <tt>if false then</tt>.
 * Once @p delay has expired, @p body
 * is executed and the @p after instruction becomes equivalent to:
 * <tt>if true then</tt>.
 *
 * For example:
@code
page "Empty page",
    nil
page "Second page",
    color "blue"
    rectangle -300, 0, 200, 100
    after 2.0,
        color "red"
        circle 0, 0, 100
        after 3.0,
            color "green"
            triangle 300, 0, 100, 100
@endcode
 * If you load this example and press the arrow key to display
 * the second page:
 * - a blue rectangle shows up immediately,
 * - followed by a red circle two seconds later,
 * - followed by a green triangle three seconds later.
 *
 * This primitive must be used inside a @ref page primitive,
 * otherwise its behavior is undefined.
 *
 * Execution of this primitive requests a refresh of the current layout
 * when the delay expires.
 *
 * @~french
 * Active un bloc de code après l'intervalle spécifié.
 *
 * Le code spécifié par @p body n'est pas exécuté avant que @p delay
 * n'ait expiré. Jusqu'à ce moment, l'instruction @ref after est équivalente
 * à : <tt>if false then</tt>.
 * Lorsque le délai a expiré, @p body est exécuté et l'instruction @ref after
 * devient équivalente à <tt>if true then</tt>.
 *
 * Par exemple:
@code
page "Page vide",
    nil
page "Deuxième page",
    color "blue"
    rectangle -300, 0, 200, 100
    after 2.0,
        color "red"
        circle 0, 0, 100
        after 3.0,
            color "green"
            triangle 300, 0, 100, 100
@endcode
 * Si vous chargez cet exemple et enfoncez la touche Flèche vers le bas pour
 * afficher la deuxième page :
 * - un rectangle bleu s'affiche immédiatement,
 * - suivi par un cercle rouge deux secondes plus tard,
 * - suivi par un triangle vert trois secondes plus tard.
 *
 * Cette primitive doit être utilisée dans une primitive @ref page,
 * faute de quoi le comportement n'est pas défini.
 *
 * L'exécution de cette primitive provoque une ré-exécution du @a layout
 * lorsque le délai expire.
 */
after (delay:real, body:tree);

/**
 * @~english
 * Enables a block of code periodically.
 *
 * The code specified by @p body is executed, and the @ref every
 * instruction is equivalent to: <tt>if true then</tt> until
 * @p interval * @p duty_cycle seconds have expired.
 * Then, the @ref every instruction becomes equivalent to:
 * <tt>if false then</tt> for a duration of @p interval * (1 - @p duty_cycle).
 *
 * @p interval is expressed in seconds. @p duty_cycle is a real value between
 * 0.0 and 1.0.
 *
 * The following example show a blinking text:
@code
page "Blinking text",
    every 1.0, 0.75,
        text_box 0, 0, 100, 100,
            text "Blinking"
@endcode
 *
 * This primitive must be used inside a @ref page primitive,
 * otherwise its behavior is undefined.
 *
 * Execution of this primitive requests a refresh of the current layout
 * when the next activation or de-activation of the code is due to occur.
 *
 * @~french
 * Active un bloc de code périodiquement.
 *
 * Le code spécifié par @p body est exécuté, et l'instruction @ref every
 * est équivalente à : <tt>if true then</tt> jusqu'à ce que
 * @p interval * @p duty_cycle secondes se soient écoulées.
 * Alors, l'instruction @ref every devient équivalente à :
 * <tt>if false then</tt> pendant une durée de
 * @p interval * (1 - @p duty_cycle).
 *
 * @p interval est exprimé en secondes. @p duty_cycle est une valeur entre
 * 0.0 et 1.0.
 *
 * L'exemple qui suit montre comment faire clignoter du texte :
@code
page "Texte clignotant",
    every 1.0, 0.75,
        text_box 0, 0, 100, 100,
            text "Clignotant"
@endcode
 *
 * Cette primitive doit être utilisée dans une primitive @ref page,
 * faute de quoi le comportement n'est pas défini.
 *
 * L'exécution de cette primitive provoque une ré-exécution du @a layout
 * lorsque la prochaine activation ou désactivation doit survenir.
 */
every (interval:real, duty_cycle:real, body:tree);

/**
 * @~english
 * Enables a block of code periodically.
 * This builtin is equivalent to @ref every(interval:real, duty_cycle:real, body:tree)
 * with a @p duty_cycle of 50%.
 * @~french
 * Active un bloc de code périodiquement.
 * Cette fonction est équivalente à @ref every(interval:real, duty_cycle:real, body:tree)
 * avec un @p duty_cycle de 50%.
 */
every (interval:real, body:tree);

/**
 * @~english
 * Sets the default refresh interval for code using time and page_time.
 *
 * This primitive controls the refresh rate of layouts that call the
 * @ref time and/or @ref page_time primitives. While code based on
 * for instance, the @ref seconds primitive does not need to be re-evaluated
 * more often than once per second, it is obvious that code depending on
 * @ref time or @ref page_time will yield different values each time it is
 * executed. Such code will actually be re-evaluated automatically every
 * @ref default_refresh seconds.
 *
 * @ref time and @ref page_time use the current value of the default refresh
 * interval to schedule the next evaluation of the current layout.
 *
 * Setting the default refresh interval to 0.0 will cause @ref time and
 * @ref page_time to schedule an immediate refresh of the current layout,
 * which will therefore be executed as fast as possible.
 *
 * However, the refresh rate may be limited on some platforms or under
 * some specific circumstances:
 * - On MacOSX, Tao3D uses a
 * <a href="http://developer.apple.com/library/mac/#documentation/QuartzCore/Reference/CVDisplayLinkRef/Reference/reference.html">
 * Core Video display link</a> to
 * trigger periodic screen refreshes. This is so that animation are as
 * smooth as possible. As a result, the granularity of the
 * refresh timer is the screen refresh rate. For most LCD displays this
 * is 60 Hz or 16.6 ms. On this platform, @ref default_refresh is set
 * to 0.0 by default.
 * - On other platforms, when vertical synchronization is supported,
 * default_refresh is initially set to 0.0. If VSync is actually
 * enabled, the refresh rate is then automatically limited by the VSync
 * clock. When VSync is disabled, however, the refresh rate is not
 * limited by the display and Tao3D will compute as many
 * frames per second as possible. You may use this configuration for
 * benchmarking purposes: simply select "Disable VSync" in the command
 * chooser (escape key). On the other hand, if the platform does not
 * support setting the VSync mode, default_refresh is initially set to
 * 0.016 to avoid uselessly taxing the CPU (that is assuming a 60 Hz
 * display).
 *
 * @returns the previous value of the default refresh interval.
 *
 * @~french
 * Définit l'intervalle de rafraîchissement par défaut pour time et page_time
 *
 * Cette primitive définit l'intervalle de rafraîchissement des @a layouts
 * qui utilisent les primitives @ref time ou @ref page_time.
 * Alors que du code basé par exemple sur la primitive @ref seconds n'a pas
 * besoin d'être ré-évalué plus d'une fois par seconde, il est clair que du
 * code qui dépend de @ref time or @ref page_time va donner des résultats
 * différents à chaque exécution. Un tel code sera ré-évalué automatiqement
 * toutes les @ref default_refresh secondes.
 *
 * @ref time et @ref page_time utilisent la valeur de l'intervalle de
 * rafraîchissement par défaut pour déclencher la prochaine évaluation du
 * @a layout.
 *
 * Si l'intervalle est réduit à 0.0, la ré-exécution provoquée par @ref time
 * et @ref page_time se fait le plus rapidement possible.
 *
 * Celà dit, le taux de rafraîchissement peut être réduit sur certaines
 * plateformes ou dans certaines circonstances :
 * - Sur MacOSX, Tao3D utilise un
 * <a href="http://developer.apple.com/library/mac/#documentation/QuartzCore/Reference/CVDisplayLinkRef/Reference/reference.html">
 * display link Core Video</a> pour déclencher les rafraîchissements
 * périodiques de l'écran. Ce qui assure la meilleure fluidité possible des
 * animations. Par conséquent, la granularité du @a timer de rafraîchissement
 * est égale au taux de rafraîchissement de l'écran. Pour la plupart des
 * écrans LCD ou projecteurs, il s'agit de 60 Hz ou 16.6 ms. Sur cette
 * plateforme, @ref default_refresh vaut 0.0 par défaut.
 * - Sur les autres plateformes, et lorsque la synchronisation verticale
 * est supportée, default_refresh vaut initialement 0.0. Si VSync est
 * réellement activée, la vitesse de rafraîchissement est alors
 * automatiquement limitée par l'horloge de synchronisation verticale.
 * Mais si VSync est désactivée, Tao3D calcule
 * le plus possible d'images par secondes. Vous pouvez utiliser cette
 * configuration pour tester les performances d'une machine : sélectionnez
 * "Désactiver VSync" dans le menu de commande (touche "Échap"). Par contre,
 * lorsque la plateforme ne permet pas de contrôler le mode de VSync,
 * default_refresh est initialement mise à 0.016 pour éviter d'utiliser
 * inutilement le processeur (on suppose on affichage à 60 Hz).
 *
 * @returns la valeur précédente de l'intervalle de rafraîchissement par
 * défaut.
 *
 * @~
 * @see @ref secExecModel
 */
real default_refresh (interval:real);

/**
 * @~english
 * The default refresh interval for code using time and page_time.
 * The default value of the default refresh interval depends on the
 * platform.
 *
 * @~french
 * Renvoie la valeur de l'intervalle de rafraîchissement par défaut.
 * La valeur par défaut dépend de la plateforme.
 * @~
 * @see default_refresh(interval:real).
 */
real default_refresh ();

/** @} */
