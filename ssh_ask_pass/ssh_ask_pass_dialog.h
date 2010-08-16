#ifndef SSH_ASK_PASS_DIALOG_H
#define SSH_ASK_PASS_DIALOG_H

#include <QDialog>

namespace Ui {
    class SshAskPassDialog;
}

class SshAskPassDialog : public QDialog {
    Q_OBJECT
public:
    SshAskPassDialog(QString text, QWidget *parent = 0);
    ~SshAskPassDialog();

    void setText(QString text);

protected:
    void changeEvent(QEvent *e);

protected slots:
    void accept();

private:
    Ui::SshAskPassDialog *ui;
};

#endif // SSH_ASK_PASS_DIALOG_H
