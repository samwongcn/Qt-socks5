#include "mainwindow.h"
#include <QApplication>
#include "server.h"
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w;
	w.show();
//	Server *s =new Server();
//	s->startServer("127.0.0.1",1080);
	return a.exec();
}
