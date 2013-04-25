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
//  (C) 2013 Jerome Forissier <jerome@taodyne.com>
//  (C) 2011 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2011, 2013 Taodyne SAS
// ****************************************************************************

#include "license.h"
#include "version.h"
#include "main.h"
#include "flight_recorder.h"
#include "tao_utf8.h"
#include "crypto.h"

#include "private_key_dsa.h"
#include "public_key_dsa.h"

#include <QFileInfo>
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
    QFileInfo info(progname);
    QString name = info.fileName();
    std::cerr << "Taodyne file signing tool version "
              << GITREV " (" GITSHA1 ")\n\n"
              << "Usage: " << +name << " <unsigned .taokey files>\n"
              << "       " << +name << " -r <files>\n\n"
              << "Sign Tao Presentations license files (.taokey format),\n"
              << "or any file when run with -r.\n\n"
              << "Options:\n"
              << "  -r    Raw mode: store public key and signature of raw\n"
              << "        file content into a file of the same name with the\n"
              << "        .sig extension.\n";
    exit(0);
}

void sign_raw(const char *path)
// ----------------------------------------------------------------------------
//    Sign SHA1 digest of file content with DSA private key, create .sig file
// ----------------------------------------------------------------------------
{
    QString qpath = QString::fromLocal8Bit(path);

    static const char pub[] = TAO_DSA_PUBLIC_KEY;
    static const char priv[] = TAO_DSA_PRIVATE_KEY;
    QByteArray qpub(pub, sizeof(pub));
    QByteArray qpriv(priv, sizeof(priv));

    QString ident = "; Signed by: Taodyne file signing tool\n"
                    "; version " GITREV " (" GITSHA1 ")\n";

    QString res = Tao::Crypto::Sign(qpath, qpub, qpriv, ident);
    if (!res.isEmpty())
    {
        std::cerr << prog << ": " << +res << "\n";
        exit(1);
    }
}
