#ifndef ANSI_TEXTEDIT_H
#define ANSI_TEXTEDIT_H
// ****************************************************************************
//  ansi_textedit.h                                                Tao project
// ****************************************************************************
//
//   File Description:
//
//     A QTextEdit widget that doesn't mind receiving (some) ANSI escape
//     sequences.
//
//
//
//
//
//
//
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include <QTextEdit>
#include <QByteArray>

namespace Tao {

class AnsiTextEdit : public QTextEdit
// ----------------------------------------------------------------------------
//   QTextEdit with ANSI escape support
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    AnsiTextEdit(QWidget *parent = 0);

public slots:
    virtual void insertAnsiText(const QByteArray &text);

private:
    void flush();
    void killEndOfLine();

private:
    enum Mode
    {
        M_Normal,          // Currently reading 'normal' characters
        M_Escape,          // Just read an escape character (0x1B)
        M_Escape_LBracket, // Read a left bracket after an escape
    };

private:
    Mode    mode;
    QString esc;
    QString str;
};

}

#endif // ANSI_TEXTEDIT_H
