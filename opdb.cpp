#include "opdb.h"
#include <QSqlDatabase>
#include <QSqlQuery>
opDb::opDb(QObject *parent)
    : QObject{parent}
{
    db=QSqlDatabase::addDatabase("QMYSQL");//连接的数据库类型
    init();

}

opDb::~opDb()
{
    db.close();
}

opDb &opDb::getintance()
{
    static opDb instance;
    return instance;
}

void opDb::init()
{
    db.setHostName("127.0.0.1"); //设置数据库的主机ip
    //设置数据库的用户名
    db.setUserName("root");
    //设置数据库的密码
    db.setPassword("123456");    //这个就是安装MySQL时设置的密码
    //设置数据库的名字
    db.setDatabaseName("cloud");
    //打开数据库（已经安装过mysql驱动了）
    if(db.open()==false){
        qDebug()<<"数据库打开失败";
    }
}

bool opDb::handleRegist(QString name, QString pwd)
{
    if(name==""||pwd=="") return false;
    qDebug()<<"注册    "<<name<<"  "<<pwd;
    QSqlQuery sqlQuery;
    QString str="insert into usrinfo(name,pwd) values('"+name+"','"+pwd+"')";
    qDebug()<<str;
    bool t=sqlQuery.exec(str);
    return t;
}

bool opDb::handleLogin(QString name, QString pwd)
{
    if(name==""||pwd=="") return false;
    QSqlQuery query;
    QString str="select pwd from usrinfo where name='"+name+"'";
    bool t=query.exec(str);
    query.next();
    if(t){
        if(pwd==query.value(0).toString()){
            QString str1="update  usrinfo set online=1 where name='"+name+"'";//将状态改为在线
            query.exec(str1);
            return true;
        }
        else return false;
    }
    return false;
}

bool opDb::handleDel(QString name, QString pwd)
{
    if(name==""||pwd=="") return false;
    QSqlQuery query;
    qDebug()<<"注销    "<<name<<"  "<<pwd;
    QString str="select pwd from usrinfo where name='"+name+"'";
    bool t=query.exec(str);
    query.next();
    if(!t) return false;
    if(query.value(0).toString()==pwd){
        QString str1="delete from usrinfo where name='"+name+"'";
       bool t1=query.exec(str1);
        return t1;
    }
    return false;
}

bool opDb::handExit(QString name)
{
    if(name=="") return false;
    QSqlQuery query;
    QString str1="update  usrinfo set online=0 where name='"+name+"'";//将状态改为离线
    bool t=query.exec(str1);
    return t;
}

QStringList opDb::onlineusr()
{
    QSqlQuery query;
    QString str1="select name from usrinfo where online=1";//将状态改为离线
    query.exec(str1);
    QStringList list;
    list.clear();
    while(query.next()){
        list.push_back(query.value(0).toString());
    }
    return list;
}

int opDb::findusr(QString name)
{
    QSqlQuery query;
    QString str1="select online from usrinfo where name='"+name+"'";//将状态改为离线
    query.exec(str1);
    if(query.next()){
        return query.value(0).toInt();
    }else{
        return -1;
    }
}

int opDb::handAddFriend(QString name1,QString name2)//name1申请方  name2被申请方
{
    if(name1==name2) return 3;
    qDebug()<<"操作数据库"<<name1<<" "<<name2;
    QSqlQuery query1;
    QString str1="select * from friend where id=(select id from usrinfo where name='"+name1+"') and friendid=(select id from usrinfo where name='"+name2+"')";
    query1.exec(str1);
    QSqlQuery query2;
    QString str2="select * from friend where id=(select id from usrinfo where name='"+name2+"') and friendid=(select id from usrinfo where name='"+name1+"')";
    query2.exec(str2);
    qDebug()<<"mysql正在判断是否为好友";
    if(query1.next()||query2.next()){//是好友
        return -1;//双方已经是好友
    }else{//不是好友
        QSqlQuery query3;
        qDebug()<<name2;
        query3.exec("select online from usrinfo where name='"+name1+"'");
        query3.next();
        qDebug() << "判断是否在线" ;
        return query3.value(0).toInt(); // 用户状态
    }
}

bool opDb::sureAddFriend(QString name1, QString name2)
{
    QSqlQuery query1;
    qDebug()<<"建立好友："<<name1<<"  "<<name2;
    QString str1="select id from usrinfo where name='"+name1+"'";
    query1.exec(str1);
    query1.next();
    QString id1=query1.value(0).toString();
    str1="select id from usrinfo where name='"+name2+"'";
    query1.exec(str1);
    query1.next();
    QString id2=query1.value(0).toString();
    str1="insert into friend values("+id1+","+id2+")";
    query1.exec(str1);
    if(query1.next()){
        return true;
    }else{
        return false;
    }
}

QStringList opDb::refreshFriend(QString name)
{
    QSqlQuery query1;
    QStringList str1;
    QString str="select name from usrinfo where id=(select id from friend where friendid=(select id from usrinfo where name='"+name+"')) AND online=1 ";
    query1.exec(str);
    while(query1.next()){
        qDebug()<<query1.value(0);
        str1.append(query1.value(0).toString());
    }
    str="select name from usrinfo where id=(select friendid from friend where id=(select id from usrinfo where name='"+name+"')) AND online=1";
    query1.exec(str);
    while(query1.next()){
        qDebug()<<query1.value(0).toString();
        str1.append(query1.value(0).toString());
    }
    return str1;
}

bool opDb::handDelFriend(QString name1, QString name2)
{
    QSqlQuery query1;
    QString str="delete from friend where id=(select id from usrinfo where name='"+name1+"') and friendid=(select id from usrinfo where name='"+name2+"')";
    query1.exec(str);
    str="delete from friend where id=(select id from usrinfo where name='"+name2+"') and friendid=(select id from usrinfo where name='"+name1+"')";
    query1.exec(str);
    if(query1.next()) return true;
    else return false;
}

