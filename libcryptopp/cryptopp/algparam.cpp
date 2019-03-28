// *****************************************************************************
// algparam.cpp                                                    Tao3D project
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
// algparam.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"

#ifndef CRYPTOPP_IMPORTS

#include "algparam.h"

NAMESPACE_BEGIN(CryptoPP)

PAssignIntToInteger g_pAssignIntToInteger = NULL;

bool CombinedNameValuePairs::GetVoidValue(const char *name, const std::type_info &valueType, void *pValue) const
{
	if (strcmp(name, "ValueNames") == 0)
		return m_pairs1.GetVoidValue(name, valueType, pValue) && m_pairs2.GetVoidValue(name, valueType, pValue);
	else
		return m_pairs1.GetVoidValue(name, valueType, pValue) || m_pairs2.GetVoidValue(name, valueType, pValue);
}

void AlgorithmParametersBase::operator=(const AlgorithmParametersBase& rhs)
{
	assert(false);
}

bool AlgorithmParametersBase::GetVoidValue(const char *name, const std::type_info &valueType, void *pValue) const
{
	if (strcmp(name, "ValueNames") == 0)
	{
		NameValuePairs::ThrowIfTypeMismatch(name, typeid(std::string), valueType);
		if (m_next.get())
		    m_next->GetVoidValue(name, valueType, pValue);
		(*reinterpret_cast<std::string *>(pValue) += m_name) += ";";
		return true;
	}
	else if (strcmp(name, m_name) == 0)
	{
		AssignValue(name, valueType, pValue);
		m_used = true;
		return true;
	}
	else if (m_next.get())
		return m_next->GetVoidValue(name, valueType, pValue);
	else
	    return false;
}

AlgorithmParameters::AlgorithmParameters()
	: m_defaultThrowIfNotUsed(true)
{
}

AlgorithmParameters::AlgorithmParameters(const AlgorithmParameters &x)
	: m_defaultThrowIfNotUsed(x.m_defaultThrowIfNotUsed)
{
	m_next.reset(const_cast<AlgorithmParameters &>(x).m_next.release());
}

AlgorithmParameters & AlgorithmParameters::operator=(const AlgorithmParameters &x)
{
	m_next.reset(const_cast<AlgorithmParameters &>(x).m_next.release());
	return *this;
}

bool AlgorithmParameters::GetVoidValue(const char *name, const std::type_info &valueType, void *pValue) const
{
	if (m_next.get())
		return m_next->GetVoidValue(name, valueType, pValue);
	else
		return false;
}

NAMESPACE_END

#endif
