#ifndef UPLOADTASK_H
#define UPLOADTASK_H

#include "common.h"
#include <QVBoxLayout>
#include <QFile>
#include "mywidget/dataprogress.h"
#include "downloadlayout.h"

//上传文件信息
struct UploadFileInfo{

    QString md5;//文件md5码
    QFile* file;//文件指针
    QString file_name;//文件名
    quint64 size;//文件大小
    QString path;//文件路径
    bool isUpload;//是否在上传
    DataProgress* dp;//上传进度条

};

//单例模式，一个程序只能有一个上传任务列表

class UploadTask
{
public:
    static UploadTask* get_instance();//保证唯一一个实例
    static void destory();

    //追加上传文件到上传文件列表
    int append_upload_list(QString path);

    bool is_empty();//判断上传队列是否为空
    bool is_upload();//是否有文件正在上传

    //取出最前的上传任务，如果任务队列没有任务正在上传，设置第0个任务上传
    UploadFileInfo* take_task();
    //删除上传完成的任务
    void delete_upload_task();
    //清空上传的文件列表
    void clear_list();

private:
    UploadTask(){}
    ~UploadTask(){}

    static UploadTask* instance;
    //class garbo唯一作用就是在析构函数中删除单例
    class Garb{
    public:
        ~Garb(){
            UploadTask::destory();
        }
    };

    //定义一个static变量，程序结束时候会自动调用它的析构
    //静态数据成员，类中声明，类外定义
    static Garb tmp_garb;

    //上传任务列表
    QList<UploadFileInfo*> list;
};

#endif // UPLOADTASK_H
