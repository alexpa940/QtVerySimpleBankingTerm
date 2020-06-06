#include "cardclient.h"
#include "ui_cardclient.h"
#include "QResizeEvent"
#include "carta.h"

bool isDig(QString str)
{
    int pos = 0;

    QRegExpValidator validator(QRegExp("[1-9]+"));

    if (validator.validate(str, pos) == QValidator::Acceptable)
        return true;

    return false;
}

CardClient::CardClient(QWidget *parent):QMainWindow(parent),ui(new Ui::CardClient)
{
    id="";
    stat=false;
    ui->setupUi(this);
    m_nNextBlockSize=0;
    m_pTcpSocket = NULL;
    ui->idList->setEnabled(false);
    ui->cardNumberEdit->setEnabled(false);
    ui->cardMounthEdit->setEnabled(false);
    ui->cardYearEdit->setEnabled(false);
    ui->cardPinEdit->setEnabled(false);
    ui->cardNameEdit->setEnabled(false);

    m_settingsFile = QApplication::applicationDirPath() + "/client.ini";
    loadSettings();
    ui->ipEdit->setInputMask("000.000.000.000");
}

void CardClient::startClient(int port, QHostAddress addr)
{
    m_pTcpSocket = new QTcpSocket(this);
    m_pTcpSocket->connectToHost(addr, port);
    connect(m_pTcpSocket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pTcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(m_pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(slotError(QAbstractSocket::SocketError)));
}

void CardClient::on_testButton_clicked()
{
    if (!addr.setAddress(ui->ipEdit->text()))
    {
        ui->log->addItem(" invalid address "+ui->ipEdit->text());
        return;
    }
    else{
        if(isDig(ui->portEdit->text()))
            startClient(ui->portEdit->text().toInt(),addr);
        else
        {
            ui->portEdit->setText("1");
            addr.setAddress(ui->ipEdit->text());
            startClient(ui->portEdit->text().toInt(),addr);
        }
    }
    Packet packet;
    packet.transaction_id=20;
    slotSendToServer(packet);
    stat=false;
}

void CardClient::stopClient()
{
    ui->portEdit->setEnabled(true);
    if (m_pTcpSocket)
    {
        disconnect(m_pTcpSocket, SIGNAL(connected()), this,SLOT(slotConnected()));
        disconnect(m_pTcpSocket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
        disconnect(m_pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(slotError(QAbstractSocket::SocketError)));
        m_pTcpSocket->close();
        m_pTcpSocket->abort();
        delete m_pTcpSocket;
        m_pTcpSocket = NULL;
        ui->log->addItem("Соединение закрыто!");
    }
    else
    {
        ui->log->addItem("Cоединение не было открыто!");
    }
}


void CardClient::slotReadyRead()
{
    QDataStream in(m_pTcpSocket);
    in.setVersion(QDataStream::Qt_5_3);
    for (;;)
    {
        if (!m_nNextBlockSize)
        {
            if (m_pTcpSocket->bytesAvailable() < sizeof(quint16))
            {
                break;
            }
            in >> m_nNextBlockSize;
        }
        if (m_pTcpSocket->bytesAvailable() < m_nNextBlockSize)
        {
            break;
        }
        QString answr;
        Packet packet;
        in >> packet;
        switch(packet.responseCode)
        {
        case 00:
            switch(packet.transaction_id)
            {
                case 110:
                case 130:
                case 210:
                    ui->summEdit->setText(QString::number(packet.amount));
                    break;
            }
            answr = "Успешно!";
            break;
        case 96:
            answr = "Таймаут запроса!";
            break;
        case 51:
            answr = "Недостаточно средств!";
            break;
        case 13:
            answr = "Сумма введена неверно!";
            break;
        case 55:
            answr = "Неверный PIN!";
            break;
        case 16:
            answr = "Неверное время действия!";
            break;
        case 15:
            answr = "Неверное имя держателя!";
            break;
        case 43:
            answr = "Карта была украдена!";
            break;
        case 41:
            answr = "Карта была утеряна!";
            break;
        case 38:
            answr = "Произошла попытка подбора PIN, карта заблокирована!";
            break;
        case 33:
            answr = "Карта просрочена!";
            break;
        case 14:
            answr = "Неизвестный номер карты!";
            break;
        case 12:
            answr = "Неверная транкзация!";
            break;
        case 98:
            answr = "Неизвестный ID терминала!";
            break;
        default:
            answr = "Ошибка передачи!";
            break;
        }
        QString strMessage = "Ответ сервера: " + answr;
        ui->log->addItem(strMessage);
        m_nNextBlockSize = 0;
        if (packet.responseCode == 96)
        {
            switch(packet.transaction_id)
            {
            case 111:
                ui->saleButton->click();
                break;
            case 131:
                ui->refundButton->click();
                break;
            }
        }
    }
}

void CardClient::slotError(QAbstractSocket::SocketError err)
{
    ui->portEdit->setEnabled(true);
    QString strError="Ошибка: "+(err==QAbstractSocket::HostNotFoundError?"Сервер не найден!":err==QAbstractSocket::RemoteHostClosedError?"Сервер закрыл соединение!":err==QAbstractSocket::ConnectionRefusedError?"Нет доступа к серверу!":QString(m_pTcpSocket->errorString()));
    if(!stat || err!=QAbstractSocket::ConnectionRefusedError)
        ui->log->addItem(strError);
    if (err==QAbstractSocket::ConnectionRefusedError)
        stat=true;
    if (m_pTcpSocket)
    {
        disconnect(m_pTcpSocket, SIGNAL(connected()), this,SLOT(slotConnected()));
        disconnect(m_pTcpSocket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
        disconnect(m_pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(slotError(QAbstractSocket::SocketError)));
        m_pTcpSocket->close();
        m_pTcpSocket->abort();
        delete m_pTcpSocket;
        m_pTcpSocket = NULL;
    }
}

void CardClient::slotSendToServer(Packet& packet)
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);
    packet.responseCode=0;
    packet.terminal_id=id;
    packet.dateTime=QDateTime::currentDateTime();
    out << quint16(0) << packet;
    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));
    m_pTcpSocket->write(arrBlock);
}

