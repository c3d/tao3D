#ifndef HTTP_UA_H
#define HTTP_UA_H
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
// This software is licensed under the GNU General Public License v3.
// See file COPYING for details.
//  (C) 2013 Jerome Forissier <jerome@taodyne.com>
//  (C) 2013 Taodyne SAS
// ****************************************************************************
#include <QString>


namespace Tao {


class HttpUserAgent
// ------------------------------------------------------------------------
//   Base class for HTTP user agents
// ------------------------------------------------------------------------
{
public:
    HttpUserAgent();
    ~HttpUserAgent() {}

    QString                  userAgent();

protected:
    // Tao info
    double                   version;
    QString                  edition;
    QString                  target;

};

}

#endif // HTTP_UA_H
