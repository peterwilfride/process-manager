#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QTableWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //connect(&process, SIGNAL(readyReadStandardOutput()), this, SLOT(readOutput()));

    //QTableWidget* table = new QTableWidget(this);

    QStringList processList = getAllProcesses();
    ui->table->setRowCount(processList.size());
    ui->table->setColumnCount(6);

    int r = 0;
    foreach(auto process, processList) {
        QString name = runCommand("cat /proc/"+process+"/status | grep -i name | awk '{print $2}'");
        QString threads = runCommand("cat /proc/"+process+"/status | grep -i threads | awk '{print $2}'");
        QString pid	= runCommand("cat /proc/"+process+"/status | grep ^PPid | awk '{print $2}'");
        QString ppid = runCommand("cat /proc/"+process+"/status | grep ^Pid | awk '{print $2}'");
        QString prioridade = runCommand("cat /proc/"+process+"/stat | awk '{print $18}'");
        QString username = runCommand("cat /proc/"+process+"/loginuid | id -nu");

        QTableWidgetItem* name_item = new QTableWidgetItem(name);
        QTableWidgetItem* threads_item = new QTableWidgetItem(threads);
        QTableWidgetItem* pid_item = new QTableWidgetItem(pid);
        QTableWidgetItem* ppid_item = new QTableWidgetItem(ppid);
        QTableWidgetItem* prioridade_item = new QTableWidgetItem(prioridade);
        QTableWidgetItem* userame_item = new QTableWidgetItem(username);

        ui->table->setItem(r, 0, name_item);
        //table->setColumnWidth(0, 350);
        ui->table->setItem(r, 1, threads_item);
        ui->table->setItem(r, 2, pid_item);
        ui->table->setItem(r, 3, ppid_item);
        ui->table->setItem(r, 4, prioridade_item);
        ui->table->setItem(r, 5, userame_item);

        r++;
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}


QStringList MainWindow::getAllProcesses()
{
    process.start("sh", QStringList() << "-c" << "ls /proc | grep -E '^[0-9]+$'");
    process.waitForFinished(-1); // will wait forever until finished

    //qDebug() << "Imprimindo saída\n";
    //qDebug() << process.readAllStandardOutput();
    QString str = process.readAllStandardOutput();
    QStringList list1 = str.split("\n");

    foreach(auto file, list1) {
        //qDebug() << file;
    }
    //qDebug() << "fim";

    return list1;
}

QString MainWindow::runCommand(QString cmd)
{
    QProcess myProc;
    myProc.start("sh", QStringList() << "-c" << cmd);
    myProc.waitForFinished(-1); // will wait forever until finished

    return myProc.readAllStandardOutput();
}


