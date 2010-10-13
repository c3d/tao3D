#include "save_test_dialog.h"
#include "ui_save_test_dialog.h"
#include <QFileDialog>

namespace Tao {

Save_test_dialog::Save_test_dialog(QWidget *parent,
                                   QString name, QString loc,
                                   int fid, QString desc) :
    QDialog(parent), name(name), loc(loc), fid(fid), desc(desc),
    ui(new Ui::Save_test_dialog)
{
    ui->setupUi(this);
    ui->nameEdit->setText(name);
    ui->locEdit->setText(loc);
    ui->fidEdit->setText(QString("%1").arg(fid));
    ui->descEdit->setDocument(new QTextDocument(desc, this));
    connect(ui->locChange, SIGNAL(clicked()), this, SLOT(changeLoc()));
}

Save_test_dialog::~Save_test_dialog()
{
    delete ui;
}

void Save_test_dialog::changeLoc()
{

    QString dir =
            QFileDialog::getExistingDirectory(this,
                                              tr("Change test location"),
                                              loc);
    ui->locEdit->setText(dir);
}

void Save_test_dialog::accept()
{
    name = ui->nameEdit->text();
    loc = ui->locEdit->text();
    fid = ui->fidEdit->text().toInt();
    desc = ui->descEdit->toPlainText();
    QDialog::accept();
}

//void Save_test_dialog::reject()
//{
//    QDialog::reject();
//}
} // End namespace Tao
