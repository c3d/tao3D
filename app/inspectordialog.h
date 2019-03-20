#ifndef INSPECTORDIALOG_H
#define INSPECTORDIALOG_H

#include <QDialog>

namespace Ui {
    class InspectorDialog;
}

class InspectorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InspectorDialog(QWidget *parent = 0);
    ~InspectorDialog();

    void addProperty();
    void addProperties();

private:
    Ui::InspectorDialog *ui;
};

#endif // INSPECTORDIALOG_H
