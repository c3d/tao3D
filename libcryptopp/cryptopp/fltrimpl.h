#ifndef CRYPTOPP_FLTRIMPL_H
#define CRYPTOPP_FLTRIMPL_H
// *****************************************************************************
// fltrimpl.h                                                      Tao3D project
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

#define FILTER_BEGIN	\
	switch (m_continueAt)	\
	{	\
	case 0:	\
		m_inputPosition = 0;

#define FILTER_END_NO_MESSAGE_END_NO_RETURN	\
		break;	\
	default:	\
		assert(false);	\
	}

#define FILTER_END_NO_MESSAGE_END	\
	FILTER_END_NO_MESSAGE_END_NO_RETURN	\
	return 0;

/*
#define FILTER_END	\
	case -1:	\
		if (messageEnd && Output(-1, NULL, 0, messageEnd, blocking))	\
			return 1;	\
	FILTER_END_NO_MESSAGE_END
*/

#define FILTER_OUTPUT3(site, statement, output, length, messageEnd, channel)	\
	{\
	case site:	\
	statement;	\
	if (Output(site, output, length, messageEnd, blocking, channel))	\
		return STDMAX(size_t(1), length-m_inputPosition);\
	}

#define FILTER_OUTPUT2(site, statement, output, length, messageEnd)	\
	FILTER_OUTPUT3(site, statement, output, length, messageEnd, DEFAULT_CHANNEL)

#define FILTER_OUTPUT(site, output, length, messageEnd)	\
	FILTER_OUTPUT2(site, 0, output, length, messageEnd)

#define FILTER_OUTPUT_BYTE(site, output)	\
	FILTER_OUTPUT(site, &(const byte &)(byte)output, 1, 0)

#define FILTER_OUTPUT2_MODIFIABLE(site, statement, output, length, messageEnd)	\
	{\
	case site:	\
	statement;	\
	if (OutputModifiable(site, output, length, messageEnd, blocking))	\
		return STDMAX(size_t(1), length-m_inputPosition);\
	}

#define FILTER_OUTPUT_MODIFIABLE(site, output, length, messageEnd)	\
	FILTER_OUTPUT2_MODIFIABLE(site, 0, output, length, messageEnd)

#define FILTER_OUTPUT2_MAYBE_MODIFIABLE(site, statement, output, length, messageEnd, modifiable)	\
	{\
	case site:	\
	statement;	\
	if (modifiable ? OutputModifiable(site, output, length, messageEnd, blocking) : Output(site, output, length, messageEnd, blocking))	\
		return STDMAX(size_t(1), length-m_inputPosition);\
	}

#define FILTER_OUTPUT_MAYBE_MODIFIABLE(site, output, length, messageEnd, modifiable)	\
	FILTER_OUTPUT2_MAYBE_MODIFIABLE(site, 0, output, length, messageEnd, modifiable)

#endif
