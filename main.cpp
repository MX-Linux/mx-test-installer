/**********************************************************************
 *  main.cpp
 **********************************************************************
 * Copyright (C) 2016 MX Authors
 *
 * Authors: Adrian
 *          dolphin_oracle
 *          MX & MEPIS Community <http://mxlinux.org>
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
#include "lockfile.h"

#include <QApplication>
#include <unistd.h>
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QIcon>
#include <QMessageBox>

#include <QDebug>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator qtTran;
    qtTran.load(QString("qt_") + QLocale::system().name());
    a.installTranslator(&qtTran);

    QTranslator appTran;
    appTran.load(QString("mx-test-repo-installer_") + QLocale::system().name(), "/usr/share/mx-test-repo-installer/locale");
    a.installTranslator(&appTran);

    // Don't start app if Synaptic/apt-get is running, lock dpkg otherwise while the program runs
    LockFile lock_file("/var/lib/dpkg/lock");
    if (lock_file.isLocked()) {
        QApplication::beep();
        QMessageBox::critical(0, QApplication::tr("Unable to get exclusive lock"),
                              QApplication::tr("Another package management application (like Synaptic or apt-get), "\
                                               "is already running. Please close that application first"));
        return 1;
    } else {
        lock_file.lock();
    }

    if (getuid() == 0) {
        MainWindow w;
        w.show();

        return a.exec();
        lock_file.unlock();

    } else {
        QApplication::beep();
        QMessageBox::critical(0, QString::null,
                              QApplication::tr("You must run this program as root."));
        return 1;
    }

}
