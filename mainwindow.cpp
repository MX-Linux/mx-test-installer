/**********************************************************************
 *  mainwindow.cpp
 **********************************************************************
 * Copyright (C) 2016 MX Authors
 *
 * Authors: dolphin_oracle
 *          Adrian
 *          MX Linux <http://mxlinux.org>
 *
 * This file is part of mx-test-repo-installer
 *
 * mx-test-repo-installer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mx-test-repo-installer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mx-test-repo-installer.  If not, see <http://www.gnu.org/licenses/>.
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
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    this->show();
    version = getVersion("mx-test-repo-installer");
    ui->treeWidget->hideColumn(5); // Status of the package: installed, upgradable, etc
    ui->treeWidget->hideColumn(6); // Displayed status true/false
    qApp->processEvents();
    ui->icon->setIcon(QIcon::fromTheme("software-update-available", QIcon("/usr/share/mx-test-repo-installer/icons/software-update-available.png")));
    //this->setWindowIcon(QIcon::fromTheme("application-x-deb", QIcon("/usr/share/mx-test-repo-installer/icons/application-x-deb.png")));
    startProgressBar();
    runCmd("build-test-package-list.sh");
    connect(ui->searchBox,SIGNAL(textChanged(QString)),this, SLOT(findPackage()));
    ui->searchBox->setFocus();
    lock_file = new LockFile("/var/lib/dpkg/lock");
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
    ui->buttonInstall->setDisabled(true);
    ui->comboFilter->setDisabled(true);

    displayMXlist(readMXlist());
}

// List available apps
QStringList MainWindow::readMXlist()
{
    QString file_content;
    QStringList mxlist;
    file_content = runCmd("cat /tmp/mx-test-repo-installer/packagelist.txt").str;
    mxlist = file_content.split("\n");
    mxlist.removeDuplicates();
    return mxlist;
}

// Dislpay available apps
void MainWindow::displayMXlist(QStringList mxlist)
{
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
    ui->treeWidget->clear();
    ui->comboFilter->setDisabled(true);

    // create a list of apps, create a hash with app_name, app_info
    foreach(item, mxlist) {
        app_name = item.section(" ", 0, 0);
        app_info = item.section(" ", 1, -1);
        apps += app_name + " "; // all the apps
        app_ver = app_info.section("  ", 0, 0).trimmed();
        app_desc = app_info.section("  ", 1, -1);
        widget_item = new QTreeWidgetItem(ui->treeWidget);
        widget_item->setFlags(widget_item->flags());
        widget_item->setCheckState(0, Qt::Unchecked);
        widget_item->setText(2, app_name);
        widget_item->setText(3, app_ver);
        widget_item->setText(4, app_desc);
        widget_item->setText(6, "true"); // all items are displayed till filtered
    }
    for (int i = 0; i < ui->treeWidget->columnCount(); ++i) {
        ui->treeWidget->resizeColumnToContents(i);
    }
    stopProgressBar();
    QString info_installed = runCmd("LC_ALL=en_US.UTF-8 apt-cache policy " + apps + "|grep Candidate -B2").str; // intalled app info
    app_info_list = info_installed.split("--"); // list of installed apps
    // create a hash of name and installed version
    foreach(item, app_info_list) {
        //qDebug() << item;
        app_name = item.section(":", 0, 0).trimmed();
        installed = item.section("\n  ", 1, 1).trimmed().section(": ", 1, 1); // Installed version
        candidate = item.section("\n  ", 2, 2).trimmed().section(": ", 1, 1);
        hashInstalled.insert(app_name, installed);
        hashCandidate.insert(app_name, candidate);
    }
    for (int i = 0; i < ui->treeWidget->columnCount(); ++i) {
        ui->treeWidget->resizeColumnToContents(i);
    }
    // process the entire list of apps
    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it) {
        widget_item = (*it);
        app_name = widget_item->text(2);
        app_ver = widget_item->text(3);
        installed = hashInstalled[app_name];
        candidate = hashCandidate[app_name];
        //qDebug() << "installed: " << installed.toString();
        //qDebug() << "candidate: " << candidate.toString();
        VersionNumber candidatetest = QString(app_ver);
        if (installed.toString() == "(none)") {
            for (int i = 0; i < ui->treeWidget->columnCount(); ++i) {
                widget_item->setToolTip(i, tr("Version ") + candidate.toString() + tr(" in stable repo"));
            }
            widget_item->setText(5, "not installed");
        } else if (installed.toString() == "") {
            for (int i = 0; i < ui->treeWidget->columnCount(); ++i) {
                widget_item->setToolTip(i, tr("Not available in stable repo"));
            }
            widget_item->setText(5, "not installed");
        } else {
            if (installed >= candidatetest) {
                for (int i = 0; i < ui->treeWidget->columnCount(); ++i) {
                    widget_item->setDisabled(true);
                    widget_item->setToolTip(i, tr("Latest version ") + installed.toString() + tr(" already installed"));
                }
                widget_item->setText(5, "installed");
            } else {
                widget_item->setIcon(1, QIcon::fromTheme("software-update-available", QIcon("/usr/share/mx-test-repo-installer/icons/software-update-available.png")));
                for (int i = 0; i < ui->treeWidget->columnCount(); ++i) {
                    widget_item->setToolTip(i, tr("Version ") + installed.toString() + tr(" installed"));
                }
                widget_item->setText(5, "upgradable");
            }
        }
        ++it;
    }
    QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
    ui->comboFilter->setEnabled(true);
    ui->buttonInstall->setEnabled(true);
}

// process keystrokes
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Find))
        search();
    if (event->key() == Qt::Key_Escape)
        closeSearch();
}

void MainWindow::closeSearch()
{
    ui->searchBox->clear();
    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it) {
        (*it)->setHidden(false);
        ++it;
    }
}

// search entered text starting with the first keystroke
void MainWindow::search()
{
    ui->searchBox->setFocus();
}

// find packages
void MainWindow::findPackage()
{
    QString word = ui->searchBox->text();
    QList<QTreeWidgetItem *> found_items = ui->treeWidget->findItems(word, Qt::MatchContains, 2);
    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it) {
      if ((*it)->text(6) == "true" && found_items.contains(*it)) {
          (*it)->setHidden(false);
      } else {
          (*it)->setHidden(true);
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
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
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
    runCmd("echo deb http://main.mepis-deb.org/mx/testrepo/ mx15 test>>/etc/apt/sources.list.d/mxtestrepotemp.list");
    //enable mx16 repo if necessary
    if (system("cat /etc/apt/sources.list.d/*.list |grep -q mx16") == 0) {
        runCmd("echo deb http://main.mepis-deb.org/mx/testrepo/ mx16 test>>/etc/apt/sources.list.d/mxtestrepotemp.list");
    }
    lock_file->unlock();
    this->hide();
    runCmd("x-terminal-emulator -e apt-get update");
    runCmd("x-terminal-emulator -e apt-get install " + aptgetlist);
    runCmd("rm -f /etc/apt/sources.list.d/mxtestrepotemp.list");
    runCmd("x-terminal-emulator -e apt-get update");
    this->show();
    lock_file->lock();
    changeset.clear();
    //qDebug() << changeset;
    closeSearch();
    start();
}


void MainWindow::on_treeWidget_itemClicked(QTreeWidgetItem *item)
{
    QString newapp = QString(item->text(2));
    if (item->checkState(0) == Qt::Checked) {
        //qDebug() << newapp;
        changeset.append(newapp);
        //qDebug() << changeset;
    } else {
        //qDebug() << "item uchecked";
        changeset.removeOne(newapp);
        //qDebug() << changeset;
    }
}


void MainWindow::on_buttonAbout_clicked()
{
    this->hide();
    QMessageBox msgBox(QMessageBox::NoIcon,
                       tr("About MX Test Repo Installer"), "<p align=\"center\"><b><h2>" +
                       tr("MX Test Repo Installer") + "</h2></b></p><p align=\"center\">" + tr("Version: ") + version + "</p><p align=\"center\"><h3>" +
                       tr("App for installing directly from MX Test Repo") +
                       "</h3></p><p align=\"center\"><a href=\"http://mxlinux.org\">http://mxlinux.org</a><br /></p><p align=\"center\">" +
                       tr("Copyright (c) MX Linux") + "<br /><br /></p>", 0, this);
    msgBox.addButton(tr("Cancel"), QMessageBox::AcceptRole); // because we want to display the buttons in reverse order we use counter-intuitive roles.
    msgBox.addButton(tr("License"), QMessageBox::RejectRole);
    if (msgBox.exec() == QMessageBox::RejectRole) {
        system("mx-viewer file:///usr/share/doc/mx-test-repo-installer/license.html '" + tr("MX Test Repo Installer").toUtf8() + " " + tr("License").toUtf8() + "'");
    }
    this->show();
}

void MainWindow::on_buttonHelp_clicked()
{
    QString cmd = QString("mx-viewer https://mxlinux.org/wiki/help-files/help-mx-test-repo-installer '%1'").arg(tr("MX Test Repo Installer"));
        system(cmd.toUtf8());
}

// filter items according to selected filter
void MainWindow::on_comboFilter_activated(const QString &arg1)
{
    QList<QTreeWidgetItem *> found_items;
    QTreeWidgetItemIterator it(ui->treeWidget);

    if (arg1 == tr("All packages")) {
        while (*it) {
            (*it)->setText(6, "true"); // Displayed flag
            (*it)->setHidden(false);
            ++it;
        }
        findPackage();
        return;
    }

    if (arg1 == tr("Upgradable")) {
        found_items = ui->treeWidget->findItems("upgradable", Qt::MatchExactly, 5);
    } else if (arg1 == tr("Installed")) {
        found_items = ui->treeWidget->findItems("installed", Qt::MatchExactly, 5);
    } else if (arg1 == tr("Not installed")) {
        found_items = ui->treeWidget->findItems("not installed", Qt::MatchExactly, 5);
    }

    while (*it) {
      if (found_items.contains(*it) ) {
          (*it)->setHidden(false);
          (*it)->setText(6, "true"); // Displayed flag
      } else {
          (*it)->setHidden(true);
          (*it)->setText(6, "false");
      }
      ++it;
    }
    findPackage();
}

