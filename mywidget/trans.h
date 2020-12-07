#ifndef TRANS_H
#define TRANS_H

#include <QWidget>
#include "common/common.h"

namespace Ui {
class Trans;
}

class Trans : public QWidget
{
    Q_OBJECT

public:
    explicit Trans(QWidget *parent = 0);
    ~Trans();

    void show_data_record(QString path = RECORDDIR);

    void show_upload();

    void show_download();
signals:
    //告诉主界面当前是哪个tab
    void current_tab_signal(QString);
private:
    Ui::Trans *ui;
};

#endif // TRANS_H
