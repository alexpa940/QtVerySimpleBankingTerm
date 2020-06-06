#include "cardserver.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CardServer w;
    w.show();

    return a.exec();
}
