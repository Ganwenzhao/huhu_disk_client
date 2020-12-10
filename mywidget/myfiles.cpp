#include "myfiles.h"
#include "ui_myfiles.h"
#include <QListWidgetItem>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkReply>
#include <QNetworkRequest>
#include "common/logininfoinstance.h"
#include "common/downloadtask.h"
#include "filepropertyinfo.h"

MyFiles::MyFiles(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MyFiles)
{
    ui->setupUi(this);
    //初始化文件列表
    init_file_listwidget();
    //添加右键菜单
    add_action_menu();
    //创建http管理对象
    m_manager = Common::getNetManager();
    //定时检查任务队列
    check_task_list();
}

MyFiles::~MyFiles()
{
    delete ui;
}

/**
 * @brief  初始化文件显示列表
 *
 * @param
 *
 *
 *
 * @returns
 *
 *
 */
void MyFiles::init_file_listwidget()
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
            this,&MyFiles::right_menu);

    //点击列表中的上传图标
    connect(ui->listWidget,&QListWidget::itemPressed,
            this,[=](QListWidgetItem* item){

        QString str  = item->text();
        if(str == "上传文件"){
            add_upload_files();
        }
    });

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
void MyFiles::add_action_menu()
{
    //==============菜单1=============
    m_menu = new Menu(this);

    //初始化菜单1
    m_download_action = new QAction("下载",this);
    m_share_action = new QAction("分享",this);
    m_dele_action = new QAction("删除",this);
    m_property_action = new QAction("属性",this);

    //动作1添加到菜单1
    m_menu->addAction(m_download_action);
    m_menu->addAction(m_share_action);
    m_menu->addAction(m_dele_action);
    m_menu->addAction(m_property_action);

    //==============菜单2=============
    m_menu_empty = new Menu(this);

    //初始化菜单2
    m_pvasc_action = new QAction("下载量升序排列",this);
    m_pvdesc_action = new QAction("下载量降序排列",this);
    m_refresh_action = new QAction("刷新文件列表",this);
    m_upload_action = new QAction("上传",this);

    //动作1添加到菜单2
    m_menu->addAction(m_pvasc_action);
    m_menu->addAction(m_pvdesc_action);
    m_menu->addAction(m_refresh_action);
    m_menu->addAction(m_upload_action);

    //====================信号槽============
    //下载
    connect(m_download_action,&QAction::triggered,[=](){
        add_download_file();
    });
    //分享
    connect(m_share_action,&QAction::triggered,[=](){
        //处理选中的文件
        deal_select_files("分享");
    });
    //删除
    connect(m_dele_action,&QAction::triggered,[=](){
        //处理选中的文件
        deal_select_files("删除");
    });
    //属性
    connect(m_property_action,&QAction::triggered,[=](){
        //处理选中的文件
        deal_select_files("属性");
    });
    //下载量升序
    connect(m_pvasc_action,&QAction::triggered,[=](){
        refresh_files(PvAsc);
    });
    //下载量降序
    connect(m_pvdesc_action,&QAction::triggered,[=](){
        refresh_files(PvDesc);
    });
    //刷新
    connect(m_refresh_action,&QAction::triggered,[=](){
        refresh_files(Normal);
    });
    //上传
    connect(m_upload_action,&QAction::triggered,[=](){
        add_upload_files();
    });

}

/**
 * @brief  添加需要上传的文件到上传文件列表
 *
 * @param
 *
 *
 *
 * @returns
 *
 *
 */
void MyFiles::add_upload_files()
{
    emit goto_transfer(TransferStatus::Upload);
    //获取上传列表实例
    UploadTask* upload_list = UploadTask::get_instance();
    if(upload_list == nullptr){
        cout<<"upload_list == nullptr";
        return;
    }
    //返回选中文件的路径，可以是多个
    QStringList list = QFileDialog::getOpenFileNames();

    for(int i = 0; i < list.size(); i++){

        int res = upload_list->append_upload_list(list.at(i));

        if(res == -1){
            QMessageBox::warning(this,"文件过大！","文件大小不能超过100M");
        }else if(res == -2){
            QMessageBox::warning(this,"添加失败！","文件已在上传队列");
        }else if(res == -3){
            cout<<"打开文件失败";
        }else if(res == -4){
            cout<<"获取布局失败";
        }
    }

}

