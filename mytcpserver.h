#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
#include <QList>
#include "mytcpsocket.h"
class Mytcpserver : public QTcpServer
{
    Q_OBJECT
public:
    Mytcpserver();
    static Mytcpserver &getInstance();
    virtual void incomingConnection(qintptr socketDescriptor);
    QList<MyTcpSocket*> socketlist;
};

#endif // MYTCPSERVER_H
