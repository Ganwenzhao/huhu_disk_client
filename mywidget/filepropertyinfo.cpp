#include "filepropertyinfo.h"
#include "ui_filepropertyinfo.h"

FilePropertyInfo::FilePropertyInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FilePropertyInfo)
{
    ui->setupUi(this);
    //label的url设置为超链接
    ui->label_url->setOpenExternalLinks(true);
}

FilePropertyInfo::~FilePropertyInfo()
{
    delete ui;
}
/**
 * @brief  设置文件属性显示
 *
 * @param file_info 指向文件属性的指针
 * @param
 *
 *
 * @returns
 *
 *
 */
void FilePropertyInfo::set_info(FileInfo *file_info)
{
    //设置显示的文件信息
    ui->label_name->setText(file_info->filename);
    ui->label_user->setText(file_info->user);
    ui->label_time->setText(file_info->time);

    if(file_info->size >= 1024 && file_info->size < 1024*1024){
        ui->label_size->setText(QString("%1KB").arg(file_info->size/1024));
    }else{
        ui->label_size->setText(QString("%1KB").arg(file_info->size/1024/1024));
    }

    if(file_info->shareStatus == 0){
        ui->label_share->setText("未分享");
    }else{
        ui->label_share->setText("已分享");
    }

    ui->label_pv->setText(QString("被下载过 %1 次").arg(file_info->pv));

    //<a href="//www.runoob.com/">菜鸟教程</a></p>
    QString tmp_url = QString("<a href=\"%1\">%2</a>").arg(file_info->url).arg(file_info->url);
    ui->label_url->setText(tmp_url);

}
