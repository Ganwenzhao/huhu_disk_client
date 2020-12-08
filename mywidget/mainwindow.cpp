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
void MainWindow::show_mainwindow(){
    //移动到中心
    m_main_cm.moveToCenter(this);
    //显示文件页面
    ui->stackedWidget->setCurrentWidget(ui->page_file);
    //刷新
    ui->page_file->refresh_files();

}

void MainWindow::manager_signals()
{
    //关闭
    connect(ui->btn_group, &ButtonGroup::close_window, this, &MainWindow::close);
    //最大化
    connect(ui->btn_group, &ButtonGroup::max_window, this, [=](){
        static bool flag = false;
        if(flag){
            this->showNormal();
        }else{
            this->showMaximized();
        }
        flag = !flag;

    });
    //最小化
    connect(ui->btn_group, &ButtonGroup::min_window, this, &MainWindow::showMinimized);
    //stackwidget切换
    //myfiles
    connect(ui->btn_group, &ButtonGroup::sig_myfile, [=](){
        ui->stackedWidget->setCurrentWidget(ui->page_file);
        ui->page_file->refresh_files();
    });
    //sharelist
    connect(ui->btn_group, &ButtonGroup::sig_sharelist, [=](){
        ui->stackedWidget->setCurrentWidget(ui->page_share);
        ui->page_share->refresh_files();
    });
    //trans
    connect(ui->btn_group, &ButtonGroup::sig_trans, [=](){
        ui->stackedWidget->setCurrentWidget(ui->page_trans);
    });
    //switch user
    connect(ui->btn_group, &ButtonGroup::sig_switch_user, [=](){
        login_again();
    });

    //switch stack
    connect(ui->page_file, &MyFiles::goto_transfer, [=](TransferStatus status){
        ui->btn_group->slot_button_click(Page::TRANSFER);
        if(status == TransferStatus::Upload){
            ui->page_trans->show_upload();
        }else if(status == TransferStatus::Download){
            ui->page_trans->show_download();
        }
    });
    connect(ui->page_share, &ShareList::goto_transfer, ui->page_file, &MyFiles::goto_transfer);
}

void MainWindow::login_again()
{
    emit switch_user();

    ui->page_file->clear_all_tasks();
    ui->page_file->clear_file_list();
    ui->page_file->clear_all_items();
}


void MainWindow::paintEvent(QPaintEvent *ev)
{
    //给窗口添加背景图
    Q_UNUSED(ev);
    QPainter p(this);
    QPixmap bg(":/image2/title_bk.jpg");

    p.drawPixmap(0,0,this->width(),this->height(),bg);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    //只允许左键拖动,持续的移动用buttons
    if(event->buttons() & Qt::LeftButton){
        //窗口跟随
        this->move(event->globalPos() - locate_differ);

    }
}

void MainWindow::mousePressEvent(QMouseEvent *ev)
{
    //如果鼠标左键按下
    if(ev->button() == Qt::LeftButton){
        locate_differ = ev->globalPos() - this->geometry().topLeft();
    }
}
