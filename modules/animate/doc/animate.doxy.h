/**
 * @~english
 * @taomoduledescription{Animate, Animation utilities for slides}
 * <tt>import Animate</tt> - Commands to create animations and transitions within a slide.@n
 *
 * The Animate module defines commands that you can use to animate elements
 * within a slide or across slides, either as a function of time or
 * in response to keyboard events.
 * 
 * A simple example, showing how to move a circle smoothly in response to the
 * left and right arrow keys:
 *
 * @~french
 * @taomoduledescription{Animate, Fonctions d'animations}
 * <tt>import Animate</tt> - Commandes pour créer des animations et des transitions dans des
 * diapositives.@n
 *
 * Le module Animate définit des commandes que vous pouvez utiliser pour
 * animer des éléments au sein d'une page, ou d'une page à l'autre, ou bien
 * en fonction du temps ou en réponse à des événements clavier.
 *
 * Voici un example simple qui déplace progressivement un cercle de gauche à
 * droite lorsqu'on appuie sur les touches «flèche à gauche» et
 * «flèche à droite» :
 * @~
 @code
import Animate

key "Right" -> skip 1
key "Left"  -> skip -1
compute_smooth_step
circle 100 * smooth_step, 0, 20
 @endcode
 * @endtaomoduledescription{Animate}
 *
 * @{
 */


/**
 * @~english
 * Interpolate a variable
 *
 * Interpolate variable @a X to reach value @a TargetX with speed of change
 * @a S.
 *
 * @~french
 * Interpole une variable
 *
 * Interpole la variable @a X pour atteindre la valeur @a TargetX avec une
 * vitesse de changement de @a S.
 */
interpolate(real S, real TargetX, real X);

/**
 * @~english
 * Interpolate three variables
 *
 * Interpolate variables @a X, @a Y and @a Z to reach values
 * @a TargetX, @a TargetY and @a TargetZ with speed of change @a S.
 *
 * @~french
 * Interpole trois variables
 *
 * Interpole les variables @a X, @a Y et @a Z pour atteindre les valeurs
 * @a TargetX, @a TargetY et @a TargetZ avec une vitesse de changement
 * de @a S.
 */
interpolate(real S, real TargetX, real TargetY, real TargetZ, real X, real Y, real Z);

/**
 * @~english
 * Interpolate a position
 *
 * Interpolate variables @a X, @a Y and @a Z to reach values
 * @a TargetX, @a TargetY and @a TargetZ with speed of change @a S, and use the
 * result as a position. After the call, the coordinate systems is
 * centered on that new position.
 *
 * @~french
 * Interpole une  position
 *
 * Interpole les variables @a X, @a Y et @a Z pour atteindre les valeurs
 * @a TargetX, @a TargetY et @a TargetZ avec une vitesse de changement
 * @a S, et utilise le résultat comme une position. Après l'appel, le système
 * de coordonnées est centré sur cette nouvelle position.
 */
interpolate_position(real S, real TargetX, real TargetY, real TargetZ, real X, real Y, real Z);

/**
 * @~english
 * Interpolate a scale along X, Y and Z axis
 *
 * Interpolate variable @a Scale to reach value @a TargetScale with
 * speed of change @a S, and use the result as a global scale along X,
 * Y and Z axis.
 *
 * @ref interpolate_scale2(real, real, real) performs the same operation
 * on X and Y only.
 *
 * @~french
 * Interpole une échelle le long des axes X, Y et Z
 *
 * Interpole la variable @a Scale pour atteindre la valeur @a TargetScale
 * avec une vitesse de changement de @a S, et utilise le résultat comme
 * un facteur d'échelle global le long des axes X, Y et Z.
 *
 * @ref interpolate_scale2(real, real, real) effectue la même opération
 * relativement aux axes X et Y seulement.
 */
interpolate_scale(real S, real TargetScale, real Scale);

/**
 * @~english
 * Interpolate a scale along X and Y axis
 *
 * Interpolate variable @a Scale to reach value @a TargetScale with
 * speed of change @a S, and use the result as a global scale along X,
 * and Y axis.
 *
 * @ref interpolate_scale(real, real, real) performs the same operation on
 * X, Y and Z.
 *
 * @~french
 * Interpole une échelle le long des axes X et Y
 *
 * Interpole la variable @a Scale pour atteindre la valeur @a TargetScale
 * avec une vitesse de changement @a S, et utilise le résultat comme un
 * facteur d'échelle global le long des axes X et Y.
 *
 * @ref interpolate_scale(real, real, real) effectue la même opération
 * relativement aux axes X, Y et Z.
 */
