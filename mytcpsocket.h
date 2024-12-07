#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include <QFile>
#include <QTimer>
class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT;
public:
    explicit MyTcpSocket(QObject *parent = nullptr);
    void recvmeg();
    void downfile();
    QString name;

    QString downpath;
    QFile mfile;
    qint64 total;
    qint64 mrecve;
    bool upload;

    QTimer* timer;
signals:
    void userexit(MyTcpSocket* tcp);
};

#endif // MYTCPSOCKET_H
