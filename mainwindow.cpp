#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QProcess>
#include <QRadioButton>
#include <QDir>
#include <QMessageBox>
#include <QHash>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->show();
    runCmd("build-test-package-list.sh");
    start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// util function for getting bash command output and error code
Output MainWindow::runCmd(QString cmd)
{
    QProcess *proc = new QProcess();
    QEventLoop loop;
    proc->setReadChannelMode(QProcess::MergedChannels);
    proc->start("/bin/bash", QStringList() << "-c" << cmd);
    proc->waitForFinished();
    Output out = {proc->exitCode(), proc->readAll().trimmed()};
    delete proc;
    return out;
}

// populate list
void MainWindow::start()
{
    displayMXlist(readMXlist());
}

// List available apps
QStringList MainWindow::readMXlist()
{
    QString file_content;
    QStringList mxlist;
    file_content = runCmd("cat " + QDir::homePath() + "/.config/testrepoinstaller/packagelist.txt").str;
    mxlist = file_content.split("\n");
    return mxlist;
}

// Dislpay available apps
void MainWindow::displayMXlist(QStringList mxlist)
{
    QHash<QString, QString> hashApp; // hash that contains (app_name, app_info) for the mxlist
    QString app_name;
    QString app_info;
    QString apps;
    QString item;
    QString installed;
    QString candidate;
    QStringList app_info_list;
    QListWidgetItem *widget_item;

    system("notify-send -i application-x-deb 'Test Repo Installer' 'List Packages'");
    ui->listWidget->clear();

    // create a list of apps, create a hash with app_name, app_info
    foreach(item, mxlist) {
        app_name = item.section(" ", 0, 0);
        app_info = item.section(" ", 1, -1);
        hashApp.insert(app_name, app_info);
        apps += app_name + " "; // all the apps
    }
    QString info_installed = runCmd("apt-cache policy " + apps + "|grep Installed -B1").str; // intalled app info
    app_info_list = info_installed.split("--"); // list of installed apps

    foreach(item, app_info_list) {
        app_name = item.section(":", 0, 0).trimmed();
        app_info = hashApp[app_name];
        installed = item.section("\n  ", 1, 1).trimmed().section(": ", 1, 1); // Installed version
        widget_item = new QListWidgetItem(ui->listWidget);
        widget_item->setFlags(widget_item->flags());
        widget_item->setCheckState(Qt::Unchecked);
        widget_item->setText(app_name +  " " + app_info);
        if (installed == "(none)") {
            widget_item->setBackgroundColor(Qt::white);
            widget_item->setToolTip("Version " + candidate + " in stable repo" );
        } else if (installed == "") {
            widget_item->setBackgroundColor(Qt::white);
            widget_item->setToolTip("Not available in stable repo" );
        } else {
            candidate = app_info.section(" ", 0, 0); // candidate version
            if (installed == candidate) {
                widget_item->setBackgroundColor(Qt::green);
                widget_item->setToolTip("Latest version already installed");
            } else {
                widget_item->setBackgroundColor(Qt::yellow);
                widget_item->setToolTip("Version " + installed + " installed");
            }
        }
    }
}


void MainWindow::on_buttonCancel_clicked()
{
    exit(0);
}


void MainWindow::on_buttonInstall_clicked()
{
    QStringListIterator changeIterator(changeset);
    QString aptgetlist;
    while (changeIterator.hasNext()) {
        QString listing = changeIterator.next();
        aptgetlist = QString(aptgetlist + " " + listing);
        qDebug() << aptgetlist;
    }
    runCmd("su-to-root -X -c 'echo deb http://main.mepis-deb.org/mx/testrepo/ mx15 test>/etc/apt/sources.list.d/mxtestrepotemp.list'");
    runCmd("su-to-root -X -c 'x-terminal-emulator -e apt-get update'");
    runCmd("su-to-root -X -c 'x-terminal-emulator -e apt-get install " + aptgetlist +"'");
    runCmd("su-to-root -X -c 'rm -f /etc/apt/sources.list.d/mxtestrepotemp.list'");
    runCmd("su-to-root -X -c 'x-terminal-emulator -e apt-get update'");
    changeset.clear();
    qDebug() << changeset;
    start();
}


void MainWindow::on_listWidget_itemClicked(QListWidgetItem *item)
{
    QString newapp = QString(item->text());
    QString newapp2 = newapp.section(' ', 0, 0);
    if (item->checkState()) {
        qDebug() << newapp;
        qDebug() << newapp2;
        changeset.append(newapp2);
        qDebug() << changeset;
    } else {
        qDebug() << "item uchecked";
        changeset.removeOne(newapp2);
        qDebug() << changeset;
    }
}


void MainWindow::on_buttonAbout_clicked()
{
}