/**
 * @brief  设置md5的json包
 *
 * @param user 用户名
 * @param token 用户令牌
 * @param md5 文件的md5编码
 * @param file_name 文件名
 *
 *
 * @returns
 *      成功：json字符串
 *      失败：空字符串
 *
 */
QByteArray MyFiles::set_md5_json(QString user,
                                 QString token,
                                 QString md5,
                                 QString file_name)
{
    QMap<QString,QVariant> tmp_var;
    tmp_var.insert("user",user);
    tmp_var.insert("token",token);
    tmp_var.insert("md5",md5);
    tmp_var.insert("file_name",file_name);

    QJsonDocument json_doc = QJsonDocument::fromVariant(tmp_var);
    if(json_doc.isNull()){
        cout<<"json_doc.isNull";
        return "";
    }

    return json_doc.toJson();
}

/**
 * @brief  上传文件处理,向服务器发送待上传文件的json信息包
 *
 * @param
 *
 *
 *
 * @returns
 *
 *
 */
void MyFiles::upload_file_action()
{
    //获取上传列表实例
    UploadTask* upload_list = UploadTask::get_instance();
    if(upload_list == nullptr){
        cout<<"upload_list == nullptr";
        return;
    }
    //如果队列为空，返回
    if(upload_list->is_empty()){
        return;
    }
    //如果当前有任务在上传，需要等待，不能上传
    if(upload_list->is_upload()){
        return;
    }

    //获取登录信息实例
    LoginInfoInstance* login = LoginInfoInstance::get_instance();

    //url
    QNetworkRequest request;
    QString url = QString("http://%1:%2/md5")
            .arg(login->get_ip())
            .arg(login->get_port());

    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    //取出队首文件信息指针
    UploadFileInfo* info = upload_list->take_task();
    //设置json数据包
    QByteArray arr = set_md5_json(login->get_user(),
                                  login->get_token(),
                                  info->md5, info->file_name);

    //post数据
    QNetworkReply* reply = m_manager->post(request, arr);
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
        /*
            秒传文件：文件已存在{"code":"005"}
                    秒传成功{"code":"006"}
                    秒传失败{"code":"007"}

        */

        if(m_cn.getCode(array) == "006"){
            //秒传成功，写入本地记录
            m_cn.writeRecord(login->get_user(),info->file_name,"006");
            //删除已经完成的上传任务
            upload_list->delete_upload_task();
        }else if(m_cn.getCode(array) == "007"){
            //秒传失败，需要上传文件本来的内容
            upload_file(info);
        }else if(m_cn.getCode(array) == "005"){
            //文件已经存在
            m_cn.writeRecord(login->get_user(),info->file_name,"005");
        }
        //token验证失败
        else if(m_cn.getCode(array) == "111"){
            QMessageBox::warning(this,"身份验证失败","请重新登录");
            //发送重新登录的信号
            emit login_again_signal();
            return;
        }
    });
}
/**
 * @brief  不能秒传，那么直接上传真正的文件内容
 *
 * @param info 文件信息
 *
 *
 *
 * @returns
 *
 *
 */
