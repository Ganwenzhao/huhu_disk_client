#ifndef DOWNLOADTASK_H
#define DOWNLOADTASK_H

#include "common.h"
#include <QVBoxLayout>
#include <QFile>
#include <Qurl>
#include "mywidget/dataprogress.h"

//上传文件信息
struct DownloadFileInfo{

    QString md5;//文件md5码
    QString user;//下载用户
    QFile* file;//文件指针
    QString file_name;//文件名
    QUrl url;//下载路径
    bool isDownload;//是否在下载
    DataProgress* dp;//下载进度条
    bool isShared;//是否为共享文件下载

};

//单例模式，一个程序只能有一个下载任务列表

class DownloadTask
{
public:
    static DownloadTask* get_instance();//保证唯一一个实例
    static void destory();

    //追加下载文件到下载文件列表
    int append_download_list(FileInfo* info,QString path, bool is_shared);

    bool is_empty();//判断下载队列是否为空
    bool is_download();//是否有文件正在下载

    //取出最前的下载任务，如果任务队列没有任务正在下载，设置第0个任务下载
    DownloadFileInfo* take_task();
    //删除下载完成的任务
    void delete_download_task();
    //清空下载的文件列表
    void clear_list();

private:
    DownloadTask(){}
    ~DownloadTask(){}

    static DownloadTask* instance;
    //class garbo唯一作用就是在析构函数中删除单例
    class Garb{
    public:
        ~Garb(){
            DownloadTask::destory();
        }
    };

    //定义一个static变量，程序结束时候会自动调用它的析构
    //静态数据成员，类中声明，类外定义
    static Garb tmp_garb;

    //下载任务列表
    QList<DownloadFileInfo*> list;
};

#endif // DOWNLOADTASK_H
