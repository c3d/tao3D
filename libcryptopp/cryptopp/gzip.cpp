// *****************************************************************************
// gzip.cpp                                                        Tao3D project
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
// gzip.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "gzip.h"

NAMESPACE_BEGIN(CryptoPP)

void Gzip::WritePrestreamHeader()
{
	m_totalLen = 0;
	m_crc.Restart();

	AttachedTransformation()->Put(MAGIC1);
	AttachedTransformation()->Put(MAGIC2);
	AttachedTransformation()->Put(DEFLATED);
	AttachedTransformation()->Put(0);		// general flag
	AttachedTransformation()->PutWord32(0);	// time stamp
	byte extra = (GetDeflateLevel() == 1) ? FAST : ((GetDeflateLevel() == 9) ? SLOW : 0);
	AttachedTransformation()->Put(extra);
	AttachedTransformation()->Put(GZIP_OS_CODE);
}

void Gzip::ProcessUncompressedData(const byte *inString, size_t length)
{
	m_crc.Update(inString, length);
	m_totalLen += (word32)length;
}

void Gzip::WritePoststreamTail()
{
	SecByteBlock crc(4);
	m_crc.Final(crc);
	AttachedTransformation()->Put(crc, 4);
	AttachedTransformation()->PutWord32(m_totalLen, LITTLE_ENDIAN_ORDER);
}

// *************************************************************

Gunzip::Gunzip(BufferedTransformation *attachment, bool repeat, int propagation)
	: Inflator(attachment, repeat, propagation)
{
}

void Gunzip::ProcessPrestreamHeader()
{
	m_length = 0;
	m_crc.Restart();

	byte buf[6];
	byte b, flags;

	if (m_inQueue.Get(buf, 2)!=2) throw HeaderErr();
	if (buf[0] != MAGIC1 || buf[1] != MAGIC2) throw HeaderErr();
	if (!m_inQueue.Skip(1)) throw HeaderErr();	 // skip extra flags
	if (!m_inQueue.Get(flags)) throw HeaderErr();
	if (flags & (ENCRYPTED | CONTINUED)) throw HeaderErr();
	if (m_inQueue.Skip(6)!=6) throw HeaderErr();    // Skip file time, extra flags and OS type

	if (flags & EXTRA_FIELDS)	// skip extra fields
	{
		word16 length;
		if (m_inQueue.GetWord16(length, LITTLE_ENDIAN_ORDER) != 2) throw HeaderErr();
		if (m_inQueue.Skip(length)!=length) throw HeaderErr();
	}

	if (flags & FILENAME)	// skip filename
		do
			if(!m_inQueue.Get(b)) throw HeaderErr();
		while (b);

	if (flags & COMMENTS)	// skip comments
		do
			if(!m_inQueue.Get(b)) throw HeaderErr();
		while (b);
}

void Gunzip::ProcessDecompressedData(const byte *inString, size_t length)
{
	AttachedTransformation()->Put(inString, length);
	m_crc.Update(inString, length);
	m_length += (word32)length;
}

void Gunzip::ProcessPoststreamTail()
{
	SecByteBlock crc(4);
	if (m_inQueue.Get(crc, 4) != 4)
		throw TailErr();
	if (!m_crc.Verify(crc))
		throw CrcErr();

	word32 lengthCheck;
	if (m_inQueue.GetWord32(lengthCheck, LITTLE_ENDIAN_ORDER) != 4)
		throw TailErr();
	if (lengthCheck != m_length)
		throw LengthErr();
}

NAMESPACE_END
