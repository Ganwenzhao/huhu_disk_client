#ifndef MYFILES_H
#define MYFILES_H

#include <QWidget>
#include <QTimer>
#include "common/uploadtask.h"
#include "common/common.h"
#include "menu.h"

namespace Ui {
class MyFiles;
}

class MyFiles : public QWidget
{
    Q_OBJECT

public:
    explicit MyFiles(QWidget *parent = 0);
    ~MyFiles();
    //初始化文件列表
    void init_file_list();
    //添加右键菜单
    void add_action_menu();

    //=================上传文件处理================
    //添加需要上传的文件到任务列表
    void add_upload_files();
    //设置md5的json包
    QByteArray set_md5_json(QString user, QString token, QString md5, QString file_name);
    //上传文件处理，先取出上传任务的第一个，传完再传其他的
    void upload_file_action();
    //上传真正的文件内容
    void upload_file(UploadFileInfo* info);

    //=============文件item展示===================
    //清空文件列表
    void clear_file_list();
    //清空所有item
    void clear_items();
    //添加上传文件的item
    void add_upload_items(QString icon_path = ":/images2/upload.png", QString name = "upload files");
    //文件的item展示
    void refresh_file_items();

    //==============显示用户的文件列表==============
    //desc降序，asc升序
    //Normal:普通用户列表 PvAsc:按照下载量升序 PvDesc:按照下载量降序
    enum Display{Normal,PvAsc,PvDesc};
    //获取服务器json文件
    QStringList get_count_status(QByteArray json);
    //显示用户文件列表
    void refresh_files(Display cmd = Normal);
    //设置json包
    QByteArray set_get_count_json(QString user,QString token);
    QByteArray set_file_list_json(QString user,QString token, int start, int count);
    //获取用户文件列表
    void get_user_file_list(Display cmd = Normal);
    //解析文件列表的json信息，放在文件列表中
    void get_file_json_info(QByteArray data);

    //==============分享和删除文件===============
    //处理选中的文件
    void deal_select_files(QString cmd = "分享");
    //设置处理文件的json包
    QByteArray set_deal_file_json(QString user, QString token, QString md5, QString filename);

    //分享文件
    void share_file(FileInfo* info);
    //删除文件
    void dele_file(FileInfo* info);

    //================获取文件属性==============
    void get_file_property(FileInfo* info);

    //================下载文件处理==============
    //添加需要下载的文件到下载任务列表
    void add_download_file();
    //下载文件处理，取出下载任务列表的队首任务，下载完后再进行下一个
    void download_file_action();

    //下载文件标志处理
    void deal_file_pv(QString md5, QString file_name);
    //清除上传和下载任务
    void clear_all_tasks();
    //定时检查处理任务队列中的任务
    void check_task_list();

signals:
    void login_again_signal();
    void goto_transfer(TransferStatus status);
private:
    //右键菜单信号的槽函数
    void right_menu(const QPoint& pos);


private:
    Ui::MyFiles *ui;

    Common m_cn;
    QNetworkAccessManager* m_manager;

    Menu *m_menu;//菜单1
    QAction* m_download_action;//下载
    QAction* m_share_action;//分享
    QAction* m_dele_action;//删除
    QAction* m_property_action;//属性


    Menu *m_menu_empty;//菜单2
    QAction* m_pvasc_action;//下载升序
    QAction* m_pvdesc_action;//下载降序
    QAction* m_refresh_action;//刷新
    QAction* m_upload_action;//上传

    int m_user_file_count;//用户文件数目
    int m_start;//文件位置起点
    int m_count;//每次请求文件的个数

    QTimer m_upload_timer;//定时检查上传队列
    QTimer m_download_timer;//定时检查下载队列

    QList<FileInfo*> m_file_list;//文件列表

};

#endif // MYFILES_H
