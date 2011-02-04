#include "inspectordialog.h"
#include "ui_inspectordialog.h"

InspectorDialog::InspectorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InspectorDialog)
{
    ui->setupUi(this);
}

InspectorDialog::~InspectorDialog()
{
    delete ui;
}

InspectorDialog::addProperty(){}
InspectorDialog::addProperties(){}
