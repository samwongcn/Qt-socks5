#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QHostAddress>
#include <QHostInfo>
#include "thread.h"

class Server : public QTcpServer
{
	Q_OBJECT
public:
	explicit Server(QObject *parent = nullptr);
	
private:
	QHostAddress ip;
	int port;
signals:
	
public slots:
	void incomingConnection(int id);		//重载分配socket的槽函数
	bool startServer(QString address, int port);		//开始服务
	void stopServer();		//停止服务
	
//	void writeToSource();
//	void writeToTarget();
};

#endif // SERVER_H
