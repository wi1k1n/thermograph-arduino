#include <QDebug>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    serial(new QSerialPort(this))
{
    ui->setupUi(this);

    serial->setBaudRate(QSerialPort::Baud9600);
    serial->setDataTerminalReady(false);

    connect(ui->actionExit, &QAction::triggered, qApp, QApplication::quit);

    connect(serial, &QSerialPort::readyRead, this, &MainWindow::serialReadyRead);
    connect(serial, &QSerialPort::errorOccurred, this, &MainWindow::handleError);

    qDebug() << "Hello from constructor!";
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionAbout_triggered() {
    qDebug() << "Hello About!";
}

void MainWindow::on_menuConnect_aboutToShow() {
//    qDebug() << "Hello from menuPort!";
    QList<QSerialPortInfo> portList = QSerialPortInfo::availablePorts();

    ui->menuConnect->clear();
    for (auto iter = portList.begin(); iter != portList.end(); ++iter) {
        QAction *qa = new QAction(iter->portName());
        qa->setCheckable(true);

        connect(qa, &QAction::triggered, this, [=](){onPortClicked(qa->text());});

        ui->menuConnect->addAction(qa);
    }
}

void MainWindow::onPortClicked(const QString &portName) {
    qDebug() << portName;
    if (portName == serial->portName()) {
        if (serial->isOpen()) {
            qDebug() << "Port: " << serial->portName() << " is already opened!";
            return;
        }
    } else {
        serial->setPortName(portName);
    }

    if (serial->open(QIODevice::ReadWrite))
        qDebug() << "Successfully opened port: " << serial->portName();
    else
        qDebug() << "Error opening port: " << serial->portName();

}

void MainWindow::handleError(QSerialPort::SerialPortError err) {
    QString errMsg;
    switch (err) {
        case QSerialPort::NoError: return;
        case QSerialPort::DeviceNotFoundError: errMsg = "Attempted to open non-existing device."; break;
        case QSerialPort::PermissionError: errMsg = "Can't open device. Access denied."; break;
        case QSerialPort::OpenError: errMsg = "Error while opening device."; break;
        case QSerialPort::WriteError: errMsg = "Error occured while writing the data."; break;
        case QSerialPort::ReadError: errMsg = "Error occured while reading the data."; break;
        case QSerialPort::ResourceError: errMsg = "Device became unavailable (disconnected?)"; break;
        default: errMsg = "Unknown error."; break;
    }
    QMessageBox::critical(this, tr("Critical Error"), QString("%1\n%2").arg(errMsg, serial->errorString()));
    if (serial->isOpen())
        serial->close();
}

void MainWindow::serialReadyRead() {
    qDebug() << "readyRead()";
}