void MyFiles::upload_file(UploadFileInfo *info)
{
    QString file_name = info->file_name;
    QFile* file = info->file;
    QString md5 = info->md5;
    quint64 size = info->size;
    DataProgress* dp = info->dp;
    QString boundary = m_cn.getBoundary();

    //获取登录信息实例
    LoginInfoInstance* login = LoginInfoInstance::get_instance();

    QByteArray data;

    /* boundary\r\n
     * Content-Disposition:form-data; user="alex";.....\r\n
     * Content-Type:..........\r\n
     * \r\n
     * 真实文件内容\r\n
     * boundary
     */

    //开始分割线
    data.append(boundary);
    data.append("\r\n");

    //插入文件信息
    data.append("Content-Disposition: form-data; ");
    data.append(QString("user=\"%1\" ").arg(login->get_user()));
    data.append(QString("filename=\"%1\" ").arg(file_name));
    data.append(QString("md5=\"%1\" ").arg(md5));
    data.append(QString("size=%1").arg(size));
    data.append("\r\n");

    data.append("Content-Type: application/octet-stream");
    data.append("\r\n");
    data.append("\r\n");

    //上传文件内容
    data.append(file->readAll());
    data.append("\r\n");

    //结束分割线
    data.append(boundary);

    QNetworkRequest req;
    QString url = QString("http://%1:%2/upload")
            .arg(login->get_ip())
            .arg(login->get_port());

    req.setUrl(QUrl(url));
    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    //发送post请求
    QNetworkReply* rep = m_manager->post(req, data);
    if(rep == nullptr){
        cout<<"upload file rep is nullptr";
        return;
    }
    //有可用数据更新progress_bar
    connect(rep, &QNetworkReply::uploadProgress,
            [=](qint64 bytes_read, qint64 bytes_total){
        if(bytes_total != 0){
            dp->set_progress(bytes_read / 1024, bytes_total / 1024);
        }
    });
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
            上传文件：上传成功{"code":"008"}
                    上传失败{"code":"009"}

        */
        if(m_cn.getCode(array) == "008"){
            //上传成功，写入本地记录
            m_cn.writeRecord(login->get_user(),info->file_name,"008");

        }else if(m_cn.getCode(array) == "009"){
            //上传失败，写入本地记录
            m_cn.writeRecord(login->get_user(),info->file_name,"009");
        }

        //获取上传列表实例
        UploadTask* upload_list = UploadTask::get_instance();
        if(upload_list == nullptr){
            cout<<"UploadTask::get_instance() == nullptr";
            return;
        }
        //删除完成的上传任务
        upload_list->delete_upload_task();

    });

}


/*====================文件条目展示===============*/
/**
* @brief  清空文件列表
*
* @param
*
*
*
* @returns
*
*
*/
void MyFiles::clear_file_list()
{
    int len = m_file_list.size();
    for(int i = 0; i < len; i++){
        FileInfo* info = m_file_list.takeFirst();
        delete info;
    }

}


/**
* @brief  清空所有文件条目
*
* @param
*
*
*
* @returns
*
*
*/
void MyFiles::clear_all_items()
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
* @brief  添加上传文件项目条目
*
* @param
*
*
*
* @returns
*
*
*/

void MyFiles::add_upload_items(QString icon_path, QString name)
{
    ui->listWidget->addItem(new QListWidgetItem(QIcon(icon_path), name));
}



/**
* @brief  刷新文件条目展示
*
* @param
*
*
*
* @returns
*
*
*/
void MyFiles::refresh_file_items()
{
    clear_all_items();

    //如果文件列表不为空，显示文件item
    if(m_file_list.isEmpty() == false){
        int count = m_file_list.size();
        for(int i = 0; i < count; ++i){
            FileInfo* info = m_file_list.at(i);
            QListWidgetItem* item = info->item;
            ui->listWidget->addItem(item);
        }
    }
    //添加上传文件item
    this->add_upload_items();
}

/**
* @brief  获取服务器json文件
*
* @param json文件
*
*
*
* @returns
*      成功：返回token和文件个数
*      失败： 返回空list
*/
QStringList MyFiles::get_count_status(QByteArray json)
{
    QJsonParseError err;
    QStringList list;

    QJsonDocument json_doc = QJsonDocument::fromJson(json,&err);

    if(err.error == QJsonParseError::NoError){

        if(json_doc.isNull() || json.isEmpty()){
            cout<<"json_doc.isNull() || json.isEmpty()!";
            return list;
        }

        if(json_doc.isObject()){
            //获取外层对象
            QJsonObject obj = json_doc.object();
            cout<<"服务器上返回的数据: "<< json;
            list.append(obj.value("token").toString());//登录token
            list.append(obj.value("num").toString());//文件个数

        }
        else{
            cout<<"erro"<<err.errorString();

        }
    }


    return list;
}