interpolate_scale2(real S, real TargetScale, real Scale);

/**
 * @~english
 * Interpolate a rotation around X axis
 *
 * Interpolate variable @a X to reach value @a TargetX with speed of change @a S,
 * then use the resulting value as a rotation angle. After the call,
 * the coordinate systems will have been rotated along the X axis by
 * @a X degrees.
 *
 * @~french
 * Interpole une rotation autour de l'axe X
 *
 * Interpole la variable @a X pour atteindre la valeur @a TargetX avec une
 * vitesse de changement @a S,
 * et utilise le résultat comme un angle de rotation. Après l'appel,
 * le système de coordonnées a tourné d'un angle @a X degrés autour de l'axe X.
 */
interpolate_xangle(real S, real TargetX, real X);

/**
 * @~english
 * Interpolate a rotation around Y axis
 *
 * Interpolate variable @a Y to reach value @a TargetY with speed of change @a S,
 * then use the resulting value as a rotation angle. After the call,
 * the coordinate systems will have been rotated along the Y axis by
 * @a Y degrees.
 *
 * @~french
 * Interpole une rotation autour de l'axe Y
 *
 * Interpole la variable @a Y pour atteindre la valeur @a TargetY avec une
 * vitesse de changement @a S,
 * et utilise le résultat comme un angle de rotation. Après l'appel,
 * le système de coordonnées a tourné d'un angle @a Y degrés autour de l'axe Y.
 */
interpolate_yangle(real S, real TargetY, real Y);

/**
 * @~english
 * Interpolate a rotation around Z axis
 *
 * Interpolate variable @a Z to reach value @a TargetZ with speed of change @a S,
 * then use the resulting value as a rotation angle. After the call,
 * the coordinate systems will have been rotated along the Z axis by
 * @a Z degrees.
 *
 * @~french
 * Interpole une rotation autour de l'axe Z
 *
 * Interpole la variable @a Z pour atteindre la valeur @a TargetZ avec une
 * vitesse de changement @a S,
 * et utilise le résultat comme un angle de rotation. Après l'appel,
 * le système de coordonnées a tourné d'un angle @a Z degrés autour de l'axe Z.
 */
interpolate_zangle(real S, real TargetZ, real Z);


/**
 * @~english
 * Interpolate a variable, adjusting for real-time
 *
 * Interpolate variable @a X to reach value @a TargetX with speed of change
 * @a S. The interpolation rate is adjusted as a function of refresh rate.
 *
 * @~french
 * Interpole une variable avec un ajustment temps réel
 *
 * Interpole la variable @a X pour atteindre la valeur @a TargetX avec une
 * vitesse de changement de @a S ajustée en fonction de la vitesse d'affichage
 */
interpolate_rt(real S, real TargetX, real X);

/**
 * @~english
 * Interpolate a position, adjusting for real-time
 *
 * Interpolate variables @a X, @a Y and @a Z to reach values
 * @a TargetX, @a TargetY and @a TargetZ with speed of change @a S, and use the
 * result as a position. After the call, the coordinate systems is
 * centered on that new position. The interpolation rate is adjusted
 * as a function of refresh rate.
 *
 * @~french
 * Interpole une  position avec un ajustment temps réel
 *
 * Interpole les variables @a X, @a Y et @a Z pour atteindre les
 * valeurs @a TargetX, @a TargetY et @a TargetZ avec une vitesse de
 * changement @a S, ajustée en fonction de la vitesse d'affichage, et
 * utilise le résultat comme une position. Après l'appel, le système
 * de coordonnées est centré sur cette nouvelle position.
 */
interpolate_position_rt(real S, real TargetX, real TargetY, real TargetZ, real X, real Y, real Z);

/**
 * @~english
 * Interpolate a scale along X, Y and Z axis, adjusting for real-time
 *
 * Interpolate variable @a Scale to reach value @a TargetScale with
 * speed of change @a S, and use the result as a global scale along X,
 * Y and Z axis. The interpolation rate is adjusted as a function of
 * refresh rate.
 *
 * @ref interpolate_scale2(real, real, real) performs the same operation
 * on X and Y only.
 *
 * @~french
 * Interpole une échelle le long des axes X, Y et Z avec un ajustment temps réel
 *
 * Interpole la variable @a Scale pour atteindre la valeur @a
 * TargetScale avec une vitesse de changement de @a S ajustée en
 * fonction de la vitesse d'affichage, et utilise le résultat comme un
 * facteur d'échelle global le long des axes X, Y et Z.
 *
 * @ref interpolate_scale2(real, real, real) effectue la même opération
 * relativement aux axes X et Y seulement.
 */
