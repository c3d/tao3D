// *****************************************************************************
// statistics.cpp                                                  Tao3D project
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
// (C) 2014-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011-2013, Jérôme Forissier <jerome@taodyne.com>
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


#include "statistics.h"

TAO_BEGIN

// ============================================================================
//
//   Per-layout statistics
//
// ============================================================================

void PerLayoutStatistics::beginExec(XL::Tree *body)
// ----------------------------------------------------------------------------
//    Begin timing the execution phase
// ----------------------------------------------------------------------------
{
    if (body)
    {
        PerLayoutStatistics *info = body->GetInfo<PerLayoutStatistics>();
        if (!info)
        {
            info = new PerLayoutStatistics;
            body->SetInfo<PerLayoutStatistics>(info);
        }
        info->execCount++;
        info->execTime.restart();
    }
}


void PerLayoutStatistics::endExec(XL::Tree *body)
// ----------------------------------------------------------------------------
//    End of execution phase timing
// ----------------------------------------------------------------------------
{
    if (body)
    {
        PerLayoutStatistics *info = body->GetInfo<PerLayoutStatistics>();
        XL_ASSERT(info);
        info->totalExecTime += info->execTime.nsecsElapsed();
    }
}


void PerLayoutStatistics::beginDraw(XL::Tree *body)
// ----------------------------------------------------------------------------
//    Begin timing the drawing phase
// ----------------------------------------------------------------------------
{
    if (body)
    {
        PerLayoutStatistics *info = body->GetInfo<PerLayoutStatistics>();
        if (!info)
        {
            info = new PerLayoutStatistics;
            body->SetInfo<PerLayoutStatistics>(info);
        }
        info->drawCount++;
        info->drawTime.restart();
    }
}


void PerLayoutStatistics::endDraw(XL::Tree *body)
// ----------------------------------------------------------------------------
//    End of drawing phase timing
// ----------------------------------------------------------------------------
{
    if (body)
    {
        PerLayoutStatistics *info = body->GetInfo<PerLayoutStatistics>();
        XL_ASSERT(info);
        info->totalDrawTime += info->drawTime.nsecsElapsed();
    }
}



// ============================================================================
//
//   Global statistics
//
// ============================================================================

void Statistics::reset()
// ----------------------------------------------------------------------------
//    Reset statistics and start a new measurement
// ----------------------------------------------------------------------------
{
    for (int i = 0; i < LAST_OP; i++)
    {
        data[i].clear();
        total[i] = 0;
        frameTotal[i] = 0;
        max[i] = 0;
        lastMaxTime[i] = 0;
    }
    intervalTimer.start();
}


bool Statistics::enable(bool on, int what)
// ----------------------------------------------------------------------------
//    Enable or disable statistics collection, return previous state
// ----------------------------------------------------------------------------
{
    if (on)
    {
        if ((enabled & what) == what)
            return true;
        if (!enabled)
            reset();
        enabled |= what;
        return false;
    }
    else
    {
        if ((enabled & what) == 0)
            return false;
        enabled &= ~what;
        return true;
    }
}


bool Statistics::isEnabled(int what)
// ----------------------------------------------------------------------------
//    Return enabled state
// ----------------------------------------------------------------------------
{
    return ((enabled & what) == what);
}


void Statistics::begin(Operation op)
// ----------------------------------------------------------------------------
//    Start measurement
// ----------------------------------------------------------------------------
{
    if (!enabled)
        return;

    XL_ASSERT(op >= 0 && op < LAST_OP);

    running[op] = true;
    timer[op].start();
}


void Statistics::end(Operation op)
// ----------------------------------------------------------------------------
//    End measurement
// ----------------------------------------------------------------------------
{
    if (!enabled)
        return;

    XL_ASSERT(op >= 0 && op < LAST_OP);

    if (!running[op])
        return;
    running[op] = false;

    qint64 now = intervalTimer.elapsed();
    int elapsed = timer[op].elapsed();

    // Append measurement to list, keeping only 'interval' seconds of data.
    // Keep total up-to-date.
    date_val val(now, elapsed);
    durations &d = data[op];
    d.append(val);
    total[op] += elapsed;
    while (now - d.first().first >= interval)
        total[op] -= d.takeFirst().second;

    frameTotal[op] += elapsed;
    if (op == FRAME)
    {
        for (int i = 0; i < LAST_OP; i++)
        {
            // Update max if current value is larger, or peak interval has expired
            if (frameTotal[i] > max[i] || (now - lastMaxTime[i]) >= interval)
            {
                max[i] = frameTotal[i];
                lastMaxTime[i] = now;
            }
            frameTotal[i] = 0;
        }
    }
}


double Statistics::fps()
// ----------------------------------------------------------------------------
//    Average frames per second during last 'interval' seconds
// ----------------------------------------------------------------------------
{
    if (!enabled || intervalTimer.elapsed() < interval)
        return -1.0;
    return (double)data[FRAME].size() * 1000 / interval;
}


int Statistics::averageTime(Operation op)
// ----------------------------------------------------------------------------
//    Average duration of specified operation during last 'interval' seconds
// ----------------------------------------------------------------------------
{
    XL_ASSERT(op >= 0 && op < LAST_OP);

    if (!enabled || intervalTimer.elapsed() < interval)
        return -1;

    durations &d = data[op];
    if (!d.size())
        return -1;
    return total[op]/d.size();
}


int Statistics::averageTimePerFrame(Operation op)
// ----------------------------------------------------------------------------
//    Average duration of operation, per frame, during last 'interval' seconds
// ----------------------------------------------------------------------------
{
    XL_ASSERT(op >= 0 && op < LAST_OP);

    if (!enabled || intervalTimer.elapsed() < interval)
        return -1;

    int frames = data[FRAME].size();
    if (!frames)
        return -1.0;
    return total[op]/frames;
}


int Statistics::maxTime(Operation op)
// ----------------------------------------------------------------------------
//    Maximum duration of specified operation during last 'interval' seconds
// ----------------------------------------------------------------------------
{
    XL_ASSERT(op >= 0 && op < LAST_OP);

    if (!enabled || intervalTimer.elapsed() < interval)
        return -1;

    return max[op];
}

TAO_END
