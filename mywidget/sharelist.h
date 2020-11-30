#ifndef SHARELIST_H
#define SHARELIST_H

#include <QWidget>
#include <QTimer>
#include "common/common.h"
#include "mywidget/menu.h"
namespace Ui {
class ShareList;
}

class ShareList : public QWidget
{
    Q_OBJECT

public:
    explicit ShareList(QWidget *parent = 0);
    ~ShareList();

    //初始化共享文件列表
    void init_sharelist_listwidget();
    //右键菜单信号的槽函数
    void right_menu(const QPoint &pos);
    //添加右键菜单
    void add_action_menu();

    //清空文件列表
    void clear_sharefile_list();
    //清空所有item
    void clear_all_items();
    //文件item展示
    void refresh_file_items();
    //显示共享文件列表
    void refresh_files();
    //设置共享文件列表json
    QByteArray set_filelist_json(int start, int count);
    //获取共享文件列表
    void get_user_filelist();
    //解析文件列表json信息，存放在文件列表之中
    void get_file_json_info(QByteArray data);

    //添加需要下载的文件到下载任务列表
    void add_download_file();
    //下载文件处理
    void download_files_action();
    //设置共享文件json包
    QByteArray set_share_file_json(QString user, QString md5, QString filename);

    //下载文件pv字段处理
    void deal_file_pv(QString md5, QString filename);
    //property：属性；cancel：取消分享，save转存文件
    enum CMD{Property, Cancel, Save};
    //处理选中的文件
    void deal_selected_file(CMD cmd = Property);
    //获取文件属性
    void get_file_property(FileInfo* info);
    //取消已经分享的文件
    void cancel_share_files(FileInfo* info);
    //转存文件
    void save_file_to_mylist(FileInfo* info);

signals:
    void goto_transfer(TransferStatus status);



private:
    Ui::ShareList *ui;

    Common m_cn;
    QNetworkAccessManager* m_manager;

    Menu *m_menu;//菜单1
    QAction* m_download_action;//下载
    QAction* m_save_action;//转存
    QAction* m_cancel_action;//删除
    QAction* m_property_action;//属性


    Menu *m_menu_empty;//菜单2
    QAction* m_refresh_action;//刷新


    int m_user_sharefile_count;//用户文件数目
    int m_start;//文件位置起点
    int m_count;//每次请求文件的个数

    QTimer m_download_timer;//定时检查下载队列

    QList<FileInfo*> m_sharefile_list;//共享文件列表
};

#endif // SHARELIST_H
