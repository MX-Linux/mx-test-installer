/**********************************************************************
 *  mainwindow.cpp
 **********************************************************************
 * Copyright (C) 2016 MX Authors
 *
 * Authors: dolphin_oracle
 *          Adrian
 *          MX & MEPIS Community <http://forum.mepiscommunity.org>
 *
 * This file is part of mx-test-installer
 *
 * mx-test-installer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mx-test-installer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mx-test-installer.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "versionnumber.h"

#include <QProcess>
#include <QRadioButton>
#include <QDir>
#include <QMessageBox>
#include <QHash>
#include <QKeyEvent>

#include <QDebug>


MainWindow::MainWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->show();
    version = getVersion("mx-test-installer");
    qApp->processEvents();
    searchBox = new QLineEdit(this);
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
    connect(proc, SIGNAL(finished(int)), &loop, SLOT(quit()));
    proc->setReadChannelMode(QProcess::MergedChannels);
    proc->start("/bin/bash", QStringList() << "-c" << cmd);
    loop.exec();
    Output out = {proc->exitCode(), proc->readAll().trimmed()};
    delete proc;
    return out;
}

// Get version of the program
QString MainWindow::getVersion(QString name)
{
    QString cmdstr = QString("dpkg -l %1 | awk 'NR==6 {print $3}'").arg(name);
    return runCmd(cmdstr).str;
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
    file_content = runCmd("cat " + QDir::homePath() + "/.config/mx-test-installer/packagelist.txt").str;
    mxlist = file_content.split("\n");
    return mxlist;
}

// Dislpay available apps
void MainWindow::displayMXlist(QStringList mxlist)
{
    QHash<QString, QString> hashApp; // hash that contains (app_name, app_info) for the mxlist
    QHash<QString, VersionNumber> hashInstalled; // hash that contains (app_name, VersionNumber) returned by apt-cache policy
    QHash<QString, VersionNumber> hashCandidate; //hash that contains (app_name, VersionNumber) returned by apt-cache policy for candidates
    QString app_name;
    QString app_info;
    QString apps;
    QString item;
    QString app_ver;
    QString app_desc;
    VersionNumber installed;
    VersionNumber candidate;
    QStringList app_info_list;
    QTreeWidgetItem *widget_item;

    //system("notify-send -i application-x-deb 'Test Repo Installer' 'List Packages'");
    startProgressBar();
    ui->treeWidget->clear();

    // create a list of apps, create a hash with app_name, app_info
    foreach(item, mxlist) {
        app_name = item.section(" ", 0, 0);
        app_info = item.section(" ", 1, -1);
        hashApp.insert(app_name, app_info);
        apps += app_name + " "; // all the apps
    }
    QString info_installed = runCmd("apt-cache policy " + apps + "|grep Candidate -B2").str; // intalled app info
    app_info_list = info_installed.split("--"); // list of installed apps
    // create a hash of name and installed version
    foreach(item, app_info_list) {
        qDebug() << item;
        app_name = item.section(":", 0, 0).trimmed();
        installed = item.section("\n  ", 1, 1).trimmed().section(": ", 1, 1); // Installed version
        candidate = item.section("\n  ", 2, 2).trimmed().section(": ", 1, 1);
        hashInstalled.insert(app_name, installed);
        hashCandidate.insert(app_name, candidate);
    }
    // process the entire list of apps
    foreach(item, mxlist) {
        app_name = item.section(" ", 0, 0);
        app_info = hashApp[app_name];
        app_ver = app_info.section("  ", 0, 0).trimmed();
        app_desc = app_info.section("  ", 1, -1);
        installed = hashInstalled[app_name];
        widget_item = new QTreeWidgetItem(ui->treeWidget);
        widget_item->setFlags(widget_item->flags());
        widget_item->setCheckState(0, Qt::Unchecked);
        widget_item->setText(1, app_name);
        widget_item->setText(2, app_ver);
        widget_item->setText(3, app_desc);
        candidate = hashCandidate[app_name];
        qDebug() << installed.toString();
        qDebug() << candidate.toString();
        VersionNumber candidatetest = QString(app_ver);
        if (installed.toString() == "(none)") {
            for (int i = 0; i < 4; ++i) {
                widget_item->setBackground(i, Qt::white);
                widget_item->setToolTip(i, "Version " + candidate.toString() + " in stable repo" );
            }
        } else if (installed.toString() == "") {
            for (int i = 0; i < 4; ++i) {
                widget_item->setBackground(i, Qt::white);
                widget_item->setToolTip(i, "Not available in stable repo" );
            }
        } else {
            if (installed >= candidatetest) {
                for (int i = 0; i < 4; ++i) {
                    widget_item->setBackground(i, Qt::green);
                    widget_item->setToolTip(i, "Latest version " + installed.toString() + " already installed");
                }
            } else {
                for (int i = 0; i < 4; ++i) {
                    widget_item->setBackground(i, Qt::yellow);
                    widget_item->setToolTip(i, "Version " + installed.toString() + " installed");
                }
            }
        }
    }

    for (int i = 0; i < ui->treeWidget->columnCount(); ++i) {
        ui->treeWidget->resizeColumnToContents(i);
    }
    stopProgressBar();
}

// process keystrokes
void MainWindow::keyPressEvent(QKeyEvent *event)
{    
    if (event->key() == Qt::Key_Escape) {
        closeSearch();
    } else if (event->text().trimmed() != "") {  // get rid of Enter and modifier key presses
        search(event->text());
    }
}

void MainWindow::closeSearch()
{
    searchBox->close();
    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it) {
        (*it)->setHidden(false);
        ++it;
    }
}

// search entered text starting with the first keystroke
void MainWindow::search(QString key)
{
    searchBox = new QLineEdit(this);
    searchBox->move(this->geometry().width() - 120,this->geometry().height() - 100);
    searchBox->setFocus();
    searchBox->show();    
    connect(searchBox,SIGNAL(textChanged(QString)),this, SLOT(findPackage()));    
    searchBox->setText(key); // process the first keystroke
}

// find packages
void MainWindow::findPackage()
{    
    QString word = searchBox->text();
    QList<QTreeWidgetItem *> found_items = ui->treeWidget->findItems(word, Qt::MatchContains, 1);
    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it) {
      if (!found_items.contains(*it) ) {
          (*it)->setHidden(true);
      } else {
          (*it)->setHidden(false);
      }
      ++it;
    }
}

void MainWindow::startProgressBar()
{
    progress = new QProgressDialog(this);
    QProgressBar *bar = new QProgressBar(progress);
    progress->setWindowModality(Qt::WindowModal);
    progress->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint |Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint);
    progress->setCancelButton(0);
    progress->setLabelText(tr("Please wait till the database is loaded."));
    progress->setAutoClose(false);
    progress->show();
    progress->setBar(bar);
    bar->setTextVisible(false);
    qApp->processEvents();
    timer = new QTimer(this);
    timer->start(20);
    connect(timer, SIGNAL(timeout()),SLOT(procTime()));
}

void MainWindow::stopProgressBar()
{
    timer->stop();
    disconnect(timer, SIGNAL(timeout()), 0, 0);
    progress->close();
}

void MainWindow::procTime()
{
    progress->setValue(progress->value() + 1);
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
    closeSearch();
    start();
}


void MainWindow::on_treeWidget_itemClicked(QTreeWidgetItem *item)
{
    QString newapp = QString(item->text(1));
    if (item->checkState(0) == Qt::Checked) {
        qDebug() << newapp;
        changeset.append(newapp);
        qDebug() << changeset;
    } else {
        qDebug() << "item uchecked";
        changeset.removeOne(newapp);
        qDebug() << changeset;
    }
}


void MainWindow::on_buttonAbout_clicked()
{
    this->hide();
    QMessageBox msgBox(QMessageBox::NoIcon,
                       tr("About MX Test Installer"), "<p align=\"center\"><b><h2>" +
                       tr("MX Test Installer") + "</h2></b></p><p align=\"center\">" + tr("Version: ") + version + "</p><p align=\"center\"><h3>" +
                       tr("App for installing directly from MX Test Repo") +
                       "</h3></p><p align=\"center\"><a href=\"http://www.mepiscommunity.org/mx\">http://www.mepiscommunity.org/mx</a><br /></p><p align=\"center\">" +
                       tr("Copyright (c) MX Linux") + "<br /><br /></p>", 0, this);
    msgBox.addButton(tr("Cancel"), QMessageBox::AcceptRole); // because we want to display the buttons in reverse order we use counter-intuitive roles.
    msgBox.addButton(tr("License"), QMessageBox::RejectRole);
    if (msgBox.exec() == QMessageBox::RejectRole) {
        system("mx-viewer file:///usr/share/doc/mx-test-installer/license.html '" + tr("MX Test Installer").toUtf8() + " " + tr("License").toUtf8() + "'");
    }
    this->show();
}

void MainWindow::on_buttonHelp_clicked()
{
    QString cmd = QString("mx-viewer http://www.mepiscommunity.org/wiki/help-files/help-mx-test-installer '%1'").arg(tr("MX Test Installer"));
        system(cmd.toUtf8());
}
