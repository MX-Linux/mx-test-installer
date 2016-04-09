#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>

namespace Ui {
class MainWindow;
}

// struct for outputing both the exit code and the strings when running a command
struct Output {
    int exit_code;
    QString str;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QStringList readMXlist();
    void displayMXlist(QStringList mxlist);
    Output runCmd(QString cmd);
    void start();
    QStringList changeset;

private slots:

    void on_buttonCancel_clicked();



    void on_buttonInstall_clicked();



    void on_listWidget_itemClicked(QListWidgetItem *item);

    void on_refreshbutton_clicked();

    void on_buttonAbout_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
