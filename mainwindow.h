#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QTimer>
#include <QProgressDialog>
#include <QProgressBar>

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

    QString version;
    QStringList changeset;    
    QStringList readMXlist();

    void displayMXlist(QStringList mxlist);
    void start();    
    void startProgressBar();
    void stopProgressBar();
    QString getVersion(QString name);
    Output runCmd(QString cmd);

private slots:
    void procTime();
    void on_buttonCancel_clicked();
    void on_buttonInstall_clicked();
    void on_buttonAbout_clicked();
    void on_listWidget_itemClicked(QListWidgetItem *item);


private:        
    QProgressDialog *progress;
    QTimer *timer;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
