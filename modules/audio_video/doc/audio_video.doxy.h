/**
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
 * @{
 */

/**
 * Plays a video.
 * This primitive creates a rectangular video display centered at @p x, @p y
 * in the plane z = 0. The size of the rectangle depends on the resolution
 * of the video source, and the scaling factors @p sx and @p sy. Set @p sx and
 * @p sy to 1.0 to keep the original resolution.
 * The @p name parameter specifies a local file or a URL; an empty string
 * @c "" stops playback of the current video.
 * @see movie_texture.
 */
movie(x:real, y:real, sx:real, sy:real, name:text);

/**
 * Creates a video texture.
 * This primitive plays an audio and/or video file and makes the video
 * available as a texture. You can subsequently map the texture on a rectangle
 * or any other shape in the 3D space.
 * The @p name parameter specifies a local file or a URL; an empty string
 * @c "" stops playback of the current video.
 */
movie_texture(name:text);

/**
 * @}
 */