interpolate_scale_rt(real S, real TargetScale, real Scale);


/**
 * @~english
 * Interpolate a rotation around X axis, adjusting for real-time
 *
 * Interpolate variable @a X to reach value @a TargetX with speed of
 * change @a S, then use the resulting value as a rotation
 * angle. After the call, the coordinate systems will have been
 * rotated along the X axis by @a X degrees. The interpolation rate is
 * adjusted as a function of refresh rate.
 *
 * @~french
 * Interpole une rotation autour de l'axe X avec un ajustment temps réel
 *
 * Interpole la variable @a X pour atteindre la valeur @a TargetX avec une
 * vitesse de changement @a S, ajustée en fonction de la vitesse d'affichage,
 * et utilise le résultat comme un angle de rotation. Après l'appel,
 * le système de coordonnées a tourné d'un angle @a X degrés autour de l'axe X.
 */
interpolate_xangle_rt(real S, real TargetX, real X);

/**
 * @~english
 * Interpolate a rotation around Y axis, adjusting for real-time
 *
 * Interpolate variable @a Y to reach value @a TargetY with speed of
 * change @a S, then use the resulting value as a rotation
 * angle. After the call, the coordinate systems will have been
 * rotated along the Y axis by @a Y degrees. The interpolation rate is
 * adjusted as a function of refresh rate.
 *
 * @~french
 * Interpole une rotation autour de l'axe Y avec un ajustment temps réel
 *
 * Interpole la variable @a Y pour atteindre la valeur @a TargetY avec une
 * vitesse de changement @a S, ajustée en fonction de la vitesse d'affichage,
 * et utilise le résultat comme un angle de rotation. Après l'appel,
 * le système de coordonnées a tourné d'un angle @a Y degrés autour de l'axe Y.
 */
interpolate_yangle_rt(real S, real TargetY, real Y);

/**
 * @~english
 * Interpolate a rotation around Z axis, adjusting for real-time
 *
 * Interpolate variable @a Z to reach value @a TargetZ with speed of
 * change @a S, then use the resulting value as a rotation
 * angle. After the call, the coordinate systems will have been
 * rotated along the Z axis by @a Z degrees. The interpolation rate is
 * adjusted as a function of refresh rate.
 *
 * @~french
 * Interpole une rotation autour de l'axe Z avec un ajustment temps réel
 *
 * Interpole la variable @a Z pour atteindre la valeur @a TargetZ avec une
 * vitesse de changement @a S, ajustée en fonction de la vitesse d'affichage,
 * et utilise le résultat comme un angle de rotation. Après l'appel,
 * le système de coordonnées a tourné d'un angle @a Z degrés autour de l'axe Z.
 */
interpolate_zangle_rt(real S, real TargetZ, real Z);

/**
 * @~english
 * Compute value to be used for fade-in effects.
 * @deprecated Use the square bracket notations instead.
 *
 * @~french
 * Calcule une valeur utilisée pour des effets d'apparition (fade-in).
 * @deprecated Utiliser les notations "crochets".
 *
 * @~
 * @see value_in_interval, page_time_in_interval
 */
real fade_in(real Value, real Duration);


/**
 * @~english
 * Compute value to be used for fade-out effects.
 * @deprecated Use the square bracket notations instead.
 *
 * @~french
 * Calcule une valeur utilisée pour des effets de disparition (fade-out).
 * @deprecated Utiliser les notations "crochets".
 *
 * @~
 * @see value_in_interval, page_time_in_interval
 */
real fade_out(real Value, real Duration);


/**
 * @~english
 * Current animation step
 *
 * This is the animation step set by @ref skip or @ref skip_to.
 *
 * @~french
 * La position courante dans l'animation
 *
 * C'est la position d'animation telle que définie par @ref skip
 * ou @ref skip_to.
 */
real step = 0.0;