void CardClient::slotConnected()
{
    ui->portEdit->setEnabled(false);
    ui->log->addItem("Подключено к серверу");
}

CardClient::~CardClient()
{
    delete ui;
}


void CardClient::closeEvent(QCloseEvent *event)
{
    stopClient();
    saveSettings();
    event->accept();
}

void CardClient::on_closeConnectionButton_clicked()
{
    stopClient();
}

void CardClient::on_balanceButton_clicked()
{
    stat=false;
    if (!m_pTcpSocket)
    {
        if (!addr.setAddress(ui->ipEdit->text()))
        {
            ui->log->addItem(" invalid address "+ui->ipEdit->text());
            return;
        }
        else{
            if(isDig(ui->portEdit->text()))
                startClient(ui->portEdit->text().toInt(),addr);
            else
            {
                ui->portEdit->setText("1");
                addr.setAddress(ui->ipEdit->text());
                startClient(ui->portEdit->text().toInt(),addr);
            }
        }
    }
    Packet packet;
    packet.amount=ui->summEdit->text().toFloat();
    packet.expiryDate=ui->cardMounthEdit->text() + ui->cardYearEdit->text();
    packet.pan=ui->cardNumberEdit->text();
    packet.pin=ui->cardPinEdit->text().toInt();
    packet.userName = ui->cardNameEdit->text();
    packet.transaction_id=220;
    slotSendToServer(packet);
}

void CardClient::on_saleButton_clicked()
{
    stat=false;
    Packet packet;
    if (!m_pTcpSocket)
    {
        if (!addr.setAddress(ui->ipEdit->text()))
        {
            ui->log->addItem(" invalid address "+ui->ipEdit->text());
            return;
        }
        else{
            if(isDig(ui->portEdit->text()))
                startClient(ui->portEdit->text().toInt(),addr);
            else
            {
                ui->portEdit->setText("1");
                addr.setAddress(ui->ipEdit->text());
                startClient(ui->portEdit->text().toInt(),addr);
            }
        }
    }
    packet.amount=ui->summEdit->text().toFloat();
    packet.expiryDate=ui->cardMounthEdit->text() + ui->cardYearEdit->text();
    packet.pan=ui->cardNumberEdit->text();
    packet.pin=ui->cardPinEdit->text().toInt();
    packet.userName = ui->cardNameEdit->text();
    packet.transaction_id=120;
    slotSendToServer(packet);
}

