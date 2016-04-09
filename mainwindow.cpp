#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QProcess>
#include <QRadioButton>
#include <QDir>
#include <QDebug>
#include <QForeachContainer>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->show();
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

// poplulate list
void MainWindow::start()
{
    displayMXlist(readMXlist());

}

// List available apps
QStringList MainWindow::readMXlist()
{
    QString home_path = QDir::homePath();
    QString file_content;
    QStringList mxlist;
    file_content = runCmd("cat " + home_path + "/.config/testrepoinstaller/packagelist.txt").str;
    mxlist = file_content.split("\n");
    return mxlist;

}

// Dislpay available apps
void MainWindow::displayMXlist(QStringList mxlist)

{
    runCmd("notify-send -i application-x-deb -t 20000 'Test Repo Installer' 'Building Test Repo Package List'");
    ui->listWidget->clear();
    QStringListIterator mxIterator(mxlist);
    while (mxIterator.hasNext()) {
        QString listing = mxIterator.next();
        QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
        item->setFlags(item->flags());
        item->setCheckState(Qt::Unchecked);
        item->setText(listing);
        QString app = listing.section(" ", 0, 0);
        qDebug() << app;
        QString installed = runCmd("apt-cache policy " + app + "|grep Installed:").str.section(": ", 1, 1);
        QString candidate = listing.section(" ", 1, 1);
        qDebug() << installed;
        qDebug() << candidate;
        if (installed == "(none)") {
            item->setBackgroundColor(Qt::white);
            item->setToolTip("Version " + candidate + " in stable repo" );
        } else {
            if (installed == "") {
                item->setBackgroundColor(Qt::white);
                item->setToolTip("Not available in stable repo" );
            } else {
                if (installed == candidate) {
                    item->setBackgroundColor(Qt::green);
                    item->setToolTip("Latest version already installed");
                } else {
                    item->setBackgroundColor(Qt::yellow);
                    item->setToolTip("Version " + installed + " installed");
                }
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

void MainWindow::on_refreshbutton_clicked()
{
    ui->listWidget->clear();
    runCmd("build-test-package-list");
    start();
}

void MainWindow::on_buttonAbout_clicked()
{
}
