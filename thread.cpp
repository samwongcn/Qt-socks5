#include "thread.h"
#include <qDebug>
Thread::Thread(int id, QObject *parent) : QThread(parent)
{
	this->socketID = id;
}

Thread::~Thread()
{
	qDebug()<<"~Thread()";
	if(this->socket != NULL)
		{
			this->socket->disconnectFromHost();
			this->socket->deleteLater();
		}
	if(this->target != NULL)
		{
			this->target->disconnectFromHost();
			this->target->deleteLater();
		}
}

void Thread::run()
{
	qDebug()<<"new Thread:"<<this->currentThreadId();
	this->socket = new QTcpSocket();
	this->target = new QTcpSocket();
	
	udpSocket = new QUdpSocket();		//UDP处理
	udpTarget = new QUdpSocket();
	
	
	if(!this->socket->setSocketDescriptor(this->socketID))
		{
			qDebug()<<"thread.cpp 13  # "<<this->socket->errorString();
		}
	
	connect(this->socket,&QTcpSocket::readyRead,this,&Thread::tempRecv,Qt::DirectConnection);	//连接到临时的接受函数中协议处理
	connect(this->socket,&QTcpSocket::disconnected,this,&Thread::leave,Qt::DirectConnection);
	
	connect(this->socket,&QTcpSocket::readyRead,this,&Thread::writeToTarget,Qt::DirectConnection);	//交给分发函数
	connect(this->target,&QTcpSocket::readyRead,this,&Thread::writeToSource,Qt::DirectConnection);
	
	connect(this->udpSocket,&QUdpSocket::readyRead,this,&Thread::udp_Turn,Qt::DirectConnection);
	qDebug()<<"start";
	this->exec();
}

