#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QTableWidget>
#include <QTimerEvent>
#include <QTimer>
#include <QProgressBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // define title of columns
    ui->table->setColumnCount(7);
    QStringList titles = {"NOME", "THREADS", "STATE", "PID", "PPID", "PRIORITY", "OWNER"};
    ui->table->setHorizontalHeaderLabels(titles);

    // hide vertical headers
    ui->table->verticalHeader()->hide();

    timer = new QTimer(this); // create it
    connect(timer, &QTimer::timeout, this, &MainWindow::TimerSlot ); // connect it

    processList = getAllProcesses();

    // preenche a tabela
    fillTable();

    update_value();
    timer->start(1000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QStringList MainWindow::getAllProcesses()
{
    //process.start("sh", QStringList() << "-c" << "ls /proc | grep -E '^[0-9]+$'");
    process.start("sh", QStringList() << "-c" << "ps -auf | tail -n +2 | awk '{print $2}'");
    process.waitForFinished(-1);

    QString str = process.readAllStandardOutput();
    QStringList list1 = str.split("\n");

    list1.removeLast();

    return list1;
}

QString MainWindow::runCommand(QString cmd)
{
    QProcess myProc;
    myProc.start("sh", QStringList() << "-c" << cmd);
    myProc.waitForFinished(-1); // will wait forever until finished

    return myProc.readAllStandardOutput();
}

void MainWindow::fillTable()
{
    foreach(auto process, processList) {
        QString pid	= runCommand("cat /proc/"+process+"/status | grep ^PPid | awk '{print $2}'");

        if (pid.isEmpty()) //process dont exist
            continue;

        QString name = runCommand("cat /proc/"+process+"/status | grep -i name | awk '{print $2}'");
        QString state = runCommand("cat /proc/"+process+"/stat | awk '{print $3}'");
        QString threads = runCommand("cat /proc/"+process+"/status | grep -i threads | awk '{print $2}'");
        QString ppid = runCommand("cat /proc/"+process+"/status | grep ^Pid | awk '{print $2}'");
        QString prioridade = runCommand("cat /proc/"+process+"/stat | awk '{print $18}'");
        QString username = runCommand("cat /proc/"+process+"/loginuid | id -nu");

        name.replace("\n", "");
        state.replace("\n", "");
        threads.replace("\n", "");
        pid.replace("\n", "");
        ppid.replace("\n", "");
        prioridade.replace("\n", "");
        username.replace("\n", "");


        QTableWidgetItem* name_item = new QTableWidgetItem(name);
        QTableWidgetItem* state_item = new QTableWidgetItem(state);
        QTableWidgetItem* threads_item = new QTableWidgetItem(threads);
        QTableWidgetItem* pid_item = new QTableWidgetItem(pid);
        QTableWidgetItem* ppid_item = new QTableWidgetItem(ppid);
        QTableWidgetItem* prioridade_item = new QTableWidgetItem(prioridade);
        QTableWidgetItem* userame_item = new QTableWidgetItem(username);

        // add rows
        ui->table->insertRow(ui->table->rowCount());
        ui->table->setItem( ui->table->rowCount()-1, 0, name_item);
        ui->table->setItem( ui->table->rowCount()-1, 1, threads_item);
        ui->table->setItem( ui->table->rowCount()-1, 2, state_item);
        ui->table->setItem( ui->table->rowCount()-1, 3, pid_item);
        ui->table->setItem( ui->table->rowCount()-1, 4, ppid_item);
        ui->table->setItem( ui->table->rowCount()-1, 5, prioridade_item);
        ui->table->setItem( ui->table->rowCount()-1, 6, userame_item);
    }
}

void MainWindow::update_value()
{
    toUpdate = toUpdate + 1;
    ui->time_label->setText(QString::number(toUpdate));

    ui->nproc_label->setText(QString::number(processList.size()));
}

void MainWindow::TimerSlot()
{
    while (ui->table->rowCount() > 0)
    {
        ui->table->removeRow(0);
    }
    processList = getAllProcesses();

    fillTable();

    update_value();
}


