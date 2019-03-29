#include "server.h"
#include <QDebug>

Server::Server(QObject *parent) : QTcpServer(parent)
{
	
}

void Server::incomingConnection(int id)
{
	Thread *t = new Thread(id);
	t->start();
}

bool Server::startServer(QString address, int port)
{
	QHostInfo info = QHostInfo::fromName(address);
	if(!info.addresses().isEmpty())
		{
			this->ip = info.addresses().first();
			this->port = port;
		}
	else
		{
			qDebug()<<"Server.Cpp #24  ip is empty";
			return false;
		}
	
	if(this->listen(this->ip,this->port))
		{
			qDebug()<<"success listen";
			return true;
		}
	return false;
	
}

void Server::stopServer()
{
	this->close();
	this->ip.clear();
	this->port = 0;
}
