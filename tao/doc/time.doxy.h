/**
 * @addtogroup time Time and time conversions
 * @ingroup TaoBuiltins
 * This group holds time relative primitives.
 *
 * Most of those primitives uses a timestamp as input. This timestamp is a real representing a number
 *  of seconds with a precision of milliseconds. The time is given in seconds since epoch. 
 * The timestamp of the current time can be retreived by the @c time primitive.
 * @{
 */

/**
 * Returns current time in second, including milliseconds.
 *
 * Returns the current document's time or the time at wich the document was frozen.
 * The returned value is a real in seconds, with millisecond precision.
 * The time is given in seconds since epoch.
 * @returns Return a fractional time, including milliseconds
 */
time ();

/**
 * @~english
 * Extracts day in year (1-365) from timestamp.
 */
/**
 * @~french
 * Extrait du timestamp le numero du jour dans l'annee (1-365).
 */
year_day (t:real);

/**
 * Extracts week day (1-7) from timestamp.
 */
week_day (t:real);

/**
 * Extracts year from timestamp.
 *
 * For instance, 2011.
 */
year (t:real);

/**
 * Extracts month (1-12) from timestamp.
 */
month (t:real);

/**
 * Extracts day (1-31) from timestamp.
 */
day (t:real);

/**
 * Extracts minutes (0-23) from timestamp.
 */
hours (t:real);

/**
 * Extracts minutes (0-59) from timestamp.
 */
minutes (t:real);

/**
 * Extracts seconds (0-59) from timestamp.
 */
seconds (t:real);

/**
 * Returns day in the month of hte current time.
 */
day ();

/**
 * Returns the hours (0-23) of the current time.
 */
hours ();

/**
 * Returns the minutes (0-59) of the current time.
 */
minutes ();

/**
 * Returns the current month number (1-12).
 */
month ();

/**
 * Returns the seconds of the current time (0-59).
 */
seconds ();

/**
 * Returns the current day of week (1-7).
 *
 * Day 1 is monday.
 */
week_day ();

/**
 * Returns the current year.
 *
 * For instance 2011.
 */
year ();

/**
 * @~english
 * Returns the current day in year (1-365).
 *
 * @~french
 * Retourne le numero du jour dans l'annee courante (1-365).
 */
year_day ();

/**
 * Returns the page time.
 *
 *  Returns the time of the current page relatively to the current document's time, 
 * or relatively to the frozen time.
 * 
 * @returns Return the page time in second, with milliseconds precision.
 */
page_time ();

/**
 * Execute a block of code after specified interval.
 *
 *  Execute @a c after the specified amount of time @a d, in seconds.
 *
 * @returns The real elapsed time.
 */
after (d:real, c:tree);

/**
 * Executes a block of code periodically.
 *
 * @todo Check this. How is the duty parameter (@a d) used?
 */
every (i:real, d:real, c:tree);

/**
 * @copybrief every(i:real, d:real, c:tree)
 *
 *  \p duty is 0.5 when not specified.
 * @copydetails every(i:real, d:real, c:tree)
 *
 */
every (i:real, c:tree);

/** @} */
