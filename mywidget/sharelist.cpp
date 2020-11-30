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
            get_user_filelist();
        }//没有共享文件
        else{
            refresh_file_items();//更新条目显示
        }



    });
}
