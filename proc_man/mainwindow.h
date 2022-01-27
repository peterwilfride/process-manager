#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QList>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QStringList getAllProcesses();
    QString runCommand(QString cmd);
    void fillTable();
    void update_value();
    void filter();

private slots:
    void TimerSlot(); // NEW slot

    void on_filter_button_clicked();

    void on_kill_Button_clicked();

    void on_stop_Button_clicked();

    void on_cont_Button_clicked();

    void on_priority_button_clicked();

    void on_table_cellDoubleClicked(int row, int column);

private:
    Ui::MainWindow *ui;
    QProcess process;
    int timerId;
    int toUpdate = 0;
    QStringList processList;
    QTimer* timer;
    QString process_str;


};
#endif // MAINWINDOW_H
