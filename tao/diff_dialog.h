#ifndef DIFF_DIALOG_H
#define DIFF_DIALOG_H
// ****************************************************************************
//  diff_dialog .cpp                                               Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "ui_diff_dialog.h"

namespace Tao {

class Repository;
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
