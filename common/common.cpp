#include <QFile>
#include <QMap>
#include <QDir>
#include <QTime>
#include <QFileInfo>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QApplication>
#include <QJsonDocument>
#include <QFileInfoList>
#include <QDesktopWidget>
#include <QCryptographicHash>
#include "des.h"
#include "common.h"

// 初始化变量

//类型图片的路径
QString Common::m_typePath = FILETYPEDIR;
//构造一个空的字符串列表
QStringList Common::m_typeList = QStringList();
//http类
QNetworkAccessManager* Common::m_netManager = new QNetworkAccessManager;

Common::Common(QObject *parent)
{
    Q_UNUSED(parent);
}
Common::~Common(){}

/**
 * @brief  窗口移动到屏幕中心
 *
 * @param tmp 	窗口对象指针
 *
 *
 *
 * @returns
 *
 *
 */

void Common::moveToCenter(QWidget *tmp)
{
    //显示窗口
    tmp->show();
    //获取屏幕信息
    QDesktopWidget* desktop = QApplication::desktop();
    //移动窗口
    tmp->move((desktop->width() - tmp->width())/2,(desktop->height() - tmp->height())/2);
}

/**
 * @brief  从配置文件中获取相应的参数
 *
 * @param title 	配置文件中title的名称
 * @param key       键
 * @param path      配置文件路径
 *
 * @returns
 *      成功:得到key对应的value
 *      失败：空串
 */
QString Common::getCfgValue(QString title, QString key, QString path)
{
    QFile file(path);

    //只读方式打开
    if(file.open(QIODevice::ReadOnly) == false){
        //打开失败
        cout<<"config file open err!";
        return "";
    }
    //读取所有内容得到json文本字符串
    QByteArray json = file.readAll();
    //关闭文件
    file.close();

    //报告解析错误的对象
    QJsonParseError err;

    //将json文本字符串转换为json文本对象
    QJsonDocument json_doc = QJsonDocument::fromJson(json, &err);
    //没有出错
    if(err.error == QJsonParseError::NoError){
        //如果json文本对象为NULL或者为空
        if(json_doc.isNull() || json_doc.isEmpty()){
            cout<<"json_doc is NULL || json_doc is empty!";
            return "";
        }
        //json文本对象不为空
        if(json_doc.isObject()){
            //先获取最外层的对象
            QJsonObject obj = json_doc.object();
            //获取title对应的json对象
            QJsonObject tmp = obj.value(title).toObject();
            //取得该对象的键列表
            QStringList list = tmp.keys();
            //遍历key列表，找到对应的key
            for(int i = 0; i < list.size(); i++){
                //如果找到
                if(list.at(i) == key){
                    return tmp.value(list.at(i)).toString();
                }
            }

        }
    }
    else{
        cout << "err = "<< err.errorString();
        return "";
    }
    return "";
}
/**
 * @brief  通过读取文件，得到文件类型，存放在m_typeList
 *
 *
 *
 *
 *
 * @returns
 *
 *
 */
void Common::getFileTypeList()
{
    //QDir类使用相对路径或者绝对路径来指向文件或目录
    QDir dir(m_typePath);
    //如果不存在
    if(!dir.exists()){
        dir.mkpath(m_typePath);
        cout << m_typePath<<"创建成功!";
    }
    //过滤文件
    dir.setFilter(QDir::Files|QDir::NoDot|QDir::NoDotDot|QDir::NoSymLinks);
    //文件排序
    dir.setSorting(QDir::Size|QDir::Reversed);

    QFileInfoList info_list = dir.entryInfoList();
    for(int i = 0; i < info_list.size(); i++){

        QFileInfo file_info = info_list.at(i);
        m_typeList.append(file_info.fileName());
    }
}
/**
 * @brief  获取文件类型对应图片的的路径
 *
 * @param type 文件类型.png
 *
 *
 *
 * @returns
 *      成功: 返回类型图片对应的文件路径
 *      失败：返回“路径+other.png”
 */
QString Common::getFileType(QString type)
{
    if(true == m_typeList.contains(type)){
        return m_typePath + "/" + type;
    }
    return m_typePath + "other.png";
}

/**
 * @brief  将登录信息写入配置文件
 *
 * @param user 用户名
 * @param pwd 密码
 * @param isRemember 是否记住密码
 * @param path 配置文件路径
 *
 * @returns
 *
 *
 */
