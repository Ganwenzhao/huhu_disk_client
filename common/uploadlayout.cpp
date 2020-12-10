#include "uploadlayout.h"

//初始化静态成员
UploadLayout* UploadLayout::instance = new UploadLayout;
UploadLayout::Garb UploadLayout::tmp_garb;


UploadLayout *UploadLayout::get_instance()
{
    return instance;
}

void UploadLayout::set_upload_layout(QWidget *p)
{
    m_wg = new QWidget(p);

    QLayout* layout = p->layout();
    cout<<"uploadlayout.cpp 18 success";
    //layout->addWidget(m_wg);
    cout<<"uploadlayout.cpp 20 success";
    //设置外边距
    //layout->setContentsMargins(0,0,0,0);
    //创建垂直布局管理器
    QVBoxLayout* vlayout = new QVBoxLayout;

    //布局设置给widget
    m_wg->setLayout(vlayout);
    vlayout->setContentsMargins(0,0,0,0);
    m_layout = vlayout;
    //添加弹簧
    vlayout->addStretch();
}

QLayout *UploadLayout::get_upload_layout()
{
    return m_layout;
}