/**
 * @~english
 * Smoothed value of current animation step
 *
 * This is a smoothed value of the animation step set by
 * @ref skip or @ref skip_to.
 * This value is only computed after you call @ref compute_smooth_step
 *
 * @~french
 * Valeur lissée de la position d'animation
 *
 * C'est une valeur lissée (interpolée) de la position d'animation définie par
 * @ref skip ou @ref skip_to.
 * Cette valeur n'est calculée que lorsque vous appelez
 * @ref compute_smooth_step.
 */
real smooth_step = 0.0;

/**
 * @~english
 * Skip in the animation
 *
 * Skip by the given amount in the animation. Typically, positive values
 * are used to move forward in the animation, and negative values to move
 * backwards.
 *
 * @~french
 * Avance ou recule dans l'animation
 *
 * Fait avancer ou reculer l'animation de @a Amount positions. Les valeurs
 * positives du paramètre @a Amount font avancer l'animation, les valeurs
 * négatives la font reculer.
 *
 * @~
 * @see skip_to, skip_directly_to
 */
skip(real Amount);

/**
 * @~english
 * Skip directly to a given step in the animation
 *
 * Skip to the given position in the animation.
 *
 * @~french
 * Avance ou recule l'animation pour se rendre à la position spécifiée
 *
 * @~
 * @see skip, skip_directly_to
 */
skip_to(real Position);


/**
 * @~english
 * Skip directly to a given step in the animation without transition
 *
 * Skip to the given position in the animation.
 *
 * @~french
 * Se rend directement à une position dans l'animation, sans transition
 *
 * Avance ou recule l'animation pour se rendre immédiatement à la position
 * spécifiée, sans transition.
 *
 * @~
 * @see skip, skip_to
 */
skip_directly_to(real Position);


/**
 * @~english
 * Zoom around a particular position in the animation
 *
 * Return a scaling factor that depends on how far @a N is from the current
 * position of the animation (as given by @ref step).
 *
 * @~french
 * Zoom autour d'une position donnée dans l'animation
 *
 * Retourne un facteur proportionnel qui dépend de l'écart entre @a N et la position
 * courante dans l'animation (la valeur de @ref step).
 */
step_scale(integer N);


/**
 * @~english
 * Compute the value of @ref smooth_step
 *
 * @~french
 * Met à jour la valeur de @ref smooth_step
 */
compute_smooth_step();


/**
 * @~english
 * Smooth a linear value between 0 and 1.
 * This returns:
 * - 0 if T < 0
 * - 1 if T > 1
 * - 3*T^2 - 2*T^3 otherwise.
 * @~french
 * Adoucit une valeur linéaire entre 0 and 1.
 * Cette fonction retourne :
 * - 0 si T < 0
 * - 1 si T > 1
 * - 3*T^2 - 2*T^3 sinon.
 * @~
 * @since 1.02 (Tao 1.20)
 */
smooth(real T);


/**
 * @~english
 * Smooth a linear value between two values @p Low and @p High.
 * This is equivalent to:
@code smooth((T-Low)/(High-Low)) @endcode
 * @~french
 * Adoucit une valeur linéaire entre deux valeurs @p Low et @p High.
 * Cette fonction est équivalente à :
@code smooth((T-Low)/(High-Low)) @endcode
 * @~
 * @since 1.02 (Tao 1.20)
 */
smooth_betwen(real T, real Low, real High);


/**
 * @~english
 * Return the ratio of the current interval.
 * This value is defined only within the body of @ref value_in_interval or
 * @ref page_time_in_interval.
 * @~french
 * Retourne le taux de complétion pour l'intervalle en cours.
 * Cette valeur n'est définie que dans le cadre de @ref value_in_interval ou
 * @ref page_time_in_interval.
 * @~
 * @since 1.02 (Tao 1.20)
 */
real ratio;


/**
 * @~english
 * Return a decreasing ratio of the current interval.
 * When used inside the body of @ref value_in_interval or
 * @ref page_time_in_interval, this value is is equal to:
 * @~french
 * Retourne le taux restant pour l'intervalle en cours.
 * Lorsqu'elle est utilisée dans le cadre de @ref value_in_interval ou
 * @ref page_time_in_interval, cette valeur est égale à :
 * @~
@code
1 - ratio
@endcode
 * @see ratio
 * @since 1.02 (Tao 1.20)
 */
real down_ratio;


