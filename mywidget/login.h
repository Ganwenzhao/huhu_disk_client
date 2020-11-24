#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <mywidget/mainwindow.h>
#include "common/common.h"
#include "common/logininfoinstance.h"

namespace Ui {
class Login;
}

class Login : public QDialog
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = 0);
    ~Login();

    //设置登录用户的json包
    QByteArray setLoginJson(QString user, QString pwd);

    //设置注册用户的Json包
    QByteArray setRegisterJson(QString user_name, QString nick_name, QString first_pwd, QString phone,  QString email);

    //得到服务器回复的登陆状态
    QStringList getLoginStatus(QByteArray json);

protected:
    //添加背景图,绘图事件函数
    void paintEvent(QPaintEvent* ev);

private slots:
    void on_toolButton_login_clicked();

    void on_toolButton_set_yes_clicked();

    void on_toolButton_reg_clicked();

private:
    Ui::Login *ui;
    //读取配置信息，设置默认登录状态和默认信息
    void read_cfg();
    //网络请求类对象
    QNetworkAccessManager* m_manager;
    //主窗口指针
    MainWindow* m_mainwind;
    Common m_cm;
};

#endif // LOGIN_H
