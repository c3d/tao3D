// ****************************************************************************
//  licence.cpp                                                     Tao project
// ****************************************************************************
//
//   File Description:
//
//     Licence check for Tao Presentation
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
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "licence.h"
#include "scanner.h"
#include "main.h"
#include "flight_recorder.h"
#include "tao_utf8.h"
#include "public_key_dsa.h"
#ifdef KEYGEN
#include "private_key_dsa.h"
#endif
#include "cryptopp/dsa.h"
#include "cryptopp/osrng.h"

#include <sstream>
#include <fstream>
#include <iostream>
#include <cstdio>

#ifndef KEYGEN
#include "license_dialog.h"
#if defined (Q_OS_MACX)
#include <QProcess>
#include <QStringList>
#include <QByteArray>
#include <QRegExp>
#elif defined (Q_OS_WIN32)
#include <windows.h>
#elif defined (Q_OS_LINUX)
#include <QFile>
#include <QByteArray>
#include <QString>
#endif
#endif

using namespace CryptoPP;

namespace Tao
{


Licences &Licences::LM()
// ----------------------------------------------------------------------------
//   Return a singleton for the Licences class
// ----------------------------------------------------------------------------
{
    static Licences lm;
    return lm;
}


Licences::Licences()
// ----------------------------------------------------------------------------
//   Initialize the licence manager by loading the licence file
// ----------------------------------------------------------------------------
    : licences()
{}


Licences::~Licences()
// ----------------------------------------------------------------------------
//   Destructor declared to be private
// ----------------------------------------------------------------------------
{}


void Licences::addLicenceFile(kstring licfname)
// ----------------------------------------------------------------------------
//   Add a licence file and the licences in it
// ----------------------------------------------------------------------------
//   The format of the licence file is something like:
//
//      name "Christophe de Dinechin"
//      address "777, Home Sweet Home Street, CA99701 Birds les Plages"
//      features "foo|bar|zoo.*|gloo[a-z]+"
//      expires 5 dec 1968
{
    RECORD(ALWAYS, "Adding licence file", licfname);

    // Licences we want to add here
    LicenceFile additional;

    // Now analyze what we got from the input text
    XL::Syntax syntax;
    XL::Positions positions;
    XL::Errors errors;
    XL::Scanner scanner(licfname, syntax, positions, errors);
    Licence licence;
    bool had_features = false, had_signature = false;
    int day = 0, month = 0, year = 0;
    enum
    {
        // REVISIT: We may want to add host, ip, MAC, display type, ...
        START, SIGNATURE, DONE, TAG,
        NAME, COMPANY, ADDRESS, EMAIL, FEATURES,
        EXPIRY_DAY, EXPIRY_MONTH, EXPIRY_YEAR,
        HOSTID, ERR
    } state = START;

    while (state != DONE)
    {
        XL::token_t tok = scanner.NextToken();
        text item;

        if (tok == XL::tokNEWLINE)
            continue;

        switch (state)
        {
        case DONE:
        case ERR:
            break;              // Keep compiler happy

        case START:
            licence = Licence();
            had_features = false;
            licence.expiry = QDate();
            state = TAG;
            // Fall through on purpose

        case TAG:
            state = ERR;       // For all error cases
            switch (tok)
            {
            case XL::tokEOF:
                state = DONE;
                break;
            case XL::tokNAME:
                item = scanner.TokenText();
                if (item == "name")
                    state = NAME;
                else if (item == "company")
                    state = COMPANY;
                else if (item == "address")
                    state = ADDRESS;
                else if (item == "email")
                    state = EMAIL;
                else if (item == "features")
                    state = FEATURES;
                else if (item == "expires")
                    state = EXPIRY_DAY;
                else if (item == "signature" || item == "digest")
                    state = SIGNATURE;
                else if (item == "hostid")
                    state = HOSTID;
                else
                    licenceError(licfname, tr("Invalid tag: %1").arg(+item));
                break;
            default:
                licenceError(licfname, tr("Invalid token"));
                break;
            } // switch(token for TAG state)
            break;

        case SIGNATURE:
            // This is normally the final state
            if (tok == XL::tokSTRING || tok == XL::tokQUOTE)
            {
                item = scanner.TextValue();
                if (verify(additional, item))
                {
                    licences.insert(licences.end(),
                                    additional.licences.begin(),
                                    additional.licences.end());
                    state = TAG;
                }
                else
                {
                    licenceError(licfname, tr("Signature verification failed"));
                    state = ERR;
                }
                additional.licences.clear();
            }
            else
            {
                licenceError(licfname, tr("Invalid signature"));
                state = ERR;
            }
            had_signature = true;
            break;

#define PARSE_IDENTITY(PTAG, PREF, PVAR)                                \
        case PTAG:                                                      \
            if (tok == XL::tokSTRING || tok == XL::tokQUOTE)            \
            {                                                           \
                /* Check consistency of supplied information */         \
                item = scanner.TextValue();                             \
                PVAR = item;                                            \
                state = TAG;                                            \
                if (PREF == "")                                         \
                    PREF = item;                                        \
                else if (item != "" && item != PREF)                    \
                    licenceError(licfname, tr("Inconsistent %1").arg(#PREF)); \
            }                                                           \
            else                                                        \
            {                                                           \
                licenceError(licfname, tr("Invalid %1").arg(#PVAR));    \
                state = ERR;                                            \
            }                                                           \
            break;

            PARSE_IDENTITY(NAME, name, additional.name);
            PARSE_IDENTITY(COMPANY, company, additional.company);
            PARSE_IDENTITY(ADDRESS, address, additional.address);
            PARSE_IDENTITY(EMAIL, email, additional.email);

        case HOSTID:
            if (tok == XL::tokSTRING || tok == XL::tokQUOTE)
            {
                text item = scanner.TextValue();
                state = TAG;
#ifndef KEYGEN
                /* Check validity of host ID  */
                if (item != "" && item != hostID())
                {
                    licenceError(licfname, tr("Invalid %1").arg("hostid"));
                    state = ERR;
                }
#endif
                additional.hostid = item;
            }

        case FEATURES:
            if (tok == XL::tokSTRING || tok == XL::tokQUOTE)
            {
                item = scanner.TextValue();
                state = TAG;
                licence.features = QRegExp(+item);
                had_features = true;
            }
            else
            {
                licenceError(licfname, tr("Invalid features pattern"));
                state = ERR;
            }
            break;

        case EXPIRY_DAY:
            if (tok == XL::tokINTEGER)
            {
                day = scanner.IntegerValue();
                state = EXPIRY_MONTH;
                if (day < 1 || day > 31)
                {
                    licenceError(licfname, tr("Invalid day"));
                    state = ERR;
                }
            }
            else
             {
                licenceError(licfname, tr("Invalid expiry day"));
                state = ERR;
            }
            break;

        case EXPIRY_MONTH:
            if (tok == XL::tokNAME || tok == XL::tokSYMBOL)
            {
                item = scanner.TokenText();
                if (item == "/" || item == "-")
                {
                    // Skip
                }
                else
                {
                    // Accept named months
                    kstring months[] =
                    {
                        "jan", "feb", "mar", "apr", "may", "jun",
                        "jul", "aug", "sep", "oct", "nov", "dec"
                    };
                    for (uint m = 0; m < 12; m++)
                    {
                        if (item.substr(0, 3) == months[m])
                        {
                            month = m+1;
                            state = EXPIRY_YEAR;
                            break;
                        }
                    }
                    if (state != EXPIRY_YEAR)
                    {
                        licenceError(licfname, tr("Invalid month name"));
                        state = ERR;
                    }
                }
            }
            else if (tok == XL::tokINTEGER)
            {
                month = scanner.IntegerValue();
                state = EXPIRY_YEAR;
                if (month < 1 || month > 12)
                {
                    licenceError(licfname, tr("Invalid expiry month"));
                    state = ERR;
                }
            }
            break;

        case EXPIRY_YEAR:
            if (tok == XL::tokSYMBOL && (scanner.TokenText() == "/" ||
                                         scanner.TokenText() == "-"))
            {
                // Skip
            }
            else if (tok == XL::tokINTEGER)
            {
                year = scanner.IntegerValue();
                if (year < 2011 || year > 2099)
                {
                    licenceError(licfname, tr("Invalid year"));
                    state = ERR;
                }
                licence.expiry = QDate(year, month, day);
                state = TAG;
            }
            else
            {
                licenceError(licfname, tr("Invalid expiry year"));
                state = ERR;
            }
            break;

        } // switch(state)

        // Check if we have a complete licence, if so enter it
        if (had_features)
        {
            additional.licences.push_back(licence);
            state = START;
        }
    } // while (state != DONE)


#ifdef KEYGEN
    if (additional.licences.size())
    {
        text signature = sign(additional);
        FILE *file = fopen(licfname, "a");
        fprintf(file, "signature \"%s\"\n", signature.c_str());
        fclose(file);
    }
#else
    if (!had_signature && state != ERR)
        licenceError(licfname, tr("Missing digital signature"));
#endif // KEYGEN
}


void Licences::addLicenceFiles(const QFileInfoList &files)
// ----------------------------------------------------------------------------
//   Add several licence files
// ----------------------------------------------------------------------------
{
    foreach (QFileInfo file, files)
    {
        text path = +file.canonicalFilePath();
        IFTRACE(fileload)
            std::cerr << "Loading license file: " << path << "\n";
        Licences::AddLicenceFile(path.c_str());
    }
}


text Licences::toText(LicenceFile &lf)
// ----------------------------------------------------------------------------
//    Create a stream for the licence contents
// ----------------------------------------------------------------------------
{
    std::ostringstream os;
    os << "name \"" << lf.name << "\"\n";
    if (lf.company.length())
        os << "company \"" << lf.company << "\"\n";
    if (lf.address.length())
        os << "address \"" << lf.address << "\"\n";
    if (lf.email.length())
        os << "email \"" << lf.email << "\"\n";
    if (lf.hostid.length())
        os << "hostid \"" << lf.hostid << "\"\n";

    // Loop on all data blocks
    uint i, max = lf.licences.size();
    for (i = 0; i < max; i++)
    {
        Licence &l = lf.licences[i];
        if (l.expiry.isValid())
        {
            os << "expires "
               << l.expiry.day() << "/"
               << l.expiry.month() << "/"
               << l.expiry.year() << "\n";
        }
        os << "features \"" << +l.features.pattern() << "\"\n";
    }
    return os.str();
}


#ifdef KEYGEN

text Licences::sign(LicenceFile &lf)
// ----------------------------------------------------------------------------
//    Sign input data with private key
// ----------------------------------------------------------------------------
{
    // Create a string for the licence contents
    text lic = toText(lf);

    // Sign data
    byte key[] = TAO_DSA_PRIVATE_KEY;
    DSA::Signer signer;
    PrivateKey &privateKey = signer.AccessPrivateKey();
    privateKey.Load(StringSource(key, sizeof(key), true).Ref());

    AutoSeededRandomPool prng;
    byte * signature = new byte[signer.MaxSignatureLength()];
    byte * data = (byte *) lic.data();
    size_t data_length = lic.length();
    size_t length = signer.SignMessage( prng, data, data_length, signature );

    // Compute the result text
    char hex[] = "0123456789ABCDEF";
    text result;
    result.reserve(2*length);
    for (uint i = 0; i < length; i++)
    {
        byte m = (byte) signature[i];
        result.push_back(hex[m >> 4]);
        result.push_back(hex[m & 15]);
    }
    delete[] signature;
    return result;
}

#endif


bool Licences::verify(LicenceFile &licences, text signature)
// ----------------------------------------------------------------------------
//    Verify if signature is valid for input data, using public key
// ----------------------------------------------------------------------------
{
    size_t length = signature.length();
    if (length % 2)
        return false;

    // Convert hex signature to binary
    text sig;
    size_t size = length/2;
    sig.reserve(size);
    std::stringstream sstr(signature);
    for (uint i = 0; i < size; i++)
    {
        char ch1, ch2;
        sstr >> ch1 >> ch2;
        int dig1, dig2;
        if (isdigit(ch1))
            dig1 = ch1 - '0';
        else if (ch1>='A' && ch1<='F')
            dig1 = ch1 - 'A' + 10;
        else if (ch1>='a' && ch1<='f')
            dig1 = ch1 - 'a' + 10;
        else
            return false;
        if (isdigit(ch2))
            dig2 = ch2 - '0';
        else if (ch2>='A' && ch2<='F')
            dig2 = ch2 - 'A' + 10;
        else if (ch2>='a' && ch2<='f')
            dig2 = ch2 - 'a' + 10;
        else
            return false;
        sig[i] = dig1*16 + dig2;
    }

    // Create a string for the licence contents
    text lic = toText(licences);

    // Load public key
    byte key[] = TAO_DSA_PUBLIC_KEY;
    DSA::Verifier verifier;
    PublicKey &publicKey = verifier.AccessPublicKey();
    publicKey.Load(StringSource(key, sizeof(key), true).Ref());

    // Verify signature
    byte * data = (byte *) lic.data();
    size_t data_length = lic.length();
    byte * bsig = (byte *) sig.data();
    bool result = verifier.VerifyMessage(data, data_length, bsig, size);

    return result;
}


struct CheckStatus
// ----------------------------------------------------------------------------
//   Information about the last licence check for a feature
// ----------------------------------------------------------------------------
{
    CheckStatus() {}
    CheckStatus(QDate when, int remaining)
        : when(when), remaining(remaining) {}

    QDate when;
    int   remaining;
};

int Licences::licenceRemainingDays(text feature)
// ----------------------------------------------------------------------------
//   Check if any of the licences grants us the given feature
// ----------------------------------------------------------------------------
{
    QDate today = QDate::currentDate();
    QString qfun = +feature;
    int result = 0;

    // Cache request during 1 day for performance and less verbose logging
    static std::map<text, CheckStatus> checked;
    if (checked.count(feature))
    {
        CheckStatus st = checked[feature];
        if (st.when.isValid() && st.when.daysTo(today) < 1)
            return st.remaining;
    }

    // Loop over all licences
    std::vector<Licence>::iterator l;
    for (l = licences.begin(); l != licences.end(); l++)
    {
        Licence &licence = *l;
        if (licence.features.exactMatch(qfun))
        {
            if (licence.expiry.isValid())
            {
                result = today.daysTo(licence.expiry);
                IFTRACE(lic)
                {
                    if (result >= 0)
                        debug() << "'" << feature << "' valid, expires in "
                                << result + 1 << " day(s)\n";
                    else
                        debug() << "'" << feature << "' expired "
                                << -1 * result << " day(s) ago\n";
                }
            }
            else
            {
                IFTRACE(lic)
                    debug() << "'" << feature << "' valid, never expires\n";
                result = INT_MAX - 1;
            }
            if (result >= 0)
            {
                result = result + 1; // If licence expires today, it's still OK
                break;
            }
        }
    }
    if (result <= 0)
    {
        // No licence matches, or they all expired.
        IFTRACE(lic)
                debug() << "'" << feature << "' not licensed\n";
    }

    // Cache result
    CheckStatus st(today, result);
    checked[feature] = st;

    return result;
}


void Licences::licenceError(kstring file, QString reason)
// ----------------------------------------------------------------------------
//   We had a problem with the licences - Quick exit
// ----------------------------------------------------------------------------
{
    RECORD(ALWAYS, "Licence error", (+reason).c_str(), 0, file, 0);
    licences.clear();

#ifdef KEYGEN
    std::cerr << "Error reading licence file " << file
              << ": " << +reason << "\n";
#else
    QString message;
    message  = tr("<h3>License Error</h3>");
    message += tr("<p>There is a problem with licence file:</p>"
                  "<center>'%1'</center>"
                  "<p>The following error was detected: %2.</p>"
                  "<p>The program will now terminate. "
                  "You need to remove the offending licence file "
                  "before trying to run the application again.</p>")
                  .arg(file).arg(reason);
    message += tr("<p>Please contact "
                  "<a href=\"http://taodyne.com/\">Taodyne</a> "
                  "to obtain valid license files.</p>");
    LicenseDialog oops(message);
    oops.show();
    oops.raise();
    oops.exec();
#endif // KEYGEN
    exit(15);
}


#ifndef KEYGEN
void Licences::Warn(text feature, int days, bool critical)
// ----------------------------------------------------------------------------
//   Remind user that the feature is not licenced or about to expire
// ----------------------------------------------------------------------------
{
    // Warn only once per feature
    static std::set<text> warned;
    if (warned.count(feature))
        return;
    warned.insert(feature);

    QString message = tr("<h3>Warning</h3>");
    if (days > 0)
    {
        message += tr("<p>The license for the following feature expires in "
                      "%n day(s):</p>"
                      "<center>%1</center>", "", days).arg(+feature);
        message += tr("<p>You may obtain new licenses from "
                      "<a href=\"http://taodyne.com/\">Taodyne</a>.</p>");
    }
    else
    {
        if (critical)
            message  = tr("<h3>Error</h3>");
        if (days == 0)
        {
            message += tr("<p>You do not have a valid license for:</p>"
                          "<center>%1</center>").arg(+feature);
        }
        else
        {
            message += tr("<p>You no longer have a valid license for:</p>"
                          "<center>%1</center>").arg(+feature);
            message += tr("<p>The license expired %n day(s) ago.</p>", "",
                          -days);
        }
        message += tr("<p>Please contact "
                      "<a href=\"http://taodyne.com/\">Taodyne</a> "
                      "to obtain valid license files.</p>");
    }
    LicenseDialog * oops = new LicenseDialog(message);
    oops->setAttribute(Qt::WA_DeleteOnClose);
    oops->show();
    oops->raise();
    if (critical)
        oops->exec(); // Blocking ; e.g. initial test in Application
    else
        oops->open(); // Non-blocking ; e.g. module (allows degraded mode)
}


text Licences::hostID()
// ----------------------------------------------------------------------------
//   Return unique host identifier
// ----------------------------------------------------------------------------
{
    static text id;

    if (id == "")
    {
#if defined (Q_OS_MACX)

        // Host ID is the hardware UUID. Available through:
        // Apple menu > About This Mac > More Info...

        QProcess proc;
        QStringList args;
        QByteArray out;
        args << "-rd1" << "-c" << "IOPlatformExpertDevice";
        proc.start("/usr/sbin/ioreg", args);
        if (proc.waitForStarted())
            if (proc.waitForFinished())
                out = proc.readAllStandardOutput();
        QString sout;
        sout.append(QString::fromUtf8(out.data()));
        QRegExp rx("\"IOPlatformUUID\" = \"([-0-9A-F]+)\"");
        if (rx.indexIn(sout) > -1)
            id = +rx.cap(1);

#elif defined (Q_OS_WIN32)

        // Host ID is the Windows Product ID. Available trough:
        // My Computer > Properties...

#ifndef KEY_WOW64_64KEY
#define KEY_WOW64_64KEY 0x0100
#endif

        DWORD flags = KEY_QUERY_VALUE;
        HKEY hKey;
again:
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                          "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0,
                          flags, &hKey) == ERROR_SUCCESS)
        {
            BYTE pid[200];
            DWORD dataLength = sizeof(pid);
            DWORD st = RegQueryValueExA(hKey, "DigitalProductId", NULL, NULL,
                                 pid, &dataLength);
            RegCloseKey(hKey);
            if (st == ERROR_SUCCESS)
            {
                id = +QString::fromLocal8Bit((char *)&pid + 8, 23);
            }
            else if (st == ERROR_FILE_NOT_FOUND &&
                     ((flags & KEY_WOW64_64KEY) == 0))
            {
                // 32-bit application running on 64-bit Windows
                flags |= KEY_WOW64_64KEY;
                goto again;
            }
        }

#elif defined (Q_OS_LINUX)

        // Host ID is the MAC address of eth0, without ":". Available through:
        // cat /sys/class/net/eth0/address | tr -d :

        QFile f("/sys/class/net/eth0/address");
        f.open(QIODevice::ReadOnly);
        QByteArray ba = f.readAll();
        if (!ba.isEmpty())
        {
            QString addr = QString::fromLocal8Bit(ba.data(), ba.size());
            addr.replace(":", "");
            id = +addr;
        }

#endif
    }

    return id;
}


#endif // KEYGEN


std::ostream & Licences::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[Licences] ";
    return std::cerr;
}


}
