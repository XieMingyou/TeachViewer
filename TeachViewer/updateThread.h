#ifndef UPDATETHREAD_H
#define UPDATETHREAD_H
#include <QThread>
#include <QStandardItem>

class UpdateThread : public QThread
{
    Q_OBJECT

public:
    explicit UpdateThread(QObject *parent = nullptr);
    void run(); //run函数是该线程的执行函数，只要这个线程被启动，那么该函数就会被调用

    QStandardItemModel *tablemodel_coordinate;//位置管理模块的模型
    int *coordinateShow;//位置管理模块中选中要显示的坐标系标志，1：伺服，2：关节，3：世界
};

#endif // UPDATETHREAD_H
