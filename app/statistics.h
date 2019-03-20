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
// This software is licensed under the GNU General Public License v3.
// See file COPYING for details.
//  (C) 1992-2011 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2011 Jerome Forissier <jerome@taodyne.com>
//  (C) 2011 Taodyne SAS
// ****************************************************************************

#include "tao.h"
#include "tree.h"
#include "info.h"
#include <QPair>
#include <QList>
#include <QTime>
#include <QElapsedTimer>

TAO_BEGIN

struct Int64Timer
// ----------------------------------------------------------------------------
//    A high resolution timer
// ----------------------------------------------------------------------------
{
    Int64Timer() : startTime(-1) {}
    void   start()   { startTime = now(); }
    qint64 elapsed() { XL_ASSERT(startTime > 0); return now() - startTime; }
    qint64 now()   { return QDateTime::currentDateTime().toMSecsSinceEpoch(); }
    qint64 startTime;
};


struct PerLayoutStatistics : XL::Info
// ----------------------------------------------------------------------------
//   Statistics collected for each layout
// ----------------------------------------------------------------------------
{
    PerLayoutStatistics():
        execCount(0), drawCount(0),
        totalExecTime(0), totalDrawTime(0),
        execTime(), drawTime(),
        sourceFile(""), sourceLine(0) {}

public:
    ulong               totalTime() { return totalExecTime + totalDrawTime; }
    void                reset()
    {
        execCount = drawCount = 0;
        totalExecTime = totalDrawTime = 0;
    }

public:
    static void         beginExec(XL::Tree *);
    static void         endExec(XL::Tree *);
    static void         beginDraw(XL::Tree *);
    static void         endDraw(XL::Tree *);

public:
    ulong               execCount;
    ulong               drawCount;
    ulong               totalExecTime;
    ulong               totalDrawTime;
    QElapsedTimer       execTime;
    QElapsedTimer       drawTime;

    text                sourceFile;
    ulong               sourceLine;
};


struct Statistics
{
public:
    enum Operation
    {
        EXEC, GC, GC_WAIT, DRAW, SELECT, FRAME,
        LAST_OP
    };
    enum Destination
    {
        TO_SCREEN = 0x1, TO_CONSOLE = 0x2
    };

    typedef QPair<qint64, int> date_val;
    typedef QList<date_val> durations;

public:
    Statistics(bool enabled = false)
        : enabled(enabled), interval(5000) { reset(); }

public:
    void    begin(Operation op);
    void    end(Operation op);
    void    reset();
    bool    enable(bool on, int what = TO_SCREEN);
    bool    isEnabled(int what = TO_SCREEN);

    double  fps();
    int     averageTime(Operation op);
    int     averageTimePerFrame(Operation op);
    int     maxTime(Operation op);

protected:
    int         enabled;
    int         interval;   // Measuring interval (ms)
    Int64Timer  intervalTimer;

    // Timing information for each event
    QTime       timer[LAST_OP];
    durations   data[LAST_OP];
    int         total[LAST_OP], frameTotal[LAST_OP];
    int         max[LAST_OP];
    int         lastMaxTime[LAST_OP];
    bool        running[LAST_OP];
};

TAO_END

#endif // STATISTICS_H
