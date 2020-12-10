#ifndef DOWNLOADLAYOUT_H
#define DOWNLOADLAYOUT_H

#include "common.h"
#include <QVBoxLayout>

//上传进度布局类
class DownloadLayout
{
public:
    static DownloadLayout* get_instance();//保证唯一一个实例
    void set_download_layout(QWidget* p);//设置布局
    QLayout* get_download_layout();//获取布局

private:
    DownloadLayout(){}
    ~DownloadLayout(){}

    static DownloadLayout* instance;

    QLayout *m_layout;
    QWidget *m_wg;

    //class garbo唯一作用就是在析构函数中删除单例
    class Garb{
    public:
        ~Garb(){
            if(DownloadLayout::instance != NULL){

                delete DownloadLayout::instance;
                DownloadLayout::instance = NULL;

                cout<<"DownloadLayout::instance is delete!";
            }
        }
    };

    //定义一个static变量，程序结束时候会自动调用它的析构
    //静态数据成员，类中声明，类外定义
    static Garb tmp_garb;
};

#endif // DOWNLOADLAYOUT_H