void CardClient::on_refundButton_clicked()
{
    stat=false;
    Packet packet;
    if (!m_pTcpSocket)
    {
        if (!addr.setAddress(ui->ipEdit->text()))
        {
            ui->log->addItem(" invalid address "+ui->ipEdit->text());
            return;
        }
        else
            if(isDig(ui->portEdit->text()))
                startClient(ui->portEdit->text().toInt(),addr);
            else
            {
                ui->portEdit->setText("1");
                addr.setAddress(ui->ipEdit->text());
                startClient(ui->portEdit->text().toInt(),addr);
            }
    }
    packet.amount=ui->summEdit->text().toFloat();
    packet.expiryDate=ui->cardMounthEdit->text() + ui->cardYearEdit->text();
    packet.pan=ui->cardNumberEdit->text();
    packet.pin=ui->cardPinEdit->text().toInt();
    packet.userName = ui->cardNameEdit->text();
    packet.transaction_id=140;
    slotSendToServer(packet);
}

void CardClient::on_generateButton_clicked()
{
    QString terminal_id;
    int diff = 'Z'-'A';
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    for(int i=0;i<10;i++)
    {
        char c = 'A'+(qrand() % diff);
        terminal_id += QChar(c);
    }
    ui->idList->setText(terminal_id);
    ui->generateButton->setEnabled(false);
    ui->deleteButton->setEnabled(true);
    id = ui->idList->text();
}

void CardClient::on_deleteButton_clicked()
{  
    id="";
    ui->idList->setText(id);
    ui->generateButton->setEnabled(true);
    ui->deleteButton->setEnabled(false);
}

void CardClient::on_clerclog_clicked()
{
    ui->log->clear();
}

void CardClient::on_pushButton_clicked()
{
    Carta *cartadd = new Carta;
    cartadd->show();
    connect(cartadd,SIGNAL(out(QString,QString,QString,QString,QString)),this,SLOT(in(QString,QString,QString,QString,QString)));
}

void CardClient::in(QString number,QString month,QString year,QString pin,QString name)
{
    ui->cardNumberEdit->setText(number);
    ui->cardMounthEdit->setText(month);
    ui->cardYearEdit->setText(year);
    ui->cardPinEdit->setText(pin);
    ui->cardNameEdit->setText(name);
}

void CardClient::loadSettings()
{
    if (QFile(m_settingsFile).exists())
    {
        QSettings settings(m_settingsFile, QSettings::IniFormat);
        settings.beginGroup("Client");
        if(isDig(settings.value("port", 1).toString()))
        {
            int port = settings.value("port", 1).toInt();
            if (ui->portEdit->isEnabled())
                ui->portEdit->setText(QString::number(port));
        }
        else
            if (ui->portEdit->isEnabled())
                ui->portEdit->setText(QString::number(1));

        QString ip = settings.value("ip").toString();
        ui->ipEdit->setText(ip);

        id = settings.value("id").toString();
        ui->idList->setText(id);
        settings.endGroup();

        settings.beginGroup("LastUser");
        ui->cardNameEdit->setText(settings.value("name").toString());
        ui->cardPinEdit->setText(settings.value("pin").toString());
        ui->cardNumberEdit->setText(settings.value("number").toString());
        ui->cardYearEdit->setText(settings.value("year").toString());
        ui->cardMounthEdit->setText(settings.value("mounth").toString());
        settings.endGroup();

    }
    return;
}

void CardClient::saveSettings()
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    settings.beginGroup("Client");
    settings.setValue("port",  ui->portEdit->text().toInt());
    settings.setValue("ip",  ui->ipEdit->text());
    settings.setValue("id",  ui->idList->text());
    settings.endGroup();

    settings.beginGroup("LastUser");
    settings.setValue("name",ui->cardNameEdit->text());
    settings.setValue("pin",ui->cardPinEdit->text());
    settings.setValue("number",ui->cardNumberEdit->text());
    settings.setValue("year",ui->cardYearEdit->text());
    settings.setValue("mounth",ui->cardMounthEdit->text());
    settings.endGroup();

}
