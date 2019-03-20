#ifndef LICENSE_H
#define LICENSE_H
// ****************************************************************************
//  license.h                                                       Tao project
// ****************************************************************************
//
//   File Description:
//
//     License check for Tao3D
//
//     Sources for information:
//     - http://www.sentientfood.com/display_story.php?articleid=3
//     - http://sigpipe.macromates.com/2004/09/05/using-openssl-for-license-keys
//
//
//
//
// ****************************************************************************
// This software is licensed under the GNU General Public License v3.
// See file COPYING for details.
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "base.h"
#ifndef KEYGEN
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

#ifndef CFG_NO_LICENSE

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

    static text Mail()
    {
        return LM().email;
    }
    static bool PublisherMatches(QString name)
    {
        return LM().publisherMatches(name);
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
    std::vector<QRegExp>publishers;
    uint                unlicensedCount;
    struct LicenseFile
    {
        std::vector<License>licenses;
        text                name;
        text                company;
        text                address;
        text                email;
        std::vector<QRegExp>publishers;
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
    bool publisherMatches(QString name);
    static std::ostream & debug();
#endif // CFG_NO_LICENSE
};

}


#endif // LICENSE_H
