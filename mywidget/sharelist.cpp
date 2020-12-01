#include "sharelist.h"
#include "ui_sharelist.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QMessageBox>
#include <QFileDialog>
#include "common/logininfoinstance.h"
#include "mywidget/filepropertyinfo.h"
#include "common/downloadtask.h"

ShareList::ShareList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShareList)
{
    ui->setupUi(this);

    init_sharelist_listwidget();

    add_action_menu();

    m_manager = Common::getNetManager();

    //定时检查下载队列
    connect(&m_download_timer,&QTimer::timeout,[=](){
        download_files_action();
    });
    //500ms检测一次
    m_download_timer.start(500);

}

ShareList::~ShareList()
{
    clear_sharefile_list();
    clear_all_items();
    delete ui;
}

/**
 * @brief  初始化共享文件显示列表
 *
 * @param
 *
 *
 *
 * @returns
 *
 *
 */
void ShareList::init_sharelist_listwidget()
{
    //设置显示模式
    ui->listWidget->setViewMode(QListView::IconMode);
    //设置图标大小
    ui->listWidget->setIconSize(QSize(80,80));
    //设置item大小
    ui->listWidget->setGridSize(QSize(100,120));

    //可以设置图标大小随QListWidget大小自动调整
    ui->listWidget->setResizeMode(QListWidget::Adjust);
    //设置列表无法拖动
    ui->listWidget->setMovement(QListView::Static);
    //ListWidget右键菜单，发出信号
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->listWidget, &QListView::customContextMenuRequested,
            this,&ShareList::right_menu);

    //点击列表中的上传图标
    connect(ui->listWidget,&QListWidget::itemPressed,
            this,[=](QListWidgetItem* item){

    });
}

/**
* @brief  右键菜单信号的槽函数
*
* @param pos 位置
*
*
*
* @returns
*
*
*/
void ShareList::right_menu(const QPoint &pos)
{
    QListWidgetItem* item = ui->listWidget->itemAt(pos);
    //没有点图标
    if(item == nullptr){
        //在点击的全局坐标显示菜单
        m_menu_empty->exec(QCursor::pos());
    }
    //选中了item
    else{
        ui->listWidget->setCurrentItem(item);
        m_menu->exec(QCursor::pos());
    }

}

/**
 * @brief  添加右键菜单
 *
 * @param
 *
 *
 *
 * @returns
 *
 *
 */
void ShareList::add_action_menu()
{
    //==============菜单1=============
    m_menu = new Menu(this);

    //初始化菜单1
    m_download_action = new QAction("下载",this);
    m_cancel_action = new QAction("取消分享",this);
    m_save_action = new QAction("转存文件",this);
    m_property_action = new QAction("属性",this);

    //动作1添加到菜单1
    m_menu->addAction(m_download_action);
    m_menu->addAction(m_cancel_action);
    m_menu->addAction(m_save_action);
    m_menu->addAction(m_property_action);

    //==============菜单2=============
    m_menu_empty = new Menu(this);

    //初始化菜单2
    m_refresh_action = new QAction("刷新文件列表",this);

    //动作1添加到菜单2
    m_menu->addAction(m_refresh_action);

    //====================信号槽============
    //下载
    connect(m_download_action,&QAction::triggered,[=](){
        add_download_file();
    });
    //取消分享
    connect(m_cancel_action,&QAction::triggered,[=](){
        //处理选中的文件
        deal_selected_file(Cancel);
    });
    //转存
    connect(m_save_action,&QAction::triggered,[=](){
        //处理选中的文件
        deal_selected_file(Save);
    });
    //属性
    connect(m_property_action,&QAction::triggered,[=](){
        //处理选中的文件
        deal_selected_file(Property);
    });
    //刷新
    connect(m_refresh_action,&QAction::triggered,[=](){
        refresh_files();
    });

}

/**
* @brief  清空共享文件列表
*
* @param
*
*
*
* @returns
*
*
*/
void ShareList::clear_sharefile_list()
{
    int len = m_sharefile_list.size();
    for(int i = 0; i < len; i++){
        FileInfo* info = m_sharefile_list.takeFirst();
        delete info;
    }

}

