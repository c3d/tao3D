#ifndef CRYPTOPP_HRTIMER_H
#define CRYPTOPP_HRTIMER_H
// *****************************************************************************
// hrtimer.h                                                       Tao3D project
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
// (C) 2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011, Jérôme Forissier <jerome@taodyne.com>
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

#include "config.h"
#ifndef HIGHRES_TIMER_AVAILABLE
#include <time.h>
#endif

NAMESPACE_BEGIN(CryptoPP)

#ifdef HIGHRES_TIMER_AVAILABLE
	typedef word64 TimerWord;
#else
	typedef clock_t TimerWord;
#endif

//! _
class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE TimerBase
{
public:
	enum Unit {SECONDS = 0, MILLISECONDS, MICROSECONDS, NANOSECONDS};
	TimerBase(Unit unit, bool stuckAtZero)	: m_timerUnit(unit), m_stuckAtZero(stuckAtZero), m_started(false) {}

	virtual TimerWord GetCurrentTimerValue() =0;	// GetCurrentTime is a macro in MSVC 6.0
	virtual TimerWord TicksPerSecond() =0;	// this is not the resolution, just a conversion factor into seconds

	void StartTimer();
	double ElapsedTimeAsDouble();
	unsigned long ElapsedTime();

private:
	double ConvertTo(TimerWord t, Unit unit);

	Unit m_timerUnit;	// HPUX workaround: m_unit is a system macro on HPUX
	bool m_stuckAtZero, m_started;
	TimerWord m_start, m_last;
};

//! measure CPU time spent executing instructions of this thread (if supported by OS)
/*! /note This only works correctly on Windows NT or later. On Unix it reports process time, and others wall clock time.
*/
class ThreadUserTimer : public TimerBase
{
public:
	ThreadUserTimer(Unit unit = TimerBase::SECONDS, bool stuckAtZero = false)	: TimerBase(unit, stuckAtZero) {}
	TimerWord GetCurrentTimerValue();
	TimerWord TicksPerSecond();
};

//! high resolution timer
class CRYPTOPP_DLL Timer : public TimerBase
{
public:
	Timer(Unit unit = TimerBase::SECONDS, bool stuckAtZero = false)	: TimerBase(unit, stuckAtZero) {}
	TimerWord GetCurrentTimerValue();
	TimerWord TicksPerSecond();
};

NAMESPACE_END

#endif
