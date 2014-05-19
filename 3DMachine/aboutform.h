#ifndef ABOUTFORM_H
#define ABOUTFORM_H

#include <QWidget>

namespace Ui {
class aboutForm;
}

class aboutForm : public QWidget
{
    Q_OBJECT

public:
    explicit aboutForm(QWidget *parent = 0);
    ~aboutForm();

private slots:
    void on_pushButton_pressed();

private:
    Ui::aboutForm *ui;
};

#endif // ABOUTFORM_H
