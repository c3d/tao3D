// ****************************************************************************
//  keygen.cpp                                                      Tao project
// ****************************************************************************
// 
//   File Description:
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2011 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2011 Taodyne SAS
// ****************************************************************************

#include "license.h"
#include "version.h"
#include "main.h"
#include "flight_recorder.h"
#include "tao_utf8.h"

#include "cryptopp/dsa.h"
#include "cryptopp/osrng.h"
#include "cryptopp/base64.h"
#include "doc_private_key_dsa.h"
#include "doc_public_key_dsa.h"

#include <QFile>
#include <QTextStream>
#include <iostream>

XL_DEFINE_TRACES

void usage(const char *progname);
void sign_raw(const char *path);

// For inline text operator +(QString s)
using namespace Tao;

// Raw mode: store public key and signature of raw file content into .sig file
bool raw = false;
char * prog = "tao_sign";

int main(int argc, char *argv[])
// ----------------------------------------------------------------------------
//    Generate key files for Tao
// ----------------------------------------------------------------------------
{
    prog = argv[0];

    XL::FlightRecorder::Initialize();
    XL::MAIN = new XL::Main(1, argv);
    for (int arg = 1; arg < argc; arg++)
    {
        if (std::string(argv[arg]) == "-h")
        {
            usage(argv[0]);
            exit(0);
        }
        else if (std::string(argv[arg]) == "-r")
        {
            raw = true;
        }
        else
        {
            // File name
            if (raw)
                sign_raw(argv[arg]);
            else
                Tao::Licenses::AddLicenseFile(argv[arg]);
        }
    }
    return 0;
}

void usage(const char *progname)
// ----------------------------------------------------------------------------
//    Show usage
// ----------------------------------------------------------------------------
{
    std::cerr << "Taodyne file signing tool version "
              << GITREV " (" GITSHA1 ")\n"
              << "Usage: " << progname << " [-r] <files>\n\n"
              << "Options:\n"
              << "  -r    Raw mode: store public key and signature of raw\n"
              << "        file content into a file of the same name with the\n"
              << "        .sig extension.\n";
    exit(0);
}

using namespace CryptoPP;

void sign_raw(const char *path)
// ----------------------------------------------------------------------------
//    Sign SHA1 digest of file content with DSA private key, create .sig file
// ----------------------------------------------------------------------------
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly))
    {
        std::cerr << prog << ": " << path << ": " << +file.errorString()
                  << "\n";
        exit(1);
    }
    QByteArray content = file.readAll();

    try
    {
        std::string b64_pubkey, b64_signature;
        QString qpub, qsig;

        byte pubkey[] = DOC_DSA_PUBLIC_KEY;
        StringSource(pubkey, sizeof(pubkey), true,
                     new Base64Encoder(
                         new StringSink(b64_pubkey)));
        qpub.append(b64_pubkey.data());
        qpub = qpub.trimmed();

        byte privkey[] = DOC_DSA_PRIVATE_KEY;
        DSA::Signer signer;
        PrivateKey &privateKey = signer.AccessPrivateKey();
        privateKey.Load(StringSource(privkey, sizeof(privkey), true).Ref());
        AutoSeededRandomPool rng;
        CryptoPP::SHA1 hash;
        StringSource((const byte*)content.data(), content.size(), true,
                   new HashFilter(hash,
                       new SignerFilter(rng, signer,
                           new Base64Encoder(
                               new StringSink(b64_signature)))));
        qsig.append(b64_signature.data());
        qsig = qsig.trimmed();

        QString outPath = QString(path) + QString(".sig");
        QFile outFile(outPath);
        if (!outFile.open(QFile::WriteOnly))
        {
            std::cerr << +outPath << ": " << +outFile.errorString() << "\n";
            exit(1);
        }
        QTextStream out(&outFile);
        out.setCodec("UTF-8");
        out << "; Signed by: Taodyne file signing tool\n";
        out << "; version " << GITREV << " (" << GITSHA1 << ")\n";
        out << "signature=\"" << qsig << "\"\n";
        out << "pubkey=\"\n" << qpub << "\n\"\n";
    }
    catch (Exception &e)
    {
        std::cerr << prog << ": " <<  e.what() << "\n";
        exit(1);
    }
}
