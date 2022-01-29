#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QTableWidget>
#include <QTimerEvent>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // get number of cpu's
    int n_cpu = runCommand("nproc --all").replace("\n","").toInt();

    // initiliaze each objectss
    for (int i = 0; i < n_cpu; i++) {
        cpu.append(new QProgressBar(this));
        cpu_label.append(new QLabel(this));
        cpu[i]->setValue(50);
        cpu_label[i]->setText("cpu_"+QString::number(i));
    }

    // set geometry of each object
    int x = 740, y = 40, w = 170, h =25;
    for (int i = 0; i < n_cpu ; i++) {
        cpu_label[i]->setGeometry(x-50, y + i*h + i*5, w, h);
        cpu[i]->setGeometry(x, y + i*h + i*5, w, h);
    }

    // set default query process
    process_str = "ps -auf | tail -n +2 | awk '{print $2}'";

    ui->warning_label->clear();

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
    //qDebug() << process_str;
    process.start("sh", QStringList() << "-c" << process_str);
    process.waitForFinished(-1);

    QString str = process.readAllStandardOutput();
    QStringList process_list = str.split("\n");

    //qDebug() << process_list;
    //qDebug() << "";

    process_list.removeLast();

    return process_list;
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
        QString pid	= runCommand("cat /proc/"+process+"/status | grep ^Pid | awk '{print $2}'");

        if (pid.isEmpty()) //process dont exist
            continue;

        QString name = runCommand("cat /proc/"+process+"/status | grep -i name | awk '{print $2}'");
        QString state = runCommand("cat /proc/"+process+"/stat | awk '{print $3}'");
        QString threads = runCommand("cat /proc/"+process+"/status | grep -i threads | awk '{print $2}'");
        QString ppid = runCommand("cat /proc/"+process+"/status | grep ^PPid | awk '{print $2}'");
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

void MainWindow::filter()
{
    if(ui->filter_edit->text().isEmpty()){
        process_str = "ps -auf | tail -n +2 | awk '{print $2}'";
    }else{
        QString query = ui->filter_edit->text();
        process_str.clear();
        process_str = "ps -auf | tail -n +2 | grep " + query + " | awk '{print $2}'";
    }
}

void MainWindow::TimerSlot()
{
    if(ui->filter_edit->text().isEmpty()) {
        process_str = "ps -auf | tail -n +2 | awk '{print $2}'";
    }

    while (ui->table->rowCount() > 0) {
        ui->table->removeRow(0);
    }
    processList = getAllProcesses();

    fillTable();

    update_value();
}



void MainWindow::on_filter_button_clicked()
{
    filter();
}

void MainWindow::on_kill_Button_clicked()
{
    if(!ui->pid_edit->text().isEmpty()){
        QString pid_filter= ui->pid_edit->text();
        runCommand("kill -9 -" + pid_filter);
    }
}

void MainWindow::on_stop_Button_clicked()
{
    if(!ui->pid_edit->text().isEmpty()){
        QString pid_filter= ui->pid_edit->text();
        runCommand("kill -19 -" + pid_filter);
    }
}

void MainWindow::on_cont_Button_clicked()
{
    if(!ui->pid_edit->text().isEmpty()){
        QString pid_filter= ui->pid_edit->text();
        runCommand("kill -18 -" + pid_filter);
    }
}

void MainWindow::on_priority_button_clicked()
{
    if(!(ui->pid_edit->text().isEmpty()||ui->priority_edit->text().isEmpty())){
        QString pid_filter = ui->pid_edit->text();
        QString current_priority = runCommand("cat /proc/"+pid_filter+"/stat | awk '{print $18}'");
        QString priority_edit = ui->priority_edit->text();
        int number_priority = abs(priority_edit.toInt() - current_priority.toInt());
        if(number_priority >= 0){
            runCommand("renice -n " + QString::number(number_priority) + " " + pid_filter);
            ui->warning_label->clear();
        }else{
            ui->warning_label->setText("<font color='red'>Prioridade deve ser >= 20!</font>");
        }
    }
}

void MainWindow::on_table_cellDoubleClicked(int row, int column)
{
    QTableWidgetItem* current_row = ui->table->item(row, column);

    QTableWidgetItem* current_pid = ui->table->item(current_row->row(), 3);

    ui->pid_edit->setText(current_pid->text());
}

