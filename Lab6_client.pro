QT += core gui
QT += network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Lab6_client\



SOURCES += main.cpp\
           cardclient.cpp\
           packet.cpp \
           carta.cpp

HEADERS  += cardclient.h\
            packet.h \
            carta.h

FORMS    += cardclient.ui\
            carta.ui

RESOURCES += icos.qrc
