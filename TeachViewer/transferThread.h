#ifndef TRANSFERTHREAD_H
#define TRANSFERTHREAD_H
#include<QThread>

class TransferThread : public QThread
{
    Q_OBJECT

public:
    explicit TransferThread(QObject *parent = nullptr);
    void run(); //run函数是该线程的执行函数，只要这个线程被启动，那么该函数就会被调用
};

#endif // TRANSFERTHREAD_H
