#-------------------------------------------------
#
# Project created by QtCreator 2016-04-06T12:22:11
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mx-test-repo-installer
TEMPLATE = app


SOURCES += main.cpp \
    versionnumber.cpp \
    mainwindow.cpp \
    lockfile.cpp

HEADERS  += mainwindow.h \
    versionnumber.h \
    lockfile.h

FORMS    += mainwindow.ui

TRANSLATIONS += translations/mx-test-repo-installer_ca.ts \
                translations/mx-test-repo-installer_de.ts \
                translations/mx-test-repo-installer_el.ts \
                translations/mx-test-repo-installer_es.ts \
                translations/mx-test-repo-installer_fr.ts \
                translations/mx-test-repo-installer_it.ts \
                translations/mx-test-repo-installer_ja.ts \
                translations/mx-test-repo-installer_lt.ts \
                translations/mx-test-repo-installer_nl.ts \
                translations/mx-test-repo-installer_pl.ts \
                translations/mx-test-repo-installer_pt.ts \
                translations/mx-test-repo-installer_ro.ts \
                translations/mx-test-repo-installer_ru.ts \
                translations/mx-test-repo-installer_sv.ts \
                translations/mx-test-repo-installer_tr.ts
