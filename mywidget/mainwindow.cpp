#include "mywidget/mainwindow.h"
#include "ui_mainwindow.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <Qfile>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QMouseEvent>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //去掉边框
    this->setWindowFlags(windowFlags()|Qt::FramelessWindowHint);
    //为菜单栏设置父类
    ui->btn_group->setParent(this);
    //处理所有信号
    //manager_signals();
    //默认显示文件窗口
    ui->stackedWidget->setCurrentIndex(0);

}

MainWindow::~MainWindow()
{

    delete ui;
}
void MainWindow::show_mainwidow(){
    //移动到中心
    m_main_cm.moveToCenter(this);
    //显示文件页面
    ui->stackedWidget->setCurrentWidget(ui->page_file);
    //刷新


}

void MainWindow::paintEvent(QPaintEvent *ev)
{
    //给窗口添加背景图
    QPainter p(this);
    QPixmap bg(":/image2/title_bk.jpg");

    p.drawPixmap(0,0,this->width(),this->height(),bg);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    //只允许左键拖动,持续的移动用buttons
    if(event->buttons() & Qt::LeftButton){
        //窗口跟随
        this->move(event->globalPos() - locate_diff2);

    }
}

void MainWindow::mousePressEvent(QMouseEvent *ev)
{
    //如果鼠标左键按下
    if(ev->button() == Qt::LeftButton){
        locate_diff2 = ev->globalPos() - this->geometry().topLeft();
    }
}
