#include "mytcpsocket.h"
#include "protocol.h"
#include <QDebug>
#include "opdb.h"
#include "mytcpserver.h"
#include <QDir>
#include <QFileInfoList>
MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{
    connect(this,&MyTcpSocket::readyRead,this,&MyTcpSocket::recvmeg);
    connect(this,&MyTcpSocket::disconnected,this,[=]{
        opDb::getintance().handExit(this->name);
        emit this->userexit(this);
    });
    this->upload=false;
    this->timer=new QTimer;
    connect(this->timer,&QTimer::timeout,this,&MyTcpSocket::downfile);
}

void MyTcpSocket::recvmeg()
{
    if(!this->upload){
        unsigned int pdulen=0;
        this->read((char*)&pdulen,sizeof(unsigned int));
        unsigned int meglen=pdulen-sizeof(PDU);
        PDU* pdu=mkPDU(meglen);
        PDU* respdu;
        this->read((char*)pdu+sizeof(unsigned int),pdulen-sizeof(unsigned int));
        switch (pdu->msgType) {
        case MSG_TYPE_REGIST_REQUEST://注册请求
        {
            respdu=mkPDU(0);
            char name[32]={"\n"};
            char pwd[32]={"\n"};
            strncpy(name,pdu->cData,32);
            strncpy(pwd,pdu->cData+32,32);
            respdu->msgType=MSG_TYPE_REGIST_RESPOND;
            if(opDb::getintance().handleRegist(name,pwd)){//注册成功
                strcpy(respdu->cData,msg_ok);
                QDir dir;
                dir.mkdir("./"+QString(name));
            }else{//注册失败
                strcpy(respdu->cData,msg_failed);
            }
            write((char*)respdu,respdu->PDUlen);

        }
        break;
        case MSG_TYPE_LOGIN_REQUEST://登录请求
        {
            respdu=mkPDU(0);
            respdu->msgType=MSG_TYPE_LOGIN_RESPOND;
            char name[32]={"\n"};
            char pwd[32]={"\n"};
            strncpy(name,pdu->cData,32);
            strncpy(pwd,pdu->cData+32,32);
            bool t=opDb::getintance().handleLogin(name,pwd);
            if(t){
                strcpy(respdu->cData,msg_ok);
                this->name=QString(name);
            }else{
                strcpy(respdu->cData,msg_failed);
            }
            write((char*)respdu,respdu->PDUlen);
        }
        break;
        case MSG_TYPE_DEL_REQUEST://注销请求
        {
            respdu=mkPDU(0);
            respdu->msgType=MSG_TYPE_DEL_RESPOND;
            char name[32]={"\n"};
            char pwd[32]={"\n"};
            strncpy(name,pdu->cData,32);
            strncpy(pwd,pdu->cData+32,32);
            bool t=opDb::getintance().handleDel(name,pwd);
            if(t){
                strcpy(respdu->cData,msg_ok);
            }else{
                strcpy(respdu->cData,msg_failed);
            }
            write((char*)respdu,respdu->PDUlen);
        }
        break;
        case MSG_TYPE_EXIT_REQUEST://退出请求
        {
            char name[32]={"\n"};
            strncpy(name,pdu->cData,32);
            opDb::getintance().handExit(name);
            respdu=mkPDU(0);
        }
        break;
        case MSG_TYPE_ONLINE_REQUEST://查询在线请求
        {
            QStringList strlist=opDb::getintance().onlineusr();
            unsigned int msglen=strlist.size()*32;
            respdu=mkPDU(msglen);
            respdu->msgType=MSG_TYPE_ONLINE_RESPOND;
            for(int i=0;i<strlist.size();i++){
                memcpy((char*)(respdu->msg)+(32)*i,strlist[i].toStdString().c_str(),strlist[i].size());
            }
            write((char*)respdu,respdu->PDUlen);
        }
        break;
        case MSG_TYPE_SEARCHUSR_REQUEST://查找好友
        {
            char name[32];
            memcpy(name,pdu->cData,32);
            int t=opDb::getintance().findusr(name);
            respdu=mkPDU(0);
            respdu->msgType=MSG_TYPE_SEARCHUSR_RESPOND;
            memcpy(respdu->cData,(char*)&t,sizeof(int));
            write((char*)respdu,respdu->PDUlen);
        }
        break;
        case MSG_TYPE_FRIEND_REQUEST://添加好友
        {
            char name1[32]={"\n"};//对方的名字
            char name2[32]={"\n"};//自己的名字
            memcpy(name1,pdu->cData,32);
            memcpy(name2,pdu->cData+32,32);
            qDebug()<<"进入服务器"<<name1<<" "<<name2;
            int t=opDb::getintance().handAddFriend(name1,name2);
            qDebug()<<"双方是否为好友："<<t;
            respdu=mkPDU(0);
            respdu->msgType=MSG_TYPE_FRIEND_RESPOND;
            if(t==-1||t==0||t==3){
                memcpy(respdu->cData,(char*)&t,sizeof(int));//-1表示已经是好友 0表示用户不在线 1表示同意 2表示不同意 3 表示不能添加自己为好友
                write((char*)respdu,respdu->PDUlen);
            }else{
                for(int i=0;i<Mytcpserver::getInstance().socketlist.size();i++){
                    if(Mytcpserver::getInstance().socketlist[i]->name==name1){
                        qDebug()<<"准备发送";
                        Mytcpserver::getInstance().socketlist[i]->write((char*)pdu,pdu->PDUlen);
                        break;
                    }
                }
            }
        }
        break;
        case MSG_TYPE_AGREE_FRIEND_RESPOND://同意好友请求
        {
            char name1[32]={"\n"};//被请求方
            char name2[32]={"\n"};//请求方
            qDebug()<<"对方已同意好友请求";
            memcpy(name1,pdu->cData,32);
            memcpy(name2,pdu->cData+32,32);
            respdu=mkPDU(0);
            respdu->msgType=MSG_TYPE_FRIEND_RESPOND;
            int k=1;//同意好友请求
            qDebug()<<"准备进入数据库  "<<name1<<"  "<<name2;
            opDb::getintance().sureAddFriend(name1,name2);//将好友加入数据库
            memcpy(respdu->cData,(char*)&k,sizeof(int));
            for(int i=0;i<Mytcpserver::getInstance().socketlist.size();i++){
                if(Mytcpserver::getInstance().socketlist[i]->name==name2){
                    Mytcpserver::getInstance().socketlist[i]->write((char*)respdu,respdu->PDUlen);
                    break;
                }
            }
            write((char*)respdu,respdu->PDUlen);

        }
        break;
        case MSG_TYPE_DISAGREE_FRIEND_RESPOND://拒绝好友请求
        {
            char name1[32]={"\n"};//被请求方
            char name2[32]={"\n"};//请求方
            memcpy(name1,pdu->cData,32);
            memcpy(name2,pdu->cData+32,32);
            respdu=mkPDU(0);
            respdu->msgType=MSG_TYPE_FRIEND_RESPOND;
            int k=2;//不同意好友请求
            memcpy(respdu->cData,(char*)&k,sizeof(int));
            for(int i=0;i<Mytcpserver::getInstance().socketlist.size();i++){
                if(Mytcpserver::getInstance().socketlist[i]->name==name2){
                    Mytcpserver::getInstance().socketlist[i]->write((char*)respdu,respdu->PDUlen);
                    break;
                }
            }
            write((char*)respdu,respdu->PDUlen);
        }
        break;
        case MSG_TYPE_REFREASH_REQUEST://刷新好友
        {
            char name1[32]={"\n"};//姓名
            memcpy(name1,pdu->cData,32);
            QStringList str1=opDb::getintance().refreshFriend(name1);
            unsigned int k=str1.size()*32;
            respdu=mkPDU(k);
            respdu->msgType=MSG_TYPE_REFREASH_RESPOND;
            for(int i=0;i<str1.size();i++){
                qDebug()<<"准备写进   "<<str1[i];
                memcpy((char*)(respdu->msg)+32*i,str1[i].toStdString().c_str(),str1[i].size());
            }
            write((char*)respdu,respdu->PDUlen);
        }
        break;
        case MSG_TYPE_DELFRIEND_REQUEST://刷新好友
        {
            char name1[32]={"\n"};//姓名
            char name2[32]={"\n"};
            memcpy(name1,pdu->cData,32);
            memcpy(name2,pdu->cData+32,32);
            opDb::getintance().handDelFriend(name1,name2);
            respdu=mkPDU(0);
            respdu->msgType=MSG_TYPE_DELFRIEND_REPOND;
            write((char*)respdu,respdu->PDUlen);
        }
        break;
        case MSG_TYPE_PRIVATECHAT_REQUEST://私聊请求
        {
            qDebug()<<"服务器已接受到私聊请求";
            char name2[32]={"\n"};//被请求人姓名
            memcpy(name2,pdu->cData+32,32);
            respdu=mkPDU(0);
            for(int i=0;i<Mytcpserver::getInstance().socketlist.size();i++){
                if(Mytcpserver::getInstance().socketlist[i]->name==name2){
                    qDebug()<<"找到对方客户端"<<name2;
                    Mytcpserver::getInstance().socketlist[i]->write((char*)pdu,pdu->PDUlen);
                    qDebug()<<"已发送到对方客户端";
                    break;
                }
            }
        }
        break;
        case MSG_TYPE_AGREE_PRIVATECHAT_RESPOND://接收私聊响应
        {

            char name1[32]={"\n"};//请求人姓名
            memcpy(name1,pdu->cData,32);
            qDebug()<<"接收到同意私聊响应"<<name1;
            for(int i=0;i<Mytcpserver::getInstance().socketlist.size();i++){
                if(Mytcpserver::getInstance().socketlist[i]->name==name1){
                    qDebug()<<"已找到请求人"<<name1;
                    Mytcpserver::getInstance().socketlist[i]->write((char*)pdu,pdu->PDUlen);
                    break;
                }
            }
            respdu=mkPDU(0);
        }
        break;
        case MSG_TYPE_DISAGREE_PRIVATECHAT_RESPOND://拒绝接收私聊响应
        {
            qDebug()<<"接收到拒绝私聊响应";
            char name1[32]={"\n"};//请求人姓名
            memcpy(name1,pdu->cData,32);
            for(int i=0;i<Mytcpserver::getInstance().socketlist.size();i++){
                if(Mytcpserver::getInstance().socketlist[i]->name==name1){
                    Mytcpserver::getInstance().socketlist[i]->write((char*)pdu,pdu->PDUlen);
                    break;
                }
            }
            respdu=mkPDU(0);
        }
        break;
        case MSG_TYPE_CHAT:
        {
            char name[32]={"\n"};
            memcpy(name,pdu->cData,32);
            for(int i=0;i<Mytcpserver::getInstance().socketlist.size();i++){
                if(Mytcpserver::getInstance().socketlist[i]->name==name){
                    Mytcpserver::getInstance().socketlist[i]->write((char*)pdu,pdu->PDUlen);
                    break;
                }
            }
            respdu=mkPDU(0);
        }
        break;
        case MSG_TYPE_CREATE_DIR_REQUEST:
        {
            char name[32]={"\n"};
            memcpy(name,pdu->cData,32);

            char newdir[32]={"\n"};
            memcpy(newdir,pdu->cData+32,32);
            QString curdir=QString((char*)pdu->msg);

            QString newpath=curdir+"/"+newdir;//新文件夹路径
            qDebug()<<newpath;
            QDir dir;
            int k=0;
            if(!dir.exists(newpath)){//之前没有创建过同名文件夹
                dir.mkdir(newpath);
                k=1;
            }
            respdu=mkPDU(0);
            respdu->msgType=MSG_TYPE_CREATE_DIR_RESPOND;
            memcpy(respdu->cData,(char*)&k,sizeof(int));
            write((char*)respdu,respdu->PDUlen);
        }
        break;
        case MSG_TYPE_FLUSH_FILE_REQUEST:
        {
            char curpath[pdu->msglen];
            memcpy(curpath, (char*)pdu->msg, pdu->PDUlen);
            //QString curpath=QString((char*)pdu->msg);
            qDebug()<<curpath;
            QDir dir;
            dir.setPath(curpath);
            QFileInfoList f=dir.entryInfoList();
            int count=f.size();
            respdu=mkPDU((sizeof(MFileInfo)*count));
            respdu->msgType=MSG_TYPE_FLUSH_FILE_RESPOND;
            MFileInfo* temp=NULL;
            for(int i=0;i<f.size();i++){
                temp=(MFileInfo*)(respdu->msg)+i;
                qDebug()<<f[i].fileName();
                memcpy(temp->caName,f[i].fileName().toStdString().c_str(),f[i].fileName().size());
                if(f[i].isDir()){//文件夹类型
                    temp->FileType=0;
                }else{//普通文件
                    temp->FileType=1;
                }
                qDebug()<<QString(temp->caName);
            }
            write((char*)respdu,respdu->PDUlen);
        }
        break;
        case MSG_TYPE_DEL_DIR_REQUEST:
        {
            char name[32]={'\0'};
            memcpy(name,pdu->cData,32);
            char* path=new char[pdu->msglen];
            memcpy(path,(char*)pdu->msg,pdu->msglen);
            QString dirpath=QString(path)+"/"+QString(name);
            QFileInfo fileinfo(dirpath);
            bool  t;
            if(fileinfo.isDir()){//只删除文件夹类型
                QDir dir;
                dir.setPath(dirpath);
                t=dir.removeRecursively();
            }
            respdu=mkPDU(0);
            respdu->msgType=MSG_TYPE_DEL_DIR_RESPOND;
            memcpy(respdu->cData,(char*)&t,sizeof(bool));
            write((char*)respdu,respdu->PDUlen);
        }
        break;
        case MSG_TYPE_RENAME_DIR_REQUEST:
        {
            char name[32]={'\0'};
            char rename[32]={'\0'};
            memcpy(rename,pdu->cData,32);//新文件夹名
            memcpy(name,pdu->cData+32,32);//旧文件夹名
            char* path=new char[pdu->msglen];
            memcpy(path,(char*)pdu->msg,pdu->msglen);
            QString curpath=QString(path);
            QDir dir;
            dir.setPath(curpath);
            bool t=dir.rename(QString(name),QString(rename));
            respdu=mkPDU(0);
            respdu->msgType=MSG_TYPE_RENAME_DIR_RESPOND;
            memcpy(respdu->cData,(char*)&t,sizeof(bool));
            write((char*)respdu,respdu->PDUlen);
        }
        break;
        case MSG_TYPE_ENTER_DIR_REQUEST:
        {
            char name[32]={'\0'};
            char* path=new char[pdu->msglen];
            memcpy(name,pdu->cData,32);
            memcpy(path,pdu->msg,pdu->msglen);
            QString name1=QString(name);
            int k=0;
            if(name1.lastIndexOf('.')>=0){
                respdu=mkPDU(0);
            }else{
                QString curpath=QString(path)+"/"+QString(name);
                QDir dir;
                dir.setPath(curpath);
                QFileInfoList f=dir.entryInfoList();
                respdu=mkPDU(sizeof(MFileInfo)*f.size());
                respdu->msgType=MSG_TYPE_ENTER_DIR_RESPOND;
                k=1;
                memcpy(respdu->cData,name,32);
                for(int i=0;i<f.size();i++){
                    MFileInfo* p=(MFileInfo*)(respdu->msg)+i;
                    memcpy(p->caName,f[i].fileName().toStdString().c_str(),f[i].fileName().size());
                    if(f[i].isDir()){
                        p->FileType=0;
                    }else
                        p->FileType=1;
                }
            }
            memcpy(respdu->cData+32,(char*)&k,sizeof(int));
            write((char*)respdu,respdu->PDUlen);
        }
        break;
        case MSG_TYPE_RETURN_DIR_REQUEST:
        {
            char* path=new char[pdu->msglen];
            memcpy(path,pdu->msg,pdu->msglen);
            qDebug()<<path;
            QDir dir;
            dir.setPath(path);
            QFileInfoList f=dir.entryInfoList();
            int count=f.size();
            respdu=mkPDU(sizeof(MFileInfo)*count);
            respdu->msgType=MSG_TYPE_RETURN_DIR_RESPOND;
            qDebug()<<count;
            for(int i=0;i<f.size();i++){
                MFileInfo* p=(MFileInfo*)(respdu->msg)+i;
                if(f[i].isDir()){
                    p->FileType=0;
                }else{
                    p->FileType=1;
                }
                memcpy(p->caName,f[i].fileName().toStdString().c_str(),f[i].fileName().size());
            }
            write((char*)respdu,respdu->PDUlen);
        }
        break;
        case MSG_TYPE_UPLOAD_FILE_REQUEST:
        {
            char name[32]={'\0'};
            qint64 filesize=0;
            memcpy(name,pdu->cData,32);
            memcpy((char*)&filesize,pdu->cData+32,sizeof(qint64));
            char* path=new char[pdu->msglen];
            memcpy(path,pdu->msg,pdu->msglen);
            QString curpath=QString(path)+"/"+QString(name);
            qDebug()<<name<<"  "<<curpath<<"   "<<filesize;
            delete []path;
            path=NULL;
            mfile.setFileName(curpath);
            if(mfile.open(QIODevice::WriteOnly)){
                this->upload=true;
                this->mrecve=0;
                this->total=filesize;
            }
            respdu=mkPDU(0);
        }
        break;
        case MSG_TYPE_DEL_FILE_REQUEST:
        {
            char name[32]={'\0'};
            memcpy(name,pdu->cData,32);
            char* path=new char[pdu->msglen];
            memcpy(path,pdu->msg,pdu->msglen);
            QString curpath=QString(path)+"/"+QString(name);
            qDebug()<<curpath;
            QFileInfo fileinfo(curpath);
            bool t=false;
            respdu=mkPDU(0);
            if(fileinfo.isFile()){
                QDir dir;
                t=dir.remove(curpath);
                respdu->msgType=MSG_TYPE_DEL_FILE_RESPOND;
                memcpy(respdu->cData,(char*)&t,sizeof(bool));
                write((char*)respdu,respdu->PDUlen);
            }
        }
        break;
        case MSG_TYPE_DOWNLOAD_FILE_REQUEST:
        {
            char name[32]={'\0'};
            char* path=new char[pdu->msglen];
            memcpy(name,pdu->cData,32);
            memcpy(path,(char*)pdu->msg,pdu->msglen);
            QString filepath=QString(path)+"/"+QString(name);
            this->downpath=filepath;
            qDebug()<<"下载服务端   "<<QString(name)<<"  "<<filepath;
            QFile file(filepath);
            qint64 filesize=file.size();
            respdu=mkPDU(0);
            qDebug()<<"文件大小为  "<<filesize;
            memcpy(respdu->cData,(char*)&filesize,sizeof(qint64));
            respdu->msgType=MSG_TYPE_DOWNLOAD_FILE_RESPOND;
            write((char*)respdu,respdu->PDUlen);
            this->timer->start(1000);
        }
        break;
        default:
            break;
        }
        free(respdu);
        free(pdu);
        respdu=NULL;
        pdu=NULL;
    }else{
        QByteArray arr=readAll();
        this->mfile.write(arr);
        this->mrecve=this->mrecve+arr.size();
        if(this->total==this->mrecve){
            this->mfile.close();
            this->upload=false;
            PDU* pdu=mkPDU(0);
            pdu->msgType=MSG_TYPE_UPLOAD_FINISH_RESPOND;
            int k=0;
            memcpy(pdu->cData,(char*)&k,sizeof(int));
            write((char*)pdu,pdu->PDUlen);
            this->mfile.close();
            free(pdu);
            pdu=NULL;
        }
    }
}

void MyTcpSocket::downfile()
{
    qDebug()<<"准备写了";
    this->timer->stop();
    QFile file(this->downpath);
    char* temp=new char[4096];
    file.open(QIODevice::ReadOnly);
    qint64 ret=0;
    while(true){
       ret=file.read(temp,4096);
        if(ret>0&&ret<=4096){
            write(temp,ret);
        }else if(ret==0){
            break;
        }
    }
    qDebug()<<"读完了";
    this->mfile.close();
}
