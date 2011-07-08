#ifndef STATISTICS_H
#define STATISTICS_H
// ****************************************************************************
//  statistics.h                                                   Tao project
// ****************************************************************************
//
//   File Description:
//
//    Record execution and drawing events and return various statistics
//    (average frames per second, average execution time per frame...)
//
//
//
//
//
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2011 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2011 Jerome Forissier <jerome@taodyne.com>
//  (C) 2011 Taodyne SAS
// ****************************************************************************


#include "tao.h"
#include <QPair>
#include <QList>
#include <Qtime>

TAO_BEGIN

struct Statistics
{
public:
    enum Operation
    {
        EXEC, GC, DRAW,
        LAST_OP
    };
    typedef QList<int> frame_times;
    typedef QPair<int, int> date_val;
    typedef QList<date_val> durations;

public:
    Statistics(bool enabled = false)
        : enabled(enabled), interval(5000) { reset(); }

public:
    void    begin(Operation op);
    void    end(Operation op);
    void    reset();
    bool    enable(bool on = true);
    bool    isEnabled();

    double  fps();
    int     averageTime(Operation op);
    int     maxTime(Operation op);

protected:
    bool        enabled;
    int         interval;   // Measuring interval (ms)
    QTime       intervalTimer;

    // FPS measurement
    frame_times frames;     // Frames shown during last interval

    // Timing for other events
    QTime       timer[LAST_OP];
    durations   data[LAST_OP];
    int         total[LAST_OP];
    int         max[LAST_OP];
    int         lastMaxTime[LAST_OP];
};

TAO_END

#endif // STATISTICS_H
