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
    init_file_list();
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
void MyFiles::init_file_list()
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

    connect(ui->listWidget, &QListView::customContextMenuRequested,this,&MyFiles::right_menu);
    //点击列表中的上传图标
    connect(ui->listWidget,&QListWidget::itemPressed,this,[=](QListWidgetItem* item){
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
            QMessageBox::warning(this,"打开失败！","检查文件位置");
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
QByteArray MyFiles::set_md5_json(QString user, QString token, QString md5, QString file_name)
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
    QString url = QString("http://%1:%2/md5").arg(login->get_ip()).arg(login->get_port());

    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    //取出队首文件信息指针
    UploadFileInfo* info = upload_list->take_task();
    //设置json数据包
    QByteArray arr = set_md5_json(login->get_user(), login->get_token(), info->md5, info->file_name);

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
    QString url = QString("http://%1:%2/upload").arg(login->get_ip()).arg(login->get_port());

    req.setUrl(QUrl(url));
    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    //发送post请求
    QNetworkReply* rep = m_manager->post(req, data);
    if(rep == nullptr){
        cout<<"upload file rep is nullptr";
        return;
    }
    //有可用数据更新progress_bar
    connect(rep, &QNetworkReply::uploadProgress,[=](qint64 bytes_read, qint64 bytes_total){
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
* @brief  清空文件条目
*
* @param
*
*
*
* @returns
*
*
*/
void MyFiles::clear_items()
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






