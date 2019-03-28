#ifndef STATISTICS_H
#define STATISTICS_H
// *****************************************************************************
// statistics.h                                                    Tao3D project
// *****************************************************************************
//
// File description:
//
//    Record execution and drawing events and return various statistics
//    (average frames per second, average execution time per frame...)
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2011,2014-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
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
