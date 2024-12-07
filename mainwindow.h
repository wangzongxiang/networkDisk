#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <mytcpserver.h>
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void loadconfig();
    QTcpSocket* tcp;
private:
    Ui::MainWindow *ui;
    QString ip;
    unsigned short port;
};
#endif // MAINWINDOW_H
