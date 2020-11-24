#include "logininfoinstance.h"

//初始化静态成员
LoginInfoInstance::Garb LoginInfoInstance::tmp_garb;
LoginInfoInstance* LoginInfoInstance::instance = new LoginInfoInstance;
//构造

LoginInfoInstance::LoginInfoInstance(){}
//析构
LoginInfoInstance::~LoginInfoInstance(){}
//拷贝构造
LoginInfoInstance::LoginInfoInstance(const LoginInfoInstance&){}
//赋值重载
LoginInfoInstance& LoginInfoInstance::operator=(const LoginInfoInstance &)
{
    return *this;
}
//获取唯一的实例
LoginInfoInstance *LoginInfoInstance::get_instance()
{
    return instance;
}
//释放空间
void LoginInfoInstance::destory()
{
    if(LoginInfoInstance::instance != nullptr){
        delete LoginInfoInstance::instance;
        LoginInfoInstance::instance = nullptr;
        cout<<"LoginInfoInstance::instance is delete!";
    }
}

void LoginInfoInstance::setLoginInfo(QString tmp_user, QString tmp_ip, QString tmp_port, QString token)
{
    this->user = tmp_user;
    this->ip = tmp_ip;
    this->port = tmp_port;
    this->token = token;
}

QString LoginInfoInstance::get_user() const
{
    return this->user;
}
QString LoginInfoInstance::get_ip() const
{
    return this->ip;
}
QString LoginInfoInstance::get_port() const
{
    return this->port;
}
QString LoginInfoInstance::get_token() const
{
    return this->token;
}


