// ****************************************************************************
//  application.h                                                  Tao project
// ****************************************************************************
//
//   File Description:
//
//    The Tao application
//
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

#include <QString>
#include <QSettings>
#include <QInputDialog>
#include <QMessageBox>
#include <QLineEdit>
#include <QDir>
#include "application.h"
#include "tao.h"

TAO_BEGIN

void TaoApplication::OpenLibrary()
{
    bool ok = false;
    QString path = QSettings().value("defaultlibrary").toString();
    if (path.isNull())
        path = QInputDialog::getText(NULL, tr("Welcome to Tao!"),
                                     tr("Please choose a folder where your "
                                        "Tao documents will be stored:"),
                                     QLineEdit::Normal,
                                     QDir::homePath() + tr("/Tao Library.tao"),
                                     &ok);
    if (path.isNull())
        return;
    docLibrary.curPath = path;
    if (!docLibrary.Open())
    {
        QMessageBox::information(0, tr("Tao Library"),
                                 tr("Failed to open Tao library"));
        return;
    }
    if (ok)
        QSettings().setValue("defaultlibrary", path);
}

TAO_END
