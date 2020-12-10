#include "buttongroup.h"
#include "ui_buttongroup.h"
#include <QPainter>
#include <QMouseEvent>


ButtonGroup::ButtonGroup(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ButtonGroup)
{
    ui->setupUi(this);

    m_mapper = new QSignalMapper(this);
    m_cur_btn = ui->myfiles;
    m_cur_btn->setStyleSheet("color:red");

    m_btns.insert(ui->myfiles->text(), ui->myfiles);
    m_btns.insert(ui->sharelist->text(), ui->sharelist);
    m_btns.insert(ui->transform->text(), ui->transform);
    m_btns.insert(ui->switchuser->text(), ui->switchuser);

    m_pages.insert(Page::MYFILE, ui->myfiles->text());
    m_pages.insert(Page::SHARE, ui->sharelist->text());
    m_pages.insert(Page::TRANSFER, ui->transform->text());
    m_pages.insert(Page::SWITCHUSR, ui->switchuser->text());

    //设置按钮信号映射
    QMap<QString, QToolButton*>::iterator it = m_btns.begin();
    for(; it != m_btns.end(); ++it){
        m_mapper->setMapping(it.value(), it.value()->text());
        connect(it.value(), SIGNAL(clicked(bool)), m_mapper, SLOT(map()));

    }
    connect(m_mapper, SIGNAL(mapped(QString)), this, SLOT(slot_button_click(QString)));
    //关闭
    connect(ui->close, &QToolButton::clicked, [=](){
       emit close_window();
    });
    //最大化
    connect(ui->max, &QToolButton::clicked, [=](){
        static bool flag = false;
        if(flag){
            ui->max->setIcon(QIcon(":/image2/title_max.png"));
        }else{
            ui->max->setIcon(QIcon(":/image2/title_normal.png"));
        }
        flag = !flag;

        emit max_window();
    });
    //最小化
    connect(ui->min, &QToolButton::clicked, [=](){
        emit min_window();
    });


}

ButtonGroup::~ButtonGroup()
{
    delete ui;
}

/**
 * @brief  按钮分发处理函数
 *
 * @param
 *
 *
 *
 * @returns
 *
 *
 */
void ButtonGroup::slot_button_click(Page cur)
{
    QString text = m_pages[cur];
    slot_button_click(text);
}

/**
 * @brief  实际按钮处理函数
 *
 * @param
 *
 *
 *
 * @returns
 *
 *
 */
void ButtonGroup::slot_button_click(QString text){

    QToolButton* btn = m_btns[text];
    if(btn == m_cur_btn && btn != ui->switchuser){
        return;
    }
    m_cur_btn->setStyleSheet("color:black");
    btn->setStyleSheet("color:red");

    m_cur_btn = btn;

    if(ui->myfiles->text() == text){
        emit sig_myfile();
    }else if(ui->sharelist->text() == text){
        emit sig_sharelist();
    }else if(ui->transform->text() == text){
        emit sig_trans();
    }else if(ui->switchuser->text() == text){
        emit sig_switch_user();
    }

}

/**
 * @brief  设置父对象
 *
 * @param
 *
 *
 *
 * @returns
 *
 *
 */
void ButtonGroup::set_parent(QWidget *parent)
{
    m_parent = parent;
}

/**
 * @brief  设置背景图
 *
 * @param
 *
 *
 *
 * @returns
 *
 *
 */
void ButtonGroup::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPixmap bg(":/image2/title_bk2.png");
    painter.drawPixmap(0, 0, width(), height(), bg);

}

/**
 * @brief  鼠标移动事件
 *
 * @param
 *
 *
 *
 * @returns
 *
 *
 */
void ButtonGroup::mouseMoveEvent(QMouseEvent *event)
{
    //只允许左键拖动,持续的移动用buttons
    if(event->buttons() & Qt::LeftButton){
        //窗口跟随
        QPoint pos = event->globalPos() - m_pos;
        m_parent->move(pos);

    }
}

/**
 * @brief  鼠标按下事件
 *
 * @param
 *
 *
 *
 * @returns
 *
 *
 */
void ButtonGroup::mousePressEvent(QMouseEvent *event)
{
    //如果鼠标左键按下
    if(event->button() == Qt::LeftButton){
        m_pos = event->globalPos() - m_parent->geometry().topLeft();
    }
}

