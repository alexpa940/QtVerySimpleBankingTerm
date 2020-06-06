#ifndef CARTA_H
#define CARTA_H

#include <QMainWindow>
#include "cardclient.h"

namespace Ui {
class Carta;
}

class Carta : public QMainWindow
{
    Q_OBJECT

public:
    explicit Carta(QWidget *parent = 0);
    ~Carta();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();

signals:
    void out(QString number, QString month, QString year, QString pin, QString name);

private:
    Ui::Carta *ui;
};

#endif // CARTA_H
