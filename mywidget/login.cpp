#include "mywidget/login.h"
#include "ui_login.h"
#include "mywidget/titlewidget.h"
#include <QPainter>
#include "common/common.h"
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>


Login::Login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);

    //初始化网络请求对象
    m_manager = Common::getNetManager();
    //初始化主窗口
    m_mainwind = new MainWindow;

    //去掉边框
    this->setWindowFlags(Qt::FramelessWindowHint | windowFlags());
    //设置图标
    this->setWindowIcon(QIcon(":/image2/main_logo.png"));
    m_mainwind->setWindowIcon(QIcon(":/image2/main_logo.png"));

    //设置登录页面前置显示
    ui->stackedWidget->setCurrentIndex(0);
    //光标跳到用户名输入处
    ui->login_name->setFocus();

    //数据格式的提示
    ui->login_name->setToolTip("合法字符:[a-z|A-Z|#|@|*|-|_|0-9|],字符数:3-16");
    ui->reg_name->setToolTip("合法字符:[a-z|A-Z|#|@|*|-|_|0-9|],字符数:3-16");
    ui->reg_nickname->setToolTip("合法字符:[a-z|A-Z|#|@|*|-|_|0-9|],字符数:6-18");
    ui->login_pwd->setToolTip("合法字符:[a-z|A-Z|#|@|*|-|_|0-9|],字符数:6-18");
    ui->reg_pwd->setToolTip("合法字符:[a-z|A-Z|#|@|*|-|_|0-9|],字符数:6-18");
    ui->reg_pwd_again->setToolTip("合法字符:[a-z|A-Z|#|@|*|-|_|0-9|],字符数:6-18");

    //密码的显示隐藏，用小黑点代替
    ui->login_pwd->setEchoMode(QLineEdit::Password);
    ui->reg_pwd->setEchoMode(QLineEdit::Password);
    ui->reg_pwd_again->setEchoMode(QLineEdit::Password);

    //读取配置文件信息，并初始化
    //read_cfg();
    //加载图片信息，以备在显示文件类型的时候使用
    m_cm.getFileTypeList();



    //注册取消按钮
    connect(ui->toolButton_cancel,&QToolButton::clicked,this,[=](){
        //清除数据
        ui->reg_name->clear();
        ui->reg_nickname->clear();
        ui->reg_pwd->clear();
        ui->reg_pwd_again->clear();
        ui->reg_phone->clear();
        ui->reg_email->clear();
        //显示登录界面
        ui->stackedWidget->setCurrentIndex(0);

    });

    //设置取消按钮
    connect(ui->toolButton_cancel_2,&QToolButton::clicked,this,[=](){
        //清除数据
        ui->lineEdit_serv_addr->clear();
        ui->lineEdit_serv_port->clear();
        //显示登录界面
        ui->stackedWidget->setCurrentIndex(0);

    });

    //注册按钮
    connect(ui->toolButton_reg_2,&QToolButton::clicked,this,[=](){
        ui->stackedWidget->setCurrentIndex(1);
        ui->reg_name->setFocus();

    });
    //显示注册页面
    connect(ui->title_widget,&TitleWidget::show_set_widget,[=](){
        ui->stackedWidget->setCurrentIndex(2);

    });

    //切换用户的信号处理
    connect(m_mainwind,&MainWindow::change_user,this,[=]{
        m_mainwind->hide();
        this->show();
    });

}

Login::~Login()
{
    delete ui;
}


/**
 * @brief  设置登录用户的Json包
 *
 * @param user 用户名
 * @param pwd 密码
 *
 *
 * @returns
 *      成功: 返回登录信息的json字符串
 *      失败：空字符串
 */
QByteArray Login::setLoginJson(QString user, QString pwd)
{
    QMap<QString,QVariant> json_login;
    json_login.insert("user",user);
    json_login.insert("pwd",pwd);

    //json格式
    /*
    {
        user:xxx
        pwd:xxx
    }
    */

    QJsonDocument json_doc = QJsonDocument::fromVariant(json_login);
    if(json_doc.isNull()){
        cout<<"json_doc is NULL!";
        return "";
    }

    return json_doc.toJson();
}

/**
 * @brief  设置注册用户的Json包
 *
 * @param user_name 用户名
 * @param nick_name 昵称
 * @param first_pwd 密码
 * @param phone     电话
 * @param email     邮件
 *
 *
 * @returns
 *      成功: 返回注册信息的json字符串
 *      失败：空字符串
 */
