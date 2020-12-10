#include "trans.h"
#include "ui_trans.h"
#include "common/downloadlayout.h"
#include "common/logininfoinstance.h"
#include "common/uploadlayout.h"
#include <QFile>


Trans::Trans(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Trans)
{
    ui->setupUi(this);

    //设置上传布局实例
    UploadLayout* uploadlayout = UploadLayout::get_instance();
    uploadlayout->set_upload_layout(ui->upload_scroll);
    //设置下载布局实例
    DownloadLayout* downloadlayout = DownloadLayout::get_instance();
    //downloadlayout->set_download_layout(ui->download_scroll);

    ui->tabWidget->setCurrentIndex(0);

    connect(ui->tabWidget, &QTabWidget::currentChanged,[=](int index){
        if(index == 0){
            emit current_tab_signal("正在上传");
        }else if(index == 1){
            emit current_tab_signal("正在下载");
        }else if(index == 2){
            emit current_tab_signal("传输记录");
            //show_data_record();
        }

    });
    //设置tab样式
    /*
    ui->tabWidget->tabBar()->setStyleSheet(

    );
    */
    //清空记录
    connect(ui->btn_clear, &QToolButton::clicked,[=](){
        LoginInfoInstance* login = LoginInfoInstance::get_instance();

        QString filename = RECORDDIR + login->get_user();
        if(QFile::exists(filename)){//如果文件存在
            QFile::remove(filename);//删除文件
            ui->record_text->clear();
        }
    });

}

Trans::~Trans()
{
    delete ui;
}


/**
* @brief  显示数据传输记录
*
* @param path 文件路径
*
*
*
* @returns
*
*
*/
void Trans::show_data_record(QString path)
{
    LoginInfoInstance* login = LoginInfoInstance::get_instance();

    QString filename = path + login->get_user();
    QFile file(filename);

    if(false == file.open(QIODevice::ReadOnly)){
        cout<<"file.open(QIODevice::ReadOnly) error!"<<endl;
        return;
    }

    QByteArray array = file.readAll();

#ifdef _WIN32
    //本地字符集转utf-8
    ui->record_text->setText(QString::fromLocal8Bit(array));
#else
    ui->record_text->setText(array);
#endif
    file.close();

}

/**
* @brief  显示上传tab
*
* @param
*
*
*
* @returns
*
*
*/
void Trans::show_upload()
{
    ui->tabWidget->setCurrentIndex(0);
}

/**
* @brief  显示下载tab
*
* @param
*
*
*
* @returns
*
*
*/
void Trans::show_download()
{
    ui->tabWidget->setCurrentIndex(1);
}

