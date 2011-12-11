// ****************************************************************************
//  assistant.h                                                    Tao project
// ****************************************************************************
//
//   File Description:
//
//     Show online documentation using the Qt Assistant application.
//
//     Inspired from the Qt documentation: Simple Text Viewer Example
//
//
//
//
//
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2011 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2011 Taodyne SAS
// ****************************************************************************


#ifndef ASSISTANT_H
#define ASSISTANT_H

#include "tao.h"
#include <QString>
#include <QStringList>

QT_BEGIN_NAMESPACE
class QProcess;
QT_END_NAMESPACE

TAO_BEGIN

class Assistant
{
public:
    Assistant();
    ~Assistant();

    void           showDocumentation(const QString &file);
    void           registerQchFiles(QStringList files);

private:
    QString        appPath();
    QStringList    collectionFileArgs();
    bool           startAssistant();

    static std::ostream & debug();

    QProcess *     proc;
};

TAO_END

#endif