QByteArray Login::setRegisterJson(QString user_name,
                                  QString nick_name,
                                  QString first_pwd,
                                  QString phone,
                                  QString email)
{
    QMap<QString,QVariant> json_reg;
    json_reg.insert("user_name",user_name);
    json_reg.insert("nick_name",nick_name);
    json_reg.insert("first_pwd",first_pwd);
    json_reg.insert("phone",phone);
    json_reg.insert("email",email);

    //json格式
    /*
    {
        user_name:xxx
        nick_name:xxx
        ....
    }
    */
    QJsonDocument json_doc = QJsonDocument::fromVariant(json_reg);
    if(json_doc.isNull()){
        cout<<"json_doc is NULL!";
        return "";
    }

    return json_doc.toJson();
}

/**
 * @brief  获取服务器回复的登录状态
 *
 * @param json json字符串
 * @param
 *
 *
 * @returns
 *      成功: 返回状态码和token
 *      失败：空字符串列表
 */
QStringList Login::getLoginStatus(QByteArray json)
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
            list.append(obj.value("code").toString());
            list.append(obj.value("token").toString());

        }
    }
    else{
        cout<<"erro";

    }

    return list;

}

//画背景图
void Login::paintEvent(QPaintEvent* ev)
{

    //给窗口添加背景图  
    QPainter p(this);
    QPixmap bg(":/image2/title_bk.jpg");

    p.drawPixmap(0,0,this->width(),this->height(),bg);

}
/**
 * @brief  点击登陆按钮
 *
 * @param
 * @param
 *
 *
 * @returns
 *
 *
 */
void Login::on_toolButton_login_clicked()
{
    //获取信息
    QString user = ui->login_name->text();
    QString pwd = ui->login_pwd->text();
    QString addr = ui->lineEdit_serv_addr->text();
    QString port = ui->lineEdit_serv_port->text();

    //数据校验
    QRegExp regexp_usr(USER_REG);
    if(!regexp_usr.exactMatch(user)){
        QMessageBox::warning(this, "警告", "用户名格式不正确");
        ui->login_name->clear();
        ui->login_name->setFocus();
    }
    QRegExp regexp_pwd(PASSWD_REG);
    if(!regexp_pwd.exactMatch(pwd)){
        QMessageBox::warning(this, "警告", "密码格式不正确");
        ui->login_pwd->clear();
        ui->login_pwd->setFocus();
    }

    //将登录信息写入cfg.json
    //登录信息加密
    m_cm.writeLoginInfo(user,pwd,ui->checkBox_isRem->isChecked());
    QByteArray array = setLoginJson(user, m_cm.getStrMd5(pwd));

    //设置登录的url
    QNetworkRequest req;
    QString url = QString("http://%1:%2/login").arg(addr).arg(port);
    req.setUrl(QUrl(url));

    //设置请求头
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));
    req.setHeader(QNetworkRequest::ContentLengthHeader,QVariant(array.size()));
    //向服务器发送post请求
    QNetworkReply* rep = m_manager->post(req,array);
    cout<<"post url:"<< url << "post data:"<< array;

    //接收服务器发回的http响应
    connect(rep,&QNetworkReply::finished,[=](){
        //出错
        if(rep->error() != QNetworkReply::NoError){
            cout<< rep->errorString();
            //释放资源
            rep->deleteLater();
            return;
        }

        //读出serv返回的数据,code和token
        QByteArray json = rep->readAll();

        //成功返回{"code",000}
        //失败返回{"code",001}
        cout<<"server return value:"<<json;
        QStringList tmp_list = getLoginStatus(json);

        if(tmp_list.at(0) == "000"){
            cout<<"登录成功";

            //设置登录信息，文件列表需要用实例
            LoginInfoInstance* p_instance = LoginInfoInstance::get_instance();
            p_instance->setLoginInfo(user,addr,port,tmp_list.at(1));

            cout<<p_instance->get_user().toUtf8().data()<<","<<p_instance->get_ip()
                  <<","<<p_instance->get_port()<<","<<p_instance->get_token();
            //隐藏此界面
            this->hide();
            //主界面显示


        }



    });








}

//服务器设置
void Login::on_toolButton_set_yes_clicked()
{



}

void Login::on_toolButton_reg_clicked()
{

}
