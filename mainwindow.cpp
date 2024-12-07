#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QMessageBox>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->loadconfig();
    Mytcpserver::getInstance().listen(QHostAddress::Any,this->port);
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::loadconfig()//加载资源文件
{
    QFile f(":/server.config");
    if(f.open(QFile::ReadOnly)){
        QByteArray arr=f.readAll();
        QString str=arr.toStdString().c_str();
        f.close();
        str.replace("\r\n"," ");
        QStringList strlist=str.split(" ");
        this->ip=strlist[0];
        this->port=strlist[1].toUShort();
    }else{
        QMessageBox::critical(this,"警告","配置文件加载失败");
    }
}
