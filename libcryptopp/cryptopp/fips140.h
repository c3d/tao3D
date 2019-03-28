#ifndef CRYPTOPP_FIPS140_H
#define CRYPTOPP_FIPS140_H
// *****************************************************************************
// fips140.h                                                       Tao3D project
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

/*! \file
	FIPS 140 related functions and classes.
*/

#include "cryptlib.h"
#include "secblock.h"

NAMESPACE_BEGIN(CryptoPP)

//! exception thrown when a crypto algorithm is used after a self test fails
class CRYPTOPP_DLL SelfTestFailure : public Exception
{
public:
	explicit SelfTestFailure(const std::string &s) : Exception(OTHER_ERROR, s) {}
};

//! returns whether FIPS 140-2 compliance features were enabled at compile time
CRYPTOPP_DLL bool CRYPTOPP_API FIPS_140_2_ComplianceEnabled();

//! enum values representing status of the power-up self test
enum PowerUpSelfTestStatus {POWER_UP_SELF_TEST_NOT_DONE, POWER_UP_SELF_TEST_FAILED, POWER_UP_SELF_TEST_PASSED};

//! perform the power-up self test, and set the self test status
CRYPTOPP_DLL void CRYPTOPP_API DoPowerUpSelfTest(const char *moduleFilename, const byte *expectedModuleMac);

//! perform the power-up self test using the filename of this DLL and the embedded module MAC
CRYPTOPP_DLL void CRYPTOPP_API DoDllPowerUpSelfTest();

//! set the power-up self test status to POWER_UP_SELF_TEST_FAILED
CRYPTOPP_DLL void CRYPTOPP_API SimulatePowerUpSelfTestFailure();

//! return the current power-up self test status
CRYPTOPP_DLL PowerUpSelfTestStatus CRYPTOPP_API GetPowerUpSelfTestStatus();

typedef PowerUpSelfTestStatus (CRYPTOPP_API * PGetPowerUpSelfTestStatus)();

CRYPTOPP_DLL MessageAuthenticationCode * CRYPTOPP_API NewIntegrityCheckingMAC();

CRYPTOPP_DLL bool CRYPTOPP_API IntegrityCheckModule(const char *moduleFilename, const byte *expectedModuleMac, SecByteBlock *pActualMac = NULL, unsigned long *pMacFileLocation = NULL);

// this is used by Algorithm constructor to allow Algorithm objects to be constructed for the self test
bool PowerUpSelfTestInProgressOnThisThread();

void SetPowerUpSelfTestInProgressOnThisThread(bool inProgress);

void SignaturePairwiseConsistencyTest(const PK_Signer &signer, const PK_Verifier &verifier);
void EncryptionPairwiseConsistencyTest(const PK_Encryptor &encryptor, const PK_Decryptor &decryptor);

void SignaturePairwiseConsistencyTest_FIPS_140_Only(const PK_Signer &signer, const PK_Verifier &verifier);
void EncryptionPairwiseConsistencyTest_FIPS_140_Only(const PK_Encryptor &encryptor, const PK_Decryptor &decryptor);

#define CRYPTOPP_DUMMY_DLL_MAC "MAC_51f34b8db820ae8"

NAMESPACE_END

#endif
