// ****************************************************************************
//  login_dialog.h                                                 Tao project
// ****************************************************************************
//
//   File Description:
//
//    A dialog box that allows the user to enter his username and password.
//    Adapted from http://qt-project.org/wiki/UserLoginDialogExample
//    License: CC BY-SA 2.5 (http://creativecommons.org/licenses/by-sa/2.5/)
//
//
//
//
//
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2013 Jerome Forissier <jerome@taodyne.com>
//  (C) 2013 Taodyne SAS
// ****************************************************************************

#include "login_dialog.h"

namespace Tao {

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
{
    setUpGUI();
    setModal(true);
}

void LoginDialog::setUpGUI()
{
    QGridLayout* formGridLayout = new QGridLayout(this);

    comboUsername = new QComboBox(this);
    comboUsername->setEditable(true);

    editPassword = new QLineEdit(this);
    editPassword->setEchoMode(QLineEdit::Password);

    labelDescription = new QLabel(this);
    labelUsername = new QLabel(this);
    labelPassword = new QLabel(this);
    labelUsername->setText(tr("Username"));
    labelUsername->setBuddy(comboUsername);
    labelPassword->setText(tr("Password"));
    labelPassword->setBuddy(editPassword);

    buttons = new QDialogButtonBox(this);
    buttons->addButton(QDialogButtonBox::Ok);
    buttons->addButton(QDialogButtonBox::Cancel);

    connect(buttons->button(QDialogButtonBox::Cancel),
            SIGNAL(clicked()),
            this, SLOT(close()));

    connect(buttons->button(QDialogButtonBox::Ok),
            SIGNAL(clicked()),
            this, SLOT(slotAcceptLogin()));

    formGridLayout->addWidget(labelDescription, 0, 0, 1, 3);
    formGridLayout->addWidget(labelUsername, 1, 0);
    formGridLayout->addWidget(comboUsername, 1, 1);
    formGridLayout->addWidget(labelPassword, 2, 0);
    formGridLayout->addWidget(editPassword, 2, 1);
    formGridLayout->addWidget(buttons, 3, 0, 1, 2);

    setLayout(formGridLayout);
}


void LoginDialog::setDescription(QString description)
{
    labelDescription->setText(description);
}


void LoginDialog::setUsername(QString username)
{
    bool found = false;
    for (int i = 0; i < comboUsername->count() && ! found ; i++)
    {
        if (comboUsername->itemText(i) == username)
        {
            comboUsername->setCurrentIndex(i);
            found = true;
        }
    }
    if (!found)
    {
        int index = comboUsername->count();
        comboUsername->addItem(username);
        comboUsername->setCurrentIndex(index);
    }

    if (username.isEmpty())
        comboUsername->setFocus();
    else
        editPassword->setFocus();
}


void LoginDialog::setPassword(QString password)
{
    editPassword->setText(password);
}


void LoginDialog::slotAcceptLogin()
{
    QString username = comboUsername->currentText();
    QString password = editPassword->text();
    int     index    = comboUsername->currentIndex();

    emit acceptLogin(username, password, index);

    close();
}

void LoginDialog::setUsernamesList(const QStringList &usernames)
{
    comboUsername->addItems(usernames);
}

}
