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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "base.h"

#include <vector>
#include <QRegExp>
#include <QDateTime>

// Features id for CheckOnce
#define NO_FEATURE 0x00000000
#define WEB        0x00000001
#define GUI        0x00000002

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

#ifndef KEYGEN
    static bool Has(text feature)
    {
        return LM().licenceRemainingDays(feature) > 0;
    }

    static int RemainingDays(text feature)
    {
        return LM().licenceRemainingDays(feature);
    }

    static void WarnUnlicenced(text feature, int days, bool critical);

    static bool Check(text feature, bool critical = false)
    {
        int days = RemainingDays(feature);
        if (days <= 0)
            WarnUnlicenced(feature, days, critical);
        return days > 0;
    }

    static bool CheckOnce(quint64 feature, bool silent = true,
                          bool critical = false);

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
    std::vector<Licence>licences;
    text                name;
    text                company;
    text                address;
    text                email;

    // Used for CheckOnce : store if the feature has already been checked and
    // the result of the check.
    static quint64 alreadyChecked;
    static quint64 licencesOK;
    static std::map<quint64, text> featuresName;

private:
    Licences();
    ~Licences();
    static Licences &LM();

    void addLicenceFile(kstring licfname);
    int  licenceRemainingDays(text feature);
    void licenceError(kstring file, QString reason);
    text toText(std::vector<Licence> &licences);
#ifdef KEYGEN
    text sign(std::vector<Licence> &licences);
#endif
    bool verify(std::vector<Licence> &licences, text signature);
    std::ostream & debug();
};

}

#endif // LICENCE_H
