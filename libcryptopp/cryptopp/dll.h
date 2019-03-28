#ifndef CRYPTOPP_DLL_H
#define CRYPTOPP_DLL_H
// *****************************************************************************
// dll.h                                                           Tao3D project
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

#if !defined(CRYPTOPP_IMPORTS) && !defined(CRYPTOPP_EXPORTS) && !defined(CRYPTOPP_DEFAULT_NO_DLL)
#ifdef CRYPTOPP_CONFIG_H
#error To use the DLL version of Crypto++, this file must be included before any other Crypto++ header files.
#endif
#define CRYPTOPP_IMPORTS
#endif

#include "aes.h"
#include "cbcmac.h"
#include "ccm.h"
#include "cmac.h"
#include "channels.h"
#include "des.h"
#include "dh.h"
#include "dsa.h"
#include "ec2n.h"
#include "eccrypto.h"
#include "ecp.h"
#include "files.h"
#include "fips140.h"
#include "gcm.h"
#include "hex.h"
#include "hmac.h"
#include "modes.h"
#include "mqueue.h"
#include "nbtheory.h"
#include "osrng.h"
#include "pkcspad.h"
#include "pssr.h"
#include "randpool.h"
#include "rsa.h"
#include "rw.h"
#include "sha.h"
#include "skipjack.h"
#include "trdlocal.h"

#ifdef CRYPTOPP_IMPORTS

#ifdef _DLL
// cause CRT DLL to be initialized before Crypto++ so that we can use malloc and free during DllMain()
#ifdef NDEBUG
#pragma comment(lib, "msvcrt")
#else
#pragma comment(lib, "msvcrtd")
#endif
#endif

#pragma comment(lib, "cryptopp")

#endif		// #ifdef CRYPTOPP_IMPORTS

#include <new>	// for new_handler

NAMESPACE_BEGIN(CryptoPP)

#if !(defined(_MSC_VER) && (_MSC_VER < 1300))
using std::new_handler;
#endif

typedef void * (CRYPTOPP_API * PNew)(size_t);
typedef void (CRYPTOPP_API * PDelete)(void *);
typedef void (CRYPTOPP_API * PGetNewAndDelete)(PNew &, PDelete &);
typedef new_handler (CRYPTOPP_API * PSetNewHandler)(new_handler);
typedef void (CRYPTOPP_API * PSetNewAndDelete)(PNew, PDelete, PSetNewHandler);

NAMESPACE_END

#endif
