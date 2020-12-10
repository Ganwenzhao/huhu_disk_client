#include "downloadtask.h"
#include <QFileInfo>
#include "common.h"
#include "downloadlayout.h"

//初始化静态成员
DownloadTask::Garb DownloadTask::tmp_garb;
DownloadTask* DownloadTask::instance = new DownloadTask;

DownloadTask *DownloadTask::get_instance()
{
    return instance;
}
/**
 * @brief  销毁单例的辅助函数
 *
 * @param
 *
 *
 *
 * @returns
 *
 *
 */
void DownloadTask::destory()
{
    if(DownloadTask::instance != nullptr){

        DownloadTask::instance->clear_list();

        delete DownloadTask::instance;
        DownloadTask::instance = nullptr;
        cout<<"DownloadTask::instance is delete!";
    }
}
/**
 * @brief  将文件加入到下载列表
 *
 * @param info 	文件信息指针
 * @param path 	文件路径
 * @param is_shared 是否已经分享
 *
 * @returns
 *  成功：0
 *  失败：-1：下载的文件已经在下载队列中
 *       -2：打开文件失败
 *
 *
 *
 *
 */
int DownloadTask::append_download_list(FileInfo* info, QString path, bool is_shared)
{
    //检查文件是否已经在下载队列中
    for(int i = 0; i < list.size(); i++ ){
        if(list.at(i)->user == info->user && list.at(i)->file_name == info->filename){
            cout<< info->filename<<"已经在下载队列中";
            return -1;
        }
    }

    QFile *file = new QFile(path);//文件指针分配空间

    if(!file->open(QIODevice::WriteOnly)){
        //如果打开文件失败，则删除file
        cout<<"open file error";
        delete file;
        file = nullptr;
        return -2;
    }

    //获取文件属性

    DownloadFileInfo* tmp = new DownloadFileInfo;

    tmp->md5 = info->md5;
    tmp->user = info->user;
    tmp->file = file;
    tmp->file_name = info->filename;
    tmp->url = info->url;
    tmp->isShared = is_shared;

    DataProgress *p = new DataProgress;//创建进度条
    p->set_filename(tmp->file_name);
    tmp->dp = p;

    //获取布局
    DownloadLayout *p_download = DownloadLayout::get_instance();

    QVBoxLayout * layout = (QVBoxLayout*)p_download->get_download_layout();
    //添加到布局，最后一个是弹簧，插入到弹簧上面
    layout->insertWidget(layout->count()-1, p);

    //加入列表
    list.append(tmp);
    cout<<tmp->url<<"已经加入下载列表";

    return 0;

}
/**
 * @brief  判断下载队列是否为空
 *
 * @param
 *
 *
 *
 * @returns
 *
 *
 */
bool DownloadTask::is_empty()
{
    return list.isEmpty();
}
/**
 * @brief  是否有文件正在下载
 *
 * @param
 *
 *
 *
 * @returns
 *
 *
 */
bool DownloadTask::is_download()
{
    //遍历队列
    for(int i = 0; i < list.size(); i++){
        if(list.at(i)->isDownload == true){
            return true;
        }

    }

    return false;
}
/**
 * @brief  第一个任务是不是共享文件的任务
 *
 * @param
 *
 *
 *
 * @returns
 *
 *
 */
bool DownloadTask::is_share_task()
{
    if(this->is_empty()){
        return false;
    }
    return list.at(0)->isShared;
}

/**
 * @brief  取出最前的下载任务，如果任务队列没有任务正在下载，设置第0个任务下载
 *
 * @param
 *
 *
 *
 * @returns 下载文件信息指针
 *
 *
 */
DownloadFileInfo *DownloadTask::take_task()
{
    if(this->is_empty()){
        return nullptr;
    }
    //取出队首任务
    DownloadFileInfo* first = list.at(0);
    list.at(0)->isDownload = true;

    return first;

}
/**
 * @brief  删除下载完成的任务
 *
 * @param
 *
 *
 *
 * @returns
 *
 *
 */
void DownloadTask::delete_download_task()
{
    //遍历队列
    for(int i = 0; i < list.size(); i++){
        if(list.at(i)->isDownload == true){
            //移除该文件
            DownloadFileInfo* tmp = list.takeAt(i);
            //获取布局
            DownloadLayout *p_download = DownloadLayout::get_instance();
            QLayout *layout = p_download->get_download_layout();
            //移除进度条
            layout->removeWidget(tmp->dp);

            //关闭打开的文件指针
            QFile *file = tmp->file;
            file->close();

            delete file;
            file = nullptr;
            delete tmp->dp;
            tmp->dp = nullptr;
            delete tmp;
            tmp = nullptr;

            return;
        }

    }
}
/**
 * @brief  清空下载的文件列表
 *
 * @param
 *
 *
 *
 * @returns
 *
 *
 */
void DownloadTask::clear_list()
{
    for(int i = 0; i < list.size(); i++){
        DownloadFileInfo* tmp = list.at(i);
        delete tmp;
        tmp = nullptr;
    }
}




