// *****************************************************************************
// trdlocal.cpp                                                    Tao3D project
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
// trdlocal.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"

#ifndef CRYPTOPP_IMPORTS
#ifdef THREADS_AVAILABLE

#include "trdlocal.h"

#ifdef HAS_WINTHREADS
#include <windows.h>
#endif

NAMESPACE_BEGIN(CryptoPP)

ThreadLocalStorage::Err::Err(const std::string& operation, int error)
	: OS_Error(OTHER_ERROR, "ThreadLocalStorage: " + operation + " operation failed with error 0x" + IntToString(error, 16), operation, error)
{
}

ThreadLocalStorage::ThreadLocalStorage()
{
#ifdef HAS_WINTHREADS
	m_index = TlsAlloc();
	if (m_index == TLS_OUT_OF_INDEXES)
		throw Err("TlsAlloc", GetLastError());
#else
	int error = pthread_key_create(&m_index, NULL);
	if (error)
		throw Err("pthread_key_create", error);
#endif
}

ThreadLocalStorage::~ThreadLocalStorage()
{
#ifdef HAS_WINTHREADS
	if (!TlsFree(m_index))
		throw Err("TlsFree", GetLastError());
#else
	int error = pthread_key_delete(m_index);
	if (error)
		throw Err("pthread_key_delete", error);
#endif
}

void ThreadLocalStorage::SetValue(void *value)
{
#ifdef HAS_WINTHREADS
	if (!TlsSetValue(m_index, value))
		throw Err("TlsSetValue", GetLastError());
#else
	int error = pthread_setspecific(m_index, value);
	if (error)
		throw Err("pthread_key_getspecific", error);
#endif
}

void *ThreadLocalStorage::GetValue() const
{
#ifdef HAS_WINTHREADS
	void *result = TlsGetValue(m_index);
	if (!result && GetLastError() != NO_ERROR)
		throw Err("TlsGetValue", GetLastError());
#else
	void *result = pthread_getspecific(m_index);
#endif
	return result;
}

NAMESPACE_END

#endif	// #ifdef THREADS_AVAILABLE
#endif
