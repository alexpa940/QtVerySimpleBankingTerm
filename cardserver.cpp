#include "cardserver.h"
#include "ui_cardserver.h"
#include "QResizeEvent"

bool isDig(QString str)
{
    int pos = 0;

    QRegExpValidator validator(QRegExp("[1-9]+"));

    if (validator.validate(str, pos) == QValidator::Acceptable)
        return true;

    return false;
}

CardServer::CardServer(QWidget *parent):QMainWindow(parent),ui(new Ui::CardServer)
{
    ui->setupUi(this);
    m_nNextBlockSize=0;
    m_ptcpServer = NULL;
    m_settingsFile = QApplication::applicationDirPath() + "/server.ini";
    loadSettings();
    flag=false;
    ui->portEdit->setInputMask("00000");
}

void CardServer::loadSettings()
{
    if (QFile(m_settingsFile).exists())
    {
        QSettings settings(m_settingsFile, QSettings::IniFormat);
        settings.beginGroup("Server");
        if(isDig(settings.value("port", 1).toString()))
        {
            int port = settings.value("port", 1).toInt();
            if (ui->portEdit->isEnabled())
                ui->portEdit->setText(QString::number(port));
        }
        else
            if (ui->portEdit->isEnabled())
                ui->portEdit->setText(QString::number(1));
        settings.endGroup();
        ui->terminals->clear();
        settings.beginGroup("Terminals");
        QStringList trmns = settings.childKeys();
        if (trmns.count() > 0)
        {
            ui->terminals->clear();
            for(int i=0; i < trmns.count(); ++i)
            {
                ui->terminals->addItem(settings.value(trmns[i]).toString());
            }
        }
        settings.endGroup();
        QStringList groups = settings.childGroups();
        if (groups.count() > 0)
        {
            for(int i=0; i < groups.count(); ++i)
            {
                if (groups[i] != "Server" && groups[i] != "Terminals")
                {
                    settings.beginGroup(groups[i]);
                    qint32 last_row = ui->cards->rowCount();
                    ui->cards->insertRow(last_row);
                    QTableWidgetItem *pan = new QTableWidgetItem(settings.value("pan").toString());
                    QTableWidgetItem *amount = new QTableWidgetItem(settings.value("amount").toString());
                    QTableWidgetItem *month = new QTableWidgetItem(settings.value("month").toString());
                    QTableWidgetItem *year = new QTableWidgetItem(settings.value("year").toString());
                    QTableWidgetItem *pin = new QTableWidgetItem(settings.value("pin").toString());
                    QTableWidgetItem *name = new QTableWidgetItem(settings.value("name").toString());
                    QTableWidgetItem *stolen = new QTableWidgetItem(settings.value("stolen").toString());
                    QTableWidgetItem *lost = new QTableWidgetItem(settings.value("lost").toString());
                    QTableWidgetItem *pinCount = new QTableWidgetItem(settings.value("pinCount").toString());
                    ui->cards->setItem( last_row, 0, pan );
                    ui->cards->setItem( last_row, 1, amount );
                    ui->cards->setItem( last_row, 2, month );
                    ui->cards->setItem( last_row, 3, year );
                    ui->cards->setItem( last_row, 4, pin );
                    ui->cards->setItem( last_row, 5, name );
                    ui->cards->setItem( last_row, 6, stolen );
                    ui->cards->setItem( last_row, 7, lost );
                    ui->cards->setItem( last_row, 8, pinCount );
                    settings.endGroup();
                }
            }
        }
    }
    return;
}

