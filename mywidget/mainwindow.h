#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "common/common.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    //显示主窗口
    void show_mainwindow();
    //处理信号
    void manager_signals();
    //重新登录
    void login_again();

signals:
    //切换用户信号
    void switch_user();
protected:
    //添加背景图,绘图事件函数
    void paintEvent(QPaintEvent* ev);

private:
    Ui::MainWindow *ui;
    Common m_main_cm;
    QPoint locate_differ;//鼠标当前位置-窗口左上角位置
};

#endif // MAINWINDOW_H
