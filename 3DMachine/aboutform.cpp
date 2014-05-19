#include "aboutform.h"
#include "ui_aboutform.h"

aboutForm::aboutForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::aboutForm)
{
    ui->setupUi(this);
}

aboutForm::~aboutForm()
{
    delete ui;
}

void aboutForm::on_pushButton_pressed()
{
    close();
}
