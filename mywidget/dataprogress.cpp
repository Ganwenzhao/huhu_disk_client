#include "dataprogress.h"
#include "ui_dataprogress.h"

DataProgress::DataProgress(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataProgress)
{
    ui->setupUi(this);
}

DataProgress::~DataProgress()
{
    delete ui;
}
/**
 * @brief  设置进度条显示的文件名
 *
 * @param name 	要显示的文件名
 *
 *
 *
 * @returns
 *
 *
 */
void DataProgress::set_filename(QString name)
{
    ui->label->setText(name + " : ");
    ui->progressBar->setMinimum(0);
    ui->progressBar->setValue(0);
}

/**
 * @brief  设置进度条的进度
 *
 * @param name 	要显示的文件名
 *
 *
 *
 * @returns
 *
 */
void DataProgress::set_progress(qint64 val, qint64 end)
{
    ui->progressBar->setValue(val);
    ui->progressBar->setMaximum(end);
}

