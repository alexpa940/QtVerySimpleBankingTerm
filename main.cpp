#include "cardclient.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CardClient w;
    w.show();

    return a.exec();
}
