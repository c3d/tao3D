#ifndef CRYPTOPP_TRDLOCAL_H
#define CRYPTOPP_TRDLOCAL_H
// *****************************************************************************
// trdlocal.h                                                      Tao3D project
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

#include "config.h"

#ifdef THREADS_AVAILABLE

#include "misc.h"

#ifdef HAS_WINTHREADS
typedef unsigned long ThreadLocalIndexType;
#else
#include <pthread.h>
typedef pthread_key_t ThreadLocalIndexType;
#endif

NAMESPACE_BEGIN(CryptoPP)

//! thread local storage
class CRYPTOPP_DLL ThreadLocalStorage : public NotCopyable
{
public:
	//! exception thrown by ThreadLocalStorage class
	class Err : public OS_Error
	{
	public:
		Err(const std::string& operation, int error);
	};

	ThreadLocalStorage();
	~ThreadLocalStorage();

	void SetValue(void *value);
	void *GetValue() const;

private:
	ThreadLocalIndexType m_index;
};

NAMESPACE_END

#endif	// #ifdef THREADS_AVAILABLE

#endif
