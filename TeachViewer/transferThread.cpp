#include <WinSock2.h>
#include<string>
#include "transferThread.h"
#include "dataDeclaration.h"
#include "dataProto.pb.h"
#include "ZipPack.h"
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"ZipPack.lib")
#define BUF_SIZE 1024
using namespace std;

TransferThread::TransferThread(QObject *parent) : QThread(parent)
{

}

void TransferThread::run()
{
    WSADATA wsd; //用于初始化Windows socket
    SOCKET sHost; //与服务器进行通信的socket
    SOCKADDR_IN servAddr; //服务器地址
    int retVal; //调用各种Socket函数的返回值

    //初始化socket动态库
    if(WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
    {
        std::cout << "WSAStartup failed !" << std::endl;
        return;
    }

    //创建用于通信的socket
    sHost = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(INVALID_SOCKET == sHost)
    {
        std::cout << "socket failed !" << std::endl;
        closesocket(sHost);
        WSACleanup();
        return;
    }

    //设置服务器Socket地址
    servAddr.sin_family = AF_INET;
    //用户需要根据实际情况修改
    servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    servAddr.sin_port = htons(9990);
    int sServerAddrlen = sizeof(servAddr);

    //连接到服务器
    retVal = ::connect(sHost, (LPSOCKADDR)&servAddr, sizeof(servAddr));
    while(SOCKET_ERROR == retVal)
    {
        retVal = ::connect(sHost, (LPSOCKADDR)&servAddr, sizeof(servAddr));
    }
//    if(SOCKET_ERROR == retVal)
//    {
//        std::cout << "connect failed !" << std::endl;
//        closesocket(sHost);
//        WSACleanup();
//        return;
//    }

    int firstTimeTransferZip = 1; //判断是否在激活程序后首次传输文件压缩包
    //在客户端与服务器之间发送和接收数据
    while (true)
    {
        Client2Server client2Server;
        client2Server.set_emergestop(emergeStop);
        client2Server.set_enablestate(enableState);
        client2Server.set_operatemode(operateMode);
        client2Server.set_activestate(activeState);
        client2Server.set_runstate(runState);
        client2Server.set_step(step);
        client2Server.set_jog(jog);
        client2Server.set_coordinate(coordinate);
        client2Server.set_upordown(upOrDown);
        client2Server.set_ovr(ovr);
        client2Server.set_progline(progLine);
        client2Server.set_transferzip(transferZip);
        client2Server.set_zipsize(zipSize);
        client2Server.set_projname(projName);
        client2Server.set_varprogname(varProgName);

        //当程序被激活而文件压缩包还没传输的时候，transferZip置为1，提醒控制器接受文件压缩包
        if (activeState == 1 && firstTimeTransferZip == 1)
        {
            firstTimeTransferZip = 0;
            transferZip = 1;
            //压缩文件
            ZipPack::PackDirToZip(L"prog_var_xml",L"prog_var_xml.zip");
            FILE *fp = fp = fopen("prog_var_xml.zip", "rb"); //用二进制方式打开文件
            if( fp == NULL )
            {
                closesocket(sHost);
                WSACleanup();
                return;
            }

            fseek(fp, 0, SEEK_END);
            zipSize = ftell(fp);
            fseek(fp, 0, SEEK_SET);

            client2Server.set_transferzip(transferZip);
            client2Server.set_zipsize(zipSize);
            client2Server.set_projname(varProgName);
            client2Server.set_varprogname(varProgName);
            string str;
            client2Server.SerializeToString(&str);

            char buf_send[BUF_SIZE];
            for(int i = 0; i < str.length(); i++)
            {
                buf_send[i] = str[i];
            }
            //向服务器发送数据
            retVal = send(sHost, buf_send, BUF_SIZE, 0);
            if(SOCKET_ERROR == retVal)
            {
                std::cout << "send failed !" << std::endl;
                closesocket(sHost);
                WSACleanup();
                return;
            }
            transferZip = 0;
            client2Server.set_transferzip(transferZip);

            char buf[10];
            if(recv(sHost, buf, 2, 0) != 2)
            {
                closesocket(sHost);
                WSACleanup();
                exit(1);
            }
            char fileBuf[BUF_SIZE];
            int size, netSize;
            memset(fileBuf, 0, BUF_SIZE);
            while((size = fread(fileBuf, 1, BUF_SIZE, fp)) > 0)
            {
                int size2 = 0;
                while( size2 < size )
                {
                    if( (netSize = send(sHost, fileBuf + size2, size - size2, 0)) < 0 )
                    {
                        closesocket(sHost);
                        WSACleanup();
                        exit(1);
                    }
                    size2 += netSize;
                }
            }
            memset(buf, 0, sizeof(buf));
            if(recv(sHost, buf, 2, 0) != 2)
            {
                closesocket(sHost);
                WSACleanup();
                exit(1);
            }
            fclose(fp);
        }
        else if (activeState == 0)
        {
            firstTimeTransferZip = 1;
        }

        string str;
        client2Server.SerializeToString(&str);

        char buf_send[BUF_SIZE];
        for(int i = 0; i < str.length(); i++)
        {
            buf_send[i] = str[i];
        }
        //向服务器发送数据
        retVal = send(sHost, buf_send, BUF_SIZE, 0);
        if(SOCKET_ERROR == retVal)
        {
            std::cout << "send failed !" << std::endl;
            closesocket(sHost);
            WSACleanup();
            return;
        }

        Server2Client server2Client;

        char buf_recv[BUF_SIZE];
        retVal = recv(sHost, buf_recv, BUF_SIZE, 0);
        if(SOCKET_ERROR == retVal)
        {
            std::cout << "recv failed !" << std::endl;
            closesocket(sHost);
            WSACleanup();
            return;
        }
        //断开连接的条件
        if(strcmp(buf_recv, "quit") == 0)
        {
            std::cout << "Quit the data thread!" << std::endl;
            break;
        }
        std::string str_recv;
        str_recv.assign(buf_recv, BUF_SIZE);
        server2Client.ParseFromString(buf_recv);
        const AxisPos& axisPos = server2Client.axispos();
        const CartPos& cartPos = server2Client.cartpos();
        const DrivePos& drivePos = server2Client.drivepos();

        ::axisPos.a1 = axisPos.a1();
        ::axisPos.a2 = axisPos.a2();
        ::axisPos.a3 = axisPos.a3();
        ::axisPos.a4 = axisPos.a4();
        ::axisPos.a5 = axisPos.a5();
        ::axisPos.a6 = axisPos.a6();
        ::cartPos.x = cartPos.x();
        ::cartPos.y = cartPos.y();
        ::cartPos.z = cartPos.z();
        ::cartPos.a = cartPos.a();
        ::cartPos.b = cartPos.b();
        ::cartPos.c = cartPos.c();
        ::drivePos.d1 = drivePos.d1();
        ::drivePos.d2 = drivePos.d2();
        ::drivePos.d3 = drivePos.d3();
        ::drivePos.d4 = drivePos.d4();
        ::drivePos.d5 = drivePos.d5();
        ::drivePos.d6 = drivePos.d6();
        ::activeProg = server2Client.activeprog();
        ::runProg = server2Client.runprog();
        ::hasReachSingularity = server2Client.hasreachsingularity();
        for(int i = 0; i < server2Client.inforeport_size(); i++)
        {
            const InfoReport& infoReport = server2Client.inforeport(i);
            ::infoReport.push_back(pair<int, string>(infoReport.level(), infoReport.content()));
        }
        if (runProg == 1)
        {
            runState = 0;
        }
        if (activeProg == 1)
        {
            runState = 0;
            activeState = 0;
        }
    }

    //释放资源
    closesocket(sHost);
    WSACleanup();
}
