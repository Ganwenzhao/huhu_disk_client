#ifndef LOGININFOINSTANCE_H
#define LOGININFOINSTANCE_H

#include <QString>
#include "common.h"
//单例模式，保存当前用户的登录信息
class LoginInfoInstance
{
public:
    static LoginInfoInstance* get_instance();//保证一个实例
    static void destory();

    void setLoginInfo(QString tmp_user, QString tmp_ip, QString tmp_port, QString token = "");//设置登陆信息

    QString get_user() const;//获取登录用户
    QString get_ip() const;//获取登录ip
    QString get_port() const;//获取登录port
    QString get_token() const;//获取登录token

private:
    //构造和析构函数私有
    LoginInfoInstance();
    ~LoginInfoInstance();
    //把构造函数和赋值运算符也设为私有，防止被复制
    LoginInfoInstance(const LoginInfoInstance&);
    LoginInfoInstance& operator=(const LoginInfoInstance& );

    //class garbo唯一作用就是在析构函数中删除单例
    class Garb{
    public:
        ~Garb(){
            LoginInfoInstance::destory();
        }
    };

    //定义一个static变量，程序结束时候会自动调用它的析构
    //静态数据成员，类中声明，类外定义
    static Garb tmp_garb;

    static LoginInfoInstance* instance;

    QString user;
    QString ip;
    QString port;
    QString token;
};

#endif // LOGININFOINSTANCE_H
