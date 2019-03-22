// *****************************************************************************
// http_ua.cpp                                                     Tao3D project
// *****************************************************************************
//
// File description:
//
//     Base class for HTTP user agents.
//
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2013, Jérôme Forissier <jerome@taodyne.com>
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

#include "http_ua.h"
#include <QRegExp>
#include "application.h"
#include "tao_process.h"

namespace Tao {

extern const char * GITREV_;


HttpUserAgent::HttpUserAgent()
// ----------------------------------------------------------------------------
//    Constructor
// ----------------------------------------------------------------------------
{
#ifdef TAO_PLAYER
    edition = "player";
#else
    edition = "unified";
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
   QString ver = GITREV_;
   QRegExp rxp("([0-9\\.]+)");
   rxp.indexIn(ver);
   version = rxp.cap(1).toDouble();
}


QString HttpUserAgent::userAgent()
// ----------------------------------------------------------------------------
//    HTTP User-Agent string
// ----------------------------------------------------------------------------
{
    QString ua("Tao3D/%1 (%2; %3; %4)");
    return ua.arg(version).arg(Application::editionStr()).arg(edition)
             .arg(target);
}

}
