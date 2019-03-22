#ifndef CRYPTOPP_MODEXPPC_H
#define CRYPTOPP_MODEXPPC_H
// *****************************************************************************
// modexppc.h                                                      Tao3D project
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

#include "modarith.h"
#include "eprecomp.h"
#include "smartptr.h"
#include "pubkey.h"

NAMESPACE_BEGIN(CryptoPP)

CRYPTOPP_DLL_TEMPLATE_CLASS DL_FixedBasePrecomputationImpl<Integer>;

class ModExpPrecomputation : public DL_GroupPrecomputation<Integer>
{
public:
	// DL_GroupPrecomputation
	bool NeedConversions() const {return true;}
	Element ConvertIn(const Element &v) const {return m_mr->ConvertIn(v);}
	virtual Element ConvertOut(const Element &v) const {return m_mr->ConvertOut(v);}
	const AbstractGroup<Element> & GetGroup() const {return m_mr->MultiplicativeGroup();}
	Element BERDecodeElement(BufferedTransformation &bt) const {return Integer(bt);}
	void DEREncodeElement(BufferedTransformation &bt, const Element &v) const {v.DEREncode(bt);}

	// non-inherited
	void SetModulus(const Integer &v) {m_mr.reset(new MontgomeryRepresentation(v));}
	const Integer & GetModulus() const {return m_mr->GetModulus();}

private:
	value_ptr<MontgomeryRepresentation> m_mr;
};

NAMESPACE_END

#endif
