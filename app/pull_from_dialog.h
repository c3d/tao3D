#ifndef PULL_FROM_DIALOG_H
#define PULL_FROM_DIALOG_H
// *****************************************************************************
// pull_from_dialog.h                                              Tao3D project
// *****************************************************************************
//
// File description:
//
//    The class to display the "Pull from" dialog box
//
//
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
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

#include "remote_selection_frame.h"
#include "ui_pull_from_dialog.h"
#include "repository.h"
#include <QDialog>
#include <QWidget>

namespace Tao {

struct Repository;

class PullFromDialog : public QDialog, private Ui::PullFromDialog
{
    Q_OBJECT

public:
    PullFromDialog(Repository *repo, QWidget *parent = 0);

public:
    QString                        pullFrom();
    Repository::ConflictResolution conflictResolution();
    int                            pullInterval();

public slots:
    virtual void accept();

private:
    Repository           *repo;
};

}

#endif // PULL_FROM_DIALOG_H
