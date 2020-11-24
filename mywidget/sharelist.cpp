#include "sharelist.h"
#include "ui_sharelist.h"

ShareList::ShareList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShareList)
{
    ui->setupUi(this);
}

ShareList::~ShareList()
{
    delete ui;
}
