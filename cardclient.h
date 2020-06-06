#ifndef CARDCLIENT_H
#define CARDCLIENT_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTime>
#include <QSettings>
#include <QFile>
#include <QCloseEvent>
#include <QListWidget>
#include "packet.h"
#include "QResizeEvent"

namespace Ui {
class CardClient;
}

class CardClient : public QMainWindow
{
    Q_OBJECT

public:
    explicit CardClient(QWidget *parent = 0);
    QTcpSocket* m_pTcpSocket;
    quint16     m_nNextBlockSize;
    QString id;
    ~CardClient();

public slots:
    void slotReadyRead   ();
    void slotError       (QAbstractSocket::SocketError);
    void slotSendToServer(Packet& str);
    void slotConnected   ();
    void in(QString number, QString month, QString year, QString pin, QString name);

private slots:
    void on_testButton_clicked();
    void on_closeConnectionButton_clicked();
    void on_balanceButton_clicked();
    void on_saleButton_clicked();
    void on_refundButton_clicked();
    void on_generateButton_clicked();
    void on_deleteButton_clicked();
    void on_clerclog_clicked();

    void on_pushButton_clicked();

private:
    Ui::CardClient *ui;
    void startClient(int port, QHostAddress addr);
    void stopClient();
    QString m_settingsFile;
    void closeEvent(QCloseEvent *event);
    bool stat;
    QHostAddress addr;
    void loadSettings();
    void saveSettings();
};

#endif // CARDCLIENT_H
