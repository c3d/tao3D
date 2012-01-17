/**
 * @~english
 * @taomoduledescription{AudioVideo, Audio and video}
 *
 * Play audio and video content.
 *
 * The primitives documented in this page provide basic audio and video
 * playback capabilities. You can use them to add music and videos
 * to your presentations. The most common audio and video formats are
 * supported on all platformas (Windows, MacOSX, Linux). However, please note
 * that some formats may be supported only on some platforms and should
 * therefore be avoided if portability of the document is important.
 *
 * One multimedia stream is associated with each call to movie or
 * movie_texture. It is possible to play several audio or video
 * files simultaneously. The following examples shows how you can control
 * the playback through a common function, @c play.
 *
 * @code
play T:text ->
    color "white"
    movie_texture T

page "First video",
    play "first.mpeg"
    rounded_rectangle 0, 0, 352, 240, 20

page "Second video",
    play "second.mpeg"
    rounded_rectangle mouse_x, mouse_y, 352, 240, 20

page "The End",
    text_box 0, 0, window_width, window_height,
        vertical_align_center
        align_center
        font "Tangerine", 72
        color "Black"
        text "That's all Folks!"
    play ""
 * @endcode
 *
 * It is currently not possible to start playing at a specified time, or to
 * select the audio or video stream.
 * @endtaomoduledescription{AudioVideo}
 *
 * @~french
 * @taomoduledescription{AudioVideo, Audio et vidéo}
 *
 * Lecteur audio et vidéo.
 *
 * Les primitives documentées ici permettent de jouer des fichiers
 * multimédias. Vous pouvez les utiliser pour ajouter de la musique et des
 * vidéos à vos présentations. Les formats de fichiers les plus courants sont
 * supportés sur toutes les plateformes (Windows, MacOSX, Linux). Mais certains
 * formats ne sont supportés que sur certaines plateformes et il convient donc
 * de les éviter s'il est important que le document puisse s'afficher sur
 * n'importe quelle plateforme.
 *
 * Un flux multimédia est associé avec chaque appel à @ref movie ou
 * @ref movie_texture. Il est possible de jouer plusieurs fichiers
 * simultanément. L'exemple qui suit montre comment contrôler la lecture de
 * plusieurs fichiers en utilisant une fonction commune, @c jouer.
 *
 * @code
jouer T:text ->
    color "white"
    movie_texture T

page "Première vidéo",
    jouer "first.mpeg"
    rounded_rectangle 0, 0, 352, 240, 20

page "Seconde vidéo",
    jouer "second.mpeg"
    rounded_rectangle mouse_x, mouse_y, 352, 240, 20

page "Fini",
    text_box 0, 0, window_width, window_height,
        vertical_align_center
        align_center
        font "Tangerine", 72
        color "Black"
        text "C'est fini !"
    jouer ""
 * @endcode
 *
 * Il n'est actuellement pas possible de démarrer la lecture à un instant
 * donné dans le fichier, ni de sélectionner le flux audio ou vidéo.
 * @endtaomoduledescription{AudioVideo}
 * @~
 * @{
 */

/**
 * @~english
 * Plays a video.
 * This primitive creates a rectangular video display centered at @p x, @p y
 * in the plane z = 0. The size of the rectangle depends on the resolution
 * of the video source, and the scaling factors @p sx and @p sy. Set @p sx and
 * @p sy to 1.0 to keep the original resolution.
 * The @p name parameter specifies a local file or a URL; an empty string
 * @c "" stops playback of the current video.
 *
 * @~french
 * Lit une vidéo.
 * Cette primitive crée un rectangle centré en (@p x, @p y) sur le plan z = 0
 * pour afficher la vidéo. La taille du rectangle dépend de la résolution de
 * la vidéo, ainsi que des facteurs de dimensionnement @p sx at @p sy. Utilisez
 * @p sx = @p sy = 1.0 pour conserver la résolution d'origine de la vidéo.
 *
 * @~
 * @see movie_texture.
 */
movie(x:real, y:real, sx:real, sy:real, name:text);

/**
 * @~english
 * Creates a video texture.
 * This primitive plays an audio and/or video file and makes the video
 * available as a texture. You can subsequently map the texture on a rectangle
 * or any other shape in the 3D space.
 * The @p name parameter specifies a local file or a URL; an empty string
 * @c "" stops playback of the current video.
 *
 * @~french
 * Crée une texture vidéo.
 * Cette primitive lit le fichier audio et/ou vidéo, et rend disponible l'image
 * sous forme de texture. Vous pouvez ensuite afficher cette texture sur un
 * rectangle ou n'importe quelle autre forme géométrique dans l'espace 3D.
 * Le paramètre @p name est le nom d'un fichier local, ou une URL. Une chaîne
 * vide @c "" arrête la lecture du fichier en cours.
 */
movie_texture(name:text);

/**
 * @}
 */
