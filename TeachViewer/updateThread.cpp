#include <QPushButton>
#include <QStandardItem>
#include <QString>
#include "widget.h"
#include "updateThread.h"
#include "dataDeclaration.h"
#include <Windows.h>
using namespace std;

UpdateThread::UpdateThread(QObject *parent) : QThread(parent)
{

}

void UpdateThread::run()
{
    while(true)
    {
        if(*coordinateShow == 1)
        {
            //显示伺服坐标
            QString str[6];
            str[0].setNum(drivePos.d1, 'f', 2);
            str[1].setNum(drivePos.d2, 'f', 2);
            str[2].setNum(drivePos.d3, 'f', 2);
            str[3].setNum(drivePos.d4, 'f', 2);
            str[4].setNum(drivePos.d5, 'f', 2);
            str[5].setNum(drivePos.d6, 'f', 2);
            for(int i = 0; i < 6; i++)
            {
                QStandardItem* item=new QStandardItem(str[i]);
                item->setTextAlignment(Qt::AlignCenter);
                tablemodel_coordinate->setItem(i, 1, item);
            }
        }
        else if(*coordinateShow == 2)
        {
            //显示关节坐标
            QString str[6];
            str[0].setNum(axisPos.a1, 'f', 2);
            str[1].setNum(axisPos.a2, 'f', 2);
            str[2].setNum(axisPos.a3, 'f', 2);
            str[3].setNum(axisPos.a4, 'f', 2);
            str[4].setNum(axisPos.a5, 'f', 2);
            str[5].setNum(axisPos.a6, 'f', 2);
            for(int i = 0; i < 6; i++)
            {
                QStandardItem* item=new QStandardItem(str[i]);
                item->setTextAlignment(Qt::AlignCenter);
                tablemodel_coordinate->setItem(i, 1, item);
            }
        }
        else if(*coordinateShow == 3)
        {
            //显示世界坐标
            QString str[6];
            str[0].setNum(cartPos.x, 'f', 2);
            str[1].setNum(cartPos.y, 'f', 2);
            str[2].setNum(cartPos.z, 'f', 2);
            str[3].setNum(cartPos.a, 'f', 2);
            str[4].setNum(cartPos.b, 'f', 2);
            str[5].setNum(cartPos.c, 'f', 2);
            for(int i = 0; i < 6; i++)
            {
                QStandardItem* item=new QStandardItem(str[i]);
                item->setTextAlignment(Qt::AlignCenter);
                tablemodel_coordinate->setItem(i, 1, item);
            }
        }
        Sleep(200); //间隔0.2s更新一次Ui界面上的数据
    }
}
