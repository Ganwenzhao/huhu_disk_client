#ifndef BUTTONGROUP_H
#define BUTTONGROUP_H

#include <QWidget>
#include <QSignalMapper>
#include <QMap>
#include <QToolButton>

namespace Ui {
class ButtonGroup;
}

//class QToolButton;
enum Page{MYFILE, SHARE, TRANSFER, SWITCHUSR};

class ButtonGroup : public QWidget
{
    Q_OBJECT

public:
    explicit ButtonGroup(QWidget *parent = 0);
    ~ButtonGroup();

public slots:
    void slot_button_click(Page cur);
    void slot_button_click(QString text);
    void set_parent(QWidget* parent);

protected:
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

signals:
    void sig_myfile();
    void sig_sharelist();
    void sig_trans();
    void sig_switch_user();
    void close_window();
    void min_window();
    void max_window();

private:
    Ui::ButtonGroup *ui;

    QPoint m_pos;
    QWidget* m_parent;
    QSignalMapper* m_mapper;
    QToolButton* m_cur_btn;
    QMap<QString, QToolButton*> m_btns;
    QMap<Page, QString> m_pages;
};

#endif // BUTTONGROUP_H