/**
* @brief  刷新文件列表
*
* @param cmd:Normal:普通用户列表
*           PvAsc:按照下载量升序
*           PvDesc:按照下载量降序
*
* @returns
*
*
*/
void MyFiles::refresh_files(MyFiles::Display cmd)
{
    m_user_file_count = 0;

    QNetworkRequest request;

    LoginInfoInstance *login = LoginInfoInstance::get_instance();

    //url
    QString url = QString("http://%1:%2/myfiles?cmd=count").
            arg(login->get_ip()).arg(login->get_port());
    request.setUrl(QUrl(url));

    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    //设置json数据包
    QByteArray data = set_get_count_json(login->get_user(),
                                         login->get_token());

    //post数据
    QNetworkReply* reply = m_manager->post(request, data);
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

        QStringList list = get_count_status(array);

        //token 验证失败
        if(list.at(0) == "111"){
            QMessageBox::warning(this,"身份验证失败","请重新登录");
            //发送重新登录的信号
            emit login_again_signal();

            return;
        }

        //获取个数
        m_user_file_count = list.at(1).toInt();

        clear_file_list();

        //如果列表里有文件
        if(m_user_file_count > 0){
            //获取文件列表
            m_start = 0;//start from 0
            m_count = 10;//get 10 each time

            //获取新的文件列表信息
            get_user_file_list(cmd);
        }
        else{
            refresh_file_items();//更新条目
        }

    });
}


/**
* @brief  设置json包
*
* @param user 用户名
* @param token 用户标识
*
*
* @returns
*       成功：返回json文本
*       失败： 返回空字符串
*
*/
QByteArray MyFiles::set_get_count_json(QString user,
                                       QString token)
{
    QMap<QString, QVariant> tmp;
    tmp.insert("user", user);
    tmp.insert("token", token);

    QJsonDocument json_doc = QJsonDocument::fromVariant(tmp);

    if(json_doc.isNull()){
        cout<<"json_doc.isNull()";
        return "";
    }

    return json_doc.toJson();

}

/**
* @brief  设置json包
*
* @param user 用户名
* @param token 用户标识
* @param start 开始位置
* @param count 取得个数
*
*
* @returns
*       成功：返回json文本
*       失败： 返回空字符串
*
*/
QByteArray MyFiles::set_file_list_json(QString user,
                                       QString token,
                                       int start,
                                       int count)
{
    QMap<QString, QVariant> tmp;
    tmp.insert("user", user);
    tmp.insert("token", token);
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
* @brief  获取用户文件列表（递归函数）
*
* @param cmd:Normal:普通用户列表
*           PvAsc:按照下载量升序
*           PvDesc:按照下载量降序
*
* @returns
*
*
*/
void MyFiles::get_user_file_list(MyFiles::Display cmd)
{
    //退出条件
    if(m_user_file_count <= 0){
        cout<<"文件列表获取结束！";
        refresh_file_items();
        return;
    }
    //如果用户请求的文件个数大于文件总数量
    else if(m_count > m_user_file_count){
        m_count = m_user_file_count;
    }

    //获取登录信息实例
    LoginInfoInstance* login = LoginInfoInstance::get_instance();
    QNetworkRequest req;

    QString tmp;
    if(cmd == Normal){
        tmp = "normal";
    }else if(cmd == PvAsc){
        tmp = "pvasc";
    }else if(PvDesc){
        tmp = "pvdesc";
    }

    QString url = QString("http://%1:%2/myfiles?/cmd=%3").
            arg(login->get_ip()).arg(login->get_port().arg(tmp));

    req.setUrl(QUrl(url));
    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = set_file_list_json(login->get_user(),
                                         login->get_token(),
                                         m_start, m_count);


    //发送post请求
    QNetworkReply* rep = m_manager->post(req, data);
    if(rep == nullptr){
        cout<<"rep is nullptr";
        return;
    }

    //改变文件列表起点位置
    m_start += m_count;
    m_user_file_count -= m_count;


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

        //token 验证失败
        if(m_cn.getCode(array) == "111"){
            QMessageBox::warning(this,"身份验证失败","请重新登录");
            //发送重新登录的信号
            emit login_again_signal();

            return;
        }
        if(m_cn.getCode(array) != "015"){
            //解析文件列表json信息，存放在文件列表中
            get_file_json_info(array);
            //递归
            get_user_file_list();
        }

    });

}

/**
* @brief  解析列表文件json信息，然后存入文件列表
*
* @param data json文本
*
*
*
* @returns
*
*
*/
void MyFiles::get_file_json_info(QByteArray data)
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
                m_file_list.append(info);
            }
        }
    }
    else{
        cout<< "err = "<<err.errorString();
    }

}

/**
* @brief  处理选中的文件
*
* @param cmd "命令"
*
*
*
* @returns
*
*
*/

