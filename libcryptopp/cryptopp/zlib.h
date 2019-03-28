#ifndef CRYPTOPP_ZLIB_H
#define CRYPTOPP_ZLIB_H
// *****************************************************************************
// zlib.h                                                          Tao3D project
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

#include "adler32.h"
#include "zdeflate.h"
#include "zinflate.h"

NAMESPACE_BEGIN(CryptoPP)

/// ZLIB Compressor (RFC 1950)
class ZlibCompressor : public Deflator
{
public:
	ZlibCompressor(BufferedTransformation *attachment=NULL, unsigned int deflateLevel=DEFAULT_DEFLATE_LEVEL, unsigned int log2WindowSize=DEFAULT_LOG2_WINDOW_SIZE, bool detectUncompressible=true)
		: Deflator(attachment, deflateLevel, log2WindowSize, detectUncompressible) {}
	ZlibCompressor(const NameValuePairs &parameters, BufferedTransformation *attachment=NULL)
		: Deflator(parameters, attachment) {}

	unsigned int GetCompressionLevel() const;

protected:
	void WritePrestreamHeader();
	void ProcessUncompressedData(const byte *string, size_t length);
	void WritePoststreamTail();

	Adler32 m_adler32;
};

/// ZLIB Decompressor (RFC 1950)
class ZlibDecompressor : public Inflator
{
public:
	typedef Inflator::Err Err;
	class HeaderErr : public Err {public: HeaderErr() : Err(INVALID_DATA_FORMAT, "ZlibDecompressor: header decoding error") {}};
	class Adler32Err : public Err {public: Adler32Err() : Err(DATA_INTEGRITY_CHECK_FAILED, "ZlibDecompressor: ADLER32 check error") {}};
	class UnsupportedAlgorithm : public Err {public: UnsupportedAlgorithm() : Err(INVALID_DATA_FORMAT, "ZlibDecompressor: unsupported algorithm") {}};
	class UnsupportedPresetDictionary : public Err {public: UnsupportedPresetDictionary() : Err(INVALID_DATA_FORMAT, "ZlibDecompressor: unsupported preset dictionary") {}};

	/*! \param repeat decompress multiple compressed streams in series
		\param autoSignalPropagation 0 to turn off MessageEnd signal
	*/
	ZlibDecompressor(BufferedTransformation *attachment = NULL, bool repeat = false, int autoSignalPropagation = -1);
	unsigned int GetLog2WindowSize() const {return m_log2WindowSize;}

private:
	unsigned int MaxPrestreamHeaderSize() const {return 2;}
	void ProcessPrestreamHeader();
	void ProcessDecompressedData(const byte *string, size_t length);
	unsigned int MaxPoststreamTailSize() const {return 4;}
	void ProcessPoststreamTail();

	unsigned int m_log2WindowSize;
	Adler32 m_adler32;
};

NAMESPACE_END

#endif
