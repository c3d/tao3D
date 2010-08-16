#include "ssh_ask_pass_dialog.h"
#include "ui_ssh_ask_pass_dialog.h"
#include <iostream>

SshAskPassDialog::SshAskPassDialog(QString text, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SshAskPassDialog)
{
    ui->setupUi(this);
    ui->label->setText(text);
    connect(ui->buttonBox, SIGNAL(rejected()),
            this, SLOT(reject()));
    connect(ui->buttonBox, SIGNAL(accepted()),
            this, SLOT(accept()));
}


SshAskPassDialog::~SshAskPassDialog()
{
    delete ui;
}


void SshAskPassDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


void SshAskPassDialog::accept()
{
    std::cout << ui->lineEdit->text().toStdString() << "\n";
    QDialog::accept();
}
