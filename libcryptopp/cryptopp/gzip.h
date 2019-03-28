#ifndef CRYPTOPP_GZIP_H
#define CRYPTOPP_GZIP_H
// *****************************************************************************
// gzip.h                                                          Tao3D project
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

#include "zdeflate.h"
#include "zinflate.h"
#include "crc.h"

NAMESPACE_BEGIN(CryptoPP)

/// GZIP Compression (RFC 1952)
class Gzip : public Deflator
{
public:
	Gzip(BufferedTransformation *attachment=NULL, unsigned int deflateLevel=DEFAULT_DEFLATE_LEVEL, unsigned int log2WindowSize=DEFAULT_LOG2_WINDOW_SIZE, bool detectUncompressible=true)
		: Deflator(attachment, deflateLevel, log2WindowSize, detectUncompressible) {}
	Gzip(const NameValuePairs &parameters, BufferedTransformation *attachment=NULL)
		: Deflator(parameters, attachment) {}

protected:
	enum {MAGIC1=0x1f, MAGIC2=0x8b,   // flags for the header
		  DEFLATED=8, FAST=4, SLOW=2};

	void WritePrestreamHeader();
	void ProcessUncompressedData(const byte *string, size_t length);
	void WritePoststreamTail();

	word32 m_totalLen;
	CRC32 m_crc;
};

/// GZIP Decompression (RFC 1952)
class Gunzip : public Inflator
{
public:
	typedef Inflator::Err Err;
	class HeaderErr : public Err {public: HeaderErr() : Err(INVALID_DATA_FORMAT, "Gunzip: header decoding error") {}};
	class TailErr : public Err {public: TailErr() : Err(INVALID_DATA_FORMAT, "Gunzip: tail too short") {}};
	class CrcErr : public Err {public: CrcErr() : Err(DATA_INTEGRITY_CHECK_FAILED, "Gunzip: CRC check error") {}};
	class LengthErr : public Err {public: LengthErr() : Err(DATA_INTEGRITY_CHECK_FAILED, "Gunzip: length check error") {}};

	/*! \param repeat decompress multiple compressed streams in series
		\param autoSignalPropagation 0 to turn off MessageEnd signal
	*/
	Gunzip(BufferedTransformation *attachment = NULL, bool repeat = false, int autoSignalPropagation = -1);

protected:
	enum {MAGIC1=0x1f, MAGIC2=0x8b,   // flags for the header
		DEFLATED=8};

	enum FLAG_MASKS {
		CONTINUED=2, EXTRA_FIELDS=4, FILENAME=8, COMMENTS=16, ENCRYPTED=32};

	unsigned int MaxPrestreamHeaderSize() const {return 1024;}
	void ProcessPrestreamHeader();
	void ProcessDecompressedData(const byte *string, size_t length);
	unsigned int MaxPoststreamTailSize() const {return 8;}
	void ProcessPoststreamTail();

	word32 m_length;
	CRC32 m_crc;
};

NAMESPACE_END

#endif
