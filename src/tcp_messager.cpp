#include "tcp_messager.h"

Tcp_Messager::Tcp_Messager()
{
}

Tcp_Messager::~Tcp_Messager()
{
	msg_send_timer->stop();
	delete msg_send_timer;
	msg_send_timer = nullptr;
    if(tcp_server->isListening()) tcp_server->close();
    if(tcp_socket != nullptr)
    {
        if(tcp_socket->state() == QAbstractSocket::ConnectedState) tcp_socket->close();
        delete tcp_socket;
        tcp_socket = nullptr;
    }
	delete tcp_server;
	tcp_server = nullptr;
}

void Tcp_Messager::startListening(QHostAddress address, int port)
{
	if (tcp_server == nullptr)
	{
		tcp_server = new QTcpServer();
		connectServerSignals();
	}
    if(!tcp_server->isListening())
    {
        if(tcp_server->listen(QHostAddress(address), port)) emit listening();
    }
	msg_ret_status = true;
	last_order = "";
	if (msg_send_timer == nullptr)
	{
		msg_send_timer = new QTimer();
		msg_send_timer->setInterval(500);
		connect(msg_send_timer, &QTimer::timeout, this, [&] {
			status_mutex.lock();
			msg_mutex.lock();
			if (msg_queue.empty())
			{
				msg_send_timer->stop();
				status_mutex.unlock();
				msg_mutex.unlock();
				return;
			}
			if (!msg_ret_status)
			{
				status_mutex.unlock();
				msg_mutex.unlock();
				return;
			}
			tcp_socket->write(msg_queue.front().toUtf8());
			emit sendMsg(msg_queue.front());
			last_order = msg_queue.front();
			msg_queue.pop_front();
			msg_mutex.unlock();
			msg_ret_status = false;
			status_mutex.unlock();
			});
	}
	}
	

void Tcp_Messager::stopListening()
{

	qDebug() << "msg_send_timer" << msg_send_timer;
	qDebug() << "tcp_socket" << tcp_socket;
	qDebug() << "tcp_server" << tcp_server;


	if (msg_send_timer != nullptr)
	{
		if(msg_send_timer->isActive()) msg_send_timer->stop();
		delete msg_send_timer;
		msg_send_timer = nullptr;
	}
	qDebug() << "22";
	if (tcp_socket != nullptr)
	{
		if (tcp_socket->state() == QAbstractSocket::ConnectedState) tcp_socket->close();
		delete tcp_socket;
		tcp_socket = nullptr;
	}
	qDebug() << "33";
	if (tcp_server != nullptr)
	{
		if(tcp_server->isListening())  tcp_server->close();
		delete tcp_server;
		tcp_server = nullptr;
	}
   
	qDebug() << "44";
	
    emit notListening();
}

void Tcp_Messager::sendMessage(QString message)
{
	qDebug() << "msg:: " << message;
    if(tcp_socket == nullptr) return;
    if(tcp_socket->state() != QAbstractSocket::ConnectedState) return;

	msg_mutex.lock();
	msg_queue.push_back(message);
	if (!msg_send_timer->isActive()) msg_send_timer->start();
	msg_mutex.unlock();
}

void Tcp_Messager::connectServerSignals()
{
    connect(tcp_server, &QTcpServer::newConnection, this, &Tcp_Messager::newConnection);
    
}

void Tcp_Messager::newConnection()
{
    while(tcp_server->hasPendingConnections())
    {
        tcp_socket = tcp_server->nextPendingConnection();
        emit connected(tcp_socket->peerAddress());
        connect(tcp_socket, &QAbstractSocket::readyRead, this, &Tcp_Messager::readData);
        connect(tcp_socket, &QAbstractSocket::stateChanged, this, &Tcp_Messager::changeState);
        connect(tcp_socket, &QAbstractSocket::destroyed, this, [&]{tcp_socket = nullptr;});
    }
}

void Tcp_Messager::changeState(QAbstractSocket::SocketState state)
{
    if(state == QAbstractSocket::ConnectedState)
    {
        emit connected(tcp_socket->peerAddress());
    }
    else if(state == QAbstractSocket::UnconnectedState)
    {
		qDebug() << "unconnect";
        emit disconnected();
        if(tcp_socket != nullptr)
        {
            if(tcp_socket->state() == QAbstractSocket::ConnectedState) tcp_socket->close();
            tcp_socket->deleteLater();
        }
    }
}

void Tcp_Messager::readData()
{
    QByteArray temp = tcp_socket->readAll();
    QString recived_message = QString::fromUtf8(temp);
	qDebug() << "rcvd: " << recived_message;
    emit recivedMsg(recived_message);
	status_mutex.lock();
	if (recived_message.contains(last_order)) msg_ret_status = true;
	qDebug() << "status: " << msg_ret_status;
	status_mutex.unlock();
	qDebug() << "A";
}