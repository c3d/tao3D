// *****************************************************************************
// assistant.h                                                     Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2013-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011-2013, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
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


#ifndef ASSISTANT_H
#define ASSISTANT_H

#include "tao.h"
#include <QObject>
#include <QString>
#include <QStringList>

QT_BEGIN_NAMESPACE
class QProcess;
QT_END_NAMESPACE

namespace Tao {

struct Assistant : QObject
// ----------------------------------------------------------------------------
//    Help viewer assistant
// ----------------------------------------------------------------------------
{
private:
    Q_OBJECT

public:
    Assistant(QWidget *parent = 0);
    ~Assistant();

    void           showDocumentation(const QString &file);

public:
    static Assistant * instance();
    static void    showKeywordHelp(const QString keyword);
    static bool    installed();

private slots:
    void           readStandardOutput();
    void           readStandardError();

private:
    void           registerQchFiles(QStringList files);
    QString        taoCollectionFilePath();
    QString        userCollectionFilePath();
    QString        userCollectionFile();
    QStringList    collectionFileArgs();
    QStringList    registeredFiles(QString collectionFile);
    QStringList    registeredNamespaces(QString collectionFile);
    QStringList    stringListDifference(const QStringList &a,
                                        const QStringList &b);
    bool           registerDocumentation(const QStringList &files,
                                         const QString &collectionFile);
    bool           unregisterDocumentation(const QStringList &files,
                                           const QString &collectionFile);
    bool           unregisterDocumentationNS(const QStringList &namespaces,
                                             const QString &collectionFile);
    bool           startAssistant();

    static std::ostream & debug();
    static QString        assistantPath();

    bool           registered;
    QProcess *     proc;
    QWidget *      parent;
    QString        warnTitle;

private:
    static Assistant *  assistant;
};

}

#endif
