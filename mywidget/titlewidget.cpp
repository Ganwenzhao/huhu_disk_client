#include "mywidget/titlewidget.h"
#include "ui_titlewidget.h"
#include <QMouseEvent>

TitleWidget::TitleWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TitleWidget)
{
    ui->setupUi(this);
    ui->logo->setPixmap(QPixmap(":/image2/main_logo.png"));
    ui->set->setStyleSheet("border:none;");
    ui->minimum->setStyleSheet("border:none;");
    ui->close->setStyleSheet("border:none;");

    m_parent = parent;

    //按钮功能
    connect(ui->set,&QToolButton::clicked,this,[=](){
        //发送自定义信号
        emit show_set_widget();
    });

    connect(ui->minimum,&QToolButton::clicked,this,[=](){
        m_parent->showMinimized();

    });

    connect(ui->close,&QToolButton::clicked,this,[=](){
        m_parent->close();

    });

}

TitleWidget::~TitleWidget()
{
    delete ui;
}

void TitleWidget::mouseMoveEvent(QMouseEvent *event)
{
    //只允许左键拖动,持续的移动用buttons
    if(event->buttons() & Qt::LeftButton){
        //窗口跟随
        m_parent->move(event->globalPos() - locate_diff);

    }
}

void TitleWidget::mousePressEvent(QMouseEvent *ev)
{
    //如果鼠标左键按下
    if(ev->button() == Qt::LeftButton){
        locate_diff = ev->globalPos() - m_parent->geometry().topLeft();
    }
}
