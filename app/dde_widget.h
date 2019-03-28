#ifndef DDE_WIDGET_H
#define DDE_WIDGET_H
// *****************************************************************************
// dde_widget.h                                                    Tao3D project
// *****************************************************************************
//
// File description:
//
//   Handling of Windows DDE events, to open .ddd file or a tao: URI.
//   Note: On MacOSX Application::event() receives a QFileOpenEvent.
//   Unfortunately, Qt does not yet support this on Windows. By creating an
//   instance of this class, we have an opportunity to receive DDE messages
//   through the windEvent() virtual method.
//   Windows DDE handling code adapted from:
//   http://developer.qt.nokia.com/wiki/
//          Assigning_a_file_type_to_an_Application_on_Windows
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2011, Catherine Burvelle <catherine@taodyne.com>
// (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011, Jérôme Forissier <jerome@taodyne.com>
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


#include <QWidget>
#include <windows.h>

namespace Tao {

class DDEWidget : public QWidget
{
    Q_OBJECT

public:
    DDEWidget();
    ~DDEWidget();

protected:
#if QT_VERSION < 0x050000
    virtual bool winEvent(MSG *message, long *result);
#else
    virtual bool nativeEvent(const QByteArray &event, void *msg, long *result);
#endif
    virtual void ddeOpenFile(const QString& filePath);

private:
    bool ddeInitiate(MSG* message, long* result);
    bool ddeExecute(MSG* message, long* result);
    bool ddeTerminate(MSG* message, long* result);
    void executeDdeCommand(const QString& command, const QString& params);

    QString appAtomName;
    QString systemTopicAtomName;
    ATOM    appAtom;
    ATOM    systemTopicAtom;

};

}

#endif
