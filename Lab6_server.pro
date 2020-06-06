QT += core gui
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Lab6_server\



SOURCES +=\
        cardserver.cpp\
        main_1.cpp \
        packet.cpp

HEADERS  += cardserver.h\
packet.h

FORMS    +=\
    cardserver.ui

RESOURCES += \
    icos.qrc
