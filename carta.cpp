#include "carta.h"
#include "ui_carta.h"
#include "cardclient.h"

Carta::Carta(QWidget *parent):QMainWindow(parent),ui(new Ui::Carta)
{
    ui->setupUi(this);
    ui->PININ->hide();
    ui->label_2->setStyleSheet("background-image:url(:/1.png)");
}

Carta::~Carta()
{
    delete ui;
}

void Carta::on_pushButton_clicked()
{
    ui->label_2->setStyleSheet("background-image:url(:/2.png)");
    ui->PININ->show();
    ui->label_2->show();
    ui->label_3->hide();
    ui->lineEdit->hide();
    ui->lineEdit_2->hide();
    ui->lineEdit_3->hide();
    ui->lineEdit_4->hide();

}

void Carta::on_pushButton_2_clicked()
{
    ui->label_2->setStyleSheet("background-image:url(:/1.png)");
    ui->PININ->hide();
    ui->label_2->show();
    ui->label_3->show();
    ui->lineEdit->show();
    ui->lineEdit_2->show();
    ui->lineEdit_3->show();
    ui->lineEdit_4->show();

}

void Carta::on_pushButton_3_clicked()
{
    emit out(ui->lineEdit->text(),ui->lineEdit_2->text(),ui->lineEdit_3->text(),ui->PININ->text(),ui->lineEdit_4->text());
    this->close();
}