void Thread::tempRecv()
{
	if(this->isReady || this->isUdp) return;
	QByteArray buf =  this->socket->readAll();
	QByteArray send;
	qDebug()<<"buf:"<<buf;
	if(this->flag == -1)	//还没完成第一步
		{
			if(buf[0] == 0x05)		//简单认证获取第一位是不是 0x05(VER)
				{
					this->flag = 1;
					send.resize(2);
					send[0] = 0x05;
					send[1] = 0x00;
					
					qDebug()<<"SEND:"<<send;
					this->socket->write(send,2);
					this->flag = 1;
				}
		}
	else if(this->flag == 1)	//完成第一步了,开始第二步
		{
			Thread::msleep(500);
			if(buf[0] == 0x05)
				{
					switch (buf[1])	//检查连接方式是什么
						{
						case 0x01:		//connect	
							this->style = 1;
							break;
						case 0x02:		//bind
							this->style = 2;
							break;
						case 0x03:		//udp
							this->style = 3;
							break;
						default:
							leave();
							return;
							break;
						}
					switch (buf[3])
						{
						case 0x01:		//ipv4
						{
							this->post = 1;	
							
							QByteArray s,w,l,q;
							s[0] = buf[4];
							w[0] = buf[5];
							l[0] = buf[6];
							q[0] = buf[7];
							
							bool ok;	//还原成IPV4地址格式
							QString ip = QString::number(QString(s.toHex()).toInt(&ok,16)) +"."+
									QString::number(QString(w.toHex()).toInt(&ok,16)) +"."+
									QString::number(QString(l.toHex()).toInt(&ok,16)) +"."+
									QString::number(QString(q.toHex()).toInt(&ok,16));
							
							
							QByteArray port = buf.mid(8,2);
							
							this->targetIp = ip;
							this->targetPort = QString(port.toHex()).toInt(&ok,16);
							qDebug()<<"80#  ip:"<<this->targetIp;
							qDebug()<<"80#  port:"<<this->targetPort;
							
							send.resize(10);
							send[0] = 0x05;
							send[1] = 0x00;
							send[2] = 0x00;
							send[3] = 0x01;
							send[4] = 0x00;
							send[5] = 0x00;
							send[6] = 0x00;
							send[7] = 0x00;
							send[8] = 0x00;
							send[9] = 0x00;
							break;
						}
						case 0x03:		//domain
						{
							this->post = 3;
							QByteArray buff;
							buff.resize(1);
							buff[0] = buf[4];
							QString str(buff.toHex());
							
							bool ok;
							int domainLength=str.toInt(&ok,16);
							//	int domainLength = atoi(buf[4]);
							qDebug()<<"domainLength:"<<domainLength;
							
							QByteArray domain(buf.mid(5,domainLength));	//获取域名
							qDebug()<<"domain:"<<domain;
							QByteArray temp(buf.mid(4+domainLength+1,2));
							str = temp.toHex();
							int port = str.toInt(&ok,16);
							
							this->targetPort = port;
							qDebug()<<"130#  ip:"<<this->targetIp;
							qDebug()<<"131#  port:"<<this->targetPort;
							send.resize(8);
							send[0] = 0x05;
							send[1] = 0x00;
							send[2] = 0x00;
							send[3] = 0x01;
							send[4] = 0x00;
							send[5] = 0x00;
							send[6] = 0x00;
							send[7] = 0x00;
							send[8] = 0x00;
							send[9] = 0x00;
							
							QHostInfo info = QHostInfo::fromName(domain);
							if(!info.addresses().isEmpty())
								{
									QHostAddress addres = info.addresses().first();
									this->targetIp = addres.toString();
								}
							else
								{
									return;
								}
							
							break;
						}
						case 0x04:
						{
							this->post = 4;
							break;
						}
						default:
							qDebug()<<"121# quit";
							leave();
							return;
						}
					
					switch (this->style) {
						case 1:
						{
							qDebug()<<"connect";
							if(!connectToTarget(this->targetIp,this->targetPort))
								{
									qDebug()<<"134# quit";
									leave();
									return;
								}
							break;
						}
						case 2:
						{
							break;
						}
						case 3:
						{
							qDebug()<<"this is udp";
							this->flagIp = this->socket->peerAddress().toString();
							send[0] = 0x05;
							send[1] = 0x00;
							send[2] = 0x00;
							send[3] = 0x01;
							send[4] = 0x00;
							send[5] = 0x00;
							send[6] = 0x00;
							send[7] = 0x00;
							send[8] = 0x00;
							send[9] = 0x00;
							//获取一个随机的端口
							udpSocket->bind();
							int localPort = udpSocket->localPort();	//获取UDP绑定的随机端口号
							QByteArray bytePort = QString::number(localPort).toLatin1().toHex();
							
							if(localPort>255)
								{
									send[8] = bytePort[0];
									send[9] = bytePort[1];
								}
							else
								{
									send[9] = bytePort[0];
								}
							QHostAddress address;
							foreach(const QHostAddress& hostAddress,QNetworkInterface::allAddresses())
								{
									
									if ( hostAddress != QHostAddress::LocalHost && hostAddress.toIPv4Address() )
										{
											address = hostAddress;
										}
									
								}
							
					//		QHostInfo info = QHostInfo::fromName(QHostInfo::localHostName());
							QStringList ipList =  address.toString().split(".");
							qDebug()<<"ipList : "<<ipList;
							if(ipList.size() != 4)
								{
									return;
								}
							
							//填充为我们的ip地址
							qDebug()<<"4 .......:"<<QString(ipList.at(0)).toLatin1()[0];
							send[4] = QString(ipList.at(0)).toLatin1().toHex()[0];
							send[5] = QString(ipList.at(1)).toLatin1().toHex()[0];
							send[6] = QString(ipList.at(2)).toLatin1().toHex()[0];
							send[7] = QString(ipList.at(3)).toLatin1().toHex()[0];
							qDebug()<<"UDP back data:"<<send;
							return;
							break;
						}
						default:
							leave();
							return;
							break;
						}
					
					this->socket->write(send,send.size());
					this->isReady = true;
				}
		}
}

void Thread::writeToSource()
{
	if(this->isReady == true)
		{
			/*
			while (this->target->bytesAvailable()>0)
				{
					QByteArray datagram;
					
					datagram.resize(this->target->bytesAvailable());
					
					this->target->read(datagram.data(), datagram.size());
					
					this->socket->write(datagram);
					qDebug()<<"to source <<:  \n"<<datagram;
				}
			
			*/
			
			this->socket->write(this->target->readAll());
		}
}

