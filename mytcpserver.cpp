#include "mytcpserver.h"

Mytcpserver::Mytcpserver() {


}
Mytcpserver &Mytcpserver::getInstance()
{
    static Mytcpserver instance;
    return instance;
}
void Mytcpserver::incomingConnection(qintptr socketDescriptor)
{
    MyTcpSocket* tcpsocket=new MyTcpSocket;
    tcpsocket->setSocketDescriptor(socketDescriptor);
    connect(tcpsocket,&MyTcpSocket::userexit,[=](MyTcpSocket* tcp){
        auto it=Mytcpserver::getInstance().socketlist.begin();
        while(*it!=tcp) it++;
        delete *it;
        *it=NULL;
        Mytcpserver::getInstance().socketlist.erase(it);
    });
    Mytcpserver::getInstance().socketlist.append(tcpsocket);
}
