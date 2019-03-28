#ifndef CRYPTOPP_VALIDATE_H
#define CRYPTOPP_VALIDATE_H
// *****************************************************************************
// validate.h                                                      Tao3D project
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
// (C) 2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
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

#include "cryptlib.h"

bool ValidateAll(bool thorough);
bool TestSettings();
bool TestOS_RNG();
bool ValidateBaseCode();

bool ValidateCRC32();
bool ValidateAdler32();
bool ValidateMD2();
bool ValidateMD4();
bool ValidateMD5();
bool ValidateSHA();
bool ValidateSHA2();
bool ValidateTiger();
bool ValidateRIPEMD();
bool ValidatePanama();
bool ValidateWhirlpool();

bool ValidateHMAC();
bool ValidateTTMAC();

bool ValidateCipherModes();
bool ValidatePBKDF();

bool ValidateDES();
bool ValidateIDEA();
bool ValidateSAFER();
bool ValidateRC2();
bool ValidateARC4();

bool ValidateRC5();
bool ValidateBlowfish();
bool ValidateThreeWay();
bool ValidateGOST();
bool ValidateSHARK();
bool ValidateSEAL();
bool ValidateCAST();
bool ValidateSquare();
bool ValidateSKIPJACK();
bool ValidateRC6();
bool ValidateMARS();
bool ValidateRijndael();
bool ValidateTwofish();
bool ValidateSerpent();
bool ValidateSHACAL2();
bool ValidateCamellia();
bool ValidateSalsa();
bool ValidateSosemanuk();
bool ValidateVMAC();
bool ValidateCCM();
bool ValidateGCM();
bool ValidateCMAC();

bool ValidateBBS();
bool ValidateDH();
bool ValidateMQV();
bool ValidateRSA();
bool ValidateElGamal();
bool ValidateDLIES();
bool ValidateNR();
bool ValidateDSA(bool thorough);
bool ValidateLUC();
bool ValidateLUC_DL();
bool ValidateLUC_DH();
bool ValidateXTR_DH();
bool ValidateRabin();
bool ValidateRW();
//bool ValidateBlumGoldwasser();
bool ValidateECP();
bool ValidateEC2N();
bool ValidateECDSA();
bool ValidateESIGN();

CryptoPP::RandomNumberGenerator & GlobalRNG();
bool RunTestDataFile(const char *filename, const CryptoPP::NameValuePairs &overrideParameters=CryptoPP::g_nullNameValuePairs, bool thorough=true);

#endif
