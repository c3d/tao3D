#ifndef DIFF_DIALOG_H
#define DIFF_DIALOG_H
// *****************************************************************************
// diff_dialog.h                                                   Tao3D project
// *****************************************************************************
//
// File description:
//
//    The class to display the Diff dialog box. This is a modeless dialog
//    which enables to pick two commits from any branch and show the textual
//    difference between the two versions.
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2014-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
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

#include "ui_diff_dialog.h"

namespace Tao {

struct Repository;
class DiffHighlighter;

class DiffDialog : public QDialog, protected Ui::DiffDialog
{
    Q_OBJECT

public:
    DiffDialog(Repository *repo, QWidget *parent = 0);

private slots:
    void          diff();

private:
    void          setupEditor();

protected:
    Repository      * repo;
    DiffHighlighter * highlighter;

};

}

#endif // DIFF_DIALOG_H