/**
* @brief  清空所有共享文件条目
*
* @param
*
*
*
* @returns
*
*
*/
void ShareList::clear_all_items()
{   //统计条目个数
    int item_count = ui->listWidget->count();
    /*
      使用takeItem(row)函数将QListWidget中的第row个item移除，
      移除需要使用delete手动释放其在堆上占用的空间
    */
    for(int i = 0; i < item_count; ++i){
        QListWidgetItem *item = ui->listWidget->takeItem(0);
        delete item;
    }
}

/**
* @brief  刷新文件item展示
*
* @param
*
*
*
* @returns
*
*
*/
void ShareList::refresh_file_items()
{
    clear_all_items();

    //如果文件列表不为空，显示文件item
    if(m_sharefile_list.isEmpty() == false){
        int count = m_sharefile_list.size();
        for(int i = 0; i < count; ++i){
            FileInfo* info = m_sharefile_list.at(i);
            QListWidgetItem* item = info->item;

            ui->listWidget->addItem(item);
        }
    }
}

/**
* @brief  刷新共享文件列表
*
* @param
*
*
*
* @returns
*
*
*/
void ShareList::refresh_files()
{
    m_user_sharefile_count = 0;

    QNetworkRequest request;

    LoginInfoInstance *login = LoginInfoInstance::get_instance();

    //url
    QString url = QString("http://%1:%2/sharefiles?cmd=count").
            arg(login->get_ip()).arg(login->get_port());

    request.setUrl(QUrl(url));

    QNetworkReply* reply = m_manager->get(request);
    if(reply == nullptr){
        cout<<"reply is nullptr";
        return;
    }

    //接收完消息后进行验证
    connect(reply, &QNetworkReply::finished,[=](){
        //如果有错误
        if(reply->error() != QNetworkReply::NoError){
            cout<<reply->errorString();
            reply->deleteLater();
            return;
        }
        QByteArray array = reply->readAll();
        //读取后释放资源
        reply->deleteLater();

        clear_sharefile_list();

        m_user_sharefile_count = array.toInt();

        if(m_user_sharefile_count > 0){
            m_start = 0;
            m_count = 10;
            get_user_sharefile_list();
        }//没有共享文件
        else{
            refresh_file_items();//更新条目显示
        }

    });
}

/**
* @brief  设置共享文件列表json
*
* @param
*
*
*
* @returns
*
*
*/
QByteArray ShareList::set_sharefile_list_json(int start, int count)
{
    QMap<QString, QVariant> tmp;

    tmp.insert("start", start);
    tmp.insert("count", count);

    QJsonDocument json_doc = QJsonDocument::fromVariant(tmp);

    if(json_doc.isNull()){
        cout<<"json_doc.isNull()";
        return "";
    }

    return json_doc.toJson();
}

/**
* @brief  获取共享文件列表
*
* @param
*
*
*
* @returns
*
*
*/
void ShareList::get_user_sharefile_list()
{
    //退出条件
    if(m_user_sharefile_count <= 0){
        cout<<"文件列表获取结束！";
        refresh_file_items();
        return;
    }
    //如果用户请求的文件个数大于文件总数量
    else if(m_count > m_user_sharefile_count){
        m_count = m_user_sharefile_count;
    }

    //获取登录信息实例
    LoginInfoInstance* login = LoginInfoInstance::get_instance();
    QNetworkRequest req;


    QString url = QString("http://%1:%2/sharefiles?/cmd=normal").
            arg(login->get_ip()).arg(login->get_port());

    req.setUrl(QUrl(url));
    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = set_sharefile_list_json(m_start, m_count);


    //发送post请求
    QNetworkReply* rep = m_manager->post(req, data);
    if(rep == nullptr){
        cout<<"rep is nullptr";
        return;
    }

    //改变文件列表起点位置
    m_start += m_count;
    m_user_sharefile_count -= m_count;


    //接收完消息后进行验证
    connect(rep, &QNetworkReply::finished,[=](){
        //如果有错误
        if(rep->error() != QNetworkReply::NoError){
            cout<<rep->errorString();
            rep->deleteLater();//释放资源
            return;
        }

        QByteArray array = rep->readAll();
        //读取后释放资源
        rep->deleteLater();

        if(m_cn.getCode(array) != "015"){
            //解析文件列表json信息，存放在文件列表中
            get_sharefile_json_info(array);
            //递归
            get_user_sharefile_list();
        }

    });
}