void CardServer::saveSettings()
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    settings.beginGroup("Server");
    settings.setValue("port",  ui->portEdit->text().toInt());
    settings.endGroup();
    settings.beginGroup("Terminals");
    if (ui->terminals->count() > 0)
    {
        for(int i=0; i < ui->terminals->count(); ++i)
        {
            settings.setValue(QString::number(i),ui->terminals->item(i)->text());
        }
    }
    settings.endGroup();
        for(int i=0; i < ui->cards->rowCount(); ++i)
        {
            settings.beginGroup(QString::number(i));
            settings.setValue("pan", ui->cards->item(i,0)->text());
            settings.setValue("amount", ui->cards->item(i,1)->text());
            settings.setValue("month", ui->cards->item(i,2)->text());
            settings.setValue("year", ui->cards->item(i,3)->text());
            settings.setValue("pin", ui->cards->item(i,4)->text());
            settings.setValue("name", ui->cards->item(i,5)->text());
            settings.setValue("stolen", ui->cards->item(i,6)->text());
            settings.setValue("lost", ui->cards->item(i,7)->text());
            settings.setValue("pinCount", ui->cards->item(i,8)->text());
            settings.endGroup();
        }
}

void CardServer::startServer(int port, QHostAddress addr)
{
    if (m_ptcpServer && flag!=true)
    {
        ui->log->addItem("Сервер уже запущен!");
        return;
    }
    m_ptcpServer = new QTcpServer(this);
    if (!m_ptcpServer->listen(addr, port))
    {
        ui->portEdit->setEnabled(true);
        ui->log->addItem(QString("Ошибка! Невозможно запустить сервер: %1").arg(m_ptcpServer->errorString()));
        m_ptcpServer->close();
        flag=true;
        return;
    }
    ui->log->addItem("Сервер успешно запущен!");
    connect(m_ptcpServer, SIGNAL(newConnection()),this,SLOT(slotNewConnection()));
}

void CardServer::stopServer()
{
    if (m_ptcpServer)
    {
        flag = false;
        disconnect(m_ptcpServer, SIGNAL(newConnection()),this,SLOT(slotNewConnection()));
        m_ptcpServer->close();
        delete m_ptcpServer;
        m_ptcpServer = NULL;
        ui->log->addItem("Сервер успешно остановлен!");}
    else
    {
        ui->log->addItem("Сервер не запущен!");
    }
}



void CardServer::slotNewConnection()
{
    QTcpSocket* pClientSocket = m_ptcpServer->nextPendingConnection();
    connect(pClientSocket, SIGNAL(disconnected()),pClientSocket, SLOT(deleteLater()));
    connect(pClientSocket, SIGNAL(readyRead()),this,SLOT(slotReadClient()));
}

void CardServer::sendToClient(QTcpSocket* pSocket, Packet& packet)
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);
    packet.dateTime=QDateTime::currentDateTime();
    out << quint16(0) << packet;
    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));
    pSocket->write(arrBlock);
}

