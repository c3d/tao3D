#ifndef CRYPTOPP_STDCPP_H
#define CRYPTOPP_STDCPP_H
// *****************************************************************************
// stdcpp.h                                                        Tao3D project
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

#if _MSC_VER >= 1500
#define _DO_NOT_DECLARE_INTERLOCKED_INTRINSICS_IN_MEMORY
#include <intrin.h>
#endif

#include <stddef.h>
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include <string>
#include <exception>
#include <typeinfo>
#include <algorithm>
#include <map>
#include <vector>

#ifdef CRYPTOPP_INCLUDE_VECTOR_CC
// workaround needed on Sun Studio 12u1 Sun C++ 5.10 SunOS_i386 128229-02 2009/09/21
#include <vector.cc>
#endif

// for alloca
#ifdef __sun
#include <alloca.h>
#elif defined(__MINGW32__) || defined(__BORLANDC__)
#include <malloc.h>
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4231)	// re-disable this
#ifdef _CRTAPI1
#define CRYPTOPP_MSVCRT6
#endif
#endif

#endif
