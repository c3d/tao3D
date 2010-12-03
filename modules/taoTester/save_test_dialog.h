#ifndef SAVE_TEST_DIALOG_H
#define SAVE_TEST_DIALOG_H

#include <QDialog>

namespace Ui {
    class Save_test_dialog;
}


class Save_test_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Save_test_dialog(QWidget *parent = 0,
                              QString name = "",
                              QString loc = "",
                              int fid = 0,
                              QString desc = "",
                              double threshold = 0.0);
    ~Save_test_dialog();

    QString name;
    QString loc;
    int fid;
    QString desc;
    double threshold;

public slots:
    void changeLoc();
    void accept();
//    void reject();

private:
    Ui::Save_test_dialog *ui;
};


#endif // SAVE_TEST_DIALOG_H
