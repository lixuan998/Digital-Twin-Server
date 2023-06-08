#pragma once

#include <QTcpServer>
#include <QObject>
#include <QTcpSocket>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QByteArray>
#include <QHostAddress>
#include <QTimer>
#include <QVector>
#include <QMutex>

class Tcp_Messager : public QObject
{
    Q_OBJECT
    //public functions
    public: 
        Tcp_Messager();
        ~Tcp_Messager();
        
        
    //private functions
    private:
        void connectServerSignals();
    //public variables
    public:

    //private variables
    private:
        QTcpServer *tcp_server = nullptr;
        QTcpSocket *tcp_socket = nullptr;
		QVector<QString> msg_queue;
		QString last_order;
		QTimer *msg_send_timer = nullptr;
		QMutex msg_mutex, status_mutex;

		bool msg_ret_status;
    //public slots
    public slots:
		void sendMessage(QString message);
		void startListening(QHostAddress address = QHostAddress::Any, int port = 8080);
		void stopListening();
    //private slots
    private slots:
        void newConnection();

        void changeState(QAbstractSocket::SocketState state);
        void readData();

    //signals
    signals:
        void recivedMsg(QString);
        void sendMsg(QString);
        void connected(QHostAddress);
        void disconnected();
        void listening();
        void notListening();
};