void Thread::writeToTarget()
{
	if(this->isReady == true)
		{
			/*
			while (this->socket->bytesAvailable()>0)
				{
					QByteArray datagram;
					
					datagram.resize(this->socket->bytesAvailable());
					
					this->socket->read(datagram.data(), datagram.size());
					
					this->target->write(datagram);
					qDebug()<<"to target <<:  \n"<<datagram;
				}
			*/
			this->target->write(this->socket->readAll());
		}
	
}

void Thread::udp_Turn()
{
	if(isUdp == true)
		{
			QHostAddress address;
			QByteArray buff;
			this->udpSocket->readDatagram(buff.data(),buff.size(),&address);
			
			if(address.toString() == flagIp)	//从客户端发出的UDP包
				{
					switch (buff[3]) {
						case 0x01:
						{
							QByteArray s,w,l,q;
							s[0] = buff[4];
							w[0] = buff[5];
							l[0] = buff[6];
							q[0] = buff[7];
							
							bool ok;	//还原成IPV4地址格式
							QString ip = QString::number(QString(s.toHex()).toInt(&ok,16)) +"."+
									QString::number(QString(w.toHex()).toInt(&ok,16)) +"."+
									QString::number(QString(l.toHex()).toInt(&ok,16)) +"."+
									QString::number(QString(q.toHex()).toInt(&ok,16));
							
							
							QByteArray port = buff.mid(8,2);
							
							this->targetIp = ip;
							this->targetPort = QString(port.toHex()).toInt(&ok,16);
							qDebug()<<"80#  ip:"<<this->targetIp;
							qDebug()<<"80#  port:"<<this->targetPort;
							
							udpSocket->writeDatagram(buff.mid(10,buff.size()-9),QHostAddress(this->targetIp),this->targetPort);
							break;
						}
						case 0x03:
						{
							QByteArray bufff;
							bufff.resize(1);
							bufff[0] = buff[4];
							QString str(bufff.toHex());
							
							bool ok;
							int domainLength=str.toInt(&ok,16);
							qDebug()<<"domainLength:"<<domainLength;
							
							QByteArray domain(buff.mid(5,domainLength));	//获取域名
							qDebug()<<"domain:"<<domain;
							QByteArray temp(buff.mid(4+domainLength+1,2));
							str = temp.toHex();
							int port = str.toInt(&ok,16);
							
							this->targetPort = port;
							qDebug()<<"131#  port:"<<this->targetPort;
							QHostInfo info = QHostInfo::fromName(domain);
							if(!info.addresses().isEmpty())
								{
									QHostAddress addres = info.addresses().first();
									this->targetIp = addres.toString();
									udpSocket->writeDatagram(buff.mid(6+domainLength+1,buff.size()-4+domainLength+1),QHostAddress(this->targetIp),this->targetPort);
								}
							else
								{
									leave();
									return;
								}
							
							break;
						}
						default:
							break;
						}
				}
			else		//从服务端发出的UDP包
				{
					
				}
		}
}

bool Thread::connectToTarget(QString ip, int port)
{
	this->target->connectToHost(QHostAddress(ip),port);
	if(this->target->waitForConnected())
		{
			return true;
		}
	else
		{
			qDebug()<<"263# ERROR:"<<this->target->errorString();
			return false;
		}
}

void Thread::leave()
{
	//	this->socket->deleteLater();
	//	this->target->deleteLater();
	
	if(this->udpSocket != NULL)
		{
			this->udpSocket->disconnectFromHost();
			this->udpSocket->deleteLater();
		}
	if(this->udpTarget != NULL)
		{
			this->udpTarget->disconnectFromHost();
			this->udpTarget->deleteLater();
		}
	
	if(this->socket != NULL)
		{
			this->socket->disconnectFromHost();
			this->socket->deleteLater();
		}
	if(this->target != NULL)
		{
			this->target->disconnectFromHost();
			this->target->deleteLater();
		}
	this->exit();
}
