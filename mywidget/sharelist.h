#ifndef SHARELIST_H
#define SHARELIST_H

#include <QWidget>

namespace Ui {
class ShareList;
}

class ShareList : public QWidget
{
    Q_OBJECT

public:
    explicit ShareList(QWidget *parent = 0);
    ~ShareList();

private:
    Ui::ShareList *ui;
};

#endif // SHARELIST_H
