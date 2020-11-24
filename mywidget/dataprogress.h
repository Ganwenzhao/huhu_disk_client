#ifndef DATAPROGRESS_H
#define DATAPROGRESS_H

#include <QWidget>

namespace Ui {
class DataProgress;
}

class DataProgress : public QWidget
{
    Q_OBJECT

public:
    explicit DataProgress(QWidget *parent = 0);
    ~DataProgress();
    //设置文件名
    void set_filename(QString name = "test");
    //设置进度条的当前值
    void set_progress(uint val = 0, uint end = 100);

private:
    Ui::DataProgress *ui;
};

#endif // DATAPROGRESS_H
