#ifndef LICENSE_H
#define LICENSE_H
// *****************************************************************************
// license.h                                                       Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2011, Catherine Burvelle <catherine@taodyne.com>
// (C) 2011-2012,2014-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011-2013, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
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
