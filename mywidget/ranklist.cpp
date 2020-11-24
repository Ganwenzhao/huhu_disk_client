#include "ranklist.h"
#include "ui_ranklist.h"

RankList::RankList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RankList)
{
    ui->setupUi(this);
}

RankList::~RankList()
{
    delete ui;
}
