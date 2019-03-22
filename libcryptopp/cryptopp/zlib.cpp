// *****************************************************************************
// zlib.cpp                                                        Tao3D project
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

ZlibDecompressor::ZlibDecompressor(BufferedTransformation *attachment, bool repeat, int propagation)
	: Inflator(attachment, repeat, propagation)
{
}

void ZlibDecompressor::ProcessPrestreamHeader()
{
	m_adler32.Restart();

	byte cmf;
	byte flags;

	if (!m_inQueue.Get(cmf) || !m_inQueue.Get(flags))
		throw HeaderErr();

	if ((cmf*256+flags) % 31 != 0)
		throw HeaderErr();	// if you hit this exception, you're probably trying to decompress invalid data

	if ((cmf & 0xf) != DEFLATE_METHOD)
		throw UnsupportedAlgorithm();

	if (flags & FDICT_FLAG)
		throw UnsupportedPresetDictionary();

	m_log2WindowSize = 8 + (cmf >> 4);
}

void ZlibDecompressor::ProcessDecompressedData(const byte *inString, size_t length)
{
	AttachedTransformation()->Put(inString, length);
	m_adler32.Update(inString, length);
}

void ZlibDecompressor::ProcessPoststreamTail()
{
	FixedSizeSecBlock<byte, 4> adler32;
	if (m_inQueue.Get(adler32, 4) != 4)
		throw Adler32Err();
	if (!m_adler32.Verify(adler32))
		throw Adler32Err();
}

NAMESPACE_END