/**
 * @~english
 * Return a smoothed version of the current interval ratio.
 * When used inside the body of @ref value_in_interval or
 * @ref page_time_in_interval, this value is is equal to:
 * @~french
 * Retourne une version adoucie du taux pour l'intervalle actuel.
 * Lorsqu'elle est utilisée dans le cadre de @ref value_in_interval ou
 * @ref page_time_in_interval, cette valeur est égale à :
 * @~
@code
smooth ratio
@endcode
 * @see smooth, ratio
 * @since 1.02 (Tao 1.20)
 */
real smooth_ratio;


/**
 * @~english
 * Return a smoothed version of the remaining interval ratio.
 * When used inside the body of @ref value_in_interval or
 * @ref page_time_in_interval, this value is is equal to:
 * @~french
 * Retourne une version adoucie du taux restant pour l'intervalle en cours.
 * Lorsqu'elle est utilisée dans le cadre de @ref value_in_interval ou
 * @ref page_time_in_interval, cette valeur est égale à :
 * @~
 * @see smooth, down_ratio
 * @since 1.02 (Tao 1.20)
 */
real smooth_down_ratio;


/**
 * @~english
 * Evaluate @p Body while @p Value is between @p A and @p B.
 * The value of @ref ratio grows linearly between 0.0 (when @p Value is equal to
 * @p A) and 1.0 (when @p Value is equal to @p B), and this also
 * updates derived variables @ref down_ratio, @ref smooth_ratio and
 * @ref smooth_down_ratio.
 *
 * This function is documented with this name due to limitations
 * in the documentation tool. The recommended notation to use this feature is
 * the following:
 *
 * @~french
 * Evalue @p Body tant que @p Value est entre @p A et @p B.
 * La valeur de @ref ratio croît linéairement entre 0.0 (lorsque @p Value vaut
 * @p A) et 1.0 (lorsque @p Value vaut @p B), et cela
 * met aussi à jour les variables dérivées @ref down_ratio,
 * @ref smooth_ratio and @ref smooth_down_ratio.
 *
 * Cette fonction est documentée sous ce nom à cause de
 * limitations de l'outil de génération de cette documentation. La notation
 * recommandée est la suivante :
 *
 * @~
@code
 Value in [A..B]
     Body
@endcode
 *
 * @~english
 * For instance:
 *
 * @~french
 * Par exemple:
 *
 * @~
@code
import Animate

W -> window_width/3
color "gray"
mouse_x in [-W/2 .. 0]
    color "gray", down_ratio
mouse_x in [0 .. W/2]
    color "gray", ratio
rectangle 0, 0, W, window_height
@endcode
 * @since 1.02 (Tao 1.20)
 */
value_in_interval(Value:real, A:real, B:real, Body);


/**
 * @~english
 * Evaluate @p Body while @p Value is between @p A and @p D
 * The value of @ref ratio grows linearly between 0 and 1 when
 * @p Value is between @p A and @p B, then stays at 1 until @p Value
 * reaches @p C, and then decreases linearly to 0 between @p C
 * and @p D. This also updates derived variables @ref down_ratio,
 * @ref smooth_ratio and @ref smooth_down_ratio.
 *
 * This function is documented with this name due to limitations
 * in the documentation tool. The recommended notation to use this feature is
 * the following:
 *
 * @~french
 * Evalue @p Body tant que @p Value est entre @p A et @p D
 * La valeur de @ref ratio croît linéairement entre 0 et 1 quand @p
 * Value est entre @p A et @p B, puis reste à 1 entre @p B et @p C,
 * puis décroit linéairement jusqu'à 0 entre @p C et @p D. Cela
 * met aussi à jour les variables dérivées @ref down_ratio,
 * @ref smooth_ratio and @ref smooth_down_ratio.
 *
 * Cette fonction est documentée sous ce nom à cause de
 * limitations de l'outil de génération de cette documentation. La notation
 * recommandée est la suivante :
 *
 * @~
@code
Value in [A..B, C..D]
    Body
@endcode
 *
 * @~english
 * For instance:
 *
 * @~french
 * Par exemple :
 *
 * @~
@code
import Animate

W -> window_width/3
mouse_x in [-W/2..-W/4, W/4..W/2]
    color "gray", ratio
    rectangle 0, 0, W, window_height
@endcode
 * @since 1.02 (Tao 1.20)
 */
value_in_intervals(Value:real, A:real, B:real, C:real, D:real, Body);


