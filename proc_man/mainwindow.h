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

private slots:
    void TimerSlot(); // NEW slot

private:
    Ui::MainWindow *ui;
    QProcess process;
    int timerId;
    int toUpdate = 0;

    QTimer* timer;


};
#endif // MAINWINDOW_H
