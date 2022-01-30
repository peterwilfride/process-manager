#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QTableWidget>
#include <QTimerEvent>
#include <QTimer>
#include <unistd.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // get number of cpu's
    n_cpu = runCommand("nproc --all").replace("\n","").toInt();

    // initialize idle and sum for each cpu
    for (int i = 0; i < n_cpu; i++) {
        last_sum.append(0);
        last_idle.append(0.0);
    }

    // Collecting CPU usage
    cpu_stats = getCPUusage();
    cpu_int = calcCPUusage(cpu_stats);

    // initiliaze each objectss
    for (int i = 0; i < n_cpu; i++) {
        cpu.append(new QProgressBar(this));
        cpu_label.append(new QLabel(this));
        cpu[i]->setValue(cpu_int[i]);
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

    // clear warning labels
    ui->warning_label->clear();
    ui->warning_label_cpu->clear();

    // define title of columns
    ui->table->setColumnCount(8);
    QStringList titles = {"NOME", "CPU %", "THREADS", "STATE", "PID", "PPID", "PRIORITY", "OWNER"};
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
        QString cpu_prct = runCommand("ps -auf | grep "+process+" | awk '{print $3}'");
        QString state = runCommand("cat /proc/"+process+"/stat | awk '{print $3}'");
        QString threads = runCommand("cat /proc/"+process+"/status | grep -i threads | awk '{print $2}'");
        QString ppid = runCommand("cat /proc/"+process+"/status | grep ^PPid | awk '{print $2}'");
        QString prioridade = runCommand("cat /proc/"+process+"/stat | awk '{print $18}'");
        QString username = runCommand("cat /proc/"+process+"/loginuid | id -nu");

        name.replace("\n", "");
        cpu_prct.replace("\n", "");
        state.replace("\n", "");
        threads.replace("\n", "");
        pid.replace("\n", "");
        ppid.replace("\n", "");
        prioridade.replace("\n", "");
        username.replace("\n", "");


        QTableWidgetItem* name_item = new QTableWidgetItem(name);
        QTableWidgetItem* cpu_prct_item = new QTableWidgetItem(cpu_prct+"%");
        QTableWidgetItem* state_item = new QTableWidgetItem(state);
        QTableWidgetItem* threads_item = new QTableWidgetItem(threads);
        QTableWidgetItem* pid_item = new QTableWidgetItem(pid);
        QTableWidgetItem* ppid_item = new QTableWidgetItem(ppid);
        QTableWidgetItem* prioridade_item = new QTableWidgetItem(prioridade);
        QTableWidgetItem* userame_item = new QTableWidgetItem(username);

        // add rows
        ui->table->insertRow(ui->table->rowCount());
        ui->table->setItem( ui->table->rowCount()-1, 0, name_item);
        ui->table->setItem( ui->table->rowCount()-1, 1, cpu_prct_item);
        ui->table->setItem( ui->table->rowCount()-1, 2, threads_item);
        ui->table->setItem( ui->table->rowCount()-1, 3, state_item);
        ui->table->setItem( ui->table->rowCount()-1, 4, pid_item);
        ui->table->setItem( ui->table->rowCount()-1, 5, ppid_item);
        ui->table->setItem( ui->table->rowCount()-1, 6, prioridade_item);
        ui->table->setItem( ui->table->rowCount()-1, 7, userame_item);
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

QList<QStringList> MainWindow::getCPUusage()
{
    cpu_stats.clear();
    for(int i = 0; i < n_cpu; i++){
        QString x = runCommand("head -"+QString::number(i+2)+" /proc/stat|tail -1");
        QStringList y = x.split(" ");
        y.removeFirst();
        y.back().replace("\n", "");
        cpu_stats.append(y);
    }
    return cpu_stats;
}

QList<int> MainWindow::calcCPUusage(QList<QStringList>& cpu_stats)
{
    cpu_int.clear();

    for(int i = 0; i < n_cpu; i++){
        // sum and idle for this cpu
        int sum = 0;
        for(int j = 0; j < 10; j++){
            sum += cpu_stats[i][j].toInt();
        }
        double idle = cpu_stats[i][3].toDouble();

        double idle_delta = idle - last_idle[i];
        double sum_delta = sum - last_sum[i];

        last_idle[i] = idle;
        last_sum[i] = sum;

        double not_idle_cpu = 100 * (1.0 - idle_delta/sum_delta);
        cpu_int.append((int)(not_idle_cpu));
    }
    return cpu_int;
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

    cpu_stats = getCPUusage();
    cpu_int = calcCPUusage(cpu_stats);

    for(int i = 0; i < n_cpu; i++){
        cpu[i]->setValue(cpu_int[i]);
    }

    //qDebug() << cpu_stats;
    //qDebug() << cpu_int;

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

    QTableWidgetItem* current_pid = ui->table->item(current_row->row(), 4);

    ui->pid_edit->setText(current_pid->text());
}


void MainWindow::on_cpu_button_clicked()
{
    if (!ui->cpu_edit->text().isEmpty()) {
        QString pid_filter = ui->pid_edit->text();
        QString cpu_range = ui->cpu_edit->text();
        QString confirm_alocation = runCommand("taskset -pc "+cpu_range + " " + pid_filter + " | tail -1").replace("\n","");
        ui->warning_label_cpu->setText(confirm_alocation);
        sleep(5);
        ui->warning_label_cpu->clear();
    } else {
        ui->warning_label_cpu->setText("<font color='red'>Deve passar CPU's para alocação!</font>");
    }
}

