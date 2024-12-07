#ifndef OPDB_H
#define OPDB_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringList>
class opDb : public QObject
{
    Q_OBJECT
public:
    explicit opDb(QObject *parent = nullptr);
    ~ opDb();
    static opDb& getintance();
    void init();
    bool handleRegist(QString name,QString pwd);
    bool handleLogin(QString name,QString pwd);
    bool handleDel(QString name,QString pwd);
    bool handExit(QString name);
    QStringList onlineusr();
    int findusr(QString name);
    int handAddFriend(QString name1,QString name2);//添加好友请求
    bool sureAddFriend(QString name1,QString name2);//确认添加好友，处理添加好友
    QStringList refreshFriend(QString name);
    bool handDelFriend(QString name1,QString name2);
signals:
private:
    QSqlDatabase db;
};

#endif // OPDB_H
