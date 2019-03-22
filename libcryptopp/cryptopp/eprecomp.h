#ifndef CRYPTOPP_EPRECOMP_H
#define CRYPTOPP_EPRECOMP_H
// *****************************************************************************
// eprecomp.h                                                      Tao3D project
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

#include "integer.h"
#include "algebra.h"
#include <vector>

NAMESPACE_BEGIN(CryptoPP)

template <class T>
class DL_GroupPrecomputation
{
public:
	typedef T Element;

	virtual bool NeedConversions() const {return false;}
	virtual Element ConvertIn(const Element &v) const {return v;}
	virtual Element ConvertOut(const Element &v) const {return v;}
	virtual const AbstractGroup<Element> & GetGroup() const =0;
	virtual Element BERDecodeElement(BufferedTransformation &bt) const =0;
	virtual void DEREncodeElement(BufferedTransformation &bt, const Element &P) const =0;
};

template <class T>
class DL_FixedBasePrecomputation
{
public:
	typedef T Element;

	virtual bool IsInitialized() const =0;
	virtual void SetBase(const DL_GroupPrecomputation<Element> &group, const Element &base) =0;
	virtual const Element & GetBase(const DL_GroupPrecomputation<Element> &group) const =0;
	virtual void Precompute(const DL_GroupPrecomputation<Element> &group, unsigned int maxExpBits, unsigned int storage) =0;
	virtual void Load(const DL_GroupPrecomputation<Element> &group, BufferedTransformation &storedPrecomputation) =0;
	virtual void Save(const DL_GroupPrecomputation<Element> &group, BufferedTransformation &storedPrecomputation) const =0;
	virtual Element Exponentiate(const DL_GroupPrecomputation<Element> &group, const Integer &exponent) const =0;
	virtual Element CascadeExponentiate(const DL_GroupPrecomputation<Element> &group, const Integer &exponent, const DL_FixedBasePrecomputation<Element> &pc2, const Integer &exponent2) const =0;
};

template <class T>
class DL_FixedBasePrecomputationImpl : public DL_FixedBasePrecomputation<T>
{
public:
	typedef T Element;

	DL_FixedBasePrecomputationImpl() : m_windowSize(0) {}

	// DL_FixedBasePrecomputation
	bool IsInitialized() const
		{return !m_bases.empty();}
	void SetBase(const DL_GroupPrecomputation<Element> &group, const Element &base);
	const Element & GetBase(const DL_GroupPrecomputation<Element> &group) const
		{return group.NeedConversions() ? m_base : m_bases[0];}
	void Precompute(const DL_GroupPrecomputation<Element> &group, unsigned int maxExpBits, unsigned int storage);
	void Load(const DL_GroupPrecomputation<Element> &group, BufferedTransformation &storedPrecomputation);
	void Save(const DL_GroupPrecomputation<Element> &group, BufferedTransformation &storedPrecomputation) const;
	Element Exponentiate(const DL_GroupPrecomputation<Element> &group, const Integer &exponent) const;
	Element CascadeExponentiate(const DL_GroupPrecomputation<Element> &group, const Integer &exponent, const DL_FixedBasePrecomputation<Element> &pc2, const Integer &exponent2) const;

private:
	void PrepareCascade(const DL_GroupPrecomputation<Element> &group, std::vector<BaseAndExponent<Element> > &eb, const Integer &exponent) const;

	Element m_base;
	unsigned int m_windowSize;
	Integer m_exponentBase;			// what base to represent the exponent in
	std::vector<Element> m_bases;	// precalculated bases
};

NAMESPACE_END

#ifdef CRYPTOPP_MANUALLY_INSTANTIATE_TEMPLATES
#include "eprecomp.cpp"
#endif

#endif
