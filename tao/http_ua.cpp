// ****************************************************************************
//  http_ua.h                                                      Tao project
// ****************************************************************************
//
//   File Description:
//
//     Base class for HTTP user agents.
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
//  (C) 2013 Taodyne SAS
// ****************************************************************************
#include "application.h"
#include "http_ua.h"
#include <process.h>
#include "version.h"
#include <QRegExp>


namespace Tao {


HttpUserAgent::HttpUserAgent()
// ----------------------------------------------------------------------------
//    Constructor
// ----------------------------------------------------------------------------
{
#ifdef TAO_EDITION
   // Get current edition
   edition = TAO_EDITION;
#else
#ifdef TAO_PLAYER
    edition = "player";
#else
    edition = "unified";
#endif
#endif

#if defined(Q_OS_MACX)
   target = "MacOSX";
#elif defined(Q_OS_WIN)
   target = "Windows";
#else
   // Check if we are on Debian or Ubuntu distribution to get .deb package
   QString cmd("uname");
   QStringList args;
   args << "-a";
   Process cp(cmd, args);
   text errors, output;
   if(cp.done(&errors, &output))
   {
       // Check OS name
       if(output.find("Ubuntu") != output.npos ||
          output.find("Debian") != output.npos)
           target = "Linux .deb";
       else
           target = "Linux .tar.bz2";

       // Check architecture
       if(output.find("x86_64") != output.npos)
           target += " x86_64";
       else
           target += " x86";
   }
#endif

   // Get current version of Tao
   QString ver = GITREV;
   QRegExp rxp("([0-9\\.]+)");
   rxp.indexIn(ver);
   version = rxp.cap(1).toDouble();
}


QString HttpUserAgent::userAgent()
// ----------------------------------------------------------------------------
//    HTTP User-Agent string
// ----------------------------------------------------------------------------
{
    QString ua("Tao Presentations/%1 (%2; %3; %4)");
    return ua.arg(version).arg(Application::editionStr()).arg(edition)
             .arg(target);
}

}
