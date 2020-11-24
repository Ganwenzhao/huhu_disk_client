#ifndef TRANS_H
#define TRANS_H

#include <QWidget>

namespace Ui {
class Trans;
}

class Trans : public QWidget
{
    Q_OBJECT

public:
    explicit Trans(QWidget *parent = 0);
    ~Trans();

private:
    Ui::Trans *ui;
};

#endif // TRANS_H
