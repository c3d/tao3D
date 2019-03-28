#ifndef CRYPTOPP_IDEA_H
#define CRYPTOPP_IDEA_H
// *****************************************************************************
// idea.h                                                          Tao3D project
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

/** \file
*/

#include "seckey.h"
#include "secblock.h"

NAMESPACE_BEGIN(CryptoPP)

//! _
struct IDEA_Info : public FixedBlockSize<8>, public FixedKeyLength<16>, public FixedRounds<8>
{
	static const char *StaticAlgorithmName() {return "IDEA";}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#IDEA">IDEA</a>
class IDEA : public IDEA_Info, public BlockCipherDocumentation
{
public:		// made public for internal purposes
#ifdef CRYPTOPP_NATIVE_DWORD_AVAILABLE
	typedef word Word;
#else
	typedef hword Word;
#endif

private:
	class CRYPTOPP_NO_VTABLE Base : public BlockCipherImpl<IDEA_Info>
	{
	public:
		unsigned int OptimalDataAlignment() const {return 2;}
		void ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const;

		void UncheckedSetKey(const byte *userKey, unsigned int length, const NameValuePairs &params);

	private:
		void EnKey(const byte *);
		void DeKey();
		FixedSizeSecBlock<Word, 6*ROUNDS+4> m_key;

	#ifdef IDEA_LARGECACHE
		static inline void LookupMUL(word &a, word b);
		void LookupKeyLogs();
		static void BuildLogTables();
		static volatile bool tablesBuilt;
		static word16 log[0x10000], antilog[0x10000];
	#endif
	};

public:
	typedef BlockCipherFinal<ENCRYPTION, Base> Encryption;
	typedef BlockCipherFinal<DECRYPTION, Base> Decryption;
};

typedef IDEA::Encryption IDEAEncryption;
typedef IDEA::Decryption IDEADecryption;

NAMESPACE_END

#endif
