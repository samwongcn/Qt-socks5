#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	this->server = new Server();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_ButtonStart_clicked()
{
    if(ui->lineEdit->text().isEmpty())
	    {
		    return;
	    }
    
    server->startServer("127.0.0.1",ui->lineEdit->text().toInt());
}

void MainWindow::on_ButtonStop_clicked()
{
    server->stopServer();
}
