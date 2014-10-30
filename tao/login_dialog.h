#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H
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
// This software is licensed under the GNU General Public License v3.
// See file COPYING for details.
//  (C) 2013 Jerome Forissier <jerome@taodyne.com>
//  (C) 2013 Taodyne SAS
// ****************************************************************************

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QComboBox>
#include <QGridLayout>
#include <QStringList>

namespace Tao {

class LoginDialog : public QDialog
{
    Q_OBJECT

private:
    QLabel *            labelDescription;
    QLabel *            labelUsername;
    QLabel *            labelPassword;

    QComboBox *         comboUsername;
    QLineEdit *         editPassword;
    QDialogButtonBox *  buttons;

    void setUpGUI();

public:
    LoginDialog(QWidget *parent = 0);

    void setDescription(QString description);
    void setUsername(QString username);
    void setPassword(QString password);
    void setUsernamesList(const QStringList &usernames);

signals:
    void acceptLogin(QString &username, QString &password, int &index);

public slots:
    void slotAcceptLogin();
};

}

#endif // LOGIN_DIALOG_H