void Common::writeLoginInfo(QString user, QString pwd, bool isRemember, QString path)
{
    //获取web_server信息
    QString ip = getCfgValue("web_serVer","ip");
    QString port = getCfgValue("web_server","port");

    QMap<QString, QVariant> web_server;
    web_server.insert("ip",ip);
    web_server.insert("port",port);

    //type_path信息
    QMap<QString, QVariant> type_path;
    type_path.insert("path",m_typePath);

    //login信息
    QMap<QString, QVariant> login;

    //登录信息des加密
    int ret = 0;

    unsigned char enc_usr[1024] = {0};
    int enc_usr_len;
    //toLocal8Bit(),转换为本地字符集，win-gbk，linux-utf8
    ret = DesEnc((unsigned char*)user.toLocal8Bit().data(), user.toLocal8Bit().size(), enc_usr, &enc_usr_len);
    //如果加密失败
    if(ret != 0){
        cout << "Usr DesEnc err";
        return;
    }

    unsigned char enc_pwd[512] = {0};
    int enc_pwd_len;
    //toLocal8Bit(),转换为本地字符集，win-gbk，linux-utf8
    ret = DesEnc((unsigned char*)user.toLocal8Bit().data(), user.toLocal8Bit().size(), enc_pwd, &enc_pwd_len);
    //如果加密失败
    if(ret != 0){
        cout << "Pwd DesEnc err";
        return;
    }

    //再次转码，通过base64加密,然后插入login
    login.insert("user",QByteArray((char*)enc_usr, enc_usr_len).toBase64());
    login.insert("pwd",QByteArray((char*)enc_pwd, enc_pwd_len).toBase64());

    //判断是否记住密码
    if(isRemember == true){
        login.insert("remember","yes");
    }else{
        login.insert("remember","no");
    }

    QMap<QString, QVariant> json;
    json.insert("web_server",web_server);
    json.insert("login",login);
    json.insert("type_path",type_path);

    QJsonDocument json_doc = QJsonDocument::fromVariant(json);

    if(json_doc.isNull() == true){
        cout<< "QJsonDocument.fromVariant(json) err!";
        return;
    }

    QFile file(path);
    if(false == file.open(QIODevice::WriteOnly)){
        cout<<"write login info file open err";
        return;
    }

    file.write(json_doc.toJson());
    file.close();

}

/**
 * @brief  将服务器信息写入配置文件
 *
 * @param ip   ip地址
 * @param port 端口
 * @param path 配置文件路径
 *
 * @returns
 *
 *
 */
void Common::writeWebInfo(QString ip, QString port, QString path)
{
    //web_server信息
    QMap<QString, QVariant> web_server;
    web_server.insert("ip",ip);
    web_server.insert("port",port);

    //type_path信息
    QMap<QString, QVariant> type_path;
    type_path.insert("path",m_typePath);

    //获取login信息
    QString user = getCfgValue("login", "user");
    QString pwd = getCfgValue("login", "pwd");
    QString remember = getCfgValue("login", "remember");

    //写入login信息
    QMap<QString, QVariant> login;
    login.insert("user", user);
    login.insert("pwd", pwd);
    login.insert("remember", remember);

    QMap<QString, QVariant> json;
    json.insert("web_server",web_server);
    json.insert("login",login);
    json.insert("type_path",type_path);

    QJsonDocument json_doc = QJsonDocument::fromVariant(json);

    if(json_doc.isNull() == true){
        cout<< "QJsonDocument.fromVariant(json) err!";
        return;
    }

    QFile file(path);

    if(false == file.open(QIODevice::WriteOnly)){
        cout<<"file open err";
        return;
    }

    file.write(json_doc.toJson());
    cout<<json_doc.toJson();
    cout<<"cfg.json has written!";
    file.close();

}

/**
 * @brief  获取某个文件的md5编码
 *
 * @param filePath   文件路径
 *
 *
 *
 * @returns
 *      成功：16进制的md5码
 *      失败： 0
 */
QString Common::getFileMd5(QString file_path)
{
    QFile local_file(file_path);
    if(local_file.open(QIODevice::ReadOnly) == false){
        qDebug()<<"open file err!";
        return 0;
    }

    QCryptographicHash ch(QCryptographicHash::Md5);

    quint64 total_bytes = 0;//总量
    quint64 bytes_written = 0;//已写
    quint64 bytes_to_write = 0;//待写
    quint64 load_size = 1024*4;//4k
    QByteArray buf;

    total_bytes = local_file.size();
    bytes_to_write = total_bytes;

    while(1){
        if(bytes_to_write > 0){
            buf = local_file.read(qMin(bytes_to_write, load_size));

            ch.addData(buf);

            bytes_written += buf.length();
            bytes_to_write -= buf.length();

            //清空buf
            buf.resize(0);
        }else{break;}

        //如果全部都加密了
        if(bytes_written == total_bytes){
            break;
        }
    }

    local_file.close();
    QByteArray md5 = ch.result();

    return md5.toHex();

}

