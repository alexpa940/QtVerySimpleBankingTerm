#ifndef CARDSERVER_H
#define CARDSERVER_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTime>
#include <QSettings>
#include <QFile>
#include <QCloseEvent>
#include "packet.h"
#include "QResizeEvent"

namespace Ui
{
class CardServer;
class QTcpServer;
class QTcpSocket;
}

class CardServer : public QMainWindow
{
    Q_OBJECT

public:
    explicit CardServer(QWidget *parent = 0);
    QTcpServer* m_ptcpServer;
    quint16     m_nNextBlockSize;
    ~CardServer();

public slots:
    virtual void slotNewConnection();
            void slotReadClient   ();

private slots:
    void on_addTerminalButton_clicked();
    void on_removeTerminalButton_clicked();
    void on_addCardButton_clicked();
    void on_removeCardButton_clicked();
    void on_clearlog_clicked();
    void on_startServerButton_clicked();

private:
    void loadSettings();
    void saveSettings();
    void startServer(int port, QHostAddress addr);
    void stopServer();
    QString m_settingsFile;
    qint32 rrn;
    Ui::CardServer *ui;
    void sendToClient(QTcpSocket* pSocket, Packet& packet);
    void closeEvent(QCloseEvent *event);
    bool flag;
};

#endif // CARDSERVER_H
