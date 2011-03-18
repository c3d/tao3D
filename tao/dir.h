#ifndef DIR_H
#define DIR_H
// ****************************************************************************
//  dir.h                                                          Tao project
// ****************************************************************************
//
//   File Description:
//
//    This is an extension of QDir with support for "recursive globbing":
//    the entryList method supports patterns such as "images/*/*.[jJ][pP][gG]".
//
//
//
//
//
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao.h"
#include <QDir>

TAO_BEGIN

class Dir : public QDir
{
public:
    Dir(const QString &path) : QDir(path) {}
    QFileInfoList entryInfoGlobList(QString pattern);
};

TAO_END

#endif // DIR_H