void MyFiles::deal_select_files(QString cmd)
{
    //获取当前选中的item
    QListWidgetItem *item = ui->listWidget->currentItem();
    if(item == nullptr){
        return;
    }
    //查找文件列表匹配的元素
    for(int i = 0; i < m_file_list.size(); ++i){
        //如果找到
        if(m_file_list.at(i)->item == item){

            if(cmd == "分享"){
                share_file(m_file_list.at(i));
            }else if(cmd == "删除"){
                dele_file(m_file_list.at(i));
            }else if(cmd == "属性"){
                get_file_property(m_file_list.at(i));
            }
            //处理完退出循环
            break;
        }
    }

}

/**
* @brief  设置处理文件的json包
*
* @param
*
*
*
* @returns
*       成功：返回json字符串
*       失败：返回空字符串
*/
QByteArray MyFiles::set_deal_file_json(QString user, QString token, QString md5, QString filename)
{
    /*
    {
        "user":"xx"
        "token":"xx"
        "md5":"xx"
        "filename":"xx"
    }
    */

    QMap<QString, QVariant> tmp;
    tmp.insert("user", user);
    tmp.insert("token", token);
    tmp.insert("md5", md5);
    tmp.insert("filename", filename);

    QJsonDocument json_doc = QJsonDocument::fromVariant(tmp);

    if(json_doc.isNull()){
        return "";
    }

    return json_doc.toJson();
}

/**
* @brief  设置处理文件的json包
*
* @param info 文件信息指针
*
*
*
* @returns
*
*
*/

void MyFiles::share_file(FileInfo *info)
{
    //已经分享
    if(info->shareStatus == 1){
        QMessageBox::warning(this, "此文件已经分享", "此文件已经分享！" );
        return;
    }
    //获取登录信息实例
    LoginInfoInstance* login = LoginInfoInstance::get_instance();
    QNetworkRequest req;

    QString url = QString("http://%1:%2/dealfile?cmd=share")
            .arg(login->get_ip())
            .arg(login->get_port()
            );

    req.setUrl(QUrl(url));
    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = set_deal_file_json(login->get_user()
                                         , login->get_token()
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
            成功: code:010
            失败：code:011
            已经有人分享此文件：code:012
            token验证失败: code:111

        */
        if(m_cn.getCode(array) == "111"){
            QMessageBox::warning(this,"身份验证失败","请重新登录");
            //发送重新登录的信号
            emit login_again_signal();

            return;

        }else if(m_cn.getCode(array) == "010"){
            info->shareStatus = 1;
            QMessageBox::information(this, "分享成功", QString("[%1]文件分享成功").arg(info->filename));

        }else if(m_cn.getCode(array) == "011"){
            QMessageBox::warning(this,"分享失败",QString("[%1]文件分享失败").arg(info->filename));

        }else if(m_cn.getCode(array) == "012"){
            QMessageBox::warning(this,"分享失败",QString("[%1]已经有人分享此文件").arg(info->filename));

        }

    });

}

/**
* @brief  删除文件
*
* @param info 文件信息指针
*
*
*
* @returns
*
*
*/
void MyFiles::dele_file(FileInfo *info)
{
    //获取登录信息实例
    LoginInfoInstance* login = LoginInfoInstance::get_instance();
    QNetworkRequest req;

    QString url = QString("http://%1:%2/dealfile?cmd=share")
            .arg(login->get_ip())
            .arg(login->get_port()
            );

    req.setUrl(QUrl(url));
    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = set_deal_file_json(login->get_user()
                                         , login->get_token()
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
            成功: code:013
            失败：code:014
        */
        if(m_cn.getCode(array) == "111"){
            QMessageBox::warning(this,"身份验证失败","请重新登录");
            //发送重新登录的信号
            emit login_again_signal();

            return;

        }else if(m_cn.getCode(array) == "013"){

            QMessageBox::information(this, "文件删除成功", QString("[%1]文件删除成功").arg(info->filename));
            //从文件列表中移除该文件，从展示item中移除该文件
            //查找文件列表匹配的元素
            for(int i = 0; i < m_file_list.size(); ++i){
                //如果找到
                if(m_file_list.at(i) == info){

                    QListWidgetItem *item = info->item;
                    //移除item
                    ui->listWidget->removeItemWidget(item);

                    delete item;

                    m_file_list.removeAt(i);

                    delete info;

                    //处理完退出循环
                    break;
                }
            }

        }else if(m_cn.getCode(array) == "014"){
            QMessageBox::warning(this,"删除失败",QString("[%1]文件删除失败").arg(info->filename));

        }

    });

}

