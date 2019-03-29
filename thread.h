#ifndef THREAD_H
#define THREAD_H
#include <QHostInfo>
#include <QHostAddress>
#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QThread>
#include <QUdpSocket>
#include <QNetworkInterface>
class Thread : public QThread
{
	Q_OBJECT
public:
	explicit Thread(int id ,QObject *parent = nullptr);
	~Thread();
private:
	int socketID = 0;
	
	//分别是和目标，源通信的socket;
	QTcpSocket *socket =NULL;
	QTcpSocket *target = NULL;
	
	QUdpSocket *udpSocket = NULL;
	QUdpSocket *udpTarget = NULL;
	
	//目标服务器ip，目标端口，连接方式，地址方式，通信协议是否完成标识
	QString targetIp;	//目标IP
	
	int targetPort = 0;		//目标端口
	int style=0;			//连接方式
	int post = 0;			//地址类型
	int flag = -1;			//记录通信协议是否完成
	
	bool isReady = false;	//Tcp是否开始转发
	bool isUdp = false;		//udp是否开始转发
	
	QString flagIp;		//记录UDP的客户地址
	int udpClientPort =0;	//记录UDP的客户端口
signals:
	
private slots:	
	void tempRecv();
	void writeToSource();		//将数据发送到源客户端
	void writeToTarget();		//同上反之，将源客户端数据发出

	void udp_Turn();
	
	bool connectToTarget(QString ip, int port);		//连接到目标服务器
public slots:
	void run();		//线程开始函数
	void leave();		//线程中断后的socket清理函数
};

#endif // THREAD_H