/**
 * @brief  获取字符串的md5编码
 *
 * @param str   字符串
 *
 *
 *
 * @returns
 *      成功：16进制的md5码
 *
 */
QString Common::getStrMd5(QString str)
{
    QByteArray res;
    res = QCryptographicHash::hash(str.toLocal8Bit(),QCryptographicHash::Md5);

    return res.toHex();
}

/**
 * @brief  生产分隔线
 *
 *
 * @returns
 *      成功：分割线字符串
 *
 */
QString Common::getBoundary()
{
    //随机种子
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    QString tmp;

    //48~122，‘0’~'A'~‘z’
    for(int i = 0; i < 16; i++){
        tmp[i] = qrand()%(122 - 48) + 48;
    }

    return QString("------WebKitFormBoundary%1").arg(tmp);
}

/**
 * @brief  获得服务器回复的状态码
 *
 *
 * @param json 服务器返回的json字符串
 *
 *
 * @returns
 *      成功："000"或者"001"
 *      失败：空串
 *
 */
QString Common::getCode(QByteArray json)
{
    QJsonParseError err;

    QJsonDocument json_doc = QJsonDocument::fromJson(json, &err);

    if(err.error == QJsonParseError::NoError){
        if(json_doc.isNull() || json_doc.isEmpty()){
            cout<<"json_doc is NULL || json_doc is empty!";
            return "";
        }
        //如果最外层是对象
        if(json_doc.isObject()){
            QJsonObject obj = json_doc.object();
            return obj.value("code").toString();
        }
    }
    else{
        cout<< "err = "<<err.errorString();
    }

    return "";

}

/**
 * @brief  传输记录写到本地
 *
 * @param user  操作用户
 * @param name  操作的文件
 * @param code 操作码
 * @param path 文件保存的路径
 *
 * @returns
 *
 *
 */
void Common::writeRecord(QString user, QString name, QString code, QString path)
{
    QString file_name = path + user;

    //检查目录是否存在，不存在就建立
    QDir dir(path);
    if(!dir.exists()){
        //目录不存在，创建
        if(dir.mkpath(path)){
            cout << path << "——目录创建成功!";
        }else{
            cout << path << "——目录创建失败!";
        }
    }
    cout << "file_name = "<< file_name.toUtf8().data();

    QByteArray arr;
    QFile file(file_name);

    //如果文件存在，先读取原来的内容
    if(file.exists()){
        if(file.open(QIODevice::ReadOnly) == false){
            cout<<"QIODevice::ReadOnly err!";
            return;
        }
        //读取原来文件的内容
        arr = file.readAll();
        file.close();
    }
    //确认文件可写
    if(file.open(QIODevice::WriteOnly) == false){
        cout<<"QIODevice::WriteOnly err!";
        return;
    }

    //记录操作
    //xxx.png   2020年10月10日17:12:22     秒传成功
    //获取当前时间
    QDateTime time = QDateTime::currentDateTime();
    //设置显示格式
    QString time_str = time.toString("yyyy-MM-dd hh:mm:ss ddd");

    QString act_str;

    if(code == "005"){
        act_str = "秒传失败，文件已经存在";
    }else if(code == "006"){
        act_str = "秒传成功";
    }else if(code == "007"){
        act_str = "秒传失败";
    }else if(code == "008"){
        act_str = "上传成功";
    }else if(code == "009"){
        act_str = "上传失败";
    }else if(code == "010"){
        act_str = "下载成功";
    }else if(code == "011"){
        act_str = "下载失败";
    }

    QString str = QString("[%1]\t[%2]\t[%3]\r\n").arg(name).arg(time_str).arg(act_str);
    cout<< str.toUtf8().data();
    //先写新内容
    file.write(str.toLocal8Bit());
    if(!arr.isEmpty()){
        //再写原来的内容
        file.write(arr);
    }

    file.close();
}

/**
 * @brief  获得http通信类对象指针
 *
 *
 * @param
 *
 *
 * @returns
 *      成功：QNetworkAccessManager对象指针
 *
 *
 */
QNetworkAccessManager* Common::getNetManager(){
    return m_netManager;
}


