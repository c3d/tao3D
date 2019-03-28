// *****************************************************************************
// fips140.cpp                                                     Tao3D project
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
// fips140.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"

#ifndef CRYPTOPP_IMPORTS

#include "fips140.h"
#include "trdlocal.h"	// needs to be included last for cygwin

NAMESPACE_BEGIN(CryptoPP)

// Define this to 1 to turn on FIPS 140-2 compliance features, including additional tests during
// startup, random number generation, and key generation. These tests may affect performance.
#ifndef CRYPTOPP_ENABLE_COMPLIANCE_WITH_FIPS_140_2
#define CRYPTOPP_ENABLE_COMPLIANCE_WITH_FIPS_140_2 0
#endif

#if (CRYPTOPP_ENABLE_COMPLIANCE_WITH_FIPS_140_2 && !defined(THREADS_AVAILABLE))
#error FIPS 140-2 compliance requires the availability of thread local storage.
#endif

#if (CRYPTOPP_ENABLE_COMPLIANCE_WITH_FIPS_140_2 && !defined(OS_RNG_AVAILABLE))
#error FIPS 140-2 compliance requires the availability of OS provided RNG.
#endif

PowerUpSelfTestStatus g_powerUpSelfTestStatus = POWER_UP_SELF_TEST_NOT_DONE;

bool FIPS_140_2_ComplianceEnabled()
{
	return CRYPTOPP_ENABLE_COMPLIANCE_WITH_FIPS_140_2;
}

void SimulatePowerUpSelfTestFailure()
{
	g_powerUpSelfTestStatus = POWER_UP_SELF_TEST_FAILED;
}

PowerUpSelfTestStatus CRYPTOPP_API GetPowerUpSelfTestStatus()
{
	return g_powerUpSelfTestStatus;
}

#if CRYPTOPP_ENABLE_COMPLIANCE_WITH_FIPS_140_2
ThreadLocalStorage & AccessPowerUpSelfTestInProgress()
{
	static ThreadLocalStorage selfTestInProgress;
	return selfTestInProgress;
}
#endif

bool PowerUpSelfTestInProgressOnThisThread()
{
#if CRYPTOPP_ENABLE_COMPLIANCE_WITH_FIPS_140_2
	return AccessPowerUpSelfTestInProgress().GetValue() != NULL;
#else
	assert(false);	// should not be called
	return false;
#endif
}

void SetPowerUpSelfTestInProgressOnThisThread(bool inProgress)
{
#if CRYPTOPP_ENABLE_COMPLIANCE_WITH_FIPS_140_2
	AccessPowerUpSelfTestInProgress().SetValue((void *)inProgress);
#endif
}

void EncryptionPairwiseConsistencyTest_FIPS_140_Only(const PK_Encryptor &encryptor, const PK_Decryptor &decryptor)
{
#if CRYPTOPP_ENABLE_COMPLIANCE_WITH_FIPS_140_2
	EncryptionPairwiseConsistencyTest(encryptor, decryptor);
#endif
}

void SignaturePairwiseConsistencyTest_FIPS_140_Only(const PK_Signer &signer, const PK_Verifier &verifier)
{
#if CRYPTOPP_ENABLE_COMPLIANCE_WITH_FIPS_140_2
	SignaturePairwiseConsistencyTest(signer, verifier);
#endif
}

NAMESPACE_END

#endif