/**
* @brief  解析文件列表json信息，存放在文件列表之中
*
* @param
*
*
*
* @returns
*
*
*/
void ShareList::get_sharefile_json_info(QByteArray data)
{
    QJsonParseError err;

    QJsonDocument json_doc = QJsonDocument::fromJson(data, &err);

    if(err.error == QJsonParseError::NoError){
        if(json_doc.isNull() || json_doc.isEmpty()){
            cout<<"json_doc is NULL || json_doc is empty!";
            return ;
        }
        //如果最外层是对象
        if(json_doc.isObject()){
            //获取最外层的对象
            QJsonObject obj = json_doc.object();
            //获取files对应的存对象的数组
            QJsonArray arr_files = obj.value("files").toArray();

            int size = arr_files.size();

            for(int i = 0; i < size; ++i){
                QJsonObject tmp = arr_files[i].toObject();

//                struct FileInfo
//                {
//                    QString md5;            // 文件md5码
//                    QString filename;       // 文件名字
//                    QString user;           // 用户
//                    QString time;           // 上传时间
//                    QString url;            // url
//                    QString type;           // 文件类型
//                    qint64 size;            // 文件大小
//                    int shareStatus;        // 是否共享, 1共享， 0不共享
//                    int pv;                 // 下载量
//                    QListWidgetItem *item;  // list widget 的item
//                };

                FileInfo* info = new FileInfo;
                info->user = tmp.value("user").toString();
                info->md5 = tmp.value("md5").toString();
                info->time = tmp.value("time").toString();
                info->filename = tmp.value("filename").toString();
                info->shareStatus = tmp.value("share_status").toInt();
                info->pv = tmp.value("pv").toInt();
                info->url = tmp.value("url").toString();
                info->size = tmp.value("size").toInt();
                info->type = tmp.value("type").toString();
                QString type = info->type + ".png";
                info->item = new QListWidgetItem(QIcon(m_cn.getFileType(type)),
                                                 info->filename);
                //添加到文件列表
                m_sharefile_list.append(info);
            }
        }
    }
    else{
        cout<< "err = "<<err.errorString();
    }

}

/**
* @brief  添加需要下载的文件到下载任务列表
*
* @param
*
*
*
* @returns
*
*
*/
void ShareList::add_download_file()
{
    emit goto_transfer(TransferStatus::Download);
    //获取当前选中的item
    QListWidgetItem *item = ui->listWidget->currentItem();
    if(item == nullptr){
        return;
    }
    //获取下载列表实例
    DownloadTask* down_instance = DownloadTask::get_instance();
    if(down_instance == nullptr){
        return;
    }

    //查找文件列表匹配的元素
    for(int i = 0; i < m_sharefile_list.size(); ++i){
        //如果找到
        if(m_sharefile_list.at(i)->item == item){

            QString file_pathname = QFileDialog::getSaveFileName(this
                                                                 , "选择保存文件路径"
                                                                 , m_sharefile_list.at(i)->filename);

            /*
                成功: code:009
                失败：code:010
            */

            int res = down_instance->append_download_list(m_sharefile_list.at(i), file_pathname, true);
            if(res == -1){
                QMessageBox::warning(this, "任务已经存在", "任务已经在下载队列中");

            }else if(res == -2){
                //记录文件下载失败
                m_cn.writeRecord(m_sharefile_list.at(i)->user, m_sharefile_list.at(i)->filename, "010");
            }
            //退出循环
            break;
        }

    }
}