/**
* @brief  获取文件属性
*
* @param info 文件信息指针
*
*
*
* @returns
*
*
*/
void MyFiles::get_file_property(FileInfo *info)
{
    FilePropertyInfo dlg;
    dlg.set_info(info);

    dlg.exec();
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
void MyFiles::add_download_file()
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
    for(int i = 0; i < m_file_list.size(); ++i){
        //如果找到
        if(m_file_list.at(i)->item == item){

            QString file_pathname = QFileDialog::getSaveFileName(this
                                                                 , "选择保存你文件路径"
                                                                 , m_file_list.at(i)->filename);

            /*
                成功: code:009
                失败：code:010
            */

            int res = down_instance->append_download_list(m_file_list.at(i), file_pathname);
            if(res == -1){
                QMessageBox::warning(this, "任务已经存在", "任务已经在下载队列中");

            }else if(res == -2){
                //记录文件下载失败
                m_cn.writeRecord(m_file_list.at(i)->user, m_file_list.at(i)->filename, "010");
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
void MyFiles::download_file_action()
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
    //第一个任务是否是共享文件的任务，不是才能下载
    if(down_instance->is_share_task()){
        return;
    }

    DownloadFileInfo* info = down_instance->take_task();

    QString file_name = info->file_name;
    QFile* file = info->file;
    QString user = info->user;
    QString md5 = info->md5;
    QUrl url = info->url;
    DataProgress* dp = info->dp;

    QNetworkReply* reply = m_manager->get(QNetworkRequest(url));

    if(reply == nullptr){
        return;
    }

    connect(reply, &QNetworkReply::finished,[=](){
        QMessageBox::information(this,"下载完成", QString("[%1]文件下载成功！").arg(file_name));
        reply->deleteLater();

        down_instance->delete_download_task();
        //写入记录
        m_cn.writeRecord(user, file_name, "010");

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
* @brief  下载文件标志处理
*
* @param md5, file_name
*
*
*
* @returns
*
*
*/
void MyFiles::deal_file_pv(QString md5, QString file_name)
{
    //获取登录信息实例
    LoginInfoInstance* login = LoginInfoInstance::get_instance();
    QNetworkRequest req;

    QString url = QString("http://%1:%2/dealfile?cmd=pv")
            .arg(login->get_ip())
            .arg(login->get_port()
            );

    req.setUrl(QUrl(url));
    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = set_deal_file_json(login->get_user()
                                         , login->get_token()
                                         , md5
                                         , file_name);

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
            for(int i = 0; i < m_file_list.size(); ++i){

                FileInfo* info = m_file_list.at(i);

                //如果找到
                if(info->md5 == md5 && info->filename == file_name){

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
* @brief  清除上传和下载列表
*
* @param
*
*
*
* @returns
*
*
*/
void MyFiles::clear_all_tasks()
{
    //获取上传列表实例
    UploadTask* upload_list = UploadTask::get_instance();
    if(upload_list == nullptr){
        cout<<"upload_list == nullptr";
        return;
    }

    upload_list->clear_list();

    //获取下载列表实例
    DownloadTask* down_instance = DownloadTask::get_instance();
    if(down_instance == nullptr){
        return;
    }

    down_instance->clear_list();
}

/**
* @brief  定时检查上传和下载列表
*
* @param
*
*
*
* @returns
*
*
*/
void MyFiles::check_task_list()
{
    connect(&m_upload_timer, &QTimer::timeout,[=]{
        //上传文件处理
        upload_file_action();
    });

    //启动定时器，500ms检测一次
    m_upload_timer.start(500);

    connect(&m_download_timer, &QTimer::timeout,[=]{
        //上传文件处理
        download_file_action();
    });

    //启动定时器，500ms检测一次
    m_download_timer.start(500);
}

/**
* @brief  显示右键菜单
*
* @param pos 位置
*
*
*
* @returns
*
*
*/
void MyFiles::right_menu(const QPoint &pos)
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
        //上传文件item没有右键菜单
        if(item->text() == "上传文件"){
            return;
        }
        m_menu->exec(QCursor::pos());
    }

}











