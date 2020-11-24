#ifndef UPLOADLAYOUT_H
#define UPLOADLAYOUT_H

#include "common.h"
#include <QVBoxLayout>

//上传进度布局类
class UploadLayout
{
public:
    static UploadLayout* get_instance();//保证唯一一个实例
    void set_upload_layout(QWidget* p);//设置布局
    QLayout* get_upload_layout();//获取布局

private:
    UploadLayout(){}
    ~UploadLayout(){}

    static UploadLayout* instance;

    QLayout *m_layout;
    QWidget *m_wg;

    //class garbo唯一作用就是在析构函数中删除单例
    class Garb{
    public:
        ~Garb(){
            if(UploadLayout::instance != nullptr){

                delete UploadLayout::instance;
                UploadLayout::instance = nullptr;

                cout<<"UploadLayout::instance is delete!";
            }
        }
    };

    //定义一个static变量，程序结束时候会自动调用它的析构
    //静态数据成员，类中声明，类外定义
    static Garb tmp_garb;
};

#endif // UPLOADLAYOUT_H
