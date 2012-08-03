#ifndef LICENCE_H
#define LICENCE_H
// ****************************************************************************
//  licence.h                                                       Tao project
// ****************************************************************************
//
//   File Description:
//
//     Licence check for Tao Presentation
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
#ifdef TAO
#include "application.h"
#include <set>
#endif
#endif

#include <vector>
#include <QRegExp>
#include <QDateTime>
#include <QFileInfoList>


namespace Tao
{

struct Licences : public QObject
// ----------------------------------------------------------------------------
//   Class checking the available licence for a given module ID or function
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    static void AddLicenceFile(kstring lfile)
    {
        return LM().addLicenceFile(lfile);
    }

    static void AddLicenceFiles(const QFileInfoList &files)
    {
        return LM().addLicenceFiles(files);
    }

#ifndef KEYGEN
    static bool Has(text feature)
    {
        return LM().licenceRemainingDays(feature) > 0;
    }

    static int RemainingDays(text feature)
    {
        return LM().licenceRemainingDays(feature);
    }

    static void Warn(text feature, int days, bool critical);

    static bool Check(text feature, bool critical = false)
    {
        int days = RemainingDays(feature);
        if (days <= 7)
            Warn(feature, days, critical);
        return days > 0;
    }

    static bool CheckImpressOrLicense(text feature)
    {
#ifdef TAO
        if (Application::isImpress())
        {
            IFTRACE(lic)
            {
                // Cache request for less verbose logging
                static std::set<text> checked;
                if (checked.count(feature) == 0)
                {
                    debug() << "'" << feature << "' not checked "
                               "(authorized because edition is Impress)\n";
                    checked.insert(feature);
                }
            }
            return true;
        }
        else
#endif
        {
            return Check(feature, false);
        }
    }

    static text hostID();

#endif

    static text Name()
    {
        return LM().name;
    }

    static text Company()
    {
        return LM().company;
    }

private:
    struct Licence
    {
        QRegExp         features;
        QDate           expiry;
    };
    friend
    std::ostream& operator << (std::ostream &o, const Licences::Licence &lic);
    std::vector<Licence>licences;
    text                name;
    text                company;
    text                address;
    text                email;
    struct LicenceFile
    {
        std::vector<Licence>licences;
        text                name;
        text                company;
        text                address;
        text                email;
        text                hostid;
    };

private:
    Licences();
    ~Licences();
    static Licences &LM();

    void addLicenceFile(kstring licfname);
    void addLicenceFiles(const QFileInfoList &files);
    int  licenceRemainingDays(text feature);
    void licenceError(kstring file, QString reason);
    text toText(LicenceFile &lf);
#ifdef KEYGEN
    text sign(LicenceFile &lf);
#endif
    bool verify(LicenceFile &lf, text signature);
    static std::ostream & debug();
};

}

#endif // LICENCE_H
