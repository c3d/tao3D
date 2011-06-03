/**
 * @addtogroup grpTime Time and time conversions
 * @ingroup TaoBuiltins
 * This group contains time related primitives.
 *
 * Time-related primitives can be grouped as follows:
 * - Primitives that return information about the current time:
 * time, seconds, hours, minutes, hours, day, month, year,
 * week_day, month_day, year_day. These primitives have an effect on the
 * execution of the block of code they belong to. See @ref secExecModel
 * for general information ; the <em>next refresh</em> value is documented
 * for each primitive below.
 * - Conversion primitives: they are variants of the above that take
 * a timestamp parameter.
 * @{
 */

/**
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
 */
real time ();

/**
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
 */
real page_time ();


/**
 * Returns the second part (0 to 59) for the current time.
 *
 * Execution of this primitive requests a refresh of the current layout
 * on the next second.
 */
integer seconds ();

/**
 * Returns the minute part (0 to 59) for the current time.
 *
 * Execution of this primitive requests a refresh of the current layout
 * on the next minute.
 */
integer minutes ();

/**
 * Returns the hour part (0 to 23) for the current time.
 *
 * Execution of this primitive requests a refresh of the current layout
 * on the next hour.
 */
integer hours ();

/**
 * Returns the day of the month (1 to 31) for the current date.
 *
 * Execution of this primitive requests a refresh of the current layout
 * on the next day.
 */
integer day ();

/**
 * Returns the number of the month (1 to 12) for the current date.
 *
 * Execution of this primitive requests a refresh of the current layout
 * on the next day (not the next month).
 */
integer month ();

/**
 * Returns the current year.
 *
 * For instance 2011.
 *
 * Execution of this primitive requests a refresh of the current layout
 * on the next day (not the next year).
 */
integer year ();

/**
 * Returns the weekday (1-7) for the current date.
 *
 * Day 1 is monday.
 *
 * Execution of this primitive requests a refresh of the current layout
 * on the next day.
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
 * Retourne le numero du jour dans l'annee courante (1-365).
 */
integer year_day ();



/**
 * Returns the second part (0 to 59) for the specified timestamp.
 */
integer seconds (t:real);

/**
 * Returns the minute part (0 to 59) for the specified timestamp.
 */
integer minutes (t:real);

/**
 * Returns the hour part (0 to 23) for the specified timestamp.
 */
integer hours (t:real);

/**
 * Returns the day of the month (1 to 31) for the specified timestamp.
 */
integer day (t:real);

/**
 * Returns the number of the month (1 to 12) for the specified timestamp.
 */
integer month (t:real);

/**
 * Returns the year for the specified timestamp.
 *
 * For instance 2011.
 */
integer year (t:real);

/**
 * Returns the weekday (1-7) for the specified timestamp.
 *
 * Day 1 is monday.
 */
integer week_day (t:real);

/**
 * @~english
 * Returns the day of the year (1 to 365 or 366 on leap years)
 * for the specified timestamp.
 *
 * @~french
 * Retourne le numero du jour dans l'annee courante (1-365).
 */
integer year_day (t:real);



/**
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
 * @code
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
 * @endcode
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
 */
after (delay:real, body:tree);

/**
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
 * @code
page "Blinking text",
    every 1.0, 0.75,
        text_box 0, 0, 100, 100,
            text "Blinking"
 * @endcode
 *
 * This primitive must be used inside a @ref page primitive,
 * otherwise its behavior is undefined.
 *
 * Execution of this primitive requests a refresh of the current layout
 * when the next activation or de-activation of the code is due to occur.
 *
 */
every (interval:real, duty_cycle:real, body:tree);

/**
 * Enables a block of code periodically.
 *
 * This builtin is equivalent to @ref every(interval:real, duty_cycle:real, body:tree)
 * with a @p duty_cycle of 50%.
 */
every (interval:real, body:tree);

/**
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
 * - On MacOSX, Tao Presentations uses a
 * <a href="http://developer.apple.com/library/mac/#documentation/QuartzCore/Reference/CVDisplayLinkRef/Reference/reference.html">
 * Core Video display link</a> to
 * trigger periodic screen refreshes. This is so that animation are as
 * smooth as possible. As a result, the granularity of the
 * refresh timer is the screen refresh rate. For most LCD displays this
 * is 60 Hz or 16.6 ms. On this platform, @ref default_refresh is set
 * to 0.0 by default.
 * - On any platform, when vertical synchronization is enabled,
 * default_refresh is initially set to 0.0, and the refresh rate is
 * automatically limited by the VSync clock. When VSync is disabled,
 * however, default_refresh is initially set to 0.016 to avoid uselessly
 * taxing the CPU (that is assuming a 60 Hz display).
 *
 * @see @ref secExecModel
 *
 * @returns the previous value of the default refresh interval.
 */
real default_refresh (interval:real);

/**
 * The default refresh interval for code using time and page_time.
 *
 * The default value of the default refresh interval depends on the
 * platform. See @ref default_refresh(interval:real).
 */
real default_refresh ();

/** @} */
