#ifndef TITLEWIDGET_H
#define TITLEWIDGET_H

#include <QWidget>

namespace Ui {
class TitleWidget;
}

class TitleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TitleWidget(QWidget *parent = 0);
    ~TitleWidget();
protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent* event);
signals:
    void show_set_widget();

private:
    Ui::TitleWidget *ui;
    QPoint locate_diff;//鼠标当前位置-窗口左上角位置
    QWidget* m_parent;
};

#endif // TITLEWIDGET_H
