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
#include "QObject"
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
