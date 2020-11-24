#include "trans.h"
#include "ui_trans.h"

Trans::Trans(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Trans)
{
    ui->setupUi(this);
}

Trans::~Trans()
{
    delete ui;
}