/**
 * @~english
 * Evaluate @p Body while @p Value is larger or equal to @p A.
 * The value of @ref ratio grows linearly between 0.0 (when @p Value is equal to
 * @p A) and 1.0 (when @p Value is equal to or larger than @p B), and this also
 * updates derived variables @ref down_ratio, @ref smooth_ratio and
 * @ref smooth_down_ratio.
 *
 * This function is documented with this name due to limitations
 * in the documentation tool. The recommended notation to use this feature is
 * the following:
 *
 * @~french
 * Evalue @p Body tant que @p Value est supérieure ou égale à @p A.
 * La valeur de @ref ratio croît linéairement entre 0.0 (lorsque @p Value vaut
 * @p A) et 1.0 (lorsque @p Value est supérieure ou égale à @p B), et cela
 * met aussi à jour les variables dérivées @ref down_ratio,
 * @ref smooth_ratio and @ref smooth_down_ratio.
 *
 * Cette fonction est documentée sous ce nom à cause de
 * limitations de l'outil de génération de cette documentation. La notation
 * recommandée est la suivante :
 *
 * @~
@code
 Value in [A..B, ]
     Body
@endcode
 *
 * @since 1.03 (Tao 1.30)
 */
value_in_interval_or_greater(Value:real, A:real, B:real, Body);


/**
 * @~english
 * Shortcut for: page_time in [A..B].
 *
 * This function is documented with this name due to limitations
 * in the documentation tool. The recommended notation to use this feature is
 * the following:
 *
 * @~french
 * Raccourci pour: page_time in [A..B].
 *
 * Cette fonction est documentée sous ce nom à cause de
 * limitations de l'outil de génération de cette documentation. La notation
 * recommandée est la suivante :
 * @~
@code
[A..B]
    Body
@endcode
 * @~english
 * Here is an example.
 * @~french
 * Voici un exemple.
 * @~
@code
import Animate

page "The [A..B] notation",
    clear_color 0, 0, 0, 1
    text_box 0, 0, window_width, window_height,
        align_center
        vertical_align_center
        font "Arial", 40
        Print 0, "The [A .. B] notation"
        Print 2, "is"
        Print 4, "a cool trick!"
    text_box 0, 0, window_width-20, window_height,
        align_right
        vertical_align_bottom
        font "Arial", 20
        color "white"
        text "Press <return> to restart"

Print Time:real, T:text ->
    after Time,
        color "white"
        [Time .. Time+0.5]
            color_hsv 30, down_ratio, 1.0
        text T
        paragraph_break

key "Return" -> goto_page page_name 1
@endcode
 * @since 1.02 (Tao 1.20)
 * @see value_in_interval
 */
page_time_in_interval(A:real, B:real, Body);


/**
 * @~english
 * Shortcut for: page_time in [A..B, ].
 *
 * This function is documented with this name due to limitations
 * in the documentation tool. The recommended notation to use this feature is
 * the following:
 *
 * @~french
 * Raccourci pour: page_time in [A..B, ].
 *
 * Cette fonction est documentée sous ce nom à cause de
 * limitations de l'outil de génération de cette documentation. La notation
 * recommandée est la suivante :
 * @~
@code
[A..B, ]
    Body
@endcode
 * @~english
 * Here is an example.
 * @~french
 * Voici un exemple.
 * @~
@code
import Animate

page "The [A..B, ] notation",
    [0..0.5, ]
        Square -1, smooth_ratio
    [0.5..1, ]
        Square 0, smooth_ratio
    [1..1.5, ]
        Square 1, smooth_ratio

Square X:real, S:real -> rectangle X*window_width/4, 0, 100*S, 100*S

key "Return" -> goto_page page_name 1
@endcode
 * @since 1.03 (Tao 1.30)
 * @see value_in_interval_or_greater
 */
page_time_in_interval_or_greater(A:real, B:real, Body);


/**
 * @~english
 * Shortcut for: page_time in [A..B, C..D].
 *
 * This function is documented with this name due to limitations
 * in the documentation tool. The recommended notation to use this feature is
 * the following:
 *
 * @~french
 * Raccourci pour: page_time in [A..B, C..D].
 *
 * Cette fonction est documentée sous ce nom à cause de
 * limitations de l'outil de génération de cette documentation. La notation
 * recommandée est la suivante :
 * @~
@code
[A..B, C..D]
    Body
@endcode
 * @since 1.02 (Tao 1.20)
 * @see value_in_intervals
 */
page_time_in_intervals(A:real, B:real, C:real, D:real, Body);

/**
 * @}
 */