/**
* @brief  下载文件处理
*
* @param
*
*
*
* @returns
*
*
*/
void ShareList::download_files_action()
{
    //获取下载列表实例
    DownloadTask* down_instance = DownloadTask::get_instance();
    if(down_instance == nullptr){
        return;
    }

    if(down_instance->is_empty()){
        return;
    }
    if(down_instance->is_download()){
        return;
    }
    //第一个任务是否是共享文件的任务，是才能下载，不是就退出
    if(down_instance->is_share_task() == false){
        return;
    }

    DownloadFileInfo* info = down_instance->take_task();

    QString file_name = info->file_name;
    QFile* file = info->file;
    QString md5 = info->md5;
    QUrl url = info->url;
    DataProgress* dp = info->dp;

    QNetworkReply* reply = m_manager->get(QNetworkRequest(url));

    if(reply == nullptr){
        down_instance->delete_download_task();//删除任务
        return;
    }

    connect(reply, &QNetworkReply::finished,[=](){
        QMessageBox::information(this,"下载完成", QString("[%1]文件下载成功！").arg(file_name));
        reply->deleteLater();

        down_instance->delete_download_task();
        //写入记录
        LoginInfoInstance* login = LoginInfoInstance::get_instance();

        m_cn.writeRecord(login->get_user(), file_name, "010");

        deal_file_pv(md5, file_name);
    });

    connect(reply, &QNetworkReply::readyRead,[=](){
        if(file != nullptr){
            file->write(reply->readAll());
        }
    });

    connect(reply, &QNetworkReply::downloadProgress,[=](qint64 bytes_writen, qint64 bytes_total){
        //进度条函数有默认初始值
        dp->set_progress(bytes_writen, bytes_total);
    });
}

/**
* @brief  设置共享文件json包
*
* @param user md5 filename
*
*
*
* @returns
*       success: json_doc
*       fail: empty string
*/
QByteArray ShareList::set_sharefile_json(QString user, QString md5, QString filename)
{
    /*
    {
        "user":"xx"
        "md5":"xx"
        "filename":"xx"
    }
    */

    QMap<QString, QVariant> tmp;
    tmp.insert("user", user);
    tmp.insert("md5", md5);
    tmp.insert("filename", filename);

    QJsonDocument json_doc = QJsonDocument::fromVariant(tmp);

    if(json_doc.isNull()){
        return "";
    }

    return json_doc.toJson();
}

/**
* @brief  下载文件pv处理
*
* @param md5 filename
*
*
*
* @returns
*
*
*/
void ShareList::deal_file_pv(QString md5, QString filename)
{
    //获取登录信息实例
    LoginInfoInstance* login = LoginInfoInstance::get_instance();
    QNetworkRequest req;

    QString url = QString("http://%1:%2/dealsharefile?cmd=pv")
            .arg(login->get_ip())
            .arg(login->get_port()
            );

    req.setUrl(QUrl(url));
    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = set_sharefile_json(login->get_user()
                                         , md5
                                         , filename);

    //发送post请求
    QNetworkReply* rep = m_manager->post(req, data);

    if(rep == nullptr){
        cout<<"rep is nullptr";
        return;
    }
    //接收完消息后进行验证
    connect(rep, &QNetworkReply::finished,[=](){
        //如果有错误
        if(rep->error() != QNetworkReply::NoError){
            cout<<rep->errorString();
            rep->deleteLater();//释放资源
            return;
        }

        QByteArray array = rep->readAll();
        //读取后释放资源
        rep->deleteLater();

        /*
            成功: code:016
            失败：code:017
        */
        if(m_cn.getCode(array) == "016"){
            //查找文件列表匹配的元素
            for(int i = 0; i < m_sharefile_list.size(); ++i){

                FileInfo* info = m_sharefile_list.at(i);

                //如果找到
                if(info->md5 == md5 && info->filename == filename){

                    int pv = info->pv;
                    info->pv = pv + 1;

                    //处理完退出循环
                    break;
                }
            }


        }
        else{
            cout<<"文件pv信息读取失败！"<<endl;
        }

    });
}

/**
* @brief  处理选中的文件
*
* @param cmd
*
*
*
* @returns
*
*
*/
void ShareList::deal_selected_file(ShareList::CMD cmd)
{
    //获取当前选中的item
    QListWidgetItem *item = ui->listWidget->currentItem();
    if(item == nullptr){
        return;
    }
    //查找文件列表匹配的元素
    for(int i = 0; i < m_sharefile_list.size(); ++i){
        //如果找到
        if(m_sharefile_list.at(i)->item == item){

            if(cmd == Property){
                get_file_property(m_sharefile_list.at(i));
            }else if(cmd == Cancel){
                cancel_share_files(m_sharefile_list.at(i));
            }else if(cmd == Save){
                save_file_to_mylist(m_sharefile_list.at(i));
            }
            //处理完退出循环
            break;
        }
    }
}