void CardServer::slotReadClient()
{
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();
    QDataStream in(pClientSocket);
    in.setVersion(QDataStream::Qt_5_3);
    for (;;)
    {
        if (!m_nNextBlockSize)
        {
            if (pClientSocket->bytesAvailable() < sizeof(quint16))
            {
                break;
            }
            in >> m_nNextBlockSize;
        }
        if (pClientSocket->bytesAvailable() < m_nNextBlockSize)
        {
            break;
        }
        Packet packet;
        in >> packet;
        ui->log->addItem(packet.dateTime.toString() + " " + "Получен запрос от клиента: " + packet.terminal_id);
        if (ui->terminals->findItems(packet.terminal_id, Qt::MatchExactly).size() > 0)
        {
            if ( packet.transaction_id == 20 )
            {
                packet.responseCode = 00;
                packet.transaction_id = 10;
                ui->log->addItem("Тип транзакции: Тест");
                goto Send;
            }
            else if (packet.transaction_id == 220 || packet.transaction_id == 120 || packet.transaction_id == 140 || packet.transaction_id == 121 || packet.transaction_id == 141)
            {
                QList<QTableWidgetItem *> card = ui->cards->findItems(packet.pan, Qt::MatchExactly);
                if (card.size() > 0)
                {
                    QDate realExpiry;
                    realExpiry.setDate(QString("20" + ui->cards->item(card[0]->row(),3)->text()).toInt(), ui->cards->item(card[0]->row(),2)->text().toInt()+1, 1);
                    if ( QDate::currentDate().daysTo(realExpiry) > 0)
                    {
                        if ( ui->cards->item(card[0]->row(),8)->text().toInt() < 3 )
                        {
                            if ( ui->cards->item(card[0]->row(),7)->text().toInt() == 0 )
                            {
                                if ( ui->cards->item(card[0]->row(),6)->text().toInt() == 0 )
                                {
                                    if ( ui->cards->item(card[0]->row(),5)->text() == packet.userName )
                                    {
                                        QDate expiry;
                                        expiry.setDate(QString("20" + packet.expiryDate.mid(2,2)).toInt(), packet.expiryDate.mid(0,2).toInt()+1, 1);
                                        if ( expiry.daysTo(realExpiry) == 0 )
                                        {
                                            if ( ui->cards->item(card[0]->row(),4)->text().toInt() == packet.pin )
                                            {
                                                if ( packet.transaction_id == 220)
                                                {
                                                    packet.responseCode = 00;
                                                    packet.transaction_id = 210;
                                                    packet.amount=ui->cards->item(card[0]->row(),1)->text().toFloat();
                                                    ui->log->addItem("Тип транзакции: Баланс");
                                                    goto Send;
                                                }
                                                else
                                                {
                                                    if( packet.amount > 0 && packet.amount < 999999)
                                                    {
                                                        if( packet.dateTime.secsTo(QDateTime::currentDateTime()) <= 60 )
                                                        {
                                                            if (packet.transaction_id == 120 || packet.transaction_id == 121)
                                                            {
                                                                packet.responseCode = 00;
                                                                if( packet.amount <= ui->cards->item(card[0]->row(),1)->text().toFloat())
                                                                {
                                                                    ui->log->addItem("Тип транзакции: Покупка");
                                                                    ui->cards->item(card[0]->row(),1)->setText(QString::number(ui->cards->item(card[0]->row(),1)->text().toFloat() - packet.amount));
                                                                }
                                                                else
                                                                {
                                                                    packet.responseCode = 51;
                                                                    ui->log->addItem("Недостаточно средств!");
                                                                    goto Send;
                                                                }
                                                            }
                                                            else
                                                            {
                                                                packet.responseCode = 00;
                                                                ui->log->addItem("Тип транзакции: Возврат");
                                                                ui->cards->item(card[0]->row(),1)->setText(QString::number(ui->cards->item(card[0]->row(),1)->text().toFloat() + packet.amount));
                                                            }
                                                            packet.amount=ui->cards->item(card[0]->row(),1)->text().toFloat();
                                                            packet.transaction_id = (int(packet.transaction_id / 10) -1) * 10;
                                                            goto Send;
                                                        }
                                                        else
                                                        {
                                                            packet.responseCode = 96;
                                                            packet.transaction_id = (int(packet.transaction_id / 10) -1) * 10 + 1;
                                                            ui->log->addItem("Таймаут запроса!");
                                                            goto Send;
                                                        }
                                                    }
                                                    else
                                                    {
                                                        packet.responseCode = 13;
                                                        ui->log->addItem("Сумма введена неверно!");
                                                        goto Send;
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                packet.responseCode = 55;
                                                ui->cards->item(card[0]->row(),8)->setText(QString::number(ui->cards->item(card[0]->row(),8)->text().toInt()+1));
                                                ui->log->addItem("Неверный PIN!"); goto Send;
                                            }
                                        }
                                        else
                                        {
                                            packet.responseCode = 16;
                                            ui->log->addItem("Неверный срок действия!");
                                            goto Send;
                                        }
                                    }
                                    else
                                    {
                                        packet.responseCode = 15;
                                        ui->log->addItem("Неверное имя держателя!");
                                        goto Send;
                                    }
                                }
                                else
                                {
                                    packet.responseCode = 43;
                                    ui->log->addItem("Карта была украдена!");
                                    goto Send;
                                }
                            }
                            else
                            {
                                packet.responseCode = 41;
                                ui->log->addItem("Карта была утеряна!");
                                goto Send;
                            }
                        }
                        else
                        {
                            packet.responseCode = 38;
                            ui->log->addItem("Произошла попытка подбора PIN!");
                            goto Send;
                        }
                    } else
                    {
                        packet.responseCode = 33;
                        ui->log->addItem("Карта просрочена!");
                        goto Send;
                    }
                }
                else
                {
                    packet.responseCode = 14;
                    ui->log->addItem("Неизвестный номер карты!");
                    goto Send;
                }
            }
            else
            {
                packet.responseCode = 12;
                ui->log->addItem("Неверная транкзация!");
                goto Send;
            }
        }
        else
        {
            packet.responseCode = 98;
            ui->log->addItem("Неизвестный ID терминала!");
            goto Send;
        }
    Send: sendToClient(pClientSocket, packet);
    m_nNextBlockSize = 0;
    }
}

CardServer::~CardServer()
{
    delete ui;
}

void CardServer::closeEvent(QCloseEvent *event)
{
    stopServer();
    saveSettings();
    event->accept();
}

void CardServer::on_addTerminalButton_clicked()
{
    if (ui->terminals->findItems(ui->idEdit->text().toUpper(), Qt::MatchExactly).size() == 0 && ui->idEdit->text().size() == 10)
    {
        ui->terminals->addItem(ui->idEdit->text().toUpper());
        ui->idEdit->setText("");
    }
}

void CardServer::on_removeTerminalButton_clicked()
{
    QListWidgetItem *it = ui->terminals->item(ui->terminals->currentRow());
    delete it;
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    settings.remove(0);
}

void CardServer::on_addCardButton_clicked()
{
    ui->cards->clearSelection();
    qint32 last_row = ui->cards->rowCount();
    ui->cards->insertRow(last_row);
    QTableWidgetItem *pan = new QTableWidgetItem("0000 0000 0000 0000");
    QTableWidgetItem *amount = new QTableWidgetItem("0.00");
    QTableWidgetItem *month = new QTableWidgetItem("01");
    QTableWidgetItem *year = new QTableWidgetItem("19");
    QTableWidgetItem *pin = new QTableWidgetItem("0000");
    QTableWidgetItem *name = new QTableWidgetItem("ИМЯ ФАМИЛИЯ");
    QTableWidgetItem *stolen = new QTableWidgetItem("0");
    QTableWidgetItem *lost = new QTableWidgetItem("0");
    QTableWidgetItem *pinCount = new QTableWidgetItem("0");
    ui->cards->setItem( last_row, 0, pan );
    ui->cards->setItem( last_row, 1, amount );
    ui->cards->setItem( last_row, 2, month );
    ui->cards->setItem( last_row, 3, year );
    ui->cards->setItem( last_row, 4, pin );
    ui->cards->setItem( last_row, 5, name );
    ui->cards->setItem( last_row, 6, stolen );
    ui->cards->setItem( last_row, 7, lost );
    ui->cards->setItem( last_row, 8, pinCount );
}

void CardServer::on_removeCardButton_clicked()
{
    if (ui->cards->selectionModel()->hasSelection())
    {
        ui->cards->removeRow(ui->cards->selectedItems()[0]->row());
        QSettings settings(m_settingsFile, QSettings::IniFormat);
        settings.remove(0);
    }
}

void CardServer::on_clearlog_clicked()
{
    ui->log->clear();
}

void CardServer::on_startServerButton_clicked()
{
    if (flag)
    {
        stopServer();
        ui->startServerButton->setText("Запустить сервер");
        flag = false;
    }
    else
    {
            if(isDig(ui->portEdit->text()))
                startServer(ui->portEdit->text().toInt(),QHostAddress::Any);
            else
            {
                ui->portEdit->setText("1");
                startServer(ui->portEdit->text().toInt(),QHostAddress::Any);
            }
        ui->startServerButton->setText("Остановить сервер");
        flag = true;
    }
}
