#include "uploadtask.h"
#include <QFileInfo>
#include "common.h"
#include "uploadlayout.h"

//初始化静态成员
UploadTask::Garb UploadTask::tmp_garb;
UploadTask* UploadTask::instance = new UploadTask;

UploadTask *UploadTask::get_instance()
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
void UploadTask::destory()
{
    if(UploadTask::instance != nullptr){

        UploadTask::instance->clear_list();

        delete UploadTask::instance;
        UploadTask::instance = nullptr;
        cout<<"UploadTask::instance is delete!";
    }
}
/**
 * @brief  将文件加入到上传列表
 *
 * @param path 	文件路径
 *
 *
 *
 * @returns
 *  成功：0
 *  失败：-1：文件过大
 *       -2：文件已经在上传列表中
 *       -3：打开文件失败
 *       -4：获取布局失败
 *
 *
 */
int UploadTask::append_upload_list(QString path)
{
    qint64 size = QFileInfo(path).size();
    //如果大于100M
    if(size > 100*1024*1024){
        cout<<"file is too big!";
        return -1;
    }
    //检查文件是否已经在上传队列中
    for(int i = 0; i < list.size(); i++ ){
        if(list.at(i)->path == path){
            cout<< QFileInfo(path).fileName()<<"已经在上传队列中";
            return -2;
        }
    }

    QFile *file = new QFile(path);//文件指针分配空间

    if(!file->open(QIODevice::ReadOnly)){
        //如果打开文件失败，则删除file
        cout<<"open file error";
        delete file;
        file = nullptr;
        return -3;
    }

    //获取文件属性
    QFileInfo info(path);
    Common cm;
    UploadFileInfo* tmp = new UploadFileInfo;

    tmp->md5 = cm.getFileMd5(path);
    tmp->file = file;
    tmp->file_name = info.fileName();
    tmp->size = size;
    tmp->path = path;
    tmp->isUpload = false;

    DataProgress *p = new DataProgress;//创建进度条
    p->set_filename(tmp->file_name);
    tmp->dp = p;

    //获取布局
    UploadLayout *p_upload = UploadLayout::get_instance();
    if(!p_upload){
        cout<<"UploadLayout::get_instance() err!";
        return -4;
    }
    QVBoxLayout * layout = (QVBoxLayout*)p_upload->get_upload_layout();

    layout->insertWidget(layout->count()-1, p);

    //加入列表
    list.append(tmp);
    cout<<tmp->file_name.toUtf8().data()<<"已经加入上传列表";

    return 0;

}
/**
 * @brief  判断上传队列是否为空
 *
 * @param
 *
 *
 *
 * @returns
 *
 *
 */
bool UploadTask::is_empty()
{
    return list.isEmpty();
}
/**
 * @brief  是否有文件正在上传
 *
 * @param
 *
 *
 *
 * @returns
 *
 *
 */
bool UploadTask::is_upload()
{
    //遍历队列
    for(int i = 0; i < list.size(); i++){
        if(list.at(i)->isUpload == true){
            return true;
        }

    }

    return false;
}

/**
 * @brief  取出最前的上传任务，如果任务队列没有任务正在上传，设置第0个任务上传
 *
 * @param
 *
 *
 *
 * @returns
 *
 *
 */
UploadFileInfo *UploadTask::take_task()
{
    //取出队首任务
    UploadFileInfo* first = list.at(0);
    list.at(0)->isUpload = true;
    return first;

}
/**
 * @brief  删除上传完成的任务
 *
 * @param
 *
 *
 *
 * @returns
 *
 *
 */
void UploadTask::delete_upload_task()
{
    //遍历队列
    for(int i = 0; i < list.size(); i++){
        if(list.at(i)->isUpload == true){
            //移除该文件
            UploadFileInfo* tmp = list.takeAt(i);
            //获取布局
            UploadLayout *p_upload = UploadLayout::get_instance();
            QLayout *layout = p_upload->get_upload_layout();
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
 * @brief  清空上传的文件列表
 *
 * @param
 *
 *
 *
 * @returns
 *
 *
 */
void UploadTask::clear_list()
{
    int n = list.size();
    for(int i = 0; i < n; i++){
        UploadFileInfo* tmp = list.at(i);
        delete tmp;
        tmp = nullptr;
    }
}




