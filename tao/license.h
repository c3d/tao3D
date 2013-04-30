#ifndef LICENSE_H
#define LICENSE_H
// ****************************************************************************
//  license.h                                                       Tao project
// ****************************************************************************
//
//   File Description:
//
//     License check for Tao Presentation
//
//     Sources for information:
//     - http://www.sentientfood.com/display_story.php?articleid=3
//     - http://sigpipe.macromates.com/2004/09/05/using-openssl-for-license-keys
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

#include "base.h"
#ifndef KEYGEN
#include "version.h"
#include "application.h"
#ifndef CFG_NO_DOC_SIGNATURE
#include "window.h"
#endif
#include <set>
#endif

#include <vector>
#include <QRegExp>
#include <QDateTime>
#include <QFileInfoList>


namespace Tao
{

struct Licenses : public QObject
// ----------------------------------------------------------------------------
//   Class checking the available license for a given module ID or function
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    static void AddLicenseFile(kstring lfile)
    {
        return LM().addLicenseFile(lfile);
    }

    static void AddLicenseFiles(const QFileInfoList &files)
    {
        return LM().addLicenseFiles(files);
    }

#ifndef KEYGEN
    static bool Has(text feature)
    {
        return LM().licenseRemainingDays(feature) > 0;
    }

    static int RemainingDays(text feature)
    {
        return LM().licenseRemainingDays(feature);
    }

    static void Warn(text feature, int days, bool critical);

    static bool Check(text feature, bool critical = false)
    {
        int days = RemainingDays(feature);
        if (days <= 7)
            Warn(feature, days, critical);
        if (days <= 0)
            LM().unlicensedCount++;
        return days > 0;
    }

    static bool CheckImpressOrLicense(text feature)
    {
        if (DontCheck())
        {
            debugTraceNotChecked(feature);
            return true;
        }
        else
        {
            return Check(feature, false);
        }
    }

    static bool HasImpressOrLicense(text feature)
    {
        if (DontCheck())
        {
            debugTraceNotChecked(feature);
            return true;
        }
        else
        {
            return Has(feature);
        }
    }

    static bool DontCheck()
    {
        if (TaoApp->edition == Application::DesignPro)
            return true;
        bool sig = false;
#ifndef CFG_NO_DOC_SIGNATURE
        Window *win = TaoApp->window();
        if (win)
            sig = win->isDocumentSigned();
#endif
        return sig;
    }

    static text hostID();

#endif // KEYGEN

    static text Name()
    {
        return LM().name;
    }

    static text Company()
    {
        return LM().company;
    }

    static uint UnlicensedCount()
    {
        return LM().unlicensedCount;
    }
    static float UnlicensedRatio()
    {
        return (float) LM().unlicensedCount / LM().licenses.size();
    }
    static void IncrementUnlicensedCount()
    {
        LM().unlicensedCount++;
    }

private:
    struct License
    {
        QRegExp         features;
        QDate           expiry;
    };
    friend
    std::ostream& operator << (std::ostream &o, const Licenses::License &lic);
    std::vector<License>licenses;
    text                name;
    text                company;
    text                address;
    text                email;
    uint                unlicensedCount;
    struct LicenseFile
    {
        std::vector<License>licenses;
        text                name;
        text                company;
        text                address;
        text                email;
        text                hostid;
    };

private:
    Licenses();
    ~Licenses();
    static Licenses &LM();

    void addLicenseFile(kstring licfname);
    void addLicenseFiles(const QFileInfoList &files);
    int  licenseRemainingDays(text feature);
    void licenseError(kstring file, QString reason);
    text toText(LicenseFile &lf);
#ifdef KEYGEN
    text sign(LicenseFile &lf);
#endif
    bool verify(LicenseFile &lf, text signature);
    static std::ostream & debug();

#ifndef KEYGEN
    static void debugTraceNotChecked(text feature)
    {
        IFTRACE(lic)
        {
            // Cache request for less verbose logging
            static std::set<text> checked;
            if (checked.count(feature) == 0)
            {
                debug() << "'" << feature << "' not checked "
                           "(authorized because edition is Design Pro"
#ifndef CFG_NO_DOC_SIGNATURE
                           " or doc is signed)"
#endif
                           "\n";
                checked.insert(feature);
            }
        }
    }
#endif
};

}

#endif // LICENSE_H