/**
* @brief  获取文件属性
*
* @param info
*
*
*
* @returns
*
*
*/
void ShareList::get_file_property(FileInfo *info)
{
    FilePropertyInfo dlg;
    dlg.set_info(info);
    dlg.exec();
}


/**
* @brief  取消文件分享
*
* @param info
*
*
*
* @returns
*
*
*/
void ShareList::cancel_share_files(FileInfo *info)
{
    //获取登录信息实例
    LoginInfoInstance* login = LoginInfoInstance::get_instance();
    QNetworkRequest req;

    //必须是文件的分享人才能取消
    if(login->get_user() != info->user){
        QMessageBox::warning(this,"操作失败","本文件不是您分享的，无法取消。");
        return;
    }

    QString url = QString("http://%1:%2/dealsharefile?cmd=cancel")
            .arg(login->get_ip())
            .arg(login->get_port()
            );

    req.setUrl(QUrl(url));
    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = set_sharefile_json(login->get_user()
                                         , info->md5
                                         , info->filename);

    //发送post请求
    QNetworkReply* rep = m_manager->post(req, data);

    if(rep == nullptr){
        cout<<"rep is nullptr";
        return;
    }

    //接收完消息后进行验证
    connect(rep, &QNetworkReply::finished,[=](){
        //如果有错误
        if(rep->error() != QNetworkReply::NoError){
            cout<<rep->errorString();
            rep->deleteLater();//释放资源
            return;
        }

        QByteArray array = rep->readAll();
        //读取后释放资源
        rep->deleteLater();

        /*
            成功: code:018
            失败：code:019
        */
     if(m_cn.getCode(array) == "018"){

            //从文件列表中移除该文件，从展示item中移除该文件
            //查找文件列表匹配的元素
            for(int i = 0; i < m_sharefile_list.size(); ++i){
                //如果找到
                if(m_sharefile_list.at(i) == info){

                    QListWidgetItem *item = info->item;
                    //移除item
                    ui->listWidget->removeItemWidget(item);

                    delete item;

                    m_sharefile_list.removeAt(i);

                    delete info;

                    //处理完退出循环
                    break;
                }
            }
            QMessageBox::warning(this,"操作成功",QString("[%1]文件取消分享成功").arg(info->filename));

        }else if(m_cn.getCode(array) == "019"){
            QMessageBox::warning(this,"操作失败",QString("[%1]取消分析失败").arg(info->filename));

        }

    });
}

/**
* @brief  转存文件
*
* @param info
*
*
*
* @returns
*
*
*/
void ShareList::save_file_to_mylist(FileInfo *info)
{
    //获取登录信息实例
    LoginInfoInstance* login = LoginInfoInstance::get_instance();
    QNetworkRequest req;

    //如果是自己分享的则无法转存
    if(login->get_user() != info->user){
        QMessageBox::warning(this, "操作失败", "本文件属于当前用户，无需转存");
        return;
    }

    QString url = QString("http://%1:%2/dealsharefile?cmd=save")
            .arg(login->get_ip())
            .arg(login->get_port()
            );

    req.setUrl(QUrl(url));
    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = set_sharefile_json(login->get_user()
                                         , info->md5
                                         , info->filename);

    //发送post请求
    QNetworkReply* rep = m_manager->post(req, data);

    if(rep == nullptr){
        cout<<"rep is nullptr";
        return;
    }

    //接收完消息后进行验证
    connect(rep, &QNetworkReply::finished,[=](){
        //如果有错误
        if(rep->error() != QNetworkReply::NoError){
            cout<<rep->errorString();
            rep->deleteLater();//释放资源
            return;
        }

        QByteArray array = rep->readAll();
        //读取后释放资源
        rep->deleteLater();

        /*
            成功: code: 020
            文件已经存在：code: 021
            失败：code: 022
        */
     if(m_cn.getCode(array) == "020"){
                QMessageBox::warning(this, "操作成功", QString("[%1]文件已经转存成功。").arg(info->filename));

     }else if(m_cn.getCode(array) == "021"){
                QMessageBox::warning(this, "操作失败", QString("[%1]此文件已经存在无需转存。").arg(info->filename));

     }
     else if(m_cn.getCode(array) == "022"){
                QMessageBox::warning(this, "操作失败", QString("[%1]文件转存失败。").arg(info->filename));

             }

    });
}




