#include "downloadlayout.h"

//初始化静态成员
DownloadLayout::Garb DownloadLayout::tmp_garb;
DownloadLayout* DownloadLayout::instance = new DownloadLayout;

DownloadLayout *DownloadLayout::get_instance()
{
    return instance;
}

void DownloadLayout::set_download_layout(QWidget *p)
{
    m_wg = new QWidget(p);

    QLayout* layout = p->layout();
    layout->addWidget(m_wg);
    //设置外边距
    layout->setContentsMargins(0,0,0,0);
    //创建垂直布局管理器
    QVBoxLayout* vlayout = new QVBoxLayout;

    //布局设置给widget
    m_wg->setLayout(vlayout);
    vlayout->setContentsMargins(0,0,0,0);
    m_layout = vlayout;
    //添加弹簧
    vlayout->addStretch();
}

QLayout *DownloadLayout::get_download_layout()
{
    return m_layout;
}